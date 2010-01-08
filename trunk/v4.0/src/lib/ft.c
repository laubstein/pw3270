/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como ft.c e possui 2143 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#include <lib3270/config.h>
#include <lib3270/api.h>
#include "globals.h"

#if defined(X3270_FT)

#include <errno.h>

#include "appres.h"
#include "actionsc.h"
#include "ft_cutc.h"
#include "ft_dftc.h"
#include "ftc.h"
#include "dialogc.h"
#include "hostc.h"
#if defined(C3270) || defined(WC3270)
#include "icmdc.h"
#endif
#include "kybdc.h"
#include "macrosc.h"
#include "objects.h"
#include "popupsc.h"
#include "screenc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "utilc.h"

static void ft_connected(int ignored);
static void ft_in3270(int ignored unused);

/* Macros. */
#define eos(s)	strchr((s), '\0')

#if defined(X3270_DISPLAY)
#define FILE_WIDTH	300	// width of file name widgets
#define MARGIN		3	// distance from margins to widgets
#define CLOSE_VGAP	0	// distance between paired toggles
#define FAR_VGAP	10	// distance between single toggles and groups
#define BUTTON_GAP	5	// horizontal distance between buttons
#define COLUMN_GAP	40	// distance between columns
#endif

#define BN	(Boolean *)NULL

// Globals.
enum ft_state ft_state = FT_NONE;		// File transfer state
char *ft_local_filename;				// Local file to transfer to/from
FILE *ft_local_file = (FILE *)NULL;		// File descriptor for local file
Boolean ft_last_cr = False;				// CR was last char in local file
Boolean ascii_flag = True;				// Convert to ascii
Boolean cr_flag = True;					// Add crlf to each line
Boolean remap_flag = True;				// Remap ASCII<->EBCDIC
unsigned long ft_length = 0;			// Length of transfer
static Boolean ft_is_cut;				// File transfer is CUT-style

static struct timeval starting_time;	// Starting time

static const struct filetransfer_callbacks	*callbacks = NULL;		// Callbacks to main application

#define snconcat(x,s,fmt,...) snprintf(x+strlen(x),s-strlen(x),fmt,__VA_ARGS__)

#define set_ft_state(x) ft_state = x

/*---[ Implement ]-------------------------------------------------------------------------------------------------------*/

 void ft_init(void)
 {
	/* Register for state changes. */
	register_schange(ST_CONNECT, ft_connected);
	register_schange(ST_3270_MODE, ft_in3270);
 }

 enum ft_state	QueryFTstate(void)
 {
 	return ft_state;
 }

 int RegisterFTCallbacks(const struct filetransfer_callbacks *cbk)
 {
 	if(!(cbk && cbk->sz == sizeof(struct filetransfer_callbacks)) )
		return EINVAL;

	callbacks = cbk;

	return 0;
 }

 static int cant_start(int errcode, const char *errmsg)
 {
	if(callbacks && callbacks->complete)
		callbacks->complete(errmsg,0,0,"");
 	return errcode;
 }

 LIB3270_EXPORT int CancelFileTransfer(int force)
 {
	if (ft_state == FT_RUNNING)
	{
		set_ft_state(FT_ABORT_WAIT);
		if(callbacks && callbacks->aborting)
			callbacks->aborting();
		return 0;
	}

	if(!force)
		return EBUSY;

	// Impatient user or hung host -- just clean up.
	ft_complete( _("Cancelled by user") );

	return ECANCELED;
 }

 LIB3270_EXPORT int BeginFileTransfer(unsigned short flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft)
 {
 	static const char	*rec	= "fvu";
 	static const char	*un[]	= { "tracks", "cylinders", "avblock" };

 	unsigned short	recfm	= (flags & FT_RECORD_FORMAT_MASK) >> 8;
 	unsigned short	units	= (flags & FT_ALLOCATION_UNITS_MASK) >> 12;

 	char 				op[4096];
 	char				buffer[4096];

	unsigned int		flen;

	Trace("%s(%s)",__FUNCTION__,local);

	if(ft_local_file)
		return cant_start(EBUSY,_( "File transfer is already active"));

	// Check remote file
	if(!*remote)
		return cant_start(EINVAL,_( "The remote file name is invalid"));

	// Open local file
	ft_local_file = fopen(local,(flags & FT_FLAG_RECEIVE) ? ((flags & FT_FLAG_APPEND) ? "a" : "w") : "r");

	if(!ft_local_file)
		return cant_start(errno,_( "Can't open local file"));

	// Set options
	dft_buffersize = dft;
	set_dft_buffersize();

	ascii_flag = ((flags & FT_FLAG_ASCII) != 0);
	cr_flag = ((flags & FT_FLAG_CRLF) != 0);
	remap_flag = ((flags & FT_FLAG_REMAP_ASCII) != 0);

	Log("%s file \"%s\"",(flags & FT_FLAG_RECEIVE) ? "Receiving" : "Sending", local);

 	/* Build the ind$file command */
 	snprintf(op,4095,"%s%s%s",
						(flags & FT_FLAG_ASCII) 	? " ASCII"	: "",
						(flags & FT_FLAG_CRLF) 		? " CRLF"	: "",
						(flags & FT_FLAG_APPEND)	? " APPEND"	: ""
			);

	if(!(flags & FT_FLAG_RECEIVE))
	{
		if(flags & FT_FLAG_TSO)
		{
			// TSO Host
			if(recfm > 0)
			{
				snconcat(op,4096," recfm(%c)",rec[recfm-1]);

				if(lrecl > 0)
					snconcat(op,4096," lrecl(%d)",lrecl);

				if(blksize > 0)
					snconcat(op,4096," blksize(%d)", blksize);
			}

			if(units > 0)
			{
				snconcat(op,4096," %s",un[units-1]);

				if(primspace > 0)
				{
					snconcat(op,4096," space(%d",primspace);
					if(secspace)
						snconcat(op,4096,",%d",secspace);
					snconcat(op,4096,"%s",")");
				}
			}
		}
		else
		{
			// VM Host
			if(recfm > 0)
			{
				snconcat(op,4096," recfm %c",rec[recfm-1]);

				if(lrecl > 0)
					snconcat(op,4096," lrecl %d",lrecl);

			}
		}
	}

	snprintf(buffer,4095,"%s %s %s",	"IND$FILE",
										(flags & FT_FLAG_RECEIVE) ? "GET" : "PUT",
										remote );

	if(*op)
	{
		if(flags & FT_FLAG_TSO)
			snconcat(buffer,4095," %s",op+1);
		else
			snconcat(buffer,4095," (%s)",op+1);
	}

	snconcat(buffer,4095,"%s","\n");

	// Erase the line and enter the command.
	flen = kybd_prime();
	if (!flen || flen < strlen(buffer) - 1)
	{
		Log("Unable to send command \"%s\"",buffer);
		fclose(ft_local_file);
		ft_local_file = NULL;
		return cant_start(-1,_( "Unable to send file-transfer request"));
	}

	Trace("Command: \"%s\"",buffer);

	(void) emulate_input(buffer, strlen(buffer), False);

	// Get this thing started.
	set_ft_state(FT_AWAIT_ACK);

	ft_last_cr = False;
	ft_is_cut = False;

	if(callbacks && callbacks->begin)
		callbacks->begin(flags,local,remote);

 	return 0;
 }

/* External entry points called by ft_dft and ft_cut. */

/* Pop up a message, end the transfer. */
void
ft_complete(const char *errmsg)
{
	double kbytes_sec = 0;
	struct timeval	t1;

	(void) gettimeofday(&t1, (struct timezone *)NULL);
	kbytes_sec = (double)ft_length / 1024.0 /
		((double)(t1.tv_sec - starting_time.tv_sec) +
		 (double)(t1.tv_usec - starting_time.tv_usec) / 1.0e6);

	Trace("%s",__FUNCTION__);

	// Close the local file.
	if(ft_local_file)
	{
		fclose(ft_local_file);
		ft_local_file = NULL;
	}
	else
	{
		Log("Unexpected call do %s(): ft_local_file is NULL",__FUNCTION__);
	}

	// Clean up the state.
	set_ft_state(FT_NONE);

	ft_update_length();

	if(callbacks && callbacks->complete)
		callbacks->complete(errmsg,ft_length,kbytes_sec,ft_is_cut ? "CUT" : "DFT");

}

/* Update the bytes-transferred count on the progress pop-up. */
void
ft_update_length(void)
{
	double kbytes_sec = 0;

	if(ft_length > 1024.0)
	{
		struct timeval	t1;

		(void) gettimeofday(&t1, (struct timezone *)NULL);
		kbytes_sec = (double)ft_length / 1024.0 /
			((double)(t1.tv_sec - starting_time.tv_sec) +
			 (double)(t1.tv_usec - starting_time.tv_usec) / 1.0e6);
	}

	Trace("%s",__FUNCTION__);

	if(callbacks && callbacks->update)
		callbacks->update(ft_length,kbytes_sec);

}

/* Process a transfer acknowledgement. */
void
ft_running(Boolean is_cut)
{
	Trace("%s",__FUNCTION__);

	ft_is_cut = is_cut;
	ft_length = 0;

	(void) gettimeofday(&starting_time, (struct timezone *)NULL);

	if (ft_state == FT_AWAIT_ACK)
		set_ft_state(FT_RUNNING);

	if(callbacks && callbacks->running)
		callbacks->running(is_cut);

	ft_update_length();

}

// Process a protocol-generated abort.
void
ft_aborting(void)
{
	Trace("%s",__FUNCTION__);

	if (ft_state == FT_RUNNING || ft_state == FT_ABORT_WAIT)
		set_ft_state(FT_ABORT_SENT);

	if(callbacks && callbacks->aborting)
		callbacks->aborting();

}

/* Process a disconnect abort. */
static void ft_connected(int ignored)
{
	if (!CONNECTED && ft_state != FT_NONE)
		ft_complete(MSG_("ftDisconnected","Host disconnected, transfer cancelled"));
}

/* Process an abort from no longer being in 3270 mode. */
static void ft_in3270(int ignored)
{
	if (!IN_3270 && ft_state != FT_NONE)
		ft_complete(MSG_("ftNot3270","Not in 3270 mode, transfer cancelled"));
}

/*
 * Script/macro action for file transfer.
 *  Transfer(option=value[,...])
 *  Options are:
 *   Direction=send|receive	default receive
 *   HostFile=name		required
 *   LocalFile=name			required
 *   Host=[tso|vm]		default tso
 *   Mode=[ascii|binary]	default ascii
 *   Cr=[add|remove|keep]	default add/remove
 *   Exist=[keep|replace|append]	default keep
 *   Recfm=[default|fixed|variable|undefined] default default
 *   Lrecl=n			no default
 *   Blksize=n			no default
 *   Allocation=[default|tracks|cylinders|avblock] default default
 *   PrimarySpace=n		no default
 *   SecondarySpace=n		no default
 *
static struct {
	const char *name;
	char *value;
	const char *keyword[4];
} tp[] = {
	{ "Direction",		CN, { "receive", "send" } },
	{ "HostFile" },
	{ "LocalFile" },
	{ "Host",		CN, { "tso", "vm" } },
	{ "Mode",		CN, { "ascii", "binary" } },
	{ "Cr",			CN, { "auto", "remove",	"add", "keep" } },
	{ "Exist",		CN, { "keep", "replace", "append" } },
	{ "Recfm",		CN, { "default", "fixed", "variable",
				      "undefined" } },
	{ "Lrecl" },
	{ "Blksize" },
	{ "Allocation",		CN, { "default", "tracks", "cylinders",
				      "avblock" } },
	{ "PrimarySpace" },
	{ "SecondarySpace" },
	{ "BufferSize" },
	{ CN }
};
enum ft_parm_name {
	PARM_DIRECTION,
	PARM_HOST_FILE,
	PARM_LOCAL_FILE,
	PARM_HOST,
	PARM_MODE,
	PARM_CR,
	PARM_EXIST,
	PARM_RECFM,
	PARM_LRECL,
	PARM_BLKSIZE,
	PARM_ALLOCATION,
	PARM_PRIMARY_SPACE,
	PARM_SECONDARY_SPACE,
	PARM_BUFFER_SIZE,
	N_PARMS
};

void
Transfer_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
	int i, k;
	Cardinal j;
	long l;
	char *ptr;
	char *ft_command = CN;

	char opts[80];
	char *op = opts + 1;
	char *cmd;
	unsigned flen;

	String *xparams = params;
	Cardinal xnparams = *num_params;

        action_debug(Transfer_action, event, params, num_params);

	ft_is_action = True;

	// Make sure we're connected.
	if (!IN_3270) {
		popup_an_error("Not connected");
		return;
	}

//#if defined(C3270) || defined(WC3270)
//	// Check for interactive mode.
//	if (xnparams == 0 && escaped) {
//	    	if (interactive_transfer(&xparams, &xnparams) < 0) {
//		    	return;
//		}
//	}
//#endif

	// Set everything to the default.
	for (i = 0; i < N_PARMS; i++) {
		Free(tp[i].value);
		if (tp[i].keyword[0] != CN)
			tp[i].value =
				NewString(tp[i].keyword[0]);
		else
			tp[i].value = CN;
	}

	// See what they specified.
	for (j = 0; j < xnparams; j++) {
		for (i = 0; i < N_PARMS; i++) {
			char *eq;
			int kwlen;

			eq = strchr(xparams[j], '=');
			if (eq == CN || eq == xparams[j] || !*(eq + 1)) {
				popup_an_error("Invalid option syntax: '%s'",
					xparams[j]);
				return;
			}
			kwlen = eq - xparams[j];
			if (!strncasecmp(xparams[j], tp[i].name, kwlen)
					&& !tp[i].name[kwlen]) {
				if (tp[i].keyword[0]) {
					for (k = 0;
					     tp[i].keyword[k] != CN && k < 4;
					     k++) {
						if (!strcasecmp(eq + 1,
							tp[i].keyword[k])) {
							break;
						}
					}
					if (k >= 4 ||
					    tp[i].keyword[k] == CN) {
						popup_an_error("Invalid option "
							"value: '%s'", eq + 1);
						return;
					}
				} else switch (i) {
				    case PARM_LRECL:
				    case PARM_BLKSIZE:
				    case PARM_PRIMARY_SPACE:
				    case PARM_SECONDARY_SPACE:
				    case PARM_BUFFER_SIZE:
					l = strtol(eq + 1, &ptr, 10);
					if (ptr == eq + 1 || *ptr) {
						popup_an_error("Invalid option "
							"value: '%s'", eq + 1);
						return;
					}
					break;
				    default:
					break;
				}
				tp[i].value = NewString(eq + 1);
				break;
			}
		}
		if (i >= N_PARMS) {
			popup_an_error("Unknown option: %s", xparams[j]);
			return;
		}
	}

	// Check for required values.
	if (tp[PARM_HOST_FILE].value == CN) {
		popup_an_error("Missing 'HostFile' option");
		return;
	}
	if (tp[PARM_LOCAL_FILE].value == CN) {
		popup_an_error("Missing 'LocalFile' option");
		return;
	}

	//
	// Start the transfer.  Much of this is duplicated from ft_start()
	// and should be made common.
	//
	if (tp[PARM_BUFFER_SIZE].value != CN)
		dft_buffersize = atoi(tp[PARM_BUFFER_SIZE].value);
	else
		dft_buffersize = 0;
	set_dft_buffersize();

	receive_flag = !strcasecmp(tp[PARM_DIRECTION].value, "receive");
	append_flag = !strcasecmp(tp[PARM_EXIST].value, "append");
	allow_overwrite = !strcasecmp(tp[PARM_EXIST].value, "replace");
	ascii_flag = !strcasecmp(tp[PARM_MODE].value, "ascii");
	if (!strcasecmp(tp[PARM_CR].value, "auto")) {
		cr_flag = ascii_flag;
	} else {
		cr_flag = !strcasecmp(tp[PARM_CR].value, "remove") ||
			  !strcasecmp(tp[PARM_CR].value, "add");
	}
	vm_flag = !strcasecmp(tp[PARM_HOST].value, "vm");
	recfm = DEFAULT_RECFM;
	for (k = 0; tp[PARM_RECFM].keyword[k] != CN && k < 4; k++) {
		if (!strcasecmp(tp[PARM_RECFM].value,
			    tp[PARM_RECFM].keyword[k]))  {
			recfm = (enum recfm)k;
			break;
		}
	}
	units = DEFAULT_UNITS;
	for (k = 0; tp[PARM_ALLOCATION].keyword[k] != CN && k < 4; k++) {
		if (!strcasecmp(tp[PARM_ALLOCATION].value,
			    tp[PARM_ALLOCATION].keyword[k]))  {
			units = (enum units)k;
			break;
		}
	}

	ft_host_filename = tp[PARM_HOST_FILE].value;
	ft_local_filename = tp[PARM_LOCAL_FILE].value;

	// See if the local file can be overwritten.
	if (receive_flag && !append_flag && !allow_overwrite) {
		ft_local_file = fopen(ft_local_filename, "r");
		if (ft_local_file != (FILE *)NULL) {
			(void) fclose(ft_local_file);
			popup_an_error("File exists");
			return;
		}
	}

	// Open the local file.
	ft_local_file = fopen(ft_local_filename,
	    receive_flag ?
		(append_flag ? "a" : "w" ) :
		"r");
	if (ft_local_file == (FILE *)NULL) {
		popup_an_errno(errno, "Open(%s)", ft_local_filename);
		return;
	}

	// Build the ind$file command
	op[0] = '\0';
	if (ascii_flag)
		strcat(op, " ascii");
	if (cr_flag)
		strcat(op, " crlf");
	if (append_flag && !receive_flag)
		strcat(op, " append");
	if (!receive_flag) {
		if (!vm_flag) {
			if (recfm != DEFAULT_RECFM) {
				// RECFM Entered, process
				strcat(op, " recfm(");
				switch (recfm) {
				    case FIXED:
					strcat(op, "f");
					break;
				    case VARIABLE:
					strcat(op, "v");
					break;
				    case UNDEFINED:
					strcat(op, "u");
					break;
				    default:
					break;
				};
				strcat(op, ")");
				if (tp[PARM_LRECL].value != CN)
					sprintf(eos(op), " lrecl(%s)",
					    tp[PARM_LRECL].value);
				if (tp[PARM_BLKSIZE].value != CN)
					sprintf(eos(op), " blksize(%s)",
					    tp[PARM_BLKSIZE].value);
			}
			if (units != DEFAULT_UNITS) {
				// Space Entered, processs it
				switch (units) {
				    case TRACKS:
					strcat(op, " tracks");
					break;
				    case CYLINDERS:
					strcat(op, " cylinders");
					break;
				    case AVBLOCK:
					strcat(op, " avblock");
					break;
				    default:
					break;
				};
				if (tp[PARM_PRIMARY_SPACE].value != CN) {
					sprintf(eos(op), " space(%s",
					    tp[PARM_PRIMARY_SPACE].value);
					if (tp[PARM_SECONDARY_SPACE].value)
						sprintf(eos(op), ",%s",
						    tp[PARM_SECONDARY_SPACE].value);
					strcat(op, ")");
				}
			}
		} else {
			if (recfm != DEFAULT_RECFM) {
				strcat(op, " recfm ");
				switch (recfm) {
				    case FIXED:
					strcat(op, "f");
					break;
				    case VARIABLE:
					strcat(op, "v");
					break;
				    default:
					break;
				};

				if (tp[PARM_LRECL].value)
					sprintf(eos(op), " lrecl %s",
					    tp[PARM_LRECL].value);
			}
		}
	}

	// Insert the '(' for VM options.
	if (strlen(op) > 0 && vm_flag) {
		opts[0] = ' ';
		opts[1] = '(';
		op = opts;
	}

	//
	// Unless the user specified a particular file transfer command,
	// translate 'ind$file' so that it will have the proper EBCDIC value,
	// regardless of the local character set.
	//
	if (appres.ft_command != CN) {
		ft_command = appres.ft_command;
	} else {
		char *s = "ind$file";
		char *t;
		unsigned char c;

		ft_command = Malloc(strlen(s) + 1);
		t = ft_command;

		while ((c = *s++)) {
			*t++ = ebc2asc[asc2ebc0[c & 0xff]];
		}
		*t = '\0';
	}

	// Build the whole command.
	cmd = xs_buffer("%s %s %s%s\\n",
	    ft_command,
	    receive_flag ? "get" : "put", ft_host_filename, op);
	if (appres.ft_command == CN)
		Free(ft_command);

	// Erase the line and enter the command.
	flen = kybd_prime();
	if (!flen || flen < strlen(cmd) - 1) {
		Free(cmd);
		popup_an_error(get_message("ftUnable"));
		return;
	}
	(void) emulate_input(cmd, strlen(cmd), False);
	Free(cmd);

	// Get this thing started.
	set_ft_state(FT_AWAIT_ACK);
	ft_is_cut = False;
}
*/

#endif

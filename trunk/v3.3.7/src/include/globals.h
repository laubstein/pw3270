/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe.
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
 * Este programa está nomeado como @@FILENAME@@ e possui @@LINES@@ linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

/* Autoconf settings. */
#include <lib3270/config.h>			/* autoconf settings */
#if defined(X3270_TN3270E) && !defined(X3270_ANSI) /*[*/
#define X3270_ANSI	1	/* RFC2355 requires NVT mode */
#endif /*]*/
#if defined(HAVE_VASPRINTF) && !defined(_GNU_SOURCE) /*[*/
#define _GNU_SOURCE		/* vasprintf isn't POSIX */
#endif /*]*/

/*
 * OS-specific #defines.  Except for the blocking-connect workarounds, these
 * should be replaced with autoconf probes as soon as possible.
 */

/*
 * BLOCKING_CONNECT_ONLY
 *   Use only blocking sockets.
 */
#if defined(sco) /*[*/
#define BLOCKING_CONNECT_ONLY	1
#endif /*]*/

#if defined(apollo) /*[*/
#define BLOCKING_CONNECT_ONLY	1
#endif /*]*/

/*
 * Compiler-specific #defines.
 */

/* 'unused' explicitly flags an unused parameter */
#if defined(__GNUC__) /*[*/
#define unused __attribute__((__unused__))
#define printflike(s,f) __attribute__ ((__format__ (__printf__, s, f)))
#else /*][*/
#define unused /* nothing */
#define printflike(s, f) /* nothing */
#endif /*]*/



/*
 * Prerequisite #includes.
 */
#include <stdio.h>				/* Unix standard I/O library */
#include <stdlib.h>				/* Other Unix library functions */
#include <unistd.h>				/* Unix system calls */
#include <ctype.h>				/* Character classes */
#include <string.h>				/* String manipulations */
#include <sys/types.h>			/* Basic system data types */
#include <sys/time.h>			/* System time-related data types */
#include <time.h>				/* C library time functions */
#include <lib3270/localdefs.h>	/* {s,tcl,c}3270-specific defines */

/*
 * Cancel out contradictory parts.
 */
#if !defined(X3270_DISPLAY) /*[*/
#undef X3270_KEYPAD
#undef X3270_MENUS
#endif /*]*/

/* Local process (-e) header files. */
#if defined(X3270_LOCAL_PROCESS) && defined(HAVE_LIBUTIL) /*[*/
#define LOCAL_PROCESS	1
#include <termios.h>
#if defined(HAVE_PTY_H) /*[*/
#include <pty.h>
#endif /*]*/
#if defined(HAVE_LIBUTIL_H) /*[*/
#include <libutil.h>
#endif /*]*/
#if defined(HAVE_UTIL_H) /*[*/
#include <util.h>
#endif /*]*/
#endif /*]*/

/* Functions we may need to supply. */
#if defined(NEED_STRTOK_R) /*[*/
extern char *strtok_r(char *str, const char *sep, char **last);
#endif /*]*/

/* Stop conflicting with curses' COLS, even if we don't link with it. */
#define COLS cCOLS

/* Simple global variables */

extern int		COLS;
extern int		ROWS;
#if defined(X3270_DISPLAY) /*[*/
extern Atom		a_3270, a_registry, a_encoding;
extern XtAppContext	appcontext;
#endif /*]*/
extern const char	*build;
extern const char	*build_rpq_timestamp;
extern const char 	*build_rpq_version;
extern int		children;
extern char		*connected_lu;
extern char		*connected_type;
extern char		*current_host;
extern unsigned short	current_port;
#if defined(X3270_DBCS) /*[*/
extern Boolean		dbcs;
#endif /*]*/
#if defined(X3270_FT) /*[*/
extern int		dft_buffersize;
#endif /*]*/
extern char		*efontname;
extern Boolean		ever_3270;
extern Boolean		exiting;
#if defined(X3270_DISPLAY) /*[*/
extern Boolean		*extended_3270font;
extern Font		*fid;
extern Boolean		*font_8bit;
#endif /*]*/
extern Boolean		flipped;
extern char		*full_current_host;
extern char		*full_efontname;
#if defined(X3270_DBCS) /*[*/
extern char		*full_efontname_dbcs;
#endif /*]*/
extern char		full_model_name[];
extern char		*funky_font;
extern char		*hostname;
#if defined(X3270_DBCS) /*[*/
extern char		*local_encoding;
#if defined(X3270_DISPLAY) /*[*/
extern char		*locale_name;
#endif /*]*/
#endif /*]*/
extern char		luname[];
#if defined(LOCAL_PROCESS) /*[*/
extern Boolean		local_process;
#endif /*]*/
extern int		maxCOLS;
extern int		maxROWS;
extern char		*model_name;
extern int		model_num;
extern Boolean		no_login_host;
extern Boolean		non_tn3270e_host;
extern int		ov_cols, ov_rows;
extern Boolean		passthru_host;
extern const char	*programname;
extern char		*qualified_host;
extern char		*reconnect_host;
extern int		screen_depth;
extern Boolean		scroll_initted;
#if defined(HAVE_LIBSSL) /*[*/
extern Boolean		secure_connection;
#endif /*]*/
extern Boolean		shifted;
extern Boolean		ssl_host;
extern Boolean		*standard_font;
extern Boolean		std_ds_host;
extern char		*termtype;
extern Widget		toplevel;
extern Boolean		visible_control;
extern int		*xtra_width;

#if defined(X3270_DISPLAY) /*[*/
extern Atom		a_delete_me;
extern Atom		a_save_yourself;
extern Atom		a_state;
extern Display		*display;
extern Pixmap		gray;
extern Pixel		keypadbg_pixel;
extern XrmDatabase	rdb;
extern Window		root_window;
extern char		*user_title;
extern unsigned char	xk_selector;
#endif /*]*/

#if defined(C3270) && defined(_WIN32) /*[*/
extern char		*instdir;
extern char		myappdata[];
#endif /*]*/

/* Data types and complex global variables */

#ifdef LIB3270

	extern enum cstate cstate;

	#define PCONNECTED	((int)cstate >= (int)RESOLVING)
	#define HALF_CONNECTED	(cstate == RESOLVING || cstate == PENDING)
	#define CONNECTED	((int)cstate >= (int)CONNECTED_INITIAL)
	#define IN_NEITHER	(cstate == CONNECTED_INITIAL)
	#define IN_ANSI		(cstate == CONNECTED_ANSI || cstate == CONNECTED_NVT)
	#define IN_3270		(cstate == CONNECTED_3270 || cstate == CONNECTED_TN3270E || cstate == CONNECTED_SSCP)
	#define IN_SSCP		(cstate == CONNECTED_SSCP)
	#define IN_TN3270E	(cstate == CONNECTED_TN3270E)
	#define IN_E		(cstate >= CONNECTED_INITIAL_E)

#endif

/*   keyboard modifer bitmap */
#define ShiftKeyDown	0x01
#define MetaKeyDown	0x02
#define AltKeyDown	0x04

/*   toggle names */
struct toggle_name {
	const char *name;
	int index;
};

/*   extended attributes */
struct ea {
	unsigned char cc;	/* EBCDIC or ASCII character code */
	unsigned char fa;	/* field attribute, it nonzero */
	unsigned char fg;	/* foreground color (0x00 or 0xf<n>) */
	unsigned char bg;	/* background color (0x00 or 0xf<n>) */
	unsigned char gr;	/* ANSI graphics rendition bits */
	unsigned char cs;	/* character set (GE flag, or 0..2) */
	unsigned char ic;	/* input control (DBCS) */
	unsigned char db;	/* DBCS state */
};
#define GR_BLINK	0x01
#define GR_REVERSE	0x02
#define GR_UNDERLINE	0x04
#define GR_INTENSIFY	0x08

#define CS_MASK		0x03	/* mask for specific character sets */
#define CS_BASE		0x00	/*  base character set (X'00') */
#define CS_APL		0x01	/*  APL character set (X'01' or GE) */
#define CS_LINEDRAW	0x02	/*  DEC line-drawing character set (ANSI) */
#define CS_DBCS		0x03	/*  DBCS character set (X'F8') */
#define CS_GE		0x04	/* cs flag for Graphic Escape */

struct ea * copy_device_buffer(int *el);	/* Return a "malloced" copy of the device buffer, set number of elements */
int set_device_buffer(struct ea *src, int el);	/* Set the contents of the device buffer for debugging purposes */

/*   translation lists */
struct trans_list {
	char			*name;
	char			*pathname;
	Boolean			is_temp;
	Boolean			from_server;
	struct trans_list	*next;
};
extern struct trans_list *trans_list;

/*   input key type */
// enum keytype { KT_STD, KT_GE };

/*   state changes */
enum state_change
{
	ST_RESOLVING,
	ST_HALF_CONNECT,
	ST_CONNECT,
	ST_3270_MODE,
	ST_LINE_MODE,
	ST_REMODEL,
	ST_PRINTER,
	ST_EXITING,
	ST_CHARSET,

	N_ST				// Always the last one
};

/* Naming convention for private actions. */
#define PA_PFX	"PA-"

/* Shorthand macros */

#define CN	((char *) NULL)
#define PN	((XtPointer) NULL)
#define Replace(var, value) { Free(var); var = (value); }

/* Configuration change masks. */
#define NO_CHANGE	0x0000	/* no change */
#define MODEL_CHANGE	0x0001	/* screen dimensions changed */
#define FONT_CHANGE	0x0002	/* emulator font changed */
#define COLOR_CHANGE	0x0004	/* color scheme or 3278/9 mode changed */
#define SCROLL_CHANGE	0x0008	/* scrollbar snapped on or off */
#define CHARSET_CHANGE	0x0010	/* character set changed */
#define ALL_CHANGE	0xffff	/* everything changed */

/* Portability macros */

/*   Equivalent of setlinebuf */

#if defined(_IOLBF) /*[*/
#define SETLINEBUF(s)	setvbuf(s, (char *)NULL, _IOLBF, BUFSIZ)
#else /*][*/
#define SETLINEBUF(s)	setlinebuf(s)
#endif /*]*/

/*   Motorola version of gettimeofday */

#if defined(MOTOROLA)
#define gettimeofday(tp,tz)	gettimeofday(tp)
#endif

/* Default DFT file transfer buffer size. */
#if defined(X3270_FT) && !defined(DFT_BUF) /*[*/
#define DFT_BUF		(4 * 1024)
#endif /*]*/

/* DBCS Preedit Types */
#if defined(X3270_DBCS) /*[*/
#define PT_ROOT			"Root"
#define PT_OVER_THE_SPOT	"OverTheSpot"
#define PT_OFF_THE_SPOT		"OffTheSpot"
#define PT_ON_THE_SPOT		"OnTheSpot"
#endif /*]*/


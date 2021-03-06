/*
 * "Software PW3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como api.h e possui 444 linhas de código.
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


#ifndef LIB3270_API_INCLUDED

#ifdef __cplusplus
	extern "C" {
#endif

		#define LIB3270_API_INCLUDED "4.2"

		#include <errno.h>

		#if defined(_WIN32)
			#include <windows.h>

			#define LIB3270_EXPORT	__declspec (dllexport)

			#if defined (HAVE_GNUC_VISIBILITY)
					#define LOCAL_EXTERN	__attribute__((visibility("hidden"))) extern
			#else
					#define LOCAL_EXTERN extern
			#endif

		#else
			#include <stdarg.h>

			// http://gcc.gnu.org/wiki/Visibility
			#if defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
					#define LOCAL_EXTERN __hidden extern
					#define LIB3270_EXPORT
			#elif defined (HAVE_GNUC_VISIBILITY)
					#define LOCAL_EXTERN	__attribute__((visibility("hidden"))) extern
					#define LIB3270_EXPORT	__attribute__((visibility("default"))) extern
			#else
					#define LOCAL_EXTERN extern
					#define LIB3270_EXPORT
			#endif


		#endif

		#ifndef HCONSOLE
			#define HCONSOLE void *
		#endif

		#ifndef ETIMEDOUT
			#define ETIMEDOUT -1238
		#endif

		#ifndef ECANCELED
			#ifdef EINTR
				#define ECANCELED EINTR
			#else
				#define ECANCELED -1125
			#endif
		#endif

		#ifndef ENOTCONN
			#define ENOTCONN -1107
		#endif

		#ifndef CN
			#define CN ((char *) NULL)
		#endif

		/* Debug & log */
		#if defined( DEBUG )
			#define Trace( fmt, ... )		fprintf(stderr, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fflush(stderr); \
											WriteLog("TRACE", "%s(%d) " fmt , __FILE__, __LINE__, __VA_ARGS__ )
		#else
			#define Trace( fmt, ... )	/* __VA_ARGS__ */
		#endif


		LIB3270_EXPORT int Set3270Log(const char *filename);
		LIB3270_EXPORT int WriteLog(const char *module, const char *fmt, ...);
		LIB3270_EXPORT int WriteRCLog(const char *module, int rc, const char *fmt, ...);

		#ifdef LIB3270_MODULE_NAME
			#define Log(fmt, ...)		WriteLog(LIB3270_MODULE_NAME,fmt,__VA_ARGS__)
		#else
			#define Log(fmt, ...)		WriteLog("MSG",fmt,__VA_ARGS__)
		#endif

		/** 3270 connection handle */
		#define LUNAME_SIZE				16
		#define FULL_MODEL_NAME_SIZE	13

		/* State change IDs. */
		typedef enum _lib3270_state
		{
			LIB3270_STATE_RESOLVING,
			LIB3270_STATE_HALF_CONNECT,
			LIB3270_STATE_CONNECT,
			LIB3270_STATE_3270_MODE,
			LIB3270_STATE_LINE_MODE,
			LIB3270_STATE_REMODEL,
			LIB3270_STATE_PRINTER,
			LIB3270_STATE_EXITING,
			LIB3270_STATE_CHARSET,

			LIB3270_STATE_USER				// Always the last one
		} LIB3270_STATE;

		#define ST_RESOLVING			LIB3270_STATE_RESOLVING
		#define ST_HALF_CONNECT			LIB3270_STATE_HALF_CONNECT
		#define ST_CONNECT				LIB3270_STATE_CONNECT
		#define ST_3270_MODE			LIB3270_STATE_3270_MODE
		#define ST_LINE_MODE			LIB3270_STATE_LINE_MODE
		#define ST_REMODEL				LIB3270_STATE_REMODEL
		#define ST_PRINTER				LIB3270_STATE_PRINTER
		#define ST_EXITING				LIB3270_STATE_EXITING
		#define ST_CHARSET				LIB3270_STATE_CHARSET
		#define N_ST					LIB3270_STATE_USER
		#define LIB3270_STATE_CHANGE	LIB3270_STATE

		/** connection state */
		#define cstate LIB3270_CSTATE
		#define NOT_CONNECTED	LIB3270_NOT_CONNECTED
		#define RESOLVING	LIB3270_RESOLVING
		#define PENDING	LIB3270_PENDING
		#define CONNECTED_INITIAL	LIB3270_CONNECTED_INITIAL
		#define CONNECTED_ANSI	LIB3270_CONNECTED_ANSI
		#define CONNECTED_3270	LIB3270_CONNECTED_3270
		#define CONNECTED_INITIAL_E	LIB3270_CONNECTED_INITIAL_E
		#define CONNECTED_NVT	LIB3270_CONNECTED_NVT
		#define CONNECTED_SSCP	LIB3270_CONNECTED_SSCP
		#define CONNECTED_TN3270E	LIB3270_CONNECTED_TN3270E

		#define LIB3270_STATUS					LIB3270_MESSAGE
		#define LIB3270_STATUS_BLANK			LIB3270_MESSAGE_NONE
		#define LIB3270_STATUS_SYSWAIT			LIB3270_MESSAGE_SYSWAIT
		#define LIB3270_STATUS_TWAIT			LIB3270_MESSAGE_TWAIT
		#define LIB3270_STATUS_CONNECTED		LIB3270_MESSAGE_CONNECTED
		#define LIB3270_STATUS_DISCONNECTED		LIB3270_MESSAGE_DISCONNECTED
		#define LIB3270_STATUS_AWAITING_FIRST	LIB3270_MESSAGE_AWAITING_FIRST
		#define LIB3270_STATUS_MINUS			LIB3270_MESSAGE_MINUS
		#define LIB3270_STATUS_PROTECTED		LIB3270_MESSAGE_PROTECTED
		#define LIB3270_STATUS_NUMERIC			LIB3270_MESSAGE_NUMERIC
		#define LIB3270_STATUS_OVERFLOW			LIB3270_MESSAGE_OVERFLOW
		#define LIB3270_STATUS_INHIBIT			LIB3270_MESSAGE_INHIBIT
		#define LIB3270_STATUS_KYBDLOCK			LIB3270_MESSAGE_KYBDLOCK
		#define LIB3270_STATUS_X				LIB3270_MESSAGE_X
		#define LIB3270_STATUS_RESOLVING		LIB3270_MESSAGE_RESOLVING
		#define LIB3270_STATUS_CONNECTING		LIB3270_MESSAGE_CONNECTING
		#define LIB3270_STATUS_USER				LIB3270_MESSAGE_USER

		#define OIA_FLAG_BOXSOLID	LIB3270_FLAG_BOXSOLID
		#define OIA_FLAG_UNDERA		LIB3270_FLAG_UNDERA
		#define OIA_FLAG_SECURE		LIB3270_FLAG_SECURE
		#define OIA_FLAG_TYPEAHEAD	LIB3270_FLAG_TYPEAHEAD
		#define OIA_FLAG_PRINTER	LIB3270_FLAG_PRINTER
		#define OIA_FLAG_REVERSE	LIB3270_FLAG_REVERSE
		#define OIA_FLAG_USER		LIB3270_FLAG_COUNT
		#define OIA_FLAG			LIB3270_FLAG

		struct lib3270_state_callback;

		typedef struct _h3270 H3270;
		struct _h3270
		{
			unsigned short 	  sz;				/**< Struct size */

			// Connection info
			int					  secure_connection;
			int					  valid_certificate;
			int      			  sock;					/**< Network socket */
			int					  net_sock;
			LIB3270_CSTATE		  cstate;				/**< Connection state */
			int					  as400;				/**< Host is AS400 */

			#if defined(_WIN32) /*[*/
			HANDLE				  sock_handle;
			#endif /*]*/

			char    			* hostname;
			char				* connected_type;
			char				* connected_lu;
			char				  luname[LUNAME_SIZE+1];

			char				  full_model_name[FULL_MODEL_NAME_SIZE+1];
			char				* model_name;
			int					  model_num;
			char       	    	* termtype;

			char				* current_host;			/**< the hostname part, stripped of qualifiers, luname and port number */
			char           		* full_current_host;	/**< the entire string, for use in reconnecting */
			char	       		* reconnect_host;
			char	       		* qualified_host;
			char	 			  auto_reconnect_inprogress;

			LIB3270_MESSAGE		  oia_status;

			unsigned char 		  oia_flag[LIB3270_FLAG_COUNT];

			unsigned short		  current_port;

			// screen info
			int					  ov_rows;
			int					  ov_cols;
//			int					  first_changed;
//			int					  last_changed;
			int					  maxROWS;
			int					  maxCOLS;
			unsigned short		  rows;
			unsigned short		  cols;
			int					  cursor_addr;
			char				  flipped;
			int					  screen_alt;			/**< alternate screen? */
			int					  is_altbuffer;

			int					  formatted;			/**< set in screen_disp */

			// host.c
			char 				  std_ds_host;
			char 				  no_login_host;
			char 				  non_tn3270e_host;
			char 				  passthru_host;
			char 				  ssl_host;
			char 				  ever_3270;

			// Widget info
			void				* widget;

			// xio
			unsigned long		  ns_read_id;
			unsigned long		  ns_exception_id;
			char				  reading;
			char				  excepting;

			/* State change callbacks. */
			struct lib3270_state_callback *st_callbacks[N_ST];
			struct lib3270_state_callback *st_last[N_ST];

			/* Session based callbacks */
			void (*configure)(H3270 *session, unsigned short rows, unsigned short cols);
			void (*update)(H3270 *session, int baddr, unsigned char c, unsigned short attr, unsigned char cursor);
			void (*changed)(H3270 *session, int bstart, int bend);

			void (*update_cursor)(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
			void (*update_oia)(H3270 *session, OIA_FLAG id, unsigned char on);
			void (*update_toggle)(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason, const char *name);
			void (*update_luname)(H3270 *session, const char *name);
			void (*update_status)(H3270 *session, LIB3270_STATUS id);
			void (*update_connect)(H3270 *session, unsigned char connected);
			void (*update_model)(H3270 *session, const char *name, int model, int rows, int cols);

			void (*set_timer)(H3270 *session, unsigned char on);
			void (*erase)(H3270 *session);
			void (*cursor)(H3270 *session, LIB3270_CURSOR id);

		};

		struct lib3270_state_callback
		{
			struct lib3270_state_callback	* next;			/**< Next callback in chain */
			void							* data;			/**< User data */
			void (*func)(H3270 *, int, void *);		/**< Function to call */
		};


		/** Type of dialog boxes */
		typedef enum _PW3270_DIALOG
		{
			PW3270_DIALOG_INFO,		/**< Simple information dialog */
			PW3270_DIALOG_CRITICAL,	/**< Critical error, user can abort application */

			PW3270_DIALOG_USER
		} PW3270_DIALOG;

		/** input key type */
		enum keytype
		{
			KT_STD,
			KT_GE
		};

		/**  extended attributes */
		struct ea
		{
			unsigned char cc;		/**< EBCDIC or ASCII character code */
			unsigned char fa;		/**< field attribute, it nonzero */
			unsigned char fg;		/**< foreground color (0x00 or 0xf<n>) */
			unsigned char bg;		/**< background color (0x00 or 0xf<n>) */
			unsigned char gr;		/**< ANSI graphics rendition bits */
			unsigned char cs;		/**< character set (GE flag, or 0..2) */
			unsigned char ic;		/**< input control (DBCS) */
			unsigned char db;		/**< DBCS state */

			/* Updated by addch() */
			unsigned char  chr;		/**< ASCII character code */
			unsigned short attr;	/**< Converted character attribute (color & etc) */

		};
		#define GR_BLINK		0x01
		#define GR_REVERSE		0x02
		#define GR_UNDERLINE	0x04
		#define GR_INTENSIFY	0x08

		#define CS_MASK			0x03	/**< mask for specific character sets */
		#define CS_BASE			0x00	/**< base character set (X'00') */
		#define CS_APL			0x01	/**< APL character set (X'01' or GE) */
		#define CS_LINEDRAW		0x02	/**< DEC line-drawing character set (ANSI) */
		#define CS_DBCS			0x03	/**< DBCS character set (X'F8') */
		#define CS_GE			0x04	/**< cs flag for Graphic Escape */

		/**
		 * Return a "malloced" copy of the device buffer, set number of elements
		 */
		LIB3270_EXPORT struct ea * copy_device_buffer(int *el);

		/**
		 * Set the contents of the device buffer for debugging purposes
		 */
		LIB3270_EXPORT int  set_device_buffer(struct ea *src, int el);

		/* File transfer */
		#define FT_FLAG_RECEIVE					0x0001
		#define FT_FLAG_ASCII					0x0002
		#define FT_FLAG_CRLF					0x0004
		#define FT_FLAG_APPEND					0x0008
		#define FT_FLAG_TSO						0x0010
		#define FT_FLAG_REMAP_ASCII				0x0020

		#define FT_RECORD_FORMAT_FIXED			0x0100
		#define FT_RECORD_FORMAT_VARIABLE		0x0200
		#define FT_RECORD_FORMAT_UNDEFINED		0x0300
		#define FT_RECORD_FORMAT_MASK 			FT_RECORD_FORMAT_UNDEFINED

		#define FT_ALLOCATION_UNITS_TRACKS		0x1000
		#define FT_ALLOCATION_UNITS_CYLINDERS	0x2000
		#define FT_ALLOCATION_UNITS_AVBLOCK		0x3000
		#define FT_ALLOCATION_UNITS_MASK		FT_ALLOCATION_UNITS_AVBLOCK

		enum ft_state
		{
			FT_NONE,		/**< No transfer in progress */
			FT_AWAIT_ACK,	/**< IND$FILE sent, awaiting acknowledgement message */
			FT_RUNNING,		/**< Ack received, data flowing */
			FT_ABORT_WAIT,	/**< Awaiting chance to send an abort */
			FT_ABORT_SENT	/**< Abort sent; awaiting response */
		};

		LIB3270_EXPORT int 				BeginFileTransfer(unsigned short flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft);
		LIB3270_EXPORT int 				CancelFileTransfer(int force);
		LIB3270_EXPORT enum ft_state	GetFileTransferState(void);

		struct filetransfer_callbacks
		{
			unsigned short sz;

			void (*begin)(unsigned short flags, const char *local, const char *remote);
			void (*complete)(const char *errmsg,unsigned long length,double kbytes_sec,const char *mode);
			void (*setlength)(unsigned long length);
			void (*update)(unsigned long length,double kbytes_sec);
			void (*running)(int is_cut);
			void (*aborting)(void);

		};

		LIB3270_EXPORT int RegisterFTCallbacks(const struct filetransfer_callbacks *cbk);

//		#define QueryCstate() lib3270_get_connection_state(NULL)

		#define PCONNECTED		lib3270_pconnected(NULL)
		#define HALF_CONNECTED	lib3270_half_connected(NULL)
		#define CONNECTED		lib3270_connected(NULL)

		#define IN_NEITHER		lib3270_in_neither(NULL)
		#define IN_ANSI			lib3270_in_ansi(NULL)
		#define IN_3270			lib3270_in_3270(NULL)
		#define IN_SSCP			lib3270_in_sscp(NULL)
		#define IN_TN3270E		lib3270_in_tn3270e(NULL)
		#define IN_E			lib3270_in_e(NULL)

		#ifndef LIB3270


			LIB3270_EXPORT enum ft_state	QueryFTstate(void);

//			#define PCONNECTED	((int) QueryCstate() >= (int)RESOLVING)
//			#define HALF_CONNECTED	(QueryCstate() == RESOLVING || QueryCstate() == PENDING)
//			#define CONNECTED	((int) QueryCstate() >= (int)CONNECTED_INITIAL)
//			#define IN_NEITHER	(QueryCstate() == CONNECTED_INITIAL)
//			#define IN_ANSI		(QueryCstate() == CONNECTED_ANSI || QueryCstate() == CONNECTED_NVT)
//			#define IN_3270		(QueryCstate() == CONNECTED_3270 || QueryCstate() == CONNECTED_TN3270E || QueryCstate() == CONNECTED_SSCP)
//			#define IN_SSCP		(QueryCstate() == CONNECTED_SSCP)
//			#define IN_TN3270E	(QueryCstate() == CONNECTED_TN3270E)
//			#define IN_E		(QueryCstate() >= CONNECTED_INITIAL_E)

		#endif

		/* I/O processing */
		struct lib3270_io_callbacks
		{
			unsigned short sz;

			unsigned long (*AddTimeOut)(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session));
			void 			(*RemoveTimeOut)(unsigned long timer);

			unsigned long (*AddInput)(int source, H3270 *session, void (*fn)(H3270 *session));
			void			(*RemoveInput)(unsigned long id);

			unsigned long (*AddExcept)(int source, H3270 *session, void (*fn)(H3270 *session));

			#if !defined(_WIN32) /*[*/
				unsigned long (*AddOutput)(int source, H3270 *session, void (*fn)(H3270 *session));
			#endif /*]*/

			int 			(*callthread)(int(*callback)(H3270 *, void *), H3270 *session, void *parm);

			int				(*Wait)(int seconds);
			int 			(*RunPendingEvents)(int wait);

		};

		#define Register3270IOCallbacks(x) lib3270_register_io_handlers(x)


		/* Screen processing */

		#define CURSOR_MODE_NORMAL		LIB3270_CURSOR_NORMAL
		#define CURSOR_MODE_WAITING		LIB3270_CURSOR_WAITING
		#define CURSOR_MODE_LOCKED		LIB3270_CURSOR_LOCKED

		typedef enum _SCRIPT_STATE
		{
			SCRIPT_STATE_NONE,
			SCRIPT_STATE_RUNNING,
			SCRIPT_STATE_HALTED,

			SCRIPT_STATE_USER

		} SCRIPT_STATE;

		typedef enum _COUNTER_ID
		{
			COUNTER_ID_CTLR_DONE,
			COUNTER_ID_RESET,

			COUNTER_ID_USER
		} COUNTER_ID;

		LIB3270_EXPORT int query_counter(COUNTER_ID id);

		#define	query_screen_change_counter() query_counter(COUNTER_ID_CTLR_DONE)

		#define COLOR_ATTR_NONE			0x0000
		#define COLOR_ATTR_FIELD		LIB3270_ATTR_FIELD
		#define COLOR_ATTR_BLINK		LIB3270_ATTR_BLINK
		#define COLOR_ATTR_UNDERLINE	LIB3270_ATTR_UNDERLINE
		#define COLOR_ATTR_INTENSIFY	LIB3270_ATTR_INTENSIFY

		#define CHAR_ATTR_CG			LIB3270_ATTR_CG
		#define CHAR_ATTR_MARKER		LIB3270_ATTR_MARKER

		#define CHAR_ATTR_UNCONVERTED	CHAR_ATTR_CG

		struct lib3270_screen_callbacks
		{
			unsigned short	sz;

			int		(*init)(void);
			int		(*popup_dialog)(H3270 *session, PW3270_DIALOG type, const char *title, const char *msg, const char *fmt, va_list arg);
			void	(*Error)(const char *fmt, va_list arg);
			void	(*Warning)(const char *fmt, va_list arg);
			void	(*SysError)(const char *title, const char *message, const char *system);
			void 	(*model_changed)(H3270 *session, const char *name, int model, int rows, int cols);
			int		(*addch)(int row, int col, unsigned char c, unsigned short attr);
			void	(*charset)(char *dcs);
			void	(*title)(char *text);
			void	(*ring_bell)(void);
			void	(*redraw)(void);
			void	(*move_cursor)(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
			int		(*set_suspended)(int state);
			void	(*set_script)(SCRIPT_STATE state);
			void	(*reset)(int lock);
			void	(*status)(H3270 *session, LIB3270_STATUS id);
			void	(*cursor)(H3270 *session, LIB3270_CURSOR mode);
			void	(*lu)(H3270 *session, const char *lu);
			void	(*set_oia)(H3270 *session, OIA_FLAG id, unsigned char on);

			void	(*erase)(H3270 *session);
			void	(*display)(H3270 *session);
			void 	(*set_viewsize)(H3270 *session, unsigned short rows, unsigned short cols);


			void	(*toggle_changed)(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason, const char *name);
			void	(*show_timer)(long seconds);

			// Console/Trace window
			HCONSOLE	(*console_new)(const char *title, const char *label);
			void		(*console_delete)(HCONSOLE hwnd);
			int			(*console_append)(HCONSOLE hwnd, const char *fmt, va_list 	args);
			char *		(*console_entry)(HCONSOLE hwnd);

		};

		struct lib3270_option
		{
			const char *name;
			enum
			{
				OPT_BOOLEAN,
				OPT_STRING,
				OPT_INTEGER,
				OPT_XRM,
				OPT_SKIP2,
				OPT_NOP,
				OPT_DONE
			} type;
			unsigned char	flag;
			const char		*res_name;
			void			*aoff;
			const char		*description;
		};

		LIB3270_EXPORT int Register3270ScreenCallbacks(const struct lib3270_screen_callbacks *cbk);

		#define new_3270_session(m) lib3270_session_new(m)

		LIB3270_EXPORT const struct lib3270_option * get_3270_option_table(int sz);

		/* Popups */
		LIB3270_EXPORT void Error(const char *fmt, ...);
		LIB3270_EXPORT void Warning(const char *fmt, ...);
		LIB3270_EXPORT void show_3270_popup_dialog(H3270 *session, PW3270_DIALOG type, const char *title, const char *msg, const char *fmt, ...);

		/* Set/Get screen contents */
		LIB3270_EXPORT int find_field_attribute(H3270 *session, int baddr);
		LIB3270_EXPORT int find_field_length(H3270 *session, int baddr);
		LIB3270_EXPORT unsigned char get_field_attribute(H3270 *session, int baddr);
		LIB3270_EXPORT int screen_read(char *dest, int baddr, int count);
		LIB3270_EXPORT void Input_String(const unsigned char *str);
		LIB3270_EXPORT void screen_size(int *rows, int *cols);

		#define query_secure_connection(h) lib3270_get_ssl_state(h)
		#define query_ssl_cert_check_status(h) lib3270_get_ssl_cert_state(h)
		#define lib3270_paste_string(str) lib3270_set_string(NULL,str)
		#define get_3270_terminal_size(h,r,c) lib3270_get_screen_size(h,r,c)

		/* Keyboard */
		LIB3270_EXPORT int			  emulate_input(char *s, int len, int pasting);

		/* Network related calls */
		LIB3270_EXPORT int 			  Get3270Socket(void);

        /* Misc calls */
		LIB3270_EXPORT void 		  popup_an_error(const char *fmt, ...);
		LIB3270_EXPORT void 		  popup_system_error(const char *title, const char *message, const char *system);
		LIB3270_EXPORT void 		  popup_a_sockerr(char *fmt, ...);

		#define query_3270_terminal_status(void) lib3270_get_program_message(NULL)

		#define set_3270_model(h,m)	lib3270_set_model(h,m)
		#define get_3270_model(h) lib3270_get_model(h)

		LIB3270_EXPORT int			  lib3270_set_model(H3270 *session, int model);
		LIB3270_EXPORT int			  lib3270_get_model(H3270 *session);

		/* Get connection info */
		#define get_connected_lu(h) lib3270_get_luname(h)
		#define get_current_host(h) lib3270_get_host(h)

		LIB3270_EXPORT SCRIPT_STATE status_script(SCRIPT_STATE state);

		#define Toggled(ix) lib3270_get_toggle(NULL,ix)
		#define CallAndWait(c,h,p) lib3270_call_thread(c,h,p)

		LIB3270_EXPORT void RunPendingEvents(int wait);
		LIB3270_EXPORT int Wait(int seconds);

        LIB3270_EXPORT int ctlr_get_cols(void) __attribute__ ((deprecated));
        LIB3270_EXPORT int ctlr_get_rows(void) __attribute__ ((deprecated));

        /* Screen calls */
		LIB3270_EXPORT void screen_resume(H3270 *session);
		LIB3270_EXPORT void screen_suspend(H3270 *session);
		LIB3270_EXPORT void screen_disp(H3270 *session);

		/* Console calls */
		LIB3270_EXPORT HCONSOLE	  console_window_new(const char *title, const char *label);
		LIB3270_EXPORT void	  console_window_delete(HCONSOLE hwnd);
		LIB3270_EXPORT int		  console_window_append(HCONSOLE hwnd, const char *fmt, ...);
		LIB3270_EXPORT char	* console_window_wait_for_user_entry(HCONSOLE hwnd);

        /* Cursor calls */
		#define cursor_get_addr(void)	lib3270_get_cursor_address(NULL)
		#define cursor_set_addr(x)		lib3270_set_cursor_address(NULL,x)
		#define cursor_move(x)			lib3270_set_cursor_address(NULL,x)

		#include <lib3270/actions.h>

		#define host_connect(n,wait) lib3270_connect(NULL,n,wait)
		#define host_reconnect(w) lib3270_reconnect(NULL,w)

		#define register_schange(tx,func) lib3270_register_schange(NULL,tx,func,NULL)
		LIB3270_EXPORT void lib3270_register_schange(H3270 *h,LIB3270_STATE tx, void (*func)(H3270 *, int, void *),void *user_data);

		/* Console/Trace window */
		LIB3270_EXPORT HCONSOLE	  console_window_new(const char *title, const char *label);
		LIB3270_EXPORT void	  console_window_delete(HCONSOLE hwnd);
		LIB3270_EXPORT int		  console_window_append(HCONSOLE hwnd, const char *fmt, ...);
		LIB3270_EXPORT char	* console_window_wait_for_user_entry(HCONSOLE hwnd);

#ifdef __cplusplus
	}
#endif

#endif // LIB3270_API_INCLUDED

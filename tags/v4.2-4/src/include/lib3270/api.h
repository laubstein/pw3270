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


#ifndef LIB3270_H_INCLUDED

#ifdef __cplusplus
	extern "C" {
#endif

		#define LIB3270_H_INCLUDED "4.2"

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


		typedef struct _h3270
		{
			unsigned short 	  sz;				/**< Struct size */

			// Connection info
			int					  secure_connection;
			int      			  sock;				/**< Network socket */

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
			char           	* termtype;

			char				* current_host;
			unsigned short	  current_port;

			// screen info
			int					  ov_rows;
			int					  ov_cols;
			int					  first_changed;
			int					  last_changed;


		} H3270;

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

		/** connection state */
		enum cstate
		{
			NOT_CONNECTED,			/**< no socket, unknown mode */
			RESOLVING,				/**< resolving hostname */
			PENDING,				/**< connection pending */
			CONNECTED_INITIAL,		/**< connected, no mode yet */
			CONNECTED_ANSI,			/**< connected in NVT ANSI mode */
			CONNECTED_3270,			/**< connected in old-style 3270 mode */
			CONNECTED_INITIAL_E,	/**< connected in TN3270E mode, unnegotiated */
			CONNECTED_NVT,			/**< connected in TN3270E mode, NVT mode */
			CONNECTED_SSCP,			/**< connected in TN3270E mode, SSCP-LU mode */
			CONNECTED_TN3270E		/**< connected in TN3270E mode, 3270 mode */
		};


		/**  extended attributes */
		struct ea
		{
			unsigned char cc;	/**< EBCDIC or ASCII character code */
			unsigned char fa;	/**< field attribute, it nonzero */
			unsigned char fg;	/**< foreground color (0x00 or 0xf<n>) */
			unsigned char bg;	/**< background color (0x00 or 0xf<n>) */
			unsigned char gr;	/**< ANSI graphics rendition bits */
			unsigned char cs;	/**< character set (GE flag, or 0..2) */
			unsigned char ic;	/**< input control (DBCS) */
			unsigned char db;	/**< DBCS state */
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

		#ifndef LIB3270

			LIB3270_EXPORT enum cstate 	QueryCstate(void);
			LIB3270_EXPORT enum ft_state	QueryFTstate(void);

			#define PCONNECTED	((int) QueryCstate() >= (int)RESOLVING)
			#define HALF_CONNECTED	(QueryCstate() == RESOLVING || QueryCstate() == PENDING)
			#define CONNECTED	((int) QueryCstate() >= (int)CONNECTED_INITIAL)
			#define IN_NEITHER	(QueryCstate() == CONNECTED_INITIAL)
			#define IN_ANSI		(QueryCstate() == CONNECTED_ANSI || QueryCstate() == CONNECTED_NVT)
			#define IN_3270		(QueryCstate() == CONNECTED_3270 || QueryCstate() == CONNECTED_TN3270E || QueryCstate() == CONNECTED_SSCP)
			#define IN_SSCP		(QueryCstate() == CONNECTED_SSCP)
			#define IN_TN3270E	(QueryCstate() == CONNECTED_TN3270E)
			#define IN_E		(QueryCstate() >= CONNECTED_INITIAL_E)

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

			int 			(*CallAndWait)(int(*callback)(void *), void *parm);

			int				(*Wait)(int seconds);
			int 			(*RunPendingEvents)(int wait);

		};

		LIB3270_EXPORT int Register3270IOCallbacks(const struct lib3270_io_callbacks *cbk);

		/* Screen processing */
		typedef enum _CURSOR_MODE
		{
			CURSOR_MODE_NORMAL,
			CURSOR_MODE_WAITING,
			CURSOR_MODE_LOCKED,

			CURSOR_MODE_USER
		} CURSOR_MODE;

		typedef enum _STATUS_CODE
		{
			STATUS_CODE_BLANK,
			STATUS_CODE_SYSWAIT,
			STATUS_CODE_TWAIT,
			STATUS_CODE_CONNECTED,
			STATUS_CODE_DISCONNECTED,
			STATUS_CODE_AWAITING_FIRST,
			STATUS_CODE_MINUS,
			STATUS_CODE_PROTECTED,
			STATUS_CODE_NUMERIC,
			STATUS_CODE_OVERFLOW,
			STATUS_CODE_INHIBIT,
			STATUS_CODE_KYBDLOCK,

			STATUS_CODE_X,
			STATUS_CODE_RESOLVING,
			STATUS_CODE_CONNECTING,

			STATUS_CODE_USER

		} STATUS_CODE;

		typedef enum _SCRIPT_STATE
		{
			SCRIPT_STATE_NONE,
			SCRIPT_STATE_RUNNING,
			SCRIPT_STATE_HALTED,

			SCRIPT_STATE_USER

		} SCRIPT_STATE;

		typedef enum _OIA_FLAG
		{
			OIA_FLAG_BOXSOLID,
			OIA_FLAG_UNDERA,
			OIA_FLAG_SECURE,
			OIA_FLAG_TYPEAHEAD,
			OIA_FLAG_PRINTER,
			OIA_FLAG_REVERSE,

			OIA_FLAG_USER
		} OIA_FLAG;

		typedef enum _COUNTER_ID
		{
			COUNTER_ID_CTLR_DONE,
			COUNTER_ID_RESET,

			COUNTER_ID_USER
		} COUNTER_ID;

		LIB3270_EXPORT int query_counter(COUNTER_ID id);

		#define	query_screen_change_counter() query_counter(COUNTER_ID_CTLR_DONE)


		#define COLOR_ATTR_NONE			0x0000
		#define COLOR_ATTR_FIELD		0x0100
		#define COLOR_ATTR_BLINK		0x0200
		#define COLOR_ATTR_UNDERLINE	0x0400
		#define COLOR_ATTR_INTENSIFY	0x0800

		#define CHAR_ATTR_CG			0x1000
		#define CHAR_ATTR_MARKER		0x2000

		#define CHAR_ATTR_UNCONVERTED	CHAR_ATTR_CG

		struct lib3270_screen_callbacks
		{
			unsigned short	sz;

			int		(*init)(void);
			int		(*popup_dialog)(H3270 *session, PW3270_DIALOG type, const char *title, const char *msg, const char *fmt, va_list arg);
			void	(*Error)(const char *fmt, va_list arg);
			void	(*Warning)(const char *fmt, va_list arg);
			void	(*SysError)(const char *title, const char *message, const char *system);
			void 	(*model_changed)(H3270 *session, const char *name, int model, int cols, int rows);
			int		(*addch)(int row, int col, int c, unsigned short attr);
			void	(*charset)(char *dcs);
			void	(*title)(char *text);
			void	(*ring_bell)(void);
			void	(*redraw)(void);
			void	(*move_cursor)(int row, int col);
			int		(*set_suspended)(int state);
			void	(*set_script)(SCRIPT_STATE state);
			void	(*reset)(int lock);
			void	(*status)(STATUS_CODE id);
			void	(*cursor)(CURSOR_MODE mode);
			void	(*lu)(const char *lu);
			void	(*set)(OIA_FLAG id, int on);

			void	(*erase)(void);
			void	(*display)(H3270 *session);
			void 	(*set_viewsize)(H3270 *session, int rows, int cols);

			void	(*toggle_changed)(int ix, int value, int reason, const char *name);
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

		LIB3270_EXPORT H3270 * new_3270_session(const char *model);

		LIB3270_EXPORT const struct lib3270_option * get_3270_option_table(int sz);

		/* Popups */
		LIB3270_EXPORT void Error(const char *fmt, ...);
		LIB3270_EXPORT void Warning(const char *fmt, ...);
		LIB3270_EXPORT void show_3270_popup_dialog(H3270 *session, PW3270_DIALOG type, const char *title, const char *msg, const char *fmt, ...);

		/* Set/Get screen contents */
		LIB3270_EXPORT int find_field_attribute(int baddr);
		LIB3270_EXPORT int find_field_length(int baddr);
		LIB3270_EXPORT unsigned char get_field_attribute(int baddr);
		LIB3270_EXPORT int screen_read(char *dest, int baddr, int count);
		LIB3270_EXPORT void Input_String(const unsigned char *str);
		LIB3270_EXPORT void screen_size(int *rows, int *cols);
		LIB3270_EXPORT int query_secure_connection(H3270 *h);
		LIB3270_EXPORT int lib3270_paste_string(const unsigned char *str);
		LIB3270_EXPORT void get_3270_terminal_size(H3270 *h, int *rows, int *cols);

		/* Keyboard */
		LIB3270_EXPORT int			  emulate_input(char *s, int len, int pasting);

		/* Network related calls */
		LIB3270_EXPORT int 			  Get3270Socket(void);

        /* Misc calls */
		LIB3270_EXPORT void 		  popup_an_error(const char *fmt, ...);
		LIB3270_EXPORT void 		  popup_system_error(const char *title, const char *message, const char *system);
		LIB3270_EXPORT void 		  popup_a_sockerr(char *fmt, ...);
		LIB3270_EXPORT STATUS_CODE	  query_3270_terminal_status(void);
		LIB3270_EXPORT int			  set_3270_model(H3270 *session, int model);
		LIB3270_EXPORT int			  get_3270_model(H3270 *session);

		/* Get connection info */
		LIB3270_EXPORT const char	* get_connected_lu(H3270 *h);
		LIB3270_EXPORT const char	* get_current_host(H3270 *h);

		LIB3270_EXPORT int Toggled(int ix);

		LIB3270_EXPORT int CallAndWait(int(*callback)(void *), void *parm);
		LIB3270_EXPORT void RunPendingEvents(int wait);
		LIB3270_EXPORT int Wait(int seconds);

		LIB3270_EXPORT void ctlr_erase(int alt);
        LIB3270_EXPORT int ctlr_get_cols(void);
        LIB3270_EXPORT int ctlr_get_rows(void);

        /* Screen calls */
		LIB3270_EXPORT void screen_resume(void);
		LIB3270_EXPORT void screen_suspend(void);
		LIB3270_EXPORT void screen_disp(H3270 *session);

		/* Console calls */
		LIB3270_EXPORT HCONSOLE	  console_window_new(const char *title, const char *label);
		LIB3270_EXPORT void	  console_window_delete(HCONSOLE hwnd);
		LIB3270_EXPORT int		  console_window_append(HCONSOLE hwnd, const char *fmt, ...);
		LIB3270_EXPORT char	* console_window_wait_for_user_entry(HCONSOLE hwnd);

        /* Cursor calls */
		LIB3270_EXPORT int cursor_get_addr(void);
        LIB3270_EXPORT int cursor_set_addr(int baddr);
		#define cursor_move(x) cursor_set_addr(x)

		#include <lib3270/actions.h>

		/* Host connect/disconnect and state change. */
		typedef enum state_change
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
		} LIB3270_STATE_CHANGE;

		LIB3270_EXPORT int host_connect(const char *n, int wait);
		LIB3270_EXPORT int host_reconnect(int wait);
		LIB3270_EXPORT void host_disconnect(H3270 *h, int disable);
		LIB3270_EXPORT void register_schange(LIB3270_STATE_CHANGE tx, void (*func)(H3270 *, int));

		/* Console/Trace window */
		LIB3270_EXPORT HCONSOLE	  console_window_new(const char *title, const char *label);
		LIB3270_EXPORT void	  console_window_delete(HCONSOLE hwnd);
		LIB3270_EXPORT int		  console_window_append(HCONSOLE hwnd, const char *fmt, ...);
		LIB3270_EXPORT char	* console_window_wait_for_user_entry(HCONSOLE hwnd);

#ifdef __cplusplus
	}
#endif

#endif // LIB3270_H_INCLUDED
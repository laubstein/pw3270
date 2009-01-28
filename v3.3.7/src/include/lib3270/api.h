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


#ifndef LIB3270_H_INCLUDED

		#define LIB3270_H_INCLUDED "2.0"

		/* Debug & log */
		#if defined( DEBUG ) && defined( linux )
			#define Trace( fmt, ... )		fprintf(stderr, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fflush(stderr); \
											WriteLog("TRACE", "%s(%d) " fmt , __FILE__, __LINE__, __VA_ARGS__ )
		#elif defined( DEBUG )
			#define Trace( fmt, ... )		WriteLog("TRACE", "%s(%d) " fmt, __FILE__, __LINE__, __VA_ARGS__ )
		#else
			#define Trace( fmt, ... )	/* __VA_ARGS__ */
		#endif


		int Set3270Log(const char *filename);
		int WriteLog(const char *module, const char *fmt, ...);
		int WriteRCLog(const char *module, int rc, const char *fmt, ...);

		#ifdef G3270_MODULE_NAME
			#define Log(fmt, ...)		WriteLog(G3270_MODULE_NAME,fmt,__VA_ARGS__)
		#else
			#define Log(fmt, ...)		WriteLog("MSG",fmt,__VA_ARGS__)
		#endif

		/**   input key type */
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

		enum cstate QueryCstate(void);

		#ifndef LIB3270
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

		int BeginFileTransfer(unsigned short flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft);

		struct filetransfer_callbacks
		{
			unsigned short sz;

			void (*begin)(unsigned short flags, const char *local, const char *remote);
			void (*complete)(const char *errmsg);
			void (*update)(unsigned long length);
			void (*running)(int is_cut);
			void (*aborting)(void);

		};

		int RegisterFTCallbacks(struct filetransfer_callbacks *cbk);

		/* I/O processing */
		struct lib3270_io_callbacks
		{
			unsigned short sz;

			unsigned long (*AddTimeOut)(unsigned long interval_ms, void (*proc)(void));
			void 			(*RemoveTimeOut)(unsigned long timer);

			unsigned long (*AddInput)(int source, void (*fn)(void));
			void			(*RemoveInput)(unsigned long id);

			unsigned long (*AddExcept)(int source, void (*fn)(void));

			#if !defined(_WIN32) /*[*/
				unsigned long (*AddOutput)(int source, void (*fn)(void));
			#endif /*]*/

			int 			(*CallAndWait)(int(*callback)(void *), void *parm);


		};

		int Register3270IOCallbacks(const struct lib3270_io_callbacks *cbk);

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


			STATUS_CODE_X,
			STATUS_CODE_RESOLVING,
			STATUS_CODE_CONNECTING,
			STATUS_CODE_USER
		} STATUS_CODE;

		typedef enum _OIA_FLAG
		{
			OIA_FLAG_BOXSOLID,
			OIA_FLAG_UNDERA,
			OIA_FLAG_SECURE,
			OIA_FLAG_TYPEAHEAD,
			OIA_FLAG_PRINTER,
			OIA_FLAG_REVERSE,
			OIA_FLAG_SCRIPT,

			OIA_FLAG_USER
		} OIA_FLAG;

		typedef enum _COUNTER_ID
		{
			COUNTER_ID_SCREEN_CHANGED,
			COUNTER_ID_CTLR_DONE,

			COUNTER_ID_USER
		} COUNTER_ID;
		int query_counter(COUNTER_ID id);

		#define	query_screen_change_counter() query_counter(COUNTER_ID_SCREEN_CHANGED)

		enum g3270_action_groups
		{
			ACTION_GROUP_COMMON,
			ACTION_GROUP_ONLINE,
			ACTION_GROUP_OFFLINE,
			ACTION_GROUP_SELECTION,
			ACTION_GROUP_CLIPBOARD,
			ACTION_GROUP_PASTE,

			ACTION_GROUP_MAX
		};




		#define COLOR_ATTR_NONE			0x0000
		#define COLOR_ATTR_FIELD		0x0100
		#define COLOR_ATTR_BLINK		0x0200
		#define COLOR_ATTR_UNDERLINE	0x0400
		#define COLOR_ATTR_INTENSIFY	0x0800

		#define CHAR_ATTR_UNCONVERTED	0x1000

		struct lib3270_screen_callbacks
		{
			unsigned short sz;

			int		(*init)(void);
			void	(*Error)(const char *s);
			void	(*Warning)(const char *s);
			void	(*setsize)(int rows, int cols);
			int		(*addch)(int row, int col, int c, unsigned short attr);
			void	(*charset)(char *dcs);
			void	(*title)(char *text);
			void	(*changed)(int bstart, int bend);
			void	(*ring_bell)(void);
			void	(*redraw)(void);
			void	(*move_cursor)(int row, int col);
			void	(*suspend)(void);
			void	(*resume)(void);
			void	(*reset)(int lock);
			void	(*status)(STATUS_CODE id);
			void	(*compose)(int on, unsigned char c, int keytype);
			void	(*cursor)(CURSOR_MODE mode);
			void	(*lu)(const char *lu);
			void	(*set)(OIA_FLAG id, int on);
			void	(*erase)(void);
			void	(*popup_an_error)(const char *msg);
			void	(*toggle_changed)(int ix, int value, int reason, const char *name);
			void	(*show_timer)(long seconds);

		};

		int Register3270ScreenCallbacks(const struct lib3270_screen_callbacks *cbk);
		const char *lib3270_init(int *argc, const char **argv);

		/* Set/Get screen contents */
		int find_field_attribute(int baddr);
		int find_field_length(int baddr);
		unsigned char get_field_attribute(int baddr);
		int 	screen_read(char *dest, int baddr, int count);
		void	Input_String(const unsigned char *str);
		void 	screen_size(int *rows, int *cols);
		int		query_secure_connection(void);

        /* Misc calls */
		int 	Get3270Socket(void);
		void 	popup_an_error(const char *fmt, ...);

		int		Toggled(int ix);

		int 	CallAndWait(int(*callback)(void *), void *parm);

		void    ctlr_erase(int alt);
		void    ctlr_set_rows_cols(int mn, int ovc, int ovr);
        int     ctlr_get_cols(void);
        int     ctlr_get_rows(void);

        /* Screen calls */
		void    screen_resume(void);
		void    screen_suspend(void);
		void    screen_disp(void);

        /* Cursor calls */
		int     cursor_get_addr(void);
        int     cursor_set_addr(int baddr);
		#define cursor_move(x) cursor_set_addr(x)


#endif // LIB3270_H_INCLUDED

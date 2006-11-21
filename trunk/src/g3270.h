
 #include <stdio.h>

 #include <gtk/gtk.h>


 #include "lib/globals.h"
 #include "lib/appres.h"
 #include "lib/lib3270.h"
 #include "log.h"

 #define UpdateWindowTitle()		SetWindowTitle(0)

 #define RedrawStatusLine() 		RedrawTerminalContents()

/*---[ Defines ]--------------------------------------------------------------*/

 #if GTK == 2
    #define USE_CLIPBOARD
 #else
    #define USE_SELECTION
 #endif

 #if GTK == 2
    #define USE_GTKIMCONTEXT
 #endif

 #ifndef TMPPATH
    TMPPATH="/tmp"
 #endif

 #ifndef LOGPATH
    LOGPATH="/var/log"
 #endif

 enum status_codes
 {
        STATUS_DISCONNECTED,           /* X Not Connected */
        STATUS_RESOLVING,              /* X Resolving */
        STATUS_CONNECTING,             /* X Connecting */
        STATUS_NONSPECIFIC,            /* X */
        STATUS_INHIBIT,                /* X Inhibit */
        STATUS_BLANK,                  /* (blank) */
        STATUS_TWAIT,                  /* X Wait */
        STATUS_SYSWAIT,                /* X SYSTEM */
        STATUS_PROTECTED,              /* X Protected */
        STATUS_NUMERIC,                /* X Numeric */
        STATUS_OVERFLOW,               /* X Overflow */
        STATUS_DBCS,                   /* X DBCS */
        STATUS_SCROLLED,               /* X Scrolled */
        STATUS_MINUS,                  /* X -f */

	STATUS_RECONNECTING
 };

 enum cursor_types
 {
 	CURSOR_TYPE_OVER,
 	CURSOR_TYPE_INSERT,

 	CURSOR_TYPE_CROSSHAIR // Must be the last one!
 };

 #define FIELD_COLORS		4
 #define CURSOR_COLORS		(CURSOR_TYPE_CROSSHAIR * 2)
 #define SELECTION_COLORS	2

 enum _STATUS_COLORS
 {
    STATUS_COLOR_BACKGROUND,
    STATUS_COLOR_SEPARATOR,
	STATUS_COLOR_CURSOR_POSITION,
	STATUS_COLOR_LUNAME,
	STATUS_COLOR_ERROR,
	STATUS_COLOR_TIME,
	STATUS_COLOR_WARNING,
	STATUS_COLOR_NORMAL,
	STATUS_COLOR_TOOGLE,
	STATUS_COLOR_SSL,

	STATUS_COLORS	// Must be the last one
 };

 #define CURSOR_TYPE_NONE	-1

 #define min(x,y) (x < y ? x : y)
 #define max(x,y) (x > y ? x : y)

 #define SetStatusMessage(x) /* */

/*---[ Structures ]-----------------------------------------------------------*/

 typedef struct _fontelement
 {
	GdkFont *fn;
	int	  	Width;
	int	  	Height;
 } FONTELEMENT;

/*---[ Globals ]--------------------------------------------------------------*/

 extern const 	SCREEN_CALLBACK g3270_screen_callbacks;
 extern const 	KEYBOARD_INFO   g3270_keyboard_info;

 extern const 	unsigned char	ebcdic2asc[];


 extern char					oia_cursor[];
 extern char					oia_LUName[];

 #define OIA_TIMER_COUNT	7
 extern char					oia_Timer[OIA_TIMER_COUNT+1];

 extern const char				*cl_hostname;
 extern GtkWidget				*top_window;
 extern GtkWidget  				*terminal;

 extern char 					*Clipboard;
 extern int						szClipboard;

 extern gboolean				WaitForScreen;

 extern GdkColor				field_cmap[FIELD_COLORS];
 extern GdkColor				cursor_cmap[CURSOR_COLORS];
 extern GdkColor				status_cmap[STATUS_COLORS];
 extern GdkColor				selection_cmap[SELECTION_COLORS];

 extern GdkColor				*terminal_cmap;
 extern int						terminal_color_count;

 extern int						cursor_type;

 extern gboolean				reconnect;


/*---[ Prototipes ]-----------------------------------------------------------*/

 int  gsource_init(void);
 void gsource_addfile(const INPUT_3270 *ip);
 void gsource_removefile(const INPUT_3270 *ip);

 void SetWindowTitle(const char *msg);

 void SetCursorPosition(int row, int col);
 void SetCursorType(int type);
 void ToogleCursor(void);
 void EnableCursor(gboolean mode);
 void SetOIAStatus(int code);
 void InvalidateCursor(void);
 void status_untiming(void);

 int  PrintTemporaryFile(const char *filename);

 void DrawTerminal(GdkDrawable *, GdkGC *, const FONTELEMENT *, int, int, int);

 void InitClipboard(GtkWidget *w);
 int  AppendToClipboard(int fromRow, int fromCol, int toRow, int toCol);
 int  CopyToClipboard(int fromRow, int fromCol, int toRow, int toCol);

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

 void action_crosshair( GtkWidget *w, gpointer data );
 void action_select_all( GtkWidget *w, gpointer   data);
 void action_copy( GtkWidget *w, gpointer data);
 void action_append( GtkWidget *w, gpointer data);
 void action_remove_selection(GtkWidget *w, gpointer data);
 void action_disconnect(GtkWidget *w, gpointer data);
 void action_connect(GtkWidget *w, gpointer data);
 void action_exit(GtkWidget *w, gpointer data);
 void action_clear(GtkWidget *w, gpointer data);
 void action_paste(GtkWidget *w, gpointer data);
 void action_BackTab(GtkWidget *w, gpointer data);

 void action_F7(GtkWidget *w, gpointer data);
 void action_F8(GtkWidget *w, gpointer data);

 void action_print(GtkWidget *w, gpointer data);
 void action_print_copy(GtkWidget *w, gpointer data);
 void action_print_selection(GtkWidget *w, gpointer data);

 void ParseInput(const gchar *string);

 void RedrawTerminalContents(void);

 int LoadMenu(const char *filename, GtkItemFactory *factory);


#ifdef QUIET_MODE
 #define NotImplemented() Log("Function %s in %s needs implementation",__FUNCTION__,__FILE__)
#else
 #define NotImplemented() /* */
#endif

/*---[ Terminal window ]------------------------------------------------------*/

 GtkWidget *g3270_new(const char *hostname);



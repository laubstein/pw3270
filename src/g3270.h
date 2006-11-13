
 #include <stdio.h>

 #include <gtk/gtk.h>


 #include "lib/globals.h"
 #include "lib/appres.h"
 #include "lib/lib3270.h"
 #include "log.h"

 #define KL_OIA_SYSWAIT		0x8000
 #define KL_OIA_CONNECTING	0x8100

 #define RedrawTerminalContents() gtk_widget_queue_draw(terminal)

/*---[ Defines ]--------------------------------------------------------------*/

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


 extern char					oia_cursor[];
 extern char					oia_LUName[];

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

/*---[ Prototipes ]-----------------------------------------------------------*/

 int  gsource_init(void);
 void gsource_addfile(const INPUT_3270 *ip);
 void gsource_removefile(const INPUT_3270 *ip);

 void SetWindowTitle(const char *msg);

 void SetCursorPosition(int row, int col);
 void SetCursorType(int type);
 void ToogleCursor(void);
 void EnableCursor(gboolean mode);
 void SetStatusCode(int code);
 void InvalidateCursor(void);

 void DrawTerminal(GdkDrawable *, GdkGC *, const FONTELEMENT *, int, int, int);

 int  AppendToClipboard(int fromRow, int fromCol, int toRow, int toCol);
 int  CopyToClipboard(int fromRow, int fromCol, int toRow, int toCol);

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

 void action_crosshair( GtkWidget *w, gpointer data );
 void action_select_all( GtkWidget *w, gpointer   data);
 void action_copy( GtkWidget *w, gpointer data);
 void action_append( GtkWidget *w, gpointer data);
 void action_remove_selection(GtkWidget *w, gpointer data);
 void action_disconnect(GtkWidget *w, gpointer data);
 void action_exit(GtkWidget *w, gpointer data);

/*---[ Terminal window ]------------------------------------------------------*/

 GtkWidget *g3270_new(const char *hostname);



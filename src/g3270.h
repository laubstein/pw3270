
 #include <stdio.h>

 #include <gtk/gtk.h>


 #include "lib/globals.h"
 #include "lib/appres.h"
 #include "lib/lib3270.h"
 #include "log.h"

 #define KL_OIA_SYSWAIT	0x8000

/*---[ Defines ]--------------------------------------------------------------*/

 enum cursor_types
 {
 	CURSOR_TYPE_OVER,
 	CURSOR_TYPE_INSERT,

 	CURSOR_TYPE_CROSSHAIR // Must be the last one!
 };

 #define CURSOR_TYPE_NONE	-1

 #define min(x,y) (x < y ? x : y)
 #define max(x,y) (x > y ? x : y)

 #define SetStatusMessage(x) /* */

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

 int  AppendToClipboard(int fromRow, int fromCol, int toRow, int toCol);
 int  CopyToClipboard(int fromRow, int fromCol, int toRow, int toCol);

 void CopySelection(void);
 void AppendSelection(void);

 void toogle_crosshair(void);

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data);


/*---[ Terminal window ]------------------------------------------------------*/

 GtkWidget *g3270_new(const char *hostname);



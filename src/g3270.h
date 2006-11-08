
 #include <stdio.h>

 #include <gtk/gtk.h>


 #include "lib/globals.h"
 #include "lib/appres.h"
 #include "lib/lib3270.h"
 #include "log.h"

/*---[ Defines ]--------------------------------------------------------------*/

 enum cursor_types
 {
 	CURSOR_TYPE_OVER,
 	CURSOR_TYPE_INSERT,

 	CURSOR_TYPE_CROSSHAIR // Must be the last one!
 };

 #define CURSOR_TYPE_NONE	-1

/*---[ Globals ]--------------------------------------------------------------*/

 extern const 	SCREEN_CALLBACK g3270_screen_callbacks;
 extern const 	KEYBOARD_INFO   g3270_keyboard_info;


 extern const char				*cl_hostname;
 extern GtkWidget				*top_window;
 extern GtkWidget  				*terminal;

/*---[ Prototipes ]-----------------------------------------------------------*/

 int  gsource_init(void);
 void gsource_addfile(const INPUT_3270 *ip);
 void gsource_removefile(const INPUT_3270 *ip);

 void SetWindowTitle(const char *msg);

 void SetCursorPosition(int row, int col);
 void SetCursorType(int type);

/*---[ Terminal window ]------------------------------------------------------*/

 GtkWidget *g3270_new(const char *hostname);



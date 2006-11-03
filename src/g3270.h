
 #include <stdio.h>

 #include <gtk/gtk.h>
 #include <glib.h>


 #include "lib/globals.h"
 #include "lib/appres.h"
 #include "lib/lib3270.h"
 #include "log.h"

 extern const 	SCREEN_CALLBACK g3270_screen_callbacks;
 extern const 	KEYBOARD_INFO   g3270_keyboard_info;
 extern 		GSource 		*fd3270;

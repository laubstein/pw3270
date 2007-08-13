

 #include "g3270.h"

// TODO (perry#9#): Rewrite using GTK's printing system


/*---[ Constants ]------------------------------------------------------------*/

/*---[ Lock/Unlock ]----------------------------------------------------------*/

/*---[ Implement ]------------------------------------------------------------*/

#if defined(DEBUG) && defined(__GTK_PRINT_OPERATION_H__)

 void action_print(GtkWidget *w, gpointer data)
 {
 	action_exec_with_screen(w,data ? data : PRINT_COMMAND);
 }

 void action_print_copy(GtkWidget *w, gpointer data)
 {
    action_exec_with_copy(w,data ? data : PRINT_COMMAND);
 }

 void action_print_selection(GtkWidget *w, gpointer data)
 {
    action_exec_with_selection(w,data ? data : PRINT_COMMAND);
 }

#else

 void action_print(GtkWidget *w, gpointer data)
 {
 	action_exec_with_screen(w,data ? data : PRINT_COMMAND);
 }

 void action_print_copy(GtkWidget *w, gpointer data)
 {
    action_exec_with_copy(w,data ? data : PRINT_COMMAND);
 }

 void action_print_selection(GtkWidget *w, gpointer data)
 {
    action_exec_with_selection(w,data ? data : PRINT_COMMAND);
 }


#endif

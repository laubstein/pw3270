
 #include <stdio.h>

 #include <gtk/gtk.h>

 #ifdef GI18N
   #include <glib/gi18n.h>
 #else
   #include <libintl.h>
   #include <locale.h>
   #define _(String) gettext(String)
 #endif

 #ifndef SYSCONFIG
   #define SYSCONFIG "/etc/sysconfig/g3270"
 #endif   

 #include "lib/globals.h"
 #include "lib/appres.h"
 #include "lib/lib3270.h"
 #include "log.h"

 #define RedrawStatusLine() 		RedrawTerminalContents()
 
// #define PRINT_COMMAND "kprinter --nodialog -t " TARGET " %s"
 #define PRINT_COMMAND "lpr %s"

/*---[ Defines ]--------------------------------------------------------------*/

 #if GTK == 2
    #define USE_CLIPBOARD
 #else
    #define USE_SELECTION
 #endif

 #ifdef __GTK_IM_CONTEXT_H__
    #define USE_GTKIMCONTEXT
 #endif

 #ifndef TMPPATH
    TMPPATH="/tmp"
 #endif

 #ifndef LOGPATH
    LOGPATH="/var/log"
 #endif

 #include "extension.h"
 
 #define TERMINAL_COLORS	16
 #define FIELD_COLORS		4
 #define CURSOR_COLORS		(CURSOR_TYPE_CROSSHAIR * 2)
 #define SELECTION_COLORS	2

 #define min(x,y) (x < y ? x : y)
 #define max(x,y) (x > y ? x : y)

 #define SetStatusMessage(x) /* */

 #define IS_FUNCTION_KEY(event)   (event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(event->state & (GDK_MOD1_MASK|GDK_CONTROL_MASK)))

/*---[ Structures ]-----------------------------------------------------------*/

 typedef struct _fontelement
 {
	GdkFont *fn;
	int	  	Width;
	int	  	Height;
 } FONTELEMENT;

 struct action_callback
 {
    const char				*name;
    GtkItemFactoryCallback	callback;
 };

 typedef struct _extension
 {
	 struct _extension	*up;
	 struct _extension	*down;
	 void					*handle;
 } EXTENSION;
 
/*---[ Globals ]--------------------------------------------------------------*/

#if defined(DATADIR) && GTK == 2
 extern GdkPixbuf	*icon;
#endif

 extern const 	SCREEN_CALLBACK g3270_screen_callbacks;
 extern const 	KEYBOARD_INFO   g3270_keyboard_info;

 extern FONTELEMENT				*font;
 extern int						top_margin;
 extern int 						left_margin;
 extern int						line_spacing;
 extern int						cursor_row;
 extern	int						cursor_col;

 extern const 	unsigned char	ebcdic2asc[];

 extern const struct action_callback action_callbacks[];
 extern const int	 action_callback_counter;

 extern char						oia_undera;
 extern char						oia_Typeahead[2];
 extern char						oia_cursor[];
 extern char						oia_LUName[];

 extern guint						oia_KeyboardState;

 #define OIA_TIMER_COUNT	7
 extern char						oia_Timer[OIA_TIMER_COUNT+1];

 extern const char				*cl_hostname;
 extern GtkWidget					*top_window;
 extern GtkWidget  				*terminal;

 extern char 						*Clipboard;
 extern int						szClipboard;

 extern gboolean					WaitForScreen;

 extern GdkColor					field_cmap[FIELD_COLORS];
 extern GdkColor					cursor_cmap[CURSOR_COLORS];
 extern GdkColor					status_cmap[STATUS_COLORS];
 extern GdkColor					selection_cmap[SELECTION_COLORS];
 extern GdkColor					terminal_cmap[TERMINAL_COLORS];

 extern int						cursor_type;

 extern gboolean					reconnect;


/*---[ Prototipes ]-----------------------------------------------------------*/

 int  gsource_init(void);
 void gsource_addfile(const INPUT_3270 *ip);
 void gsource_removefile(const INPUT_3270 *ip);

 void UpdateWindowTitle(void);

 void SetCursorPosition(int row, int col);
 void SetCursorType(int type);
 void ToogleCursor(void);
 void EnableCursor(gboolean mode);
 void SetOIAStatus(int code);
 void InvalidateCursor(void);
 void status_untiming(void);

 int  PrintTemporaryFile(const char *filename);

 void DrawTerminal(GdkDrawable *, GdkGC *, const FONTELEMENT *, int, int, int);
 int  PaintBuffer(GdkDrawable *, GdkGC *, const FONTELEMENT *, int, int, int);
 void PaintStatus(int, GdkDrawable *, GdkGC *, const FONTELEMENT *, int, int, int);

 void InitClipboard(GtkWidget *w);
 int  AddToClipboard(int fromRow, int fromCol, int toRow, int toCol, int clear, int table);

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data);

 void action_crosshair( GtkWidget *w, gpointer data );
 void action_disconnect(GtkWidget *w, gpointer data);
 void action_connect(GtkWidget *w, gpointer data);
 void action_exit(GtkWidget *w, gpointer data);
 void action_clear(GtkWidget *w, gpointer data);

 void action_select_all( GtkWidget *w, gpointer   data);
 void action_copy( GtkWidget *w, gpointer data);
 void action_append( GtkWidget *w, gpointer data);
 void action_remove_selection(GtkWidget *w, gpointer data);
 void action_paste(GtkWidget *w, gpointer data);
 void action_copy_as_table(GtkWidget *w, gpointer data);

 void action_exec_with_selection(GtkWidget *w, gpointer data);
 void action_exec_with_copy(GtkWidget *w, gpointer data);
 void action_exec_with_screen(GtkWidget *w, gpointer data);

 void action_BackTab(GtkWidget *w, gpointer data);

 void action_F7(GtkWidget *w, gpointer data);
 void action_F8(GtkWidget *w, gpointer data);
 void action_F19(GtkWidget *w, gpointer data);
 void action_F20(GtkWidget *w, gpointer data);
 void action_F23(GtkWidget *w, gpointer data);
 void action_F24(GtkWidget *w, gpointer data);

 void action_Tab(GtkWidget *w, gpointer data);

 void action_print(GtkWidget *w, gpointer data);
 void action_print_copy(GtkWidget *w, gpointer data);
 void action_print_selection(GtkWidget *w, gpointer data);

 void action_Down(GtkWidget *w, gpointer data);
 void action_Right(GtkWidget *w, gpointer data);
 void action_Up(GtkWidget *w, gpointer data);
 void action_Left(GtkWidget *w, gpointer data);
 void action_EraseEOF(GtkWidget *w, gpointer data);
 void action_Home(GtkWidget *w, gpointer data);

 void action_Clear(GtkWidget *w, gpointer data);
 void action_Reset(GtkWidget *w, gpointer data);
 void action_Delete(GtkWidget *w, gpointer data);
 void action_Erase(GtkWidget *w, gpointer data);
 void action_Enter(GtkWidget *w, gpointer data);
 void action_Insert(GtkWidget *w, gpointer data);
 void action_Newline(GtkWidget *w, gpointer data);

 void action_SelectUp(GtkWidget *w, gpointer data);
 void action_SelectDown(GtkWidget *w, gpointer data);
 void action_SelectLeft(GtkWidget *w, gpointer data);
 void action_SelectRight(GtkWidget *w, gpointer data);
 void action_RestoreSelection(GtkWidget *w, gpointer data);
 void action_SelectField(GtkWidget *w, gpointer data);

 void action_SelectionUp(GtkWidget *w, gpointer data);
 void action_SelectionDown(GtkWidget *w, gpointer data);
 void action_SelectionLeft(GtkWidget *w, gpointer data);
 void action_SelectionRight(GtkWidget *w, gpointer data);

 void action_PreviousWord(GtkWidget *w, gpointer data);
 void action_NextWord(GtkWidget *w, gpointer data);
 void action_DeleteWord(GtkWidget *w, gpointer data);
 void action_DeleteField(GtkWidget *w, gpointer data);

#ifdef __GTK_COLOR_BUTTON_H__ 
 void action_set_colors(GtkWidget *w, gpointer data);
#endif 

 void action_Redraw(GtkWidget *w, gpointer data);
 void action_AboutBox(GtkWidget *w, gpointer data);

 void ParseInput(const gchar *string);

 void RedrawTerminalContents(void);

 GtkWidget	*LoadMenu(GtkWidget *window);
 GtkWidget	*LoadToolbar(GtkWidget *window);
 void 		LoadImages(GdkDrawable *drawable, GdkGC *gc);

 gchar *CopyTerminalContents(int bRow, int bCol, int fRow, int fCol, int table);

#ifdef QUIET_MODE
 #define NotImplemented() Log("Function %s in %s needs implementation",__FUNCTION__,__FILE__)
#else
 #define NotImplemented() /* */
#endif

/*---[ Mouse ]----------------------------------------------------------------*/

 void		SetMousePointer(GdkCursor *cursor);
 void		LoadMousePointers(GdkDrawable *drawable, GdkGC *gc);
 void		DrawSelectionBox(GdkDrawable *drawable, GdkGC *gc);
 void		ConfigureSelectionBox(void);

 gboolean	Mouse2Terminal(long x, long y, long *cRow, long *cCol);

 gboolean	mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
 gboolean	mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
 gboolean	mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
 gboolean	mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

/*---[ Terminal window ]------------------------------------------------------*/

 GtkWidget	*g3270_new(const char *hostname);
 void		LoadTerminalColors(void);

/*---[ Extensions/Plugins ]---------------------------------------------------*/

 EXTENSION				*OpenExtension(const char *path);
 int					CloseExtension(EXTENSION *ext);
 int					LoadExtensions(const char *dir);
 void					UnloadExtensions(void);
 void 					CallExtension(EXTENSION *ext, const char *function, GtkWidget *widget);
 void      			SetExtensionsChar(const char *function, const char *parameter);
 GtkItemFactoryCallback	QueryActionCallback(const char *name);

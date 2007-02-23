
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

// #ifdef DEBUG
//   #define LockThreads()			DBGPrintf("Lock %p",g_thread_self()); gdk_lock()
//   #define UnlockThreads()			DBGPrintf("Unlock %p",g_thread_self()); gdk_unlock()
// #else
   #define LockThreads()			gdk_lock()
   #define UnlockThreads()			gdk_unlock()
// #endif

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

 enum status_codes
 {
        STATUS_DISCONNECTED,           /*  0 X Not Connected */
        STATUS_RESOLVING,              /*  1 X Resolving */
        STATUS_CONNECTING,             /*  2 X Connecting */
        STATUS_NONSPECIFIC,            /*  3 X */
        STATUS_INHIBIT,                /*  4 X Inhibit */
        STATUS_BLANK,                  /*  5 (blank) */
        STATUS_TWAIT,                  /*  6 X Wait */
        STATUS_SYSWAIT,                /*  7 X SYSTEM */
        STATUS_PROTECTED,              /*  8 X Protected */
        STATUS_NUMERIC,                /*  9 X Numeric */
        STATUS_OVERFLOW,               /* 10 X Overflow */
        STATUS_DBCS,                   /* 11 X DBCS */
        STATUS_SCROLLED,               /* 12 X Scrolled */
        STATUS_MINUS,                  /* 13 X -f */
        STATUS_AWAITING_FIRST,	       /* 14 X Wait */
	    STATUS_CONNECTED,			   /* 15 Connected */

		STATUS_RECONNECTING
 };

 enum cursor_types
 {
 	CURSOR_TYPE_OVER,
 	CURSOR_TYPE_INSERT,

 	CURSOR_TYPE_CROSSHAIR // Must be the last one!
 };

 #define TERMINAL_COLORS	16
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
	STATUS_COLOR_CONNECTED,
	STATUS_COLOR_KEYBOARD,
	STATUS_COLOR_CONNECT_ICON,

	STATUS_COLORS	// Must be the last one
 };

 #define STATUS_COLOR_CNCT 		STATUS_COLOR_CONNECT_ICON
 #define STATUS_COLOR_TYPEAHEAD STATUS_COLOR_TOOGLE

 #define CURSOR_TYPE_NONE	-1

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

 #ifndef GDK_NUMLOCK_MASK
    #define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

 #ifndef GDK_ALT_MASK
     #define GDK_ALT_MASK GDK_MOD1_MASK
 #endif

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


 void action_Redraw(GtkWidget *w, gpointer data);
 void action_AboutBox(GtkWidget *w, gpointer data);

 void ParseInput(const gchar *string);

 void RedrawTerminalContents(void);

 GtkWidget	*LoadMenu(GtkWidget *window);
 GtkWidget	*LoadToolbar(GtkWidget *window);
 void 		LoadImages(GdkDrawable *drawable, GdkGC *gc);

 void gdk_lock(void);
 void gdk_unlock(void);

 gchar *CopyTerminalContents(int bRow, int bCol, int fRow, int fCol, int table);

#ifdef QUIET_MODE
 #define NotImplemented() Log("Function %s in %s needs implementation",__FUNCTION__,__FILE__)
#else
 #define NotImplemented() /* */
#endif

/*---[ Mouse ]----------------------------------------------------------------*/

 void	  SetMousePointer(GdkCursor *cursor);
 void 	  LoadMousePointers(GdkDrawable *drawable, GdkGC *gc);
 void	  DrawSelectionBox(GdkDrawable *drawable, GdkGC *gc);
 void	  ConfigureSelectionBox(void);


 gboolean Mouse2Terminal(long x, long y, long *cRow, long *cCol);

 gboolean mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
 gboolean mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
 gboolean mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
 gboolean mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

/*---[ Terminal window ]------------------------------------------------------*/

 GtkWidget *g3270_new(const char *hostname);

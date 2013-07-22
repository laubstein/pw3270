/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
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
 * Este programa está nomeado como gui.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#ifndef GUI_H_INCLUDED

	#define GUI_H_INCLUDED

	#ifdef WIN32
		#include <windows.h>
	#endif

	#include <lib3270/config.h>
	#define ENABLE_NLS
	#define GETTEXT_PACKAGE PACKAGE_NAME

	#include <libintl.h>
	#include <glib/gi18n.h>

	#include <gtk/gtk.h>
	#include <gdk/gdk.h>
	#include <glib.h>

	#if defined( HAVE_IGEMAC )
		#include <gtkosxapplication.h>
		#define HAVE_DOCK 1
	#endif

	#include <lib3270.h>
	#include <lib3270/toggle.h>

 	#define CURSOR_MODE_3270 (LIB3270_CURSOR_USER+9)

	enum _drag_type
	{
		DRAG_TYPE_TOP_LEFT, 	// Top-left
		DRAG_TYPE_TOP_RIGHT,	// Top-right
		DRAG_TYPE_TOP,			// Topcairo +parse +color
		DRAG_TYPE_BOTTOM_LEFT,	// Bottom-left
		DRAG_TYPE_BOTTOM_RIGHT,	// Bottom-right
		DRAG_TYPE_BOTTOM,		// Bottom
		DRAG_TYPE_LEFT,			// Left
		DRAG_TYPE_RIGHT,		// Right
		DRAG_TYPE_INSIDE,		// Inside

		DRAG_TYPE_NONE
	};

	LOCAL_EXTERN int 			drag_type;
	LOCAL_EXTERN LIB3270_CURSOR	cursor_mode;

	#define MAX_CHR_LENGTH 4
	typedef struct _element
	{
		gchar				ch[MAX_CHR_LENGTH];	/**< UTF-8 string */

		gboolean			changed;			/**< Element needs update? */

		unsigned short	fg;						/**< Foreground color */
		unsigned short	bg;						/**< Background color */

		unsigned short	cg;						/**< CG character */

		#define SELECTION_BOX_LEFT			0x10
		#define SELECTION_BOX_RIGHT			0x20
		#define SELECTION_BOX_TOP			0x40
		#define SELECTION_BOX_BOTTOM		0x80

		#define ELEMENT_STATUS_SELECTED		0x08

		#define ELEMENT_STATUS_NORMAL 		0x00
		#define ELEMENT_STATUS_FIELD_MARKER	0x01
		#define ELEMENT_STATUS_FIELD_MASK	0x07

		unsigned char		status;

	} ELEMENT;

	LOCAL_EXTERN struct _screen
	{
		int		length;			/**< Screen buffer length (row * cols) */
		int		rows;			/**< Screen rows */
		int		cols;			/**< Screen cols */
		ELEMENT content[1];		/**< Screen contents */
	} *screen;

	LOCAL_EXTERN struct _view
	{
		int	 rows;			/**< View rows */
		int	 cols;			/**< View cols */
		int	 left;			/**< Left margin */
		int	 top;			/**< Top margin */
	} view;

    enum GUI_TOGGLE
    {
        GUI_TOGGLE_BOLD,
        GUI_TOGGLE_KEEP_SELECTED,
        GUI_TOGGLE_UNDERLINE,
        GUI_TOGGLE_CONNECT_ON_STARTUP,
        GUI_TOGGLE_KP_ALTERNATIVE,		/**< Keypad +/- move to next/previous field */
        GUI_TOGGLE_BEEP,

        GUI_TOGGLE_COUNT
    };

    #define TOGGLED_BOLD			    gui_toggle_state[GUI_TOGGLE_BOLD]
    #define TOGGLED_KEEP_SELECTED	    gui_toggle_state[GUI_TOGGLE_KEEP_SELECTED]
    #define TOGGLED_SMART_PASTE		    gui_toggle_state[GUI_TOGGLE_SMART_PASTE]
    #define TOGGLED_UNDERLINE		    gui_toggle_state[GUI_TOGGLE_UNDERLINE]
    #define TOGGLED_CONNECT_ON_STARTUP  gui_toggle_state[GUI_TOGGLE_CONNECT_ON_STARTUP]
    #define TOGGLED_KP_ALTERNATIVE		gui_toggle_state[GUI_TOGGLE_KP_ALTERNATIVE]
    #define TOGGLED_BEEP				gui_toggle_state[GUI_TOGGLE_BEEP]

    LOCAL_EXTERN gboolean		  gui_toggle_state[GUI_TOGGLE_COUNT];
	LOCAL_EXTERN const gchar	* gui_toggle_name[GUI_TOGGLE_COUNT+1];

	enum TERMINAL_COLOR
	{
		TERMINAL_COLOR_BACKGROUND,
		TERMINAL_COLOR_BLUE,
		TERMINAL_COLOR_RED,
		TERMINAL_COLOR_PINK,
		TERMINAL_COLOR_GREEN,
		TERMINAL_COLOR_TURQUOISE,
		TERMINAL_COLOR_YELLOW,
		TERMINAL_COLOR_WHITE,
		TERMINAL_COLOR_BLACK,
		TERMINAL_COLOR_DARK_BLUE,
		TERMINAL_COLOR_ORANGE,
		TERMINAL_COLOR_PURPLE,
		TERMINAL_COLOR_DARK_GREEN,
		TERMINAL_COLOR_DARK_TURQUOISE,
		TERMINAL_COLOR_MUSTARD,
		TERMINAL_COLOR_GRAY,

		TERMINAL_COLOR_FIELD_DEFAULT,
		TERMINAL_COLOR_FIELD_INTENSIFIED,
		TERMINAL_COLOR_FIELD_PROTECTED,
		TERMINAL_COLOR_FIELD_PROTECTED_INTENSIFIED,

		TERMINAL_COLOR_SELECTED_BG,
		TERMINAL_COLOR_SELECTED_FG,
		TERMINAL_COLOR_SELECTED_BORDER,

		TERMINAL_COLOR_CURSOR_BACKGROUND,
		TERMINAL_COLOR_CROSS_HAIR,

		// Oia Colors (Must be the last block)
		TERMINAL_COLOR_OIA_BACKGROUND,
		TERMINAL_COLOR_OIA_FOREGROUND,
		TERMINAL_COLOR_OIA_SEPARATOR,
		TERMINAL_COLOR_OIA_STATUS_OK,
		TERMINAL_COLOR_OIA_STATUS_INVALID,

		TERMINAL_COLOR_COUNT
	};

	#define TERMINAL_COLOR_CURSOR_FOREGROUND	TERMINAL_COLOR_BACKGROUND
	#define TERMINAL_COLOR_FIELD				TERMINAL_COLOR_FIELD_DEFAULT
	#define TERMINAL_COLOR_DEFAULT_FG			TERMINAL_COLOR_WHITE

	#define TERMINAL_COLOR_OIA					TERMINAL_COLOR_OIA_BACKGROUND

	#define TERMINAL_COLOR_OIA_CURSOR 			TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_LUNAME			TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_STATUS_WARNING	TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_SCRIPT_STATE		TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_SSL_STATE		TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_SHIFT_STATE		TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_INSERT_STATE		TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_ALT_STATE		TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_TYPEAHEAD_STATE	TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_CAPS_STATE		TERMINAL_COLOR_OIA_FOREGROUND
	#define TERMINAL_COLOR_OIA_UNDERA			TERMINAL_COLOR_OIA_FOREGROUND

	#define TERMINAL_COLOR_OIA_TIMER			TERMINAL_COLOR_OIA_STATUS_OK
	#define TERMINAL_COLOR_OIA_SPINNER			TERMINAL_COLOR_OIA_STATUS_OK

	#define CURSOR_MODE_SHOW	0x80
	#define CURSOR_MODE_ENABLED	0x40
	#define CURSOR_MODE_CROSS 	0x02
	#define CURSOR_MODE_BASE	0x04

	#define CHARSET 				charset ? charset : "ISO-8859-1"
	#define IS_FUNCTION_KEY(event)	(event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(event->state & (GDK_MOD1_MASK|GDK_CONTROL_MASK)))

	#ifndef GDK_ALT_MASK
		#define GDK_ALT_MASK GDK_MOD1_MASK
	#endif

	#define PopupAWarning(fmt, ...)	Warning(fmt,__VA_ARGS__)
	#define PopupAnError( x )		Error("%s",x);
	#define WarningPopup(fmt, ...)	Warning(fmt,__VA_ARGS__);

#if defined( HAVE_IGEMAC )
	LOCAL_EXTERN GtkOSXApplication	* osxapp;
#endif

 	LOCAL_EXTERN const gchar		* program_name;
 	LOCAL_EXTERN const gchar		* program_version;
 	LOCAL_EXTERN const gchar		* program_release;
 	LOCAL_EXTERN const gchar		* program_fullversion;

	LOCAL_EXTERN const char		* on_lu_command;

	LOCAL_EXTERN GtkWidget			* topwindow;
	LOCAL_EXTERN GtkWidget			* terminal;
	LOCAL_EXTERN GdkColor			  color[TERMINAL_COLOR_COUNT+1];
	LOCAL_EXTERN H3270				* hSession;
	LOCAL_EXTERN GtkIMContext		* input_method;

	// Pixmaps
	LOCAL_EXTERN GdkPixmap			* pixmap_cursor;	/**< Pixmap with cursor image */
	LOCAL_EXTERN GdkPixmap			* pixmap_terminal;	/**< Pixmap with terminal contents */

	enum POPUPS
	{
		POPUP_MENU_DEFAULT,
		POPUP_MENU_SELECTION,

		POPUP_MENU_COUNT
	};

	LOCAL_EXTERN GtkMenu			* popup_menu[POPUP_MENU_COUNT];

	enum OIA_PIXMAP
	{
		OIA_PIXMAP_LOCKED,		// 0 = Locked
		OIA_PIXMAP_UNLOCKED,	// 1 = Unlocked
		OIA_PIXMAP_WARNING,		// 2 = Warning

		OIA_PIXMAP_COUNT
	};

	#define get_terminal_pixmap()		pixmap_terminal
	#define get_cursor_pixmap()			pixmap_cursor
	#define cairo_set_3270_color(cr,x)	gdk_cairo_set_source_color(cr,color+x)
	#define valid_terminal_window()		(pixmap_terminal != NULL)

#ifdef MOUSE_POINTER_CHANGE
	LOCAL_EXTERN GdkCursor        		* wCursor[CURSOR_MODE_3270];
#endif

	LOCAL_EXTERN gint					  cMode;

	LOCAL_EXTERN gint					  cursor_position;
	LOCAL_EXTERN GdkRectangle			  rCursor;
	LOCAL_EXTERN gboolean			      cursor_blink;

	LOCAL_EXTERN gboolean 				  WaitingForChanges;
	LOCAL_EXTERN char					* charset;
	LOCAL_EXTERN gchar 					* window_title;
	LOCAL_EXTERN gboolean				  screen_updates_enabled;

	// Paths
	LOCAL_EXTERN gchar					* program_data;
	LOCAL_EXTERN gchar					* program_logo;
	LOCAL_EXTERN gchar					* program_config_file;
	LOCAL_EXTERN gchar					* program_config_filename_and_path;

#ifdef HAVE_PLUGINS
	LOCAL_EXTERN gchar					* plugin_path;
	LOCAL_EXTERN gchar					* plugin_list;
#endif

	typedef enum _action_group
	{
		ACTION_GROUP_DEFAULT,
		ACTION_GROUP_ONLINE,
		ACTION_GROUP_OFFLINE,
		ACTION_GROUP_SELECTION,
		ACTION_GROUP_CLIPBOARD,
		ACTION_GROUP_PASTE,
		ACTION_GROUP_FT,

		ACTION_GROUP_MAX

	} ACTION_GROUP_ID;

	typedef enum _action_id
	{
		ACTION_COPY_AS_TABLE,
		ACTION_COPY_AS_IMAGE,
		ACTION_PASTENEXT,
		ACTION_UNSELECT,
		ACTION_RESELECT,

		ACTION_ID_MAX

	} 	ACTION_ID;

	extern const struct lib3270_io_callbacks program_io_callbacks;
	extern const struct lib3270_screen_callbacks program_screen_callbacks;

	int			CreateTopWindow(void);

	GtkWidget 	*CreateTerminalWindow(void);

	gboolean 	StartPlugins(const gchar *startup_script);
	gboolean 	StopPlugins(void);

	void 		PrintConsole(const gchar *fmt, ...);

	void 		ParseInput(const gchar *string);
	void		LoadImages(GdkDrawable *drawable, GdkGC *gc);
	void 		InvalidatePixmaps(GdkDrawable *drawable, GdkGC *gc);
	void 		ReloadPixmaps(void);
	void 		Reselect(void);
	void 		set_rectangle_select(H3270 *session, int value, LIB3270_TOGGLE_TYPE reason);
	void 		SetStatusCode(H3270 *session, LIB3270_STATUS id);
	void 		SetTerminalFont(const gchar *fontname);

	void		init_gui_toggles(void);
	GtkWidget * widget_from_action_name(const gchar *name);

	LOCAL_EXTERN gboolean	  check_key_action(GtkWidget *widget, GdkEventKey *event);

	LOCAL_EXTERN void		  unselect(void);
	LOCAL_EXTERN void		  reselect(void);

	LOCAL_EXTERN void		  update_cursor_position(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
	LOCAL_EXTERN void		  update_cursor_info(void);
	LOCAL_EXTERN void		  queue_draw_cursor(void);
	LOCAL_EXTERN void		  update_cursor_pixmap(void);

	LOCAL_EXTERN void		  gui_toogle_set_active(enum GUI_TOGGLE id, gboolean active);
	LOCAL_EXTERN void		  gui_toogle_set_visible(enum GUI_TOGGLE id, gboolean visible);

	LOCAL_EXTERN void		  load_screen_size_menu(GtkWidget *topmenu);


#ifdef X3270_FT
	int 		initft(void);
	int 		create_ft_progress_dialog(void);
#endif

	gboolean	PFKey(guint key);

	#define		GetSelection() GetScreenContents(0)
	LOCAL_EXTERN void	  check_clipboard_contents(void);
	LOCAL_EXTERN void	  clear_clipboard_string(void);

	gchar 				* GetClipboard(void);
	gchar 				* GetScreenContents(gboolean all);

//	gboolean 	mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
	gboolean 	mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
	gboolean 	mouse_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);
	gboolean 	mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	gboolean 	mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

	int 			LoadColors(void);
	GtkUIManager	*LoadApplicationUI(GtkWidget *widget);
	void			UpdateKeyboardState(guint state);
	int				PrintText(const char *name, gchar *text);
	void 			RestoreWindowSize(const gchar *group, GtkWidget *widget);
	void 			SaveWindowSize(const gchar *group, GtkWidget *widget);
	int 			GetFunctionKey(GdkEventKey *event);
	int 			wait4negotiations(const char *cl_hostname);
	GdkPixbuf 		*LoadLogo(void);

	void			FontChanged(void);

	int 			CloseConfigFile(void);
	int				OpenConfigFile(void);

	LOCAL_EXTERN	GCallback 		  get_action_callback_by_name(const gchar *name);

	gchar 			*GetString(const gchar *group, const gchar *key, const gchar *def);
	void 			SetString(const gchar *group, const gchar *key, const gchar *val);

	gboolean 		GetBoolean(const gchar *group, const gchar *key, const gboolean def);
	void 			SetBoolean(const gchar *group, const gchar *key, const gboolean val);

	gint 			GetInt(const gchar *group, const gchar *key, gint def);
	void			SetInt(const gchar *group, const gchar *key, gint val);

	void 			DisableNetworkActions(void);
	void			settitle(char *text);
	GKeyFile 		*GetConf(void);

	void 			RunExternalProgramWithText(const gchar *cmd, const gchar *str);

	LOCAL_EXTERN int		  get_selected_rectangle(GdkRectangle *rect);

	void	set_monocase(H3270 *session, int value, LIB3270_TOGGLE_TYPE reason);

	// Plugins
	LOCAL_EXTERN GModule	* get_plugin_by_name(const gchar *plugin_name);
	LOCAL_EXTERN gboolean 	  get_symbol_by_name(GModule *module, gpointer *pointer, const gchar *fmt, ...);

	LOCAL_EXTERN int 		  LoadPlugins(void);
	LOCAL_EXTERN int		  UnloadPlugins(void);
	LOCAL_EXTERN void		  CallPlugins(const gchar *name, const gchar *arg);

	// Screen Rendering
	enum text_layout
	{
		TEXT_LAYOUT_NORMAL,
		TEXT_LAYOUT_UNDERLINE
	};

	LOCAL_EXTERN void 		  update_terminal_contents(void);
	LOCAL_EXTERN void 		  draw_region(cairo_t *cr, int bstart, int bend, GdkColor *clr, GdkRectangle *r);

	LOCAL_EXTERN void 		  update_region(int bstart, int bend);
	LOCAL_EXTERN cairo_t	* get_terminal_cairo_context(void);
	LOCAL_EXTERN GdkGC		* get_terminal_cached_gc(void);

	#define DrawOIA(draw, clr); /* */

	// Toolbar & Keypad
	// LOCAL_EXTERN void 		  configure_toolbar(GtkWidget *toolbar, GtkWidget *menu, const gchar *label);
	LOCAL_EXTERN void		  keypad_set_sensitive(GtkWidget *window, gboolean state);

	// Console/Trace window
	LOCAL_EXTERN HCONSOLE	  gui_console_window_new(const char *title, const char *label);
	LOCAL_EXTERN void		  gui_console_window_delete(HCONSOLE hwnd);
	LOCAL_EXTERN int		  gui_console_window_append(HCONSOLE hwnd, const char *fmt, va_list args);
	LOCAL_EXTERN char 		* gui_console_window_wait_for_user_entry(HCONSOLE hwnd);

	// Clipboard
	LOCAL_EXTERN void 		  update_paste_action(GtkClipboard *clipboard, const gchar *text, gpointer data);

	// Command interpreter
	typedef gchar * (*PW3270_COMMAND_POINTER)(gint argc, const gchar **argv);
	#define PW3270_COMMAND_ENTRY(name) PW3270_MODULE_EXPORT gchar * pw3270_command_entry_ ## name (int argc, const gchar **argv )

	// Command interpreter & script support
	LOCAL_EXTERN PW3270_COMMAND_POINTER get_command_pointer(const gchar *cmd);
	LOCAL_EXTERN int					run_script_command_line(const gchar *script, GPid *pid);
	LOCAL_EXTERN int 		  			script_interpreter( const gchar *script_type, const gchar *script_name, const gchar *script_text, int argc, const gchar **argv, GPid *pid);
	LOCAL_EXTERN void 		  			run_script_list( const gchar *scripts );

#endif // GUI_H_INCLUDED

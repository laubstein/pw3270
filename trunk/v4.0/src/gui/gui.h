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
 * Este programa está nomeado como main.c e possui 346 linhas de código.
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

	#include <lib3270/api.h>
 	#define CURSOR_MODE_3270 (CURSOR_MODE_USER+9)

	enum _drag_type
	{
		DRAG_TYPE_TOP_LEFT, 	// Top-left
		DRAG_TYPE_TOP_RIGHT,	// Top-right
		DRAG_TYPE_TOP,			// Top
		DRAG_TYPE_BOTTOM_LEFT,	// Bottom-left
		DRAG_TYPE_BOTTOM_RIGHT,	// Bottom-right
		DRAG_TYPE_BOTTOM,		// Bottom
		DRAG_TYPE_LEFT,			// Left
		DRAG_TYPE_RIGHT,		// Right
		DRAG_TYPE_INSIDE,		// Inside

		DRAG_TYPE_NONE
	};

	extern int 			drag_type;
	extern CURSOR_MODE		cursor_mode;

	#define MAX_CHR_LENGTH 3
	typedef struct _element
	{
		gchar				ch[MAX_CHR_LENGTH];	/**< UTF-8 string */
		unsigned short	fg;					/**< Foreground color */
		unsigned short	bg;					/**< Background color */

		unsigned short	cg;					/**< CG character */

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
	extern ELEMENT *screen;

    enum GUI_TOGGLE
    {
        GUI_TOGGLE_BOLD,
        GUI_TOGGLE_KEEP_SELECTED,
        GUI_TOGGLE_UNDERLINE,

        GUI_TOGGLE_COUNT
    };

    #define TOGGLED_BOLD			gui_toggle[GUI_TOGGLE_BOLD]
    #define TOGGLED_KEEP_SELECTED	gui_toggle[GUI_TOGGLE_KEEP_SELECTED]
    #define TOGGLED_SMART_PASTE		gui_toggle[GUI_TOGGLE_SMART_PASTE]
    #define TOGGLED_UNDERLINE		gui_toggle[GUI_TOGGLE_UNDERLINE]

    extern gboolean gui_toggle[GUI_TOGGLE_COUNT];

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

		TERMINAL_COLOR_CURSOR,
		TERMINAL_COLOR_CROSS_HAIR,

		// Oia Colors
		TERMINAL_COLOR_OIA_BACKGROUND,
		TERMINAL_COLOR_OIA,
		TERMINAL_COLOR_OIA_SEPARATOR,
		TERMINAL_COLOR_OIA_STATUS_OK,
		TERMINAL_COLOR_OIA_STATUS_INVALID,

		TERMINAL_COLOR_COUNT
	};

	#define TERMINAL_COLOR_OIA_CURSOR 			TERMINAL_COLOR_OIA_STATUS_OK
	#define TERMINAL_COLOR_OIA_LU 				TERMINAL_COLOR_OIA_STATUS_OK
	#define TERMINAL_COLOR_OIA_TIMER			TERMINAL_COLOR_OIA_STATUS_OK
	#define TERMINAL_COLOR_SSL 					TERMINAL_COLOR_OIA
	#define TERMINAL_COLOR_OIA_STATUS_WARNING	TERMINAL_COLOR_OIA
	#define TERMINAL_COLOR_FIELD				TERMINAL_COLOR_FIELD_DEFAULT
	#define TERMINAL_COLOR_DEFAULT_FG			TERMINAL_COLOR_WHITE

	#define CURSOR_MODE_SHOW	0x80
	#define CURSOR_MODE_ENABLED	0x40
	#define CURSOR_MODE_CROSS 	0x02
	#define CURSOR_MODE_BASE	0x04

	#define OIAROW				(top_margin+1+(fontHeight*terminal_rows))
	#define CHARSET 			charset ? charset : "ISO-8859-1"
	#define IS_FUNCTION_KEY(event)   (event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(event->state & (GDK_MOD1_MASK|GDK_CONTROL_MASK)))

	#ifndef GDK_ALT_MASK
		#define GDK_ALT_MASK GDK_MOD1_MASK
	#endif

	#define PopupAWarning(fmt, ...)	Warning(fmt,__VA_ARGS__)
	#define PopupAnError( x )		Error("%s",x);
	#define WarningPopup(fmt, ...)	Warning(fmt,__VA_ARGS__);

 	extern const gchar *program_name;
 	extern const gchar *program_version;
 	extern const gchar *program_release;
 	extern const gchar *program_fullversion;

	extern GtkWidget				*topwindow;
	extern GtkWidget				*keypad;
	extern GdkPixmap				*pixmap;
	extern GtkWidget				*SelectionPopup;
	extern GtkWidget				*DefaultPopup;
	extern GtkWidget				*terminal;
	extern GdkColor					color[TERMINAL_COLOR_COUNT+1];

#ifdef MOUSE_POINTER_CHANGE
	extern GdkCursor        		*wCursor[CURSOR_MODE_3270];
#endif

	extern PangoFontDescription	*font;
	extern gint					cMode;

	extern int 					terminal_rows;
	extern int				 		terminal_cols;
	extern int						left_margin;
	extern int						top_margin;

	extern gint					fontWidth;
	extern gint					fontHeight;

	extern gint					cCol;
	extern gint					cRow;
	extern gboolean 				WaitingForChanges;
	extern char					*charset;
	extern gchar 					*window_title;
	extern gboolean				screen_suspended;

	extern gchar					*program_data;
	extern gchar					*program_logo;
	extern gchar					*program_config_file;
	extern gchar					*program_config_filename_and_path;

#ifdef HAVE_PLUGINS
	extern gchar					*plugin_list;
#endif

	extern GtkActionGroup			*action_group[ACTION_GROUP_MAX];

	#define common_actions		action_group[ACTION_GROUP_COMMON]
	#define online_actions		action_group[ACTION_GROUP_ONLINE]
	#define offline_actions		action_group[ACTION_GROUP_OFFLINE]
	#define selection_actions	action_group[ACTION_GROUP_SELECTION]
	#define clipboard_actions	action_group[ACTION_GROUP_CLIPBOARD]
	#define paste_actions		action_group[ACTION_GROUP_PASTE]
	#define ft_actions			action_group[ACTION_GROUP_FT]

	extern const struct lib3270_io_callbacks program_io_callbacks;
	extern const struct lib3270_screen_callbacks program_screen_callbacks;

	int			CreateTopWindow(void);
	GtkWidget	*CreateKeypadWindow(void);

	GtkWidget 	*CreateTerminalWindow(void);

	gboolean 	StartPlugins(const gchar *startup_script);

	int 		DrawScreen(GdkColor *clr, GdkDrawable *draw);
	void 		DrawElement(GdkDrawable *draw, GdkColor *clr, GdkGC *gc, int x, int y, ELEMENT *el);
	void 		DrawOIA(GdkDrawable *draw, GdkColor *clr);
	void 		PrintConsole(const gchar *fmt, ...);

	gboolean 	KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
	void 		ParseInput(const gchar *string);
	void 		MoveCursor(int row, int col);
	void 		InvalidateCursor(void);
	void		LoadImages(GdkDrawable *drawable, GdkGC *gc);
	void 		InvalidatePixmaps(GdkDrawable *drawable, GdkGC *gc);
	void 		ReloadPixmaps(void);
	void 		Reselect(void);
	void 		set_rectangle_select(int value, int reason);
	void 		SetStatusCode(STATUS_CODE id);
	void 		SetTerminalFont(const gchar *fontname);
	void 		RedrawCursor(void);

#ifdef X3270_FT
	int 		initft(void);
#endif

	gboolean	PFKey(int key);
	void 		DrawCursorPosition(void);

	#define		GetSelection() GetScreenContents(0)
	gchar 		*GetClipboard(void);
	gchar 		*GetScreenContents(gboolean all);

	gboolean 	mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
	gboolean 	mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
	gboolean 	mouse_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data);
	gboolean 	mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	gboolean 	mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

	void 		action_SelectAll(GtkWidget *w, gpointer user_data);
	void		action_SelectLeft(GtkWidget *w, gpointer user_data);
	void		action_SelectUp(GtkWidget *w, gpointer user_data);
	void		action_SelectRight(GtkWidget *w, gpointer user_data);
	void		action_SelectDown(GtkWidget *w, gpointer user_data);
	void		action_SelectionLeft(GtkWidget *w, gpointer user_data);
	void		action_SelectionUp(GtkWidget *w, gpointer user_data);
	void		action_SelectionRight(GtkWidget *w, gpointer user_data);
	void		action_SelectionDown(GtkWidget *w, gpointer user_data);

	void 			action_Append(void);
	void 			action_Copy(void);
	void 			action_CopyAsTable(void);
	void 			action_SelectField(void);
	void			action_ClearSelection(void);
	void 			action_Save(void);
	void			action_Restore(void);
	void 			action_Redraw(void);
	void 			action_SelectColors(void);
	void 			action_SetHostname(void);
	void 			action_Paste(void);
	void			action_PasteSelection(void);
	void 			action_PasteNext(void);
	void 			action_PasteTextFile(void);
	void 			action_Download(void);
	void 			action_Upload(void);

	int 			LoadColors(void);
	GtkUIManager	*LoadApplicationUI(GtkWidget *widget);
	void 			DrawElement(GdkDrawable *draw, GdkColor *clr, GdkGC *gc, int x, int y, ELEMENT *el);
	void			UpdateKeyboardState(guint state);
	int				PrintText(const char *name, gchar *text);
	void 			RestoreWindowSize(const gchar *group, GtkWidget *widget);
	void 			SaveWindowSize(const gchar *group, GtkWidget *widget);
	int 			GetFunctionKey(GdkEventKey *event);
	int 			wait4negotiations(const char *cl_hostname);
	GdkPixbuf 		*LoadLogo(void);
	GdkGC 			*getCachedGC(GdkDrawable *draw);

	void			FontChanged(void);

	int 			CloseConfigFile(void);
	int				OpenConfigFile(void);

	GCallback 		get_action_callback_by_name(const gchar *name);

	gchar 			*GetString(const gchar *group, const gchar *key, const gchar *def);
	void 			SetString(const gchar *group, const gchar *key, const gchar *val);

	gboolean 		GetBoolean(const gchar *group, const gchar *key, const gboolean def);
	void 			SetBoolean(const gchar *group, const gchar *key, const gboolean val);

	gint 			GetInt(const gchar *group, const gchar *key, gint def);
	void			SetInt(const gchar *group, const gchar *key, gint val);

	void 			DisableNetworkActions(void);
	void 			ClearClipboard(void);
	void			settitle(char *text);
	GKeyFile 		*GetConf(void);

	void			program_quit(void);

	enum _SELECT_MODE
	{
		SELECT_MODE_NONE,
		SELECT_MODE_TEXT,
		SELECT_MODE_RECTANGLE,
		SELECT_MODE_FIELD,
		SELECT_MODE_COPY,
		SELECT_MODE_APPEND,
		SELECT_MODE_DRAG,

		SELECT_MODE_INVALID
	};

	void	SetSelectionMode(int m);
	int 	GetSelectedRectangle(GdkRectangle *rect);

	void	set_monocase(int value, int reason);
	int 	LoadPlugins(void);
	int		UnloadPlugins(void);
	void	CallPlugins(const gchar *name, const gchar *arg);

	void 	SetHostname(const gchar *hostname);

	enum text_layout
	{
		TEXT_LAYOUT_NORMAL,
		TEXT_LAYOUT_UNDERLINE
	};

	#define TEXT_LAYOUT_OIA TEXT_LAYOUT_NORMAL

	PangoLayout * getPangoLayout(enum text_layout id);

	#ifdef USE_FONT_CACHE
		void clear_font_cache(void);
	#else
		#define clear_font_cache() /* */
	#endif


	// Console/Trace window
	HCONSOLE	  gui_console_window_new(const char *title, const char *label);
	void		  gui_console_window_delete(HCONSOLE hwnd);
	int			  gui_console_window_append(HCONSOLE hwnd, const char *fmt, va_list args);
	char 		* gui_console_window_wait_for_user_entry(HCONSOLE hwnd);

#endif // GUI_H_INCLUDED

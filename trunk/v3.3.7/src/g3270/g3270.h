/*
* Copyright 2008, Banco do Brasil S.A.
*
* This file is part of g3270
*
* This program file is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program in a file named COPYING; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
*
* Authors:
*
* Perry Werneck<perry.werneck@gmail.com>
*
*/

#ifndef G3270_H_INCLUDED

	#define G3270_H_INCLUDED

	#include <lib3270/config.h>
	#define ENABLE_NLS
	#define GETTEXT_PACKAGE PACKAGE_NAME

	#include <libintl.h>
	#include <glib/gi18n.h>

	#include <gtk/gtk.h>
	#include <gdk/gdk.h>
	#include <glib.h>

	#include <lib3270/api.h>

	#define MAX_CHR_LENGTH 3
	typedef struct _element
	{
		gchar				ch[MAX_CHR_LENGTH];
		unsigned short	fg;
		unsigned short	bg;
		unsigned short	extended;
		gboolean			selected;
	} ELEMENT;
	extern ELEMENT *screen;

    enum GUI_TOGGLE
    {
        GUI_TOGGLE_BOLD,
        GUI_TOGGLE_KEEP_SELECTED,
        GUI_TOGGLE_EXTENDED_PASTE,

        GUI_TOGGLE_COUNT
    };

    #define TOGGLED_BOLD			gui_toggle[GUI_TOGGLE_BOLD]
    #define TOGGLED_KEEP_SELECTED	gui_toggle[GUI_TOGGLE_KEEP_SELECTED]
    #define TOGGLED_EXTENDED_PASTE	gui_toggle[GUI_TOGGLE_EXTENDED_PASTE]

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

	#define OIAROW				(top_margin+1+(fHeight*terminal_rows))
	#define CHARSET 			charset ? charset : "ISO-8859-1"
	#define IS_FUNCTION_KEY(event)   (event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(event->state & (GDK_MOD1_MASK|GDK_CONTROL_MASK)))

	#ifndef GDK_ALT_MASK
		#define GDK_ALT_MASK GDK_MOD1_MASK
	#endif

	#define PopupAWarning(fmt, ...)	PopupAnError(fmt,__VA_ARGS__)


	extern GtkWidget				*topwindow;
	extern GtkWidget				*keypad;
	extern GdkPixmap				*pixmap;
	extern GtkWidget				*SelectionPopup;
	extern GtkWidget				*DefaultPopup;
	extern GtkWidget				*terminal;
	extern GdkColor				color[TERMINAL_COLOR_COUNT+1];
	extern GdkCursor        		*wCursor[CURSOR_MODE_USER];
	extern PangoFontDescription	*font;
	extern gint					cMode;

	extern int 					terminal_rows;
	extern int				 		terminal_cols;
	extern int						left_margin;
	extern int						top_margin;
	extern int 					fWidth;
	extern int 					fHeight;
	extern gint					cCol;
	extern gint					cRow;
	extern gboolean 				WaitingForChanges;
	extern char					*charset;
	extern gchar 					*window_title;

	extern GtkActionGroup			*action_group[ACTION_GROUP_MAX];

	#define common_actions		action_group[ACTION_GROUP_COMMON]
	#define online_actions		action_group[ACTION_GROUP_ONLINE]
	#define offline_actions		action_group[ACTION_GROUP_OFFLINE]
	#define selection_actions	action_group[ACTION_GROUP_SELECTION]
	#define clipboard_actions	action_group[ACTION_GROUP_CLIPBOARD]

	extern const struct lib3270_io_callbacks g3270_io_callbacks;
	extern const struct lib3270_screen_callbacks g3270_screen_callbacks;

	int			CreateTopWindow(void);
	GtkWidget	*CreateKeypadWindow(void);

	GtkWidget 	*CreateTerminalWindow(void);

	int 		DrawScreen(GtkWidget *widget, GdkColor *clr, GdkDrawable *draw);
	void 		DrawOIA(GtkWidget *widget, GdkColor *clr, GdkDrawable *draw);

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
	gboolean	PFKey(int key);

	#define		GetSelection() GetScreenContents(0)
	gchar 		*GetClipboard(void);
	gchar 		*GetScreenContents(gboolean all);

	gboolean 	mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
	gboolean 	mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
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
	void 			action_SelectField(void);
	void			action_ClearSelection(void);
	void 			action_Save(void);
	void			action_Restore(void);
	void 			action_Redraw(void);
	void 			action_SelectColors(void);
	void 			action_SetHostname(void);
	void 			action_Paste(void);
	void 			action_PasteNext(void);
	void 			action_PasteTextFile(void);


	int 			LoadColors(void);
	GtkUIManager	*LoadApplicationUI(GtkWidget *widget);
	void 			DrawElement(GdkDrawable *draw, GdkColor *clr, GdkGC *gc, PangoLayout *layout, int x, int y, ELEMENT *el);
	void			UpdateKeyboardState(guint state);
	int				PrintText(const char *name, gchar *text);
	gchar 			*FindSystemConfigFile(const gchar *name);
	void 			RestoreWindowSize(const gchar *group, GtkWidget *widget);
	void 			SaveWindowSize(const gchar *group, GtkWidget *widget);
	int 			GetFunctionKey(GdkEventKey *event);
	int 			wait4negotiations(const char *cl_hostname);
	GdkPixbuf 		*LoadLogo(void);

	void			FontChanged(void);

	int 			CloseConfigFile(void);
	int				OpenConfigFile(void);

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

	void			g3270_quit(void);

	enum _SELECT_MODE
	{
		SELECT_MODE_NONE,
		SELECT_MODE_TEXT,
		SELECT_MODE_RECTANGLE,
		SELECT_MODE_FIELD,
		SELECT_MODE_COPY,
		SELECT_MODE_APPEND,

		SELECT_MODE_INVALID
	};

	void	SetSelectionMode(int m);
	void	set_monocase(int value, int reason);
	int 	LoadPlugins(void);
	int		UnloadPlugins(void);
	void	CallPlugins(const gchar *name, const gchar *arg);

	void 	SetHostname(const gchar *hostname);

	void 	PopupAnError(const gchar *fmt, ...);
	void 	WarningPopup(const gchar *fmt, ...);

#endif // G3270_H_INCLUDED

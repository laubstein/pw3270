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

	#include <gtk/gtk.h>
	#include <glib.h>
	#include <glib/gi18n.h>

	#include <lib3270/api.h>

	#define MAX_CHR_LENGTH 3
	typedef struct _element
	{
		gchar		ch[MAX_CHR_LENGTH];
		short		fg;
		short		bg;
		gboolean	selected;
	} ELEMENT;
	extern ELEMENT *screen;

	enum TERMINAL_COLOR
	{
		TERMINAL_COLOR_00,
		TERMINAL_COLOR_01,
		TERMINAL_COLOR_02,
		TERMINAL_COLOR_03,
		TERMINAL_COLOR_04,
		TERMINAL_COLOR_05,
		TERMINAL_COLOR_06,
		TERMINAL_COLOR_07,
		TERMINAL_COLOR_08,
		TERMINAL_COLOR_09,
		TERMINAL_COLOR_10,
		TERMINAL_COLOR_11,
		TERMINAL_COLOR_12,
		TERMINAL_COLOR_13,
		TERMINAL_COLOR_14,
		TERMINAL_COLOR_15,

		TERMINAL_COLOR_FIELD_DEFAULT,
		TERMINAL_COLOR_FIELD_INTENSIFIED,
		TERMINAL_COLOR_FIELD_PROTECTED,
		TERMINAL_COLOR_FIELD_PROTECTED_INTENSIFIED,

		TERMINAL_COLOR_SELECTED_FG,
		TERMINAL_COLOR_SELECTED_BG,

		TERMINAL_COLOR_CURSOR,
		TERMINAL_COLOR_CROSS_HAIR,

		// Oia Colors
		TERMINAL_COLOR_OIA_SEPARATOR,
		TERMINAL_COLOR_OIA,
		TERMINAL_COLOR_OIA_BACKGROUND,
		TERMINAL_COLOR_OIA_STATUS_OK,
		TERMINAL_COLOR_OIA_STATUS_INVALID,

		TERMINAL_COLOR_COUNT
	};

	#define TERMINAL_COLOR_OIA_CURSOR 			TERMINAL_COLOR_OIA_STATUS_OK
	#define TERMINAL_COLOR_OIA_LU 				TERMINAL_COLOR_OIA_STATUS_OK
	#define TERMINAL_COLOR_SSL 					TERMINAL_COLOR_OIA
	#define TERMINAL_COLOR_OIA_STATUS_WARNING	TERMINAL_COLOR_OIA
	#define TERMINAL_COLOR_FIELD				TERMINAL_COLOR_FIELD_DEFAULT

	#define CURSOR_MODE_SHOW	0x80
	#define CURSOR_MODE_ENABLED	0x40
	#define CURSOR_MODE_CROSS 	0x02
	#define CURSOR_MODE_BASE	0x04

	#define OIAROW				(top_margin+(fHeight*terminal_rows))
	#define CHARSET 			charset ? charset : "ISO-8859-1"

	extern GtkWidget				*topwindow;
	extern GdkPixmap				*pixmap;
	extern GtkWidget				*SelectionPopup;
	extern GtkWidget				*DefaultPopup;
	extern GtkWidget				*terminal;
	extern GdkColor				color[TERMINAL_COLOR_COUNT+1];
	extern GdkCursor        		*wCursor[CURSOR_MODE_USER];
	extern PangoFontDescription	*font;
	extern gint					cMode;
	extern GdkPixbuf 				*main_icon;


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

	extern const struct lib3270_io_callbacks g3270_io_callbacks;
	extern const struct lib3270_screen_callbacks g3270_screen_callbacks;

	int			CreateTopWindow(void);
	GtkWidget 	*CreateTerminalWindow(void);

	int 		DrawScreen(GtkWidget *widget, GdkColor *clr, GdkDrawable *draw);
	void 		DrawOIA(GtkWidget *widget, GdkColor *clr, GdkDrawable *draw);

	gboolean 	KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
	void 		ParseInput(const gchar *string);
	void 		MoveCursor(int row, int col);
	void 		InvalidateCursor(void);
	void 		LoadImages(GdkDrawable *drawable, GdkGC *gc);
	void 		Reselect(void);
	void 		set_rectangle_select(int value, int reason);
	void 		SetStatusCode(STATUS_CODE id);

	#define		GetSelection() GetScreenContents(0)
	gchar 		*GetClipboard(void);
	gchar 		*GetScreenContents(gboolean all);

	gboolean 	mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
	gboolean 	mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
	gboolean 	mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	gboolean 	mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

	void 		action_PageDown(GtkWidget *w, gpointer user_data);
	void 		action_PageUP(GtkWidget *w, gpointer user_data);
	void 		action_Paste(GtkWidget *w, gpointer user_data);
	void 		action_PasteNext(GtkWidget *w, gpointer user_data);
	void 		action_SelectAll(GtkWidget *w, gpointer user_data);
	void		action_SelectLeft(GtkWidget *w, gpointer user_data);
	void		action_SelectUp(GtkWidget *w, gpointer user_data);
	void		action_SelectRight(GtkWidget *w, gpointer user_data);
	void		action_SelectDown(GtkWidget *w, gpointer user_data);

	void 		action_Append(void);
	void 		action_Copy(void);
	void 		action_SelectField(void);
	void		action_ClearSelection(void);
	void 		action_Save(void);
	void		action_Restore(void);
	void 		action_Redraw(void);


	GtkUIManager	*LoadApplicationUI(GtkWidget *widget);
	void 			DrawElement(GdkDrawable *draw, GdkColor *clr, GdkGC *gc, PangoLayout *layout, int x, int y, ELEMENT *el);
	void			UpdateKeyboardState(guint state);
	int				PrintText(const char *name, gchar *text);
	gchar 			*FindSystemConfigFile(const gchar *name);

	int 			CloseConfigFile(void);
	int				OpenConfigFile(void);
	gchar 			*GetString(const gchar *group, const gchar *key, const gchar *def);


#endif // G3270_H_INCLUDED

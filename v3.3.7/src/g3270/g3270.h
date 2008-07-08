/*
 * Modifications and original code Copyright 1993, 1994, 1995, 1996,
 *    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *   All Rights Reserved.  GTRC hereby grants public use of this software.
 *   Derivative works based on this software must incorporate this copyright
 *   notice.
 *
 * c3270 and wc3270 are distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	g3270.h
 *		g3270 external functions
 */

#ifndef G3270_H_INCLUDED

	#define G3270_H_INCLUDED

	#include <gtk/gtk.h>
	#include <lib3270/api.h>

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
		TERMINAL_COLOR_CURSOR,
		TERMINAL_COLOR_CROSS_HAIR,
		TERMINAL_COLOR_OIA,
		TERMINAL_COLOR_OIA_BACKGROUND,
		TERMINAL_COLOR_OIA_STATUS_OK,
		TERMINAL_COLOR_OIA_STATUS_INVALID,

		TERMINAL_COLOR_COUNT
	};

	#define TERMINAL_COLOR_OIA_CURSOR 	TERMINAL_COLOR_OIA_STATUS_OK
	#define TERMINAL_COLOR_OIA_LU 		TERMINAL_COLOR_OIA_STATUS_OK
	#define TERMINAL_COLOR_SSL 			TERMINAL_COLOR_OIA_STATUS_OK

	#define CURSOR_MODE_SHOW	0x80
	#define CURSOR_MODE_ENABLED	0x40
	#define CURSOR_MODE_CROSS 	0x02
	#define CURSOR_MODE_BASE	0x04

	#define OIAROW				(top_margin+(fHeight*terminal_rows))

	extern GtkWidget				*topwindow;
	extern GdkPixmap				*pixmap;
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

	gboolean 	mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data);
	gboolean 	mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data);
	gboolean 	mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
	gboolean 	mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);

	void 			action_PageDown(GtkWidget *w, gpointer user_data);
	void 			action_PageUP(GtkWidget *w, gpointer user_data);
	GtkUIManager	*LoadApplicationUI(GtkWidget *widget);

#endif // G3270_H_INCLUDED

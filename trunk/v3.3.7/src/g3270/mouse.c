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


 #include <globals.h>
 #include "g3270.h"
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/toggle.h>

/*---[ Defines ]--------------------------------------------------------------*/

 #define DecodePosition(event,row,col)	col = ( (((unsigned long) event->x) - left_margin)/fWidth ); \
										row = ( (((unsigned long) event->y) - top_margin)/fHeight );


 enum _SELECTING_MODE
 {
	SELECTING_NONE,
	SELECTING_NORMAL,
	SELECTING_RECTANGLE,


	SELECTING_INVALID
 };

/*---[ Prototipes ]-----------------------------------------------------------*/

/*---[ Constants ]------------------------------------------------------------*/

/*---[ Statics ]--------------------------------------------------------------*/

 static int startRow 	= 0;
 static int startCol 	= 0;
 static int endRow 	= 0;
 static int endCol		= 0;
 static int mode		= SELECTING_NONE;

/*---[ Globals ]--------------------------------------------------------------*/

 gboolean	WaitingForChanges 	= TRUE;
 GtkWidget	*SelectionPopup		= 0;
 GtkWidget	*DefaultPopup		= 0;

/*---[ Implement ]------------------------------------------------------------*/

 static void SetSelection(gboolean selected)
 {
 	int pos;

	if(!screen)
		return;

	for(pos = 0; pos < (terminal_rows * terminal_cols);pos++)
		screen[pos].selected = selected;

	if(terminal && pixmap)
	{
		DrawScreen(terminal,color,pixmap);
		DrawOIA(terminal,color,pixmap);
		gtk_widget_queue_draw(terminal);
	}
 }

 gboolean mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
	// http://library.gnome.org/devel/gdk/stable/gdk-Event-Structures.html#GdkEventButton
	GtkWidget *w;

	switch( ((event->type & 0x0F) << 4) | (event->button & 0x0F))
	{
	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 1:
		ClearSelection();
		DecodePosition(event,startRow,startCol);
		Trace("Button 1 clicked at %ld,%ld (%d,%d)",(long) event->x, (long) event->y,startRow,startCol);
		break;

	case ((GDK_2BUTTON_PRESS & 0x0F) << 4) | 1:
		Trace("Button 1 double-clicked at %ld,%ld",(long) event->x, (long) event->y);
		break;

	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 3:
		w = (mode == SELECTING_NONE) ? DefaultPopup : SelectionPopup;
		Trace("Button 2 clicked at %ld,%ld Menu: %p",(long) event->x, (long) event->y, w);
		if(w)
		{
			gtk_menu_set_screen(GTK_MENU(w), gtk_widget_get_screen(widget));
			gtk_menu_popup(GTK_MENU(w), NULL, NULL, NULL, NULL, event->button,event->time);
		}
		break;

	case ((GDK_2BUTTON_PRESS & 0x0F) << 4) | 3:
		Trace("Button 2 double-clicked at %ld,%ld",(long) event->x, (long) event->y);
		break;

	default:
		Trace("Unexpected mouse click %d with button %d",event->type, event->button);
 		return 0;
	}

 	return 0;
 }

 gboolean mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
	int row,col;

	DecodePosition(event,row,col);

	Trace("Button %d release",event->button);

 	switch(mode)
 	{
	case SELECTING_NONE:	// Single click, just move cursor
		if(event->button == 1)
			cursor_move((row*terminal_cols)+col);
		break;


 	}

    return 0;
 }

 static void UpdateSelectedRectangle(void)
 {
 	int			x,y,row,col;
	GdkGC		*gc		= NULL;
	PangoLayout *layout	= NULL;
 	int			pos		= 0;
 	int			left;
 	int			right;
 	int			top;
 	int			bottom;

	if(!(mode && screen && terminal))
		return;

	// Clear selection box, invalidate drawing area.
	if(pixmap)
		gc = gdk_gc_new(pixmap);

	if(terminal)
		layout = gtk_widget_create_pango_layout(terminal," ");

	if(startRow > endRow)
	{
		top = endRow;
		bottom = startRow;
	}
	else
	{
		top = startRow;
		bottom = endRow;
	}

	if(startCol > endCol)
	{
		left = endCol;
		right = endCol;
	}
	else
	{
		left = startCol;
		right = endCol;
	}

	y = top_margin;
	for(row = 0; row < terminal_rows;row++)
	{
		x = left_margin;
		for(col = 0; col < terminal_cols;col++)
		{
			gboolean selected = (col >= left && col <= right && row >= top && row <= bottom);

			if(screen[pos].selected != selected)
			{
				// Changed, mark to update
				screen[pos].selected = selected;
				DrawElement(pixmap,color,gc,layout,x,y,screen+pos);
				gtk_widget_queue_draw_area(terminal,x,y,fWidth,fHeight);
			}
			pos++;
			x += fWidth;
		}
		y += fHeight;
	}

	if(layout)
		g_object_unref(layout);

	if(gc)
		gdk_gc_destroy(gc);

 }

 static void UpdateSelectedText(void)
 {
 	int			start	= (startRow * terminal_cols)+startCol;
 	int			end 	= (endRow * terminal_cols)+endCol;
 	int			pos		= 0;
 	int			x,y,row,col;
	GdkGC		*gc		= NULL;
	PangoLayout *layout	= NULL;

	if(!(screen && terminal))
		return;

 	if(start > end)
 	{
		int temp = start;
		start = end;
		end = temp;
 	}

	if(pixmap)
		gc = gdk_gc_new(pixmap);

	if(terminal)
		layout = gtk_widget_create_pango_layout(terminal," ");

	y = top_margin;
	for(row = 0; row < terminal_rows;row++)
	{
		x = left_margin;
		for(col = 0; col < terminal_cols;col++)
		{
			gboolean selected = (pos >= start && pos <= end) ? TRUE : FALSE;
			if(screen[pos].selected != selected)
			{
				// Changed, mark to update
				screen[pos].selected = selected;
				DrawElement(pixmap,color,gc,layout,x,y,screen+pos);
				gtk_widget_queue_draw_area(terminal,x,y,fWidth,fHeight);
			}
			pos++;
			x += fWidth;
		}
		y += fHeight;
	}

	if(layout)
		g_object_unref(layout);

	if(gc)
		gdk_gc_destroy(gc);


 }

 void set_rectangle_select(int value, int reason)
 {
 	if(mode != SELECTING_RECTANGLE && mode != SELECTING_NORMAL)
		return;

	ClearSelection();
	mode = value ? SELECTING_RECTANGLE : SELECTING_NORMAL;

	if(mode == SELECTING_NORMAL)
		UpdateSelectedText();
	else
		UpdateSelectedRectangle();
 }

 void Reselect(void)
 {
	mode = Toggled(RECTANGLE_SELECT) ? SELECTING_RECTANGLE : SELECTING_NORMAL;
	if(mode == SELECTING_NORMAL)
		UpdateSelectedText();
	else
		UpdateSelectedRectangle();
 }

 gboolean mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
 {
 	switch(mode)
 	{
	case SELECTING_NONE:	// Starting selection
		mode = Toggled(RECTANGLE_SELECT) ? SELECTING_RECTANGLE : SELECTING_NORMAL;
		return mouse_motion(widget,event,user_data); // Recursive call to update selection box

	case SELECTING_RECTANGLE:
		DecodePosition(event,endRow,endCol);
		UpdateSelectedRectangle();
		break;

	case SELECTING_NORMAL:
		DecodePosition(event,endRow,endCol);
		UpdateSelectedText();
		break;

 	}
    return 0;
 }

 gboolean mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
 {
    // FIXME (perry#1#): Read association from scroll to function key from configuration file.
    if(event->direction < 2 && !WaitingForChanges)
 	{
		if(event->direction)
			action_PageDown(widget,0);
		else
			action_PageUP(widget,0);
	}

 	return 0;
 }

 void ClearSelection(void)
 {
 	SetSelection(FALSE);
	mode = SELECTING_NONE;
 }

 void action_SelectAll(GtkWidget *w, gpointer user_data)
 {
 	SetSelection(TRUE);
	mode = Toggled(RECTANGLE_SELECT) ? SELECTING_RECTANGLE : SELECTING_NORMAL;
 }

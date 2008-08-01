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
 #include <lib3270/3270ds.h>

/*---[ Defines ]--------------------------------------------------------------*/

 #define DecodePosition(event,row,col)	col = ( (((unsigned long) event->x) - left_margin)/fWidth ); \
										row = ( (((unsigned long) event->y) - top_margin)/fHeight );


 enum _SELECT_MODE
 {
	SELECT_MODE_NONE,
	SELECT_MODE_TEXT,
	SELECT_MODE_RECTANGLE,
	SELECT_MODE_FIELD,
	SELECT_MODE_COPY,
	SELECT_MODE_APPEND,

	SELECT_INVALID
 };

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void UpdateSelectedRegion(int start, int end);
 static void SelectField(int row, int col);

/*---[ Constants ]------------------------------------------------------------*/

/*---[ Statics ]--------------------------------------------------------------*/

 static int startRow 	= 0;
 static int startCol 	= 0;
 static int endRow 	= 0;
 static int endCol		= 0;
 static int mode		= SELECT_MODE_NONE;

/*---[ Globals ]--------------------------------------------------------------*/

 gboolean	WaitingForChanges 	= TRUE;
 GtkWidget	*SelectionPopup		= 0;
 GtkWidget	*DefaultPopup		= 0;

/*---[ Implement ]------------------------------------------------------------*/

 static void SetMode(int m)
 {
 	static const gchar	*name[] = { "Copy", "Append" };
 	int						f;

 	if(m == mode)
		return;

	mode = m;

	for(f=0;f<G_N_ELEMENTS(name);f++)
		gtk_action_set_sensitive(gtk_action_group_get_action(main_actions,name[f]),(mode != SELECT_MODE_NONE));
 }

 static int CheckForFunction(int baddr, int length)
 {
 	int ret = 0;
	if( !(*screen[baddr+1].ch == 'F' && FA_IS_PROTECTED(get_field_attribute(baddr))) )
		return ret;

	baddr++;
	while(--length > 0)
	{
		baddr++;
		if(*screen[baddr].ch > '9' || *screen[baddr].ch < '0')
			return 0;

		ret = (ret*10)+(*screen[baddr].ch - '0');
		Trace("Field@%d: %c",baddr,*screen[baddr].ch);
	}

 	return ret;
 }

 void action_SelectField(void)
 {
 	int 		pos;
 	gboolean	redraw	= FALSE;

	if(!screen)
		return;

	for(pos = 0; pos < (terminal_rows * terminal_cols);pos++)
	{
		if(screen[pos].selected)
		{
			redraw = TRUE;
			screen[pos].selected = FALSE;
		}
	}

	SelectField(cRow,cCol);

	if(redraw && terminal && pixmap)
	{
		DrawScreen(terminal,color,pixmap);
		DrawOIA(terminal,color,pixmap);
		gtk_widget_queue_draw(terminal);
	}

 }

 static void SelectField(int row, int col)
 {
 	int baddr = find_field_attribute((row * terminal_cols) + col);
 	int length = find_field_length(baddr);
 	int function;

	Trace("Field %d with %d bytes",baddr,length);

	if(length < 0 || length > ((terminal_cols * terminal_cols) - baddr))
		return;

	function = CheckForFunction(baddr,length);
	if(function > 0)
	{
		// Double-click in "F*", request function key
		char buffer[10];
		sprintf(buffer,"%d",function);
		action_internal(PF_action, IA_DEFAULT, buffer, CN);
		return;
	}

	startRow = (baddr+1) / terminal_cols;
	startCol = (baddr+1) % terminal_cols;

	UpdateSelectedRegion(baddr+1,baddr+length);

	baddr += length;
	endRow = baddr / terminal_cols;
	endCol = baddr % terminal_cols;

 }

 static void SetSelection(gboolean selected)
 {
 	int 		pos;
 	gboolean	redraw	= FALSE;

	if(!screen)
		return;

	for(pos = 0; pos < (terminal_rows * terminal_cols);pos++)
	{
		if(screen[pos].selected != selected)
		{
			redraw = TRUE;
			screen[pos].selected = selected;
		}
	}

	if(redraw && terminal && pixmap)
	{
		DrawScreen(terminal,color,pixmap);
		DrawOIA(terminal,color,pixmap);
		gtk_widget_queue_draw(terminal);
	}
 }

 #define BUTTON_FLAG_COMBO	0x80

 static gint button_flags = 0;

 gboolean mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
	// http://library.gnome.org/devel/gdk/stable/gdk-Event-Structures.html#GdkEventButton
	GtkWidget *w;

	Trace("Button press: %d",event->button);

	switch( ((event->type & 0x0F) << 4) | (event->button & 0x0F) | button_flags)
	{
	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 1:
		button_flags |= BUTTON_FLAG_COMBO;
		action_ClearSelection();
		DecodePosition(event,startRow,startCol);
		Trace("Button 1 clicked at %ld,%ld (%d,%d)",(long) event->x, (long) event->y,startRow,startCol);
		break;

	case ((GDK_2BUTTON_PRESS & 0x0F) << 4) | 1 | BUTTON_FLAG_COMBO:
		DecodePosition(event,startRow,startCol);
		SetMode(SELECT_MODE_FIELD);
		Trace("Button 1 double-clicked at %ld,%ld (%d,%d)",(long) event->x, (long) event->y,startRow,startCol);
		break;

	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 3:
		w = (mode == SELECT_MODE_NONE) ? DefaultPopup : SelectionPopup;
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

	case ((GDK_2BUTTON_PRESS & 0x0F) << 4) | 3 | BUTTON_FLAG_COMBO:
		Trace("Button 2 double-clicked in combo mode at %ld,%ld",(long) event->x, (long) event->y);
		SetMode(SELECT_MODE_APPEND);
		break;

	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 3 | BUTTON_FLAG_COMBO:
		Trace("Button 2 clicked in combo mode at %ld,%ld",(long) event->x, (long) event->y);
		SetMode(SELECT_MODE_COPY);
		break;

#ifdef DEBUG
	default:
		Trace("Unexpected mouse click %d with button %d flag: %d",event->type, event->button,button_flags);
#endif
	}

 	return 0;
 }

 gboolean mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
	int row,col;

	DecodePosition(event,row,col);

	Trace("Button %d release",event->button);

	switch( ((mode & 0x0F) << 4) | (event->button & 0x0F))
 	{
	case ((SELECT_MODE_NONE & 0x0F) << 4) | 1: // Single click, just move cursor
		action_ClearSelection();
		cursor_move((row*terminal_cols)+col);
		break;

	case ((SELECT_MODE_FIELD & 0x0F) << 4) | 1:	// Double click, select field
		Trace("Selecting field (button: %d)",event->button);
		SelectField(startRow,startCol);
		break;

	case ((SELECT_MODE_COPY & 0x0F) << 4) | 1:
		action_Copy();
		action_ClearSelection();
		break;

	case ((SELECT_MODE_APPEND & 0x0F) << 4) | 1:
		action_Append();
		action_ClearSelection();
		break;

#ifdef DEBUG
	default:
		Trace("Unexpected action %04x",((mode & 0x0F) << 4) | (event->button & 0x0F));
#endif
 	}

	if(event->button == 1)
		button_flags &= ~BUTTON_FLAG_COMBO;

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
 	UpdateSelectedRegion((startRow * terminal_cols)+startCol,(endRow * terminal_cols)+endCol);
 }

 static void UpdateSelectedRegion(int start, int end)
 {
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
 	if(mode != SELECT_MODE_RECTANGLE && mode != SELECT_MODE_TEXT)
		return;

	action_ClearSelection();
	SetMode(value ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);

	if(mode == SELECT_MODE_TEXT)
		UpdateSelectedText();
	else
		UpdateSelectedRectangle();
 }

 void Reselect(void)
 {
	SetMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
	if(mode == SELECT_MODE_TEXT)
		UpdateSelectedText();
	else
		UpdateSelectedRectangle();
 }

 gboolean mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
 {
 	switch(mode)
 	{
	case SELECT_MODE_NONE:	// Starting selection
		SetMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
		return mouse_motion(widget,event,user_data); // Recursive call to update selection box

	case SELECT_MODE_RECTANGLE:
		DecodePosition(event,endRow,endCol);
		UpdateSelectedRectangle();
		break;

	case SELECT_MODE_TEXT:
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

 void action_ClearSelection(void)
 {
	SetSelection(FALSE);
	SetMode(SELECT_MODE_NONE);
 }

 void action_SelectAll(GtkWidget *w, gpointer user_data)
 {
 	SetSelection(TRUE);
	SetMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
 }

 void action_SelectLeft(GtkWidget *w, gpointer user_data)
 {
 	if(mode == SELECT_MODE_NONE)
 	{
 		SetMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
 		startRow = endRow = cRow;
 		startCol = endCol = cCol;
 	}

 	action_internal(Left_action, IA_DEFAULT, CN, CN);
	endRow = cRow;
	endCol = cCol;

	Reselect();
 }

 void action_SelectUp(GtkWidget *w, gpointer user_data)
 {
 	if(mode == SELECT_MODE_NONE)
 	{
 		SetMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
 		startRow = endRow = cRow;
 		startCol = endCol = cCol;
 	}

 	action_internal(Up_action, IA_DEFAULT, CN, CN);
	endRow = cRow;
	endCol = cCol;

	Reselect();
 }

 void action_SelectRight(GtkWidget *w, gpointer user_data)
 {
 	if(mode == SELECT_MODE_NONE)
 	{
 		SetMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
 		startRow = endRow = cRow;
 		startCol = endCol = cCol;
 	}

 	action_internal(Right_action, IA_DEFAULT, CN, CN);
	endRow = cRow;
	endCol = cCol;

	Reselect();
 }

 void action_SelectDown(GtkWidget *w, gpointer user_data)
 {
 	if(mode == SELECT_MODE_NONE)
 	{
 		SetMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
 		startRow = endRow = cRow;
 		startCol = endCol = cCol;
 	}

 	action_internal(Down_action, IA_DEFAULT, CN, CN);
	endRow = cRow;
	endCol = cCol;

	Reselect();
 }


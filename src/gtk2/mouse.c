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
 * Este programa está nomeado como mouse.c e possui 914 linhas de código.
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


 #include "gui.h"
 #include "actions.h"
 #include "fonts.h"

// #include <globals.h>
// #include <lib3270/kybdc.h>
 #include <lib3270/toggle.h>
 #include <lib3270/3270ds.h>

/*---[ Defines ]--------------------------------------------------------------*/

 enum SELECT_MODE
 {
	SELECT_MODE_NONE,
	SELECT_MODE_TEXT,
	SELECT_MODE_RECTANGLE,
	SELECT_MODE_WORD,
	SELECT_MODE_FIELD,
	SELECT_MODE_COPY,
	SELECT_MODE_APPEND,
	SELECT_MODE_DRAG,

	SELECT_MODE_INVALID
 };

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void UpdateSelectedRegion(int start, int end);
 static void SetDragType(int type);
 static void SetSelection(gboolean selected);
 static void SetSelectionMode(enum SELECT_MODE m);

/*---[ Constants ]------------------------------------------------------------*/

/*---[ Statics ]--------------------------------------------------------------*/

 static int 				startRow 	= -1;
 static int 				startCol 	= 0;
 static int 				endRow 		= 0;
 static int 				endCol		= 0;
 static int 				dragRow		= 0;
 static int 				dragCol		= 0;
 static enum SELECT_MODE	select_mode	= SELECT_MODE_INVALID;

/*---[ Globals ]--------------------------------------------------------------*/

 GtkMenu	* popup_menu[POPUP_MENU_COUNT]	= { NULL };
 int 		  drag_type						= DRAG_TYPE_NONE;

/*---[ Implement ]------------------------------------------------------------*/

 int get_selected_rectangle(GdkRectangle *rect)
 {
	// First check if the selection area isn't rectangular.
 	if(!Toggled(RECTANGLE_SELECT))
 	{
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "Invalid action" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Can't copy non rectangular area" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",_( "Activate rectangle select option and try again." ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

		return EINVAL;
 	}

	rect->x			= startCol;
	rect->y			= startRow;
	rect->width		= (endCol - startCol)+1;
	rect->height	= (endRow - startRow)+1;

	return 0;
 }

 static void SetSelectionMode(enum SELECT_MODE m)
 {
 	if(m == select_mode)
		return;

	switch( (int) m)
	{
		case SELECT_MODE_RECTANGLE:
			gtk_action_set_sensitive(action_by_id[ACTION_COPY_AS_TABLE],TRUE);
			gtk_action_set_sensitive(action_by_id[ACTION_COPY_AS_IMAGE],TRUE);
			gtk_action_set_sensitive(action_by_id[ACTION_RESELECT],FALSE);
			gtk_action_set_sensitive(action_by_id[ACTION_UNSELECT],TRUE);
			break;

		case SELECT_MODE_TEXT:
			gtk_action_set_sensitive(action_by_id[ACTION_COPY_AS_TABLE],FALSE);
			gtk_action_set_sensitive(action_by_id[ACTION_COPY_AS_IMAGE],FALSE);
			gtk_action_set_sensitive(action_by_id[ACTION_RESELECT],FALSE);
			gtk_action_set_sensitive(action_by_id[ACTION_UNSELECT],TRUE);
			break;

		case SELECT_MODE_WORD:
			gtk_action_set_sensitive(action_by_id[ACTION_RESELECT],FALSE);
			gtk_action_set_sensitive(action_by_id[ACTION_UNSELECT],TRUE);
			break;

		case SELECT_MODE_NONE:
			gtk_action_set_sensitive(action_by_id[ACTION_RESELECT],select_mode != SELECT_MODE_INVALID);
			gtk_action_set_sensitive(action_by_id[ACTION_UNSELECT],FALSE);
			break;
	}

	select_mode = m;
	action_group_set_sensitive(ACTION_GROUP_SELECTION,(select_mode == SELECT_MODE_NONE) ? FALSE : TRUE );

 }

 static int CheckForFunction(int baddr, int length)
 {
 	int ret = 0;

	if( !(*screen->content[baddr+1].ch == 'F' && FA_IS_PROTECTED(get_field_attribute(NULL,baddr))) )
		return ret;

	baddr++;
	while(--length > 0)
	{
		baddr++;

		if(*screen->content[baddr].ch > '9' || *screen->content[baddr].ch < '0')
			return 0;

		ret = (ret*10)+(*screen->content[baddr].ch - '0');
	}

	Trace("%s: %d",__FUNCTION__,ret);
 	return ret;
 }

 PW3270_ACTION( selectword )
 {
 	if(valid_terminal_window())
 	{
		int start = cursor_position;
		int end = start;
		int length;
		int sz = screen->rows * screen->cols;

		if(g_ascii_isspace(*screen->content[start].ch))
		{
			action_selectfield(0);
			return;
		}

		SetSelection(FALSE);

		while(start > 0 && !g_ascii_isspace(*screen->content[start].ch))
			start--;

		while(end < sz && !g_ascii_isspace(*screen->content[end].ch))
			end++;

		length = end-start;

		if(length < 4 )
		{
			int function = CheckForFunction(start,length-1);
			if(!lib3270_pfkey(function))
				return;
		}
		start++;
		end--;

		startRow = start / screen->cols;
		startCol = start % screen->cols;

		UpdateSelectedRegion(start,end);

		endRow = end / screen->cols;
		endCol = end % screen->cols;

		SetSelectionMode(SELECT_MODE_WORD);
 	}
 }


 PW3270_ACTION( selectfield )
 {
 	if(valid_terminal_window())
 	{
		int baddr = find_field_attribute(NULL,cursor_position);
		int length = find_field_length(NULL,baddr);

		SetSelection(FALSE);

//		Trace("%s: %d %d",__FUNCTION__,length,((screen->rows * screen->cols) - baddr));
//		if(length < 0 || length > ((screen->rows * screen->cols) - baddr))
//			return;

		if(length < 3)
		{
			int function = CheckForFunction(baddr,length);
			if(!lib3270_pfkey(function))
				return;
		}

		startRow = (baddr+1) / screen->cols;
		startCol = (baddr+1) % screen->cols;

		UpdateSelectedRegion(baddr+1,baddr+length);

		baddr += length;
		endRow = baddr / screen->cols;
		endCol = baddr % screen->cols;

		SetSelectionMode(SELECT_MODE_FIELD);
 	}
 }

 static void SetSelection(gboolean selected)
 {
 	int 			pos;
 	int				start = -1;
 	int				end   = -1;
	unsigned char	status;

	if(!screen)
		return;

	for(pos = 0; pos < (screen->rows * screen->cols);pos++)
	{
		status = screen->content[pos].status & ELEMENT_STATUS_FIELD_MASK;
		if(selected)
			status |= ELEMENT_STATUS_SELECTED;

		if(screen->content[pos].status != status)
		{
			if(start < 0)
				start = pos;
			end = pos;
			screen->content[pos].status = status;
		}
	}

	if(selected)
	{
		// Set selection borders
		int first = 0;
		int last  = (screen->rows-1)*screen->cols;
		for(pos = 0;pos < screen->cols;pos++)
		{
			screen->content[first++].status |= SELECTION_BOX_TOP;
			screen->content[last++].status |= SELECTION_BOX_BOTTOM;
		}

		first = 0;
		last = screen->cols-1;

		for(pos = 0;pos < screen->rows;pos++)
		{
			screen->content[first].status |= SELECTION_BOX_LEFT;
			screen->content[last].status |= SELECTION_BOX_RIGHT;
			first += screen->cols;
			last += screen->cols;
		}
	}

	if(valid_terminal_window() && start >= 0)
	{
		GdkRectangle r;
		cairo_t *cr = get_terminal_cairo_context();
		draw_region(cr,start,end,color,&r);
		cairo_destroy(cr);
		gtk_widget_queue_draw_area(terminal,view.left,view.top,view.cols*fontWidth,view.rows*terminal_font_info.spacing);
	}
 }

 static int DecodePosition(long x, long y, int *row, int *col)
 {
 	int rc = 0;

 	if(x < view.left)
 	{
 		*col = 0;
 		rc = -1;
 	}
	else if(x > (view.left+(view.cols * fontWidth)))
	{
		*col = view.cols-1;
		rc = -1;
	}
	else
	{
		*col = ((((unsigned long) x) - view.left)/fontWidth);
	}

	if(y < view.top)
	{
		*row = 0;
		rc = -1;
	}
	else if(y > (view.top+(view.rows * terminal_font_info.spacing)))
	{
		*row = view.rows-1;
		rc = -1;
	}
	else
	{
		*row = ((((unsigned long) y) - view.top)/terminal_font_info.spacing);
	}

//	Trace("%s x=%ld y=%ld r=%d c=%d",__FUNCTION__,x,y,*row,*col);

	return rc;
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

		switch(drag_type)
		{
		case DRAG_TYPE_NONE:
			if(select_mode == SELECT_MODE_WORD)
			{
				int r,c;

				if(DecodePosition(event->x,event->y,&r,&c))
				{
					unselect();
				}
				else if(screen->content[(r*screen->cols)+c].status & ELEMENT_STATUS_SELECTED)
				{
					SetSelectionMode(SELECT_MODE_FIELD);
				}
				else
				{
					startRow = r;
					startCol = r;
					button_flags |= BUTTON_FLAG_COMBO;
					unselect();
				}
			}
			else
			{
				Trace("Single click - mode = %d (Unselect)",select_mode);
				unselect();

				if(!DecodePosition(event->x,event->y,&startRow,&startCol))
					button_flags |= BUTTON_FLAG_COMBO;
			}
			break;

		case DRAG_TYPE_INSIDE:
			SetSelectionMode(SELECT_MODE_DRAG);
			DecodePosition(event->x,event->y,&dragRow,&dragCol);
			dragRow -= startRow;
			dragCol -= startCol;
			Trace("Selection mode Drag: %d (Position: %d,%d)",SELECT_MODE_DRAG,dragRow,dragCol);
			break;

		default:
			Trace("Selection mode Drag: %d",SELECT_MODE_DRAG);
			SetSelectionMode(SELECT_MODE_DRAG);

		}
		break;

	case ((GDK_2BUTTON_PRESS & 0x0F) << 4) | 1 | BUTTON_FLAG_COMBO:
		DecodePosition(event->x,event->y,&startRow,&startCol);
		SetSelectionMode(SELECT_MODE_WORD);
		Trace("Button 1 double-clicked at %ld,%ld (%d,%d)",(long) event->x, (long) event->y,startRow,startCol);
		break;

	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 3:
		w = GTK_WIDGET(popup_menu[(select_mode == SELECT_MODE_NONE) ? POPUP_MENU_DEFAULT : POPUP_MENU_SELECTION]);
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
		SetSelectionMode(SELECT_MODE_APPEND);
		break;

	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 3 | BUTTON_FLAG_COMBO:
		Trace("Button 2 clicked in combo mode at %ld,%ld",(long) event->x, (long) event->y);
		SetSelectionMode(SELECT_MODE_COPY);
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
	int row = 0;
	int col = 0;

	DecodePosition(event->x,event->y,&row,&col);

	Trace("Button %d release Action: %d",event->button, (int) ( ((select_mode & 0x0F) << 4) | (event->button & 0x0F)));

	switch( ((select_mode & 0x0F) << 4) | (event->button & 0x0F))
 	{
	case ((SELECT_MODE_NONE & 0x0F) << 4) | 1: // Single click on button 1
		Trace("Single click (button: %d)",event->button);
		unselect();
		if(row >= 0 && row <= view.rows && col >= 0 && col <= view.cols)
			cursor_move((row*view.cols)+col);
		break;

	case ((SELECT_MODE_NONE & 0x0F) << 4) | 2: // Single click on button 2
		Trace("Single click (button: %d)",event->button);
		action_paste(0);
		break;

	case ((SELECT_MODE_FIELD & 0x0F) << 4) | 1: // Single click on button 1
		Trace("Single click (button: %d)",event->button);
		action_selectfield(0);
		break;

	case ((SELECT_MODE_WORD & 0x0F) << 4) | 1:	// Double click, select word
		Trace("Selecting word (button: %d)",event->button);
		action_selectword(0);
		break;

	case ((SELECT_MODE_COPY & 0x0F) << 4) | 1:
		Trace("Copy text (button: %d)",event->button);
		action_copy(0);
		unselect();
		break;

	case ((SELECT_MODE_APPEND & 0x0F) << 4) | 1:
		Trace("Append text (button: %d)",event->button);
		action_append(0);
		unselect();
		break;

	case ((SELECT_MODE_DRAG & 0x0F) << 4) | 1: // Left Drag
		select_mode &= ~SELECT_MODE_DRAG;
		select_mode |= SELECT_MODE_RECTANGLE;
		SetDragType(DRAG_TYPE_NONE);
		Trace("Ending selection drag (Button: %d New mode: %d)",event->button,select_mode);
		break;

	case ((SELECT_MODE_RECTANGLE & 0x0F) << 4) | 1: // End rectangle select
		break;

#ifdef DEBUG
	default:
		Trace("Unexpected action %04x mode: %d",((select_mode & 0x0F) << 4) | (event->button & 0x0F),select_mode);
#endif
 	}

	if(event->button == 1)
		button_flags &= ~BUTTON_FLAG_COMBO;

    return 0;
 }

 static void UpdateSelectedRectangle(void)
 {
 	int		  		row,col;
	int		  		pos		= 0;
 	int		  		left;
 	int		  		right;
 	int		  		top;
 	int		  		bottom;

//	if(!(select_mode && screen))
//		return;

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
		right = startCol;
	}
	else
	{
		left = startCol;
		right = endCol;
	}

	for(row = 0; row < screen->rows;row++)
	{
		int scol	= -1;
		int ecol	= -1;
		int saddr	= -1;
		int eaddr	= -1;

		for(col = 0; col < screen->cols;col++)
		{
			unsigned char status = screen->content[pos].status & ELEMENT_STATUS_FIELD_MASK;

			if(col >= left && col <= right && row >= top && row <= bottom)
			{
				status |= ELEMENT_STATUS_SELECTED;

				if(col == left)
					status |= SELECTION_BOX_LEFT;

				if(col == right)
					status |= SELECTION_BOX_RIGHT;

				if(row == top)
					status |= SELECTION_BOX_TOP;

				if(row == bottom)
					status |= SELECTION_BOX_BOTTOM;

			}

			if(screen->content[pos].status != status)
			{
				screen->content[pos].status = status;

				if(saddr < 0)
				{
					saddr = pos;
					scol  = col;
				}
				eaddr = pos;
				ecol  = col;
			}

			pos++;
		}

		if(saddr >= 0)
		{
			GdkRectangle r;
			cairo_t *cr = get_terminal_cairo_context();
			draw_region(cr,saddr,eaddr,color,&r);
			cairo_destroy(cr);
		}
	}

	cursor_move((bottom*view.cols)+right);

	if(valid_terminal_window())
		gtk_widget_queue_draw_area(terminal,view.left,view.top,view.cols*fontWidth,view.rows*terminal_font_info.spacing);

 }

 static void UpdateSelectedText(void)
 {
 	UpdateSelectedRegion((startRow * screen->cols)+startCol,(endRow * screen->cols)+endCol);
 }

 static void UpdateSelectedRegion(int bstart, int bend)
 {
 	int pos		= 0;
 	int row,col;

 	if(bstart > bend)
 	{
		int temp = bstart;
		bstart = bend;
		bend = temp;
 	}

	for(row = 0; row < screen->rows;row++)
	{
		int saddr = -1;
		int eaddr = -1;

		for(col = 0; col < screen->cols;col++)
		{
			unsigned char status = screen->content[pos].status & ELEMENT_STATUS_FIELD_MASK;

			if (pos >= bstart && pos <= bend)
			{
				status |= ELEMENT_STATUS_SELECTED;

				if(!(row && (screen->content[pos-screen->cols].status) & ELEMENT_STATUS_SELECTED))
					status |= SELECTION_BOX_TOP;

				if(!(pos && col && (screen->content[pos-1].status) & ELEMENT_STATUS_SELECTED))
					status |= SELECTION_BOX_LEFT;

				if(pos+1 > bend || col == (screen->cols-1))
					status |= SELECTION_BOX_RIGHT;

				if((pos+screen->cols) > bend)
					status |= SELECTION_BOX_BOTTOM;
			}

			if(screen->content[pos].status != status)
			{
				screen->content[pos].status = status;
				if(saddr < 0)
					saddr = pos;
				eaddr = pos;
			}
			pos++;
		}

		if(saddr >= 0)
		{
			GdkRectangle r;
			cairo_t *cr = get_terminal_cairo_context();
			draw_region(cr,saddr,eaddr,color,&r);
			cairo_destroy(cr);
		}

	}

	if(valid_terminal_window())
		gtk_widget_queue_draw_area(terminal,view.left,view.top,view.cols*fontWidth,view.rows*terminal_font_info.spacing);

 }

 void set_rectangle_select(H3270 *session, int value, LIB3270_TOGGLE_TYPE reason)
 {
 	if(select_mode != SELECT_MODE_RECTANGLE && select_mode != SELECT_MODE_TEXT)
		return;

	unselect();

	if(value)
	{
		SetSelectionMode(SELECT_MODE_RECTANGLE);
		UpdateSelectedRectangle();
	}
	else
	{
		SetSelectionMode(SELECT_MODE_TEXT);
		UpdateSelectedText();
	}

 }

 void reselect(void)
 {
	SetSelectionMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
	if(select_mode == SELECT_MODE_TEXT)
		UpdateSelectedText();
	else
		UpdateSelectedRectangle();
 }

 static void SetDragType(int type)
 {
 	if(drag_type == type)
		return;

 	drag_type = type;

#ifdef MOUSE_POINTER_CHANGE
 	if(terminal && terminal->window)
 	{
 		Trace("Type: %d",type);

 		if(type >= 0)
			gdk_window_set_cursor(terminal->window,wCursor[LIB3270_CURSOR_USER+type]);
 		else
			gdk_window_set_cursor(terminal->window,wCursor[cursor_mode]);
 	}
#endif

 }

 gboolean mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
 {
	int row = 0;
	int col = 0;

	if(button_flags & BUTTON_FLAG_COMBO)
	{
		// Moving with button 1 pressed, update selection
		switch( ((int) select_mode) )
		{
		case SELECT_MODE_NONE:	// Start selection
			SetSelectionMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
			return mouse_motion(widget,event,user_data); // Recursive call to update selection box

		case SELECT_MODE_RECTANGLE:
			DecodePosition(event->x,event->y,&endRow,&endCol);
			UpdateSelectedRectangle();
			break;

		case SELECT_MODE_TEXT:
			DecodePosition(event->x,event->y,&endRow,&endCol);
			UpdateSelectedText();
			break;

		}
	}
	else if(select_mode == SELECT_MODE_RECTANGLE)
	{
		int row = 0;
		int col = 0;

		if(DecodePosition(event->x,event->y,&row,&col))
		{
			SetDragType(DRAG_TYPE_NONE);
		}
		else if(row < startRow || row > endRow || col < startCol || col > endCol)
		{
			SetDragType(DRAG_TYPE_NONE);
		}
		else if(row == startRow && col == startCol)
		{
			SetDragType(DRAG_TYPE_TOP_LEFT);
		}
		else if(row == startRow && col == endCol)
		{
			SetDragType(DRAG_TYPE_TOP_RIGHT);
		}
		else if(row == startRow)
		{
			SetDragType(DRAG_TYPE_TOP);
		}
		else if(row == endRow && col == startCol)
		{
			SetDragType(DRAG_TYPE_BOTTOM_LEFT);
		}
		else if(row == endRow && col == endCol)
		{
			SetDragType(DRAG_TYPE_BOTTOM_RIGHT);
		}
		else if(row == endRow)
		{
			SetDragType(DRAG_TYPE_BOTTOM);
		}
		else if(col == startCol)
		{
			SetDragType(DRAG_TYPE_LEFT);
		}
		else if(col == endCol)
		{
			SetDragType(DRAG_TYPE_RIGHT);
		}
		else if(col >= startCol && col <= endCol && row >= startRow && row <= endRow)
		{
			SetDragType(DRAG_TYPE_INSIDE);
		}
		else
		{
			SetDragType(DRAG_TYPE_NONE);
		}

	}
	else if(select_mode == SELECT_MODE_DRAG)
	{
		int r;
		int c;

		DecodePosition(event->x,event->y,&row,&col);

//		Trace("Drag_type: %d Position: %d,%d",drag_type,row,col);

		switch(drag_type)
		{
		case DRAG_TYPE_TOP_LEFT:
			if(row <= endRow)
				startRow = row;
			if(col <= endCol)
				startCol = col;
			break;

		case DRAG_TYPE_TOP_RIGHT:
			if(row <= endRow)
				startRow = row;
			if(col >= startCol)
				endCol = col;
			break;

		case DRAG_TYPE_TOP:
			if(row <= endRow)
				startRow = row;
			break;

		case DRAG_TYPE_BOTTOM_LEFT:
			if(row >= startRow)
				endRow = row;
			if(col <= endCol)
				startCol = col;
			break;

		case DRAG_TYPE_BOTTOM_RIGHT:
			if(row >= startRow)
				endRow = row;
			if(col >= startCol)
				endCol = col;
			break;

		case DRAG_TYPE_BOTTOM:
			if(row >= startRow)
				endRow = row;
			break;

		case DRAG_TYPE_LEFT:
			if(col <= endCol)
				startCol = col;
			break;

		case DRAG_TYPE_RIGHT:
			if(col >= startCol)
				endCol = col;
			break;

		case DRAG_TYPE_INSIDE:

			r = endRow - startRow;
			c = endCol - startCol;

			startRow = row-dragRow;

			// Get new row
			if(startRow < 0)
				startRow = 0;
			endRow = startRow + r;
			if(endRow >= (view.rows-1))
			{
				endRow = (view.rows-1);
				startRow = endRow - r;
			}

			// Get new col
			startCol = col-dragCol;
			if(startCol < 0)
				startCol = 0;
			endCol = startCol + c;

			if(endCol >= (view.cols-1))
			{
				endCol = (view.cols-1);
				startCol = endCol - c;
			}

			break;
		}

		UpdateSelectedRectangle();

	}

    return 0;
 }

 void unselect(void)
 {
 	if(select_mode == SELECT_MODE_NONE)
		return;

 	SetDragType(DRAG_TYPE_NONE);
	SetSelection(FALSE);
	SetSelectionMode(SELECT_MODE_NONE);
 }

 PW3270_ACTION( selectall )
 {
 	SetSelection(TRUE);
	SetSelectionMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);
 }

 static void doSelect(H3270 *hSession, int (*call)(void))
 {
 	int row;
 	int col;

 	if(select_mode == SELECT_MODE_NONE)
 	{
 		SetSelectionMode(Toggled(RECTANGLE_SELECT) ? SELECT_MODE_RECTANGLE : SELECT_MODE_TEXT);

 		startRow = endRow = (cursor_position / screen->cols);
 		startCol = endCol = (cursor_position % screen->cols);
 	}

 	call();

 	row = cursor_position / screen->cols;
 	col = cursor_position % screen->cols;

	endRow = row;
	endCol = col;

	reselect();
 }

 PW3270_ACTION( selectleft )
 {
 	doSelect(hSession,lib3270_cursor_left);
 }

 PW3270_ACTION( selectright )
 {
 	doSelect(hSession,lib3270_cursor_right);
 }

 PW3270_ACTION( selectup )
 {
 	doSelect(hSession,lib3270_cursor_up);
 }

 PW3270_ACTION( selectdown )
 {
 	doSelect(hSession,lib3270_cursor_down);
 }

 static void MoveSelection(int row, int col)
 {
 	if(select_mode == SELECT_MODE_NONE)
		return;

	startRow 	+= row;
	endRow 		+= row;
	startCol 	+= col;
	endCol 		+= col;

	reselect();

 }

 PW3270_ACTION( selectionup )
 {
	if(startRow > 0 && endRow > 0)
		MoveSelection(-1,0);
 }

 PW3270_ACTION( selectiondown )
 {
 	int maxrow = screen->rows-1;

	if(startRow < maxrow && endRow < maxrow)
		MoveSelection(1,0);
 }

 PW3270_ACTION( selectionleft )
 {
	if(startCol > 0 && endCol > 0)
		MoveSelection(0,-1);
 }

 PW3270_ACTION( selectionright )
 {
 	int maxcol = screen->cols-1;

	if(startCol < maxcol && endCol < maxcol)
		MoveSelection(0,1);
 }

 gboolean mouse_enter(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
 {
 	return 0;
 }



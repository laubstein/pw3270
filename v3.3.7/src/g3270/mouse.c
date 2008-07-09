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
 #include <lib3270/screenc.h>

/*---[ Defines ]--------------------------------------------------------------*/

 #define DecodePosition(event,row,col)	col = ( (((unsigned long) event->x) - left_margin)/fWidth ); \
										row = ( (((unsigned long) event->y) - top_margin)/fHeight );


/*---[ Prototipes ]-----------------------------------------------------------*/

/*---[ Constants ]------------------------------------------------------------*/

/*---[ Statics ]--------------------------------------------------------------*/

 int startRow 	= -1;
 int startCol 	= -1;
 int mode		= 0;

/*---[ Globals ]--------------------------------------------------------------*/

 gboolean WaitingForChanges = TRUE;

/*---[ Implement ]------------------------------------------------------------*/

 static void ClearSelection(void)
 {
 	if(!mode)
		return;

	// Clear selection box, invalidate drawing area.


	mode = 0;
 }

 gboolean mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
 	ClearSelection();
 	DecodePosition(event,startRow,startCol);
 	Trace("Mouse press at %ld,%ld (%d,%d)",(long) event->x, (long) event->y,startRow,startCol);
 	return 0;
 }

 gboolean mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
	int row,col;

	DecodePosition(event,row,col);

 	switch(mode)
 	{
	case 0:	// Single click, just move cursor
		cursor_move((row*terminal_rows)+col);
		break;


 	}

    return 0;
 }

 gboolean mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
 {
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


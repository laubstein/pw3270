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

#include "g3270.h"
#include <malloc.h>

/*---[ Structures ]----------------------------------------------------------------------------------------*/

 typedef struct _element
 {
 	int ch;
 	int attr;
 } ELEMENT;

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

 static void title(char *text);
 static void setsize(int rows, int cols);
 static void addch(int row, int col, int c, int attr);

/*---[ Globals ]-------------------------------------------------------------------------------------------*/

 const struct lib3270_screen_callbacks g3270_screen_callbacks =
 {
	sizeof(struct lib3270_screen_callbacks),

	setsize,	// void (*setsize)(int rows, int cols);
	addch,		// void (*addch)(int row, int col, int c, int attr);
	NULL,		// void (*charset)(char *dcs);
	title,		// void (*title)(char *text);
	NULL,		// void (*changed)(int bstart, int bend);
	NULL,		// void (*ring_bell)(void);
	NULL,		// void (*redraw)(void);
	NULL,		// void (*refresh)(void);
	NULL,		// void (*suspend)(void);
	NULL,		// void (*resume)(void);

 };

 static int 		terminal_rows	= 0;
 static int 		terminal_cols	= 0;
 static ELEMENT	*screen			= NULL;

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 static void title(char *text)
 {
 	Trace("Window %p title: \"%s\"",topwindow,text);
 	if(topwindow)
		gtk_window_set_title(GTK_WINDOW(topwindow),text);

 }

 static void setsize(int rows, int cols)
 {
 	if(screen)
 	{
		g_free(screen);
		screen = NULL;
 	}

	if(rows && cols)
	{
		screen = g_malloc(sizeof(ELEMENT) *(rows*cols));
		terminal_rows = rows;
		terminal_cols = cols;
	}

 	Trace("Terminal set to %dx%d, screen set to %p",rows,cols,screen);

 }

 static void addch(int row, int col, int c, int attr)
 {
 	ELEMENT *el;

 	if(!screen)
		return;

 	el = screen + (row*terminal_cols)+col;

	el->ch = c;
	el->attr = attr;

	// TODO (perry#1#): Update pixmap, queue screen redraw.

 }

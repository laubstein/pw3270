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
 #include <globals.h>
 #include <lib3270/kybdc.h>
 #include <string.h>

 #define DEFAULT_CLIPBOARD GDK_NONE

/*---[ Statics ]------------------------------------------------------------------------------------------------*/

 static gchar *contents = NULL;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 gchar * GetClipboard(void)
 {
	return g_strdup(contents ? contents : "");
 }

 static void setClipboardcontents(void)
 {
 	if(!(contents && terminal))
		return;

	Trace("Clipboard set to:\n%s",contents);
	gtk_clipboard_set_text(gtk_widget_get_clipboard(topwindow,DEFAULT_CLIPBOARD),contents,-1);
 }

 void action_Copy(GtkWidget *w, gpointer user_data)
 {
 	if(contents)
		g_free(contents);
	contents = GetSelection();
	setClipboardcontents();
 }

 void action_Append(GtkWidget *w, gpointer user_data)
 {
 	gchar *sel;
 	gchar *last = contents;

 	if(!contents)
 	{
		action_Copy(w,user_data);
		return;
 	}

	sel = GetSelection();
	if(!sel)
		return;

	contents = g_strconcat(last,sel,NULL);

	g_free(sel);
	g_free(last);

	setClipboardcontents();
 }

 static void paste_string(gchar *str)
 {
 	int remaining;

 	if(!str)
		return;

 	if(contents)
 	{
		g_free(contents);
		contents = NULL;
 	}

	Trace("Paste buffer:\n%s",str);

	remaining = emulate_input(str,-1,True);
    if(remaining < 1)
		return;

	// Save the rest of the string in the contents buffer
	contents = g_convert(str+remaining, -1, "UTF-8", CHARSET, NULL, NULL, NULL);

 }

 static void clipboard_text_received(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
 	gchar *buffer;
 	gchar *ptr;

 	if(!text)
		return;

	buffer = g_convert(text, -1, CHARSET, "UTF-8", NULL, NULL, NULL);

    if(!buffer)
    {
    	// TODO (perry#3#): Notify user!
        Log("Error converting clipboard string to charset %s",CHARSET);
        return;
    }

    /* Remove TABS */
    for(ptr = buffer;*ptr;ptr++)
    {
		if(*ptr == '\t')
			*ptr = ' ';
    }

	paste_string(buffer);

	g_free(buffer);

 }

 void action_Paste(GtkWidget *w, gpointer user_data)
 {
	gtk_clipboard_request_text(gtk_widget_get_clipboard(topwindow,DEFAULT_CLIPBOARD),clipboard_text_received,(gpointer) 0);
 }

 void action_PasteNext(GtkWidget *w, gpointer user_data)
 {
 	if(contents)
		paste_string(contents);
 }




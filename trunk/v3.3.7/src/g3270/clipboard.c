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
 	{
		gtk_action_group_set_sensitive(clipboard_actions,FALSE);
		return;
 	}

	Trace("Clipboard set to:\n%s",contents);
	gtk_clipboard_set_text(gtk_widget_get_clipboard(topwindow,GDK_NONE),contents,-1);
	gtk_action_group_set_sensitive(clipboard_actions,TRUE);

 }

 void action_Copy(void)
 {
 	if(contents)
		g_free(contents);
	contents = GetSelection();
	setClipboardcontents();
 }

 void action_Append(void)
 {
 	gchar *sel;
 	gchar *last = contents;

 	if(!last)
 	{
		action_Copy();
		return;
 	}

	sel = GetSelection();
	if(!sel)
		return;

	contents = g_strconcat(last,"\n",sel,NULL);

	g_free(sel);
	g_free(last);

	setClipboardcontents();
 }

 static void paste_string(gchar *str)
 {
 	int			remaining = -1;
 	gchar		*saved;

 	if(!str)
 	{
 		g_free(contents);
 		contents = NULL;
		return;
 	}

	if(TOGGLED_SMART_PASTE)
	{
		int 	addr = cursor_get_addr();
		int		max  = ((terminal_rows-1)*terminal_cols);
		char	buffer[2];

		remaining = strlen(str);

		while(remaining > 0 && addr < max)
		{
			screen_read(buffer, addr, 1);
			if(*buffer != *str)
			{
				// Changed, move and insert
				cursor_set_addr(addr);
				if(emulate_input(str,1,True) < 0)
					remaining = 0;
			}
			remaining--;
			addr++;
			str++;
		}

	}
	else
	{
		Trace("Paste buffer:\n%s",str);
		remaining = emulate_input(str,-1,True);
	}

	saved = contents;
    if(remaining > 0)
		contents = g_strdup(str+(strlen(str)-(remaining+1)));
	else
		contents = NULL;

	gtk_action_set_sensitive(gtk_action_group_get_action(online_actions,"PasteNext"),contents != NULL);

	g_free(saved);

	screen_disp();
 }

 static void process_text_received(const gchar *text)
 {
 	gchar *buffer;
 	gchar *ptr;

 	if(!text)
		return;

	buffer = g_convert(text, -1, CHARSET, "UTF-8", NULL, NULL, NULL);

    if(!buffer)
    {
    	// TODO (perry#3#): Notify user!
        PopupAnError( N_( "Error converting clipboard string to charset %s" ),CHARSET);
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

 static void clipboard_text_received(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
	process_text_received(text);
 }

#ifdef USE_PRIMARY_SELECTION
static void primary_text_received(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
 	if(!text)
 	{
		Trace("Primary clipboard is empty, requesting default %p",clipboard);
 		gtk_clipboard_request_text(gtk_widget_get_clipboard(topwindow,GDK_NONE),clipboard_text_received,(gpointer) 0);
 		return;
 	}
	Trace("Pasting primary selection %p",clipboard);
	process_text_received(text);
 }
#endif

 void action_PasteTextFile(void)
 {
	GKeyFile 	*conf	= GetConf();
 	gchar		*ptr;
	GtkWidget 	*dialog = gtk_file_chooser_dialog_new( _( "Paste text file" ),
														 GTK_WINDOW(topwindow),
														 GTK_FILE_CHOOSER_ACTION_OPEN,
														 GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														 GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
														 NULL );


	ptr = g_key_file_get_string(conf,"uri","PasteTextFile",NULL);
	if(ptr)
	{
		gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(dialog),ptr);
		g_free(ptr);
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GError		*error = NULL;
		gchar		*filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gsize		sz = 0;
		gchar		*buffer;

		gtk_widget_set_sensitive(dialog,FALSE);
		gtk_widget_set_sensitive(topwindow,FALSE);

		g_key_file_set_string(conf,"uri","PasteTextFile",gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog)));

		Trace("Loading %s",filename);

		if(!g_file_get_contents(filename, &buffer, &sz, &error))
		{
			PopupAnError( N_( "Error loading %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		else
		{
			process_text_received(buffer);
		}

		g_free(buffer);

		gtk_widget_set_sensitive(topwindow,TRUE);

	}

	gtk_widget_destroy(dialog);

 }

 void action_Paste(void)
 {
	gtk_clipboard_request_text(gtk_widget_get_clipboard(topwindow,GDK_NONE),clipboard_text_received,(gpointer) 0);
 }

 void action_PasteSelection(void)
 {
#ifdef USE_PRIMARY_SELECTION
	gtk_clipboard_request_text(gtk_widget_get_clipboard(topwindow,GDK_SELECTION_PRIMARY),primary_text_received,(gpointer) 0);
#else
	action_PasteNext();
#endif
 }

 void action_PasteNext(void)
 {
 	if(contents)
		paste_string(contents);
	else
		action_Paste();
 }

 void ClearClipboard(void)
 {
	gtk_action_group_set_sensitive(clipboard_actions,FALSE);
 	if(contents)
 	{
 		g_free(contents);
 		contents = NULL;
 	}
 }



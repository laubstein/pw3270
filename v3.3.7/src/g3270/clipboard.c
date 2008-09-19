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

 static gchar 			*contents = NULL;
// static const gchar	*clipboard_actions[] = { "SaveClipboard", "PrintClipboard" };

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
	gtk_clipboard_set_text(gtk_widget_get_clipboard(topwindow,DEFAULT_CLIPBOARD),contents,-1);
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
 	int			remaining;
 	gchar		*saved;

 	if(!str)
 	{
 		g_free(contents);
 		contents = NULL;
		return;
 	}

	Trace("Paste buffer:\n%s",str);

	remaining = emulate_input(str,-1,True);

	saved = contents;
    if(remaining > 0)
		contents = g_strdup(str+(strlen(str)-(remaining+1)));
	else
		contents = NULL;

	g_free(saved);

	screen_disp();
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
		gsize		sz;
		gchar		*buffer;

		g_key_file_set_string(conf,"uri","PasteTextFile",gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog)));

		if(!g_file_get_contents(filename, &buffer, &sz, &error))
		{
			PopupAnError( N_( "Error loading %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		else
		{
			paste_string(buffer);
		}

		g_free(buffer);
	}

	gtk_widget_destroy(dialog);

 }

 void action_Paste(void)
 {
	gtk_clipboard_request_text(gtk_widget_get_clipboard(topwindow,DEFAULT_CLIPBOARD),clipboard_text_received,(gpointer) 0);
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



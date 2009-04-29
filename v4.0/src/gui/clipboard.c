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
 * Este programa está nomeado como clipboard.c e possui 285 linhas de código.
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
 #include <globals.h>
 #include <lib3270/kybdc.h>
 #include <string.h>

/*---[ Statics ]------------------------------------------------------------------------------------------------*/

 static gchar *clipboard_data = NULL;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 gchar * GetClipboard(void)
 {
	return g_strdup(clipboard_data ? clipboard_data : "");
 }

 static void setClipboardcontents(gchar *new_data)
 {
 	if(clipboard_data)
	{
		g_free(clipboard_data);
		clipboard_data = NULL;
	}

 	if(!(new_data && terminal))
 	{
		gtk_action_group_set_sensitive(clipboard_actions,FALSE);
		return;
 	}

	clipboard_data = new_data;

	gtk_clipboard_set_text(gtk_widget_get_clipboard(topwindow,GDK_NONE),clipboard_data,-1);
	gtk_action_group_set_sensitive(clipboard_actions,TRUE);

 }

 void action_Copy(void)
 {
	setClipboardcontents(GetSelection());
 }

 void action_Append(void)
 {
 	gchar *sel;

 	if(!clipboard_data)
 	{
		action_Copy();
		return;
 	}

	sel = GetSelection();
	if(!sel)
		return;

	setClipboardcontents(g_strconcat(clipboard_data,"\n",sel,NULL));
	g_free(sel);
 }

 static void paste_string(gchar *str)
 {
 	int			remaining = -1;
 	gchar		*saved;

 	if(!str)
 	{
 		g_free(clipboard_data);
 		clipboard_data = NULL;
		return;
 	}

	screen_suspend();

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

	saved = clipboard_data;
    if(remaining > 0)
		clipboard_data = g_strdup(str+(strlen(str)-(remaining+1)));
	else
		clipboard_data = NULL;

	gtk_action_set_sensitive(gtk_action_group_get_action(online_actions,"PasteNext"),clipboard_data != NULL);

	g_free(saved);

	screen_resume();

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
        Warning( N_( "Can't convert clipboard string to charset %s" ),CHARSET);
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
			Warning( N_( "Can't load %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
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
 	if(clipboard_data)
		paste_string(clipboard_data);
	else
		action_Paste();
 }

 void ClearClipboard(void)
 {
 	setClipboardcontents(NULL);
 }

 void action_CopyAsTable(void)
 {
	GdkRectangle 	rect;
	gboolean		*cols;
	int				row, col;
	ELEMENT			*el;
	GString			*buffer;

 	if(GetSelectedRectangle(&rect))
 	{
 		// FIXME (perry#3#): First check if the selection area isn't rectangular.
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "Invalid action" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Can't copy non rectangular area" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",_( "Activate rectangle select option and try again." ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
 		return;
 	}

 	// Find column delimiters
 	cols = g_malloc0(rect.width * sizeof(gboolean));
 	for(row=0;row < rect.height;row++)
 	{
		el = screen+(((row+rect.y) * terminal_cols)+rect.x);
		for(col = 0;col < rect.width;col++)
		{
			if(*el->ch && !g_ascii_isspace(*el->ch))
				cols[col] = TRUE;

			el++;
		}
 	}

	// Read screen contents
	buffer = g_string_sized_new(rect.height * rect.width);

 	for(row=0;row < rect.height;row++)
 	{
		el = screen+(((row+rect.y) * terminal_cols)+rect.x);
		col = 0;
		while(col < rect.width)
		{
			// if isn't the first column add column delimiter
			if(col)
				g_string_append_c(buffer,'\t');

			// Find column start
			while(!cols[col] && col < rect.width)
			{
				col++;
				el++;
			}

			// Copy column content
			while(cols[col] && col < rect.width)
			{
				g_string_append(buffer,*el->ch ? el->ch : " ");
				col++;
				el++;
			}

		}

		// Add row delimiter
		g_string_append_c(buffer,'\n');

 	}

	Trace("Tabela lida:\n%s\n",buffer->str);

	gtk_clipboard_set_text(gtk_widget_get_clipboard(topwindow,GDK_NONE),buffer->str,-1);

	g_string_free(buffer,TRUE);
 	g_free(cols);
 	setClipboardcontents(NULL);
 }


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
 #include <lib3270/toggle.h>
 #include <string.h>

/*---[ Statics ]------------------------------------------------------------------------------------------------*/

 static gchar *clipboard_string = NULL;

 static enum _clipboard_mode
 {
 	CLIPBOARD_MODE_NONE,	/**< No data in clipboard */
 	CLIPBOARD_MODE_TEXT,	/**< Clipboard contains a text string */
 	CLIPBOARD_MODE_TABLE,	/**< Clipboard contains a table */
 } clipboard_mode = CLIPBOARD_MODE_NONE;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 gchar * GetClipboard(void)
 {
 	if(!clipboard_string)
		return g_strdup("");

	return g_strdup(clipboard_string);
 }

 enum
 {
  TARGET_STRING,
  TARGET_TEXT,
  TARGET_COMPOUND_TEXT,
  TARGET_UTF8_STRING
 };

 static void clipboard_get(GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, gpointer data)
 {
 	Trace("%s called",__FUNCTION__);

 	if(clipboard_string)
 	{
 		gsize	sz;
 		gchar 	*utf_clipboard = g_convert_with_fallback(clipboard_string,-1,"UTF-8",CHARSET,"",NULL,&sz,NULL);

 		if(utf_clipboard)
 		{
			gtk_selection_data_set_text(selection_data, utf_clipboard,sz);
			g_free(utf_clipboard);
 		}
 	}
 }

 static void clipboard_clear(GtkClipboard *clipboard, gpointer data)
 {
 	Trace("%s called",__FUNCTION__);
 }

 static void UpdateClipboard(void)
 {
	static const GtkTargetEntry targets[] =
	{
		{ "STRING",			0, 	TARGET_STRING			},
		{ "TEXT",			0, 	TARGET_TEXT				},
		{ "COMPOUND_TEXT",	0,	TARGET_COMPOUND_TEXT	},
		{ "UTF8_STRING",	0,	TARGET_UTF8_STRING		}
	};

	Trace("%s called",__FUNCTION__);

 	if(clipboard_string)
 	{
		gtk_clipboard_set_with_data (	gtk_widget_get_clipboard(topwindow,GDK_NONE),
										targets, G_N_ELEMENTS (targets),
										clipboard_get, clipboard_clear,
										clipboard_string );


		set_action_group_sensitive_state(ACTION_GROUP_CLIPBOARD,TRUE);
 	}
 	else
 	{
 		clipboard_mode = CLIPBOARD_MODE_NONE;
		set_action_group_sensitive_state(ACTION_GROUP_CLIPBOARD,FALSE);
 	}
 }

 static void paste_string(const gchar *str)
 {
 	int 	sz;
 	gchar	*last_clipboard;

 	if(!str)
 	{
		ClearClipboard();
		return;
 	}

	Trace("Pasting %p with %d bytes)",str,strlen(str));
	sz = lib3270_paste_string((const unsigned char *) str);
	Trace("Paste returned %d (string has %d bytes)",sz,strlen(str));

	if(sz < 1)
	{
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_OK,
													"%s", _(  "Unable to paste clipboard string" ));

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Action failed" ) );
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s", sz == -EINVAL ? _( "Keyboard is locked" ) : _( "Unexpected error" ) );

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);


		return;
	}

	/* Update clipboard contents */
	last_clipboard = clipboard_string;

	if(strlen(str) > sz)
	{
		clipboard_mode		= CLIPBOARD_MODE_TEXT;
		clipboard_string	= g_strdup(str+sz);
	}
	else
	{
		clipboard_mode		= CLIPBOARD_MODE_NONE;
		clipboard_string	= NULL;
	}

	if(last_clipboard)
		g_free(last_clipboard);

	UpdateClipboard();
	set_action_sensitive_by_name("PasteNext",clipboard_string != NULL);

	screen_resume();

	Trace("%s - ends (clipboard_string=%p)",__FUNCTION__,clipboard_string);

 }

 static void process_text_received(const gchar *text, const gchar *encoding)
 {
 	gchar 	*buffer = NULL;
 	gchar 	*ptr;
 	GError	*error = NULL;

 	if(!text)
		return;

	buffer = g_convert(text, -1, CHARSET, encoding, NULL, NULL, &error);

 	Trace("%s buffer=%p error=%p",__FUNCTION__,buffer,error);

    if(!buffer)
    {
    	/* Falhou ao converter - Reajusta e tenta de novo ( Ver ticket #77 ) */
    	int f;

    	static const struct _xlat
    	{
    		const gchar *from;
    		const gchar *to;
    	} xlat[] =
    	{
    		{ "–",		"-"		},
    		{ "→",		"->"	},
    		{ "←",		"<-" 	},
    		{ "©",		"(c)"	},
    		{ "↔",		"<->"	},
    		{ "™",		"(TM)"	},
    		{ "®",		"(R)"	},
    		{ "“",		"\""	},
    		{ "”",		"\""	},
    		{ "…",		"..."	},
    	};

		gchar *string = g_strdup(text);

		if(error)
		{
			Trace("%s error=%p",__FUNCTION__,error);
			g_error_free(error);
			error = NULL;
		}

		// FIXME (perry#1#): Is there any better way for a "sed" here?
		for(f=0;f<G_N_ELEMENTS(xlat);f++)
		{
			gchar *ptr = g_strstr_len(string,-1,xlat[f].from);

			if(ptr)
			{
				gchar *old = string;
				gchar **tmp = g_strsplit(old,xlat[f].from,-1);
				string = g_strjoinv(xlat[f].to,tmp);
				g_strfreev(tmp);
				g_free(old);
			}
		}

		Trace("%s error=%p",__FUNCTION__,error);
		buffer = g_convert(string, -1, CHARSET, encoding, NULL, NULL, &error);

		if(!buffer)
		{
			gchar **ln = g_strsplit(string,"\n",-1);

			for(f=0;ln[f];f++)
			{
				gchar *str = g_convert(ln[f], -1, CHARSET, encoding, NULL, NULL, &error);

				if(error)
				{
					g_error_free(error);
					error = 0;
				}

				if(!str)
				{
					GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
																GTK_DIALOG_DESTROY_WITH_PARENT,
																GTK_MESSAGE_ERROR,
																GTK_BUTTONS_OK,
																_(  "Can't convert line %d from %s to %s" ),f+1, encoding, CHARSET);

					gtk_window_set_title(GTK_WINDOW(dialog), _( "Charset error" ) );
					if(error)
					{
						gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s\n%s", error->message ? error->message : N_( "Unexpected error" ), ln[f]);
						g_error_free(error);
						error = 0;
					}
					else
					{
						gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s", ln[f]);
					}
					gtk_dialog_run(GTK_DIALOG (dialog));
					gtk_widget_destroy(dialog);
					return;

				}
				else
				{
					g_free(str);
				}
			}

			g_strfreev(ln);
			g_free(string);
		}

		g_free(string);

		if(error)
		{
			g_error_free(error);
			error = 0;
		}


    	if(!buffer)
    	{
			GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
														GTK_DIALOG_DESTROY_WITH_PARENT,
														GTK_MESSAGE_ERROR,
														GTK_BUTTONS_OK,
														_(  "Can't convert text from %s to %s" ), encoding, CHARSET);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Charset error" ) );
			if(error)
			{
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s", error->message ? error->message : N_( "Unexpected error" ));
				g_error_free(error);
				error = 0;
			}
			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

			return;
    	}
    }

	if(error)
		g_error_free(error);

    /* Remove TABS */
    for(ptr = buffer;*ptr;ptr++)
    {
		if(*ptr == '\t')
			*ptr = ' ';
    }

	Trace("Received text:%p (%d bytes) Clipboard is %p\n",buffer,strlen(buffer),clipboard_string);

	paste_string(buffer);

	Trace("%s: Releasing buffer %p",__FUNCTION__,buffer);
	g_free(buffer);

	Trace("%s: ends",__FUNCTION__);
 }

#if defined( USE_SELECTIONS )
 void update_paste_action(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
	set_action_group_sensitive_state(ACTION_GROUP_PASTE,text ? TRUE : FALSE);
 }
#endif

 void clipboard_text_received(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
 	Trace("%s begins",__FUNCTION__);
	process_text_received(text,"UTF-8");
 	Trace("%s ends",__FUNCTION__);
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
	process_text_received(text,"UTF-8");
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

		buffer = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
		g_key_file_set_string(conf,"uri","PasteTextFile",buffer);
		g_free(buffer);

		Trace("Loading %s",filename);

		if(!g_file_get_contents(filename, &buffer, &sz, &error))
		{
			Warning( N_( "Can't load %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		else
		{
			const gchar *charset = NULL;
			g_get_charset(&charset);
			process_text_received(buffer,charset);
		}

		g_free(filename);
		g_free(buffer);

		gtk_widget_set_sensitive(topwindow,TRUE);

	}

	gtk_widget_destroy(dialog);

 }

 void action_Paste(void)
 {
 	Trace("%s begins",__FUNCTION__);
	gtk_clipboard_request_text(gtk_widget_get_clipboard(topwindow,GDK_NONE),clipboard_text_received,(gpointer) 0);
 	Trace("%s ends",__FUNCTION__);
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
 	Trace("%s begins",__FUNCTION__);
 	if(clipboard_string && *clipboard_string)
		paste_string(clipboard_string);
	else
		action_Paste();
 	Trace("%s ends",__FUNCTION__);
 }

 void ClearClipboard(void)
 {
	if(clipboard_string)
	{
		g_free(clipboard_string);
		clipboard_string = NULL;
	}
	UpdateClipboard();
	set_action_sensitive_by_name("PasteNext",FALSE);
 }

 static void CopyAsText(gboolean append)
 {
	gchar *last_clipboard = clipboard_string;

	if(append && last_clipboard)
	{
		gchar *ptr = GetSelection();
		clipboard_string = g_strconcat(last_clipboard,"\n",ptr,NULL);
		g_free(ptr);
	}
	else
	{
		clipboard_string = GetSelection();
	}

	if(last_clipboard)
		g_free(last_clipboard);

	clipboard_mode = CLIPBOARD_MODE_TEXT;
	UpdateClipboard();

 }

 static void CopyAsTable(gboolean append)
 {
	GdkRectangle 	rect;
	gboolean		*cols;
	int				row, col;
	ELEMENT			*el;
	GString			*buffer;
	gchar 			*last_clipboard;

	Trace("%s (append: %s mode: %d)",__FUNCTION__,append ? "Yes" : "No",(int) clipboard_mode);

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
		// Add row delimiter
		if(buffer->len > 0)
			g_string_append_c(buffer,'\n');

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
 	}

 	g_free(cols);

	Trace("Tabela lida:\n%s\n",buffer->str);

	/* Update clipboard contents */
	last_clipboard = clipboard_string;

	if(append && last_clipboard)
	{
		clipboard_string = g_strconcat(last_clipboard,"\n",buffer->str,NULL);
		g_string_free(buffer,TRUE);
	}
	else
	{
		clipboard_string = g_string_free(buffer,FALSE);
	}

	if(last_clipboard)
		g_free(last_clipboard);

	clipboard_mode = CLIPBOARD_MODE_TABLE;
	UpdateClipboard();

 }

 void action_CopyAsTable(void)
 {
 	CopyAsTable(FALSE);
 }

 void action_Copy(void)
 {
 	CopyAsText(FALSE);
 }

 void action_Append(void)
 {
 	if(clipboard_mode == CLIPBOARD_MODE_TABLE)
 		CopyAsTable(TRUE);
 	else
 		CopyAsText(TRUE);
 }



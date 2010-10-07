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
 #include "fonts.h"
 #include "actions.h"
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

 static void paste_string(const gchar *str)
 {
 	int 	sz;
 	gchar	*last_clipboard;

 	if(!str)
 	{
		ClearClipboard();
		return;
 	}

//	Trace("Pasting %p with %d bytes)",str,strlen(str));
	sz = lib3270_paste_string((const unsigned char *) str);
//	Trace("Paste returned %d (string has %d bytes)",sz,strlen(str));

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

	gtk_action_set_sensitive(action_by_id[ACTION_PASTENEXT],clipboard_string != NULL);

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
    		{ "•",		"*"		},
    		{ "․",		"."		},
    		{ "·",		"*"		},

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

 void clipboard_text_received(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
 	Trace("%s begins",__FUNCTION__);
	process_text_received(text,"UTF-8");
 	Trace("%s ends",__FUNCTION__);
 }

 PW3270_ACTION( pastetextfile )
 {
	GKeyFile 	*conf	= GetConf();
 	gchar		*ptr;
	GtkWidget 	*dialog = gtk_file_chooser_dialog_new( _( "Paste text file" ),
														 GTK_WINDOW(topwindow),
														 GTK_FILE_CHOOSER_ACTION_OPEN,
														 GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														 GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
														 NULL );

#ifdef DEBUG
	{
		static const struct _encoding
		{
			const gchar *descr;
			const gchar *name;
		} encoding[] =
		{
			{ N_( "Unicode" ),						"UTF-8"		},
			{ N_( "Windows Latin-1 "),				"CP1252"	},
			{ N_( "Latin Alphabet No. 1" ),			"iso8859-1"	},
		};

		const gchar 	*syschr		= NULL;
		GtkWidget 		*expander	= gtk_expander_new(_( "Text encoding") );
		GtkWidget		*frame		= gtk_scrolled_window_new(NULL,NULL);
		GtkTreeModel	*model 		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
		GtkWidget		*view		= gtk_tree_view_new_with_model(model);
		int				 f;

		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view),FALSE);
		gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW(view), -1, "0", gtk_cell_renderer_text_new(), "text", 0, NULL);
		gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW(view), -1, "1", gtk_cell_renderer_text_new(), "text", 1, NULL);

		g_get_charset(&syschr);
		if(syschr)
		{
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter,  0, _( "System default" ), 1, syschr, -1);
			gtk_tree_selection_select_iter(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)),&iter);
		}

		for(f=0;f<G_N_ELEMENTS(encoding);f++)
		{
			if(!syschr || strcmp(syschr,encoding[f].name))
			{
				GtkTreeIter iter;
				gtk_list_store_append((GtkListStore *) model,&iter);
				gtk_list_store_set((GtkListStore *) model, &iter,  0, gettext(encoding[f].descr), 1, encoding[f].name, -1);
			}
		}

		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(frame),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(frame),view);
        gtk_container_add(GTK_CONTAINER(expander),frame);

		gtk_widget_show_all(expander);
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog),expander);

	}
#endif

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
			GtkWidget *dialog;

			// Can't parse UI definition, notify user
			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_ERROR,
												GTK_BUTTONS_CLOSE,
												_(  "Can't load %s" ), filename);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Paste error" ) );

			if(error->message)
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s",error->message);

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

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

#ifndef WIN32
 static void clipboard_text_check(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
 	Trace("%s called with text=%p",__FUNCTION__,text);
	action_group_set_sensitive(ACTION_GROUP_PASTE,text != NULL);
 }
#endif

 void check_clipboard_contents(void)
 {
#ifdef WIN32
	action_group_set_sensitive(ACTION_GROUP_PASTE,TRUE);

#else
	action_group_set_sensitive(ACTION_GROUP_PASTE,FALSE);
	gtk_clipboard_request_text(gtk_widget_get_clipboard(topwindow,GDK_SELECTION_CLIPBOARD),clipboard_text_check,(gpointer) 0);
#endif
 }

 PW3270_ACTION( paste )
 {
 	Trace("%s begins",__FUNCTION__);
	gtk_clipboard_request_text(gtk_widget_get_clipboard(topwindow,GDK_SELECTION_CLIPBOARD),clipboard_text_received,(gpointer) 0);
 	Trace("%s ends",__FUNCTION__);
 }

 PW3270_ACTION( pastenext )
 {
 	Trace("%s begins",__FUNCTION__);
 	if(clipboard_string && *clipboard_string)
		paste_string(clipboard_string);
	else
		action_paste(action);
 	Trace("%s ends",__FUNCTION__);
 }

 void ClearClipboard(void)
 {
	if(clipboard_string)
	{
		g_free(clipboard_string);
		clipboard_string = NULL;
	}
	gtk_action_set_sensitive(action_by_id[ACTION_PASTENEXT],FALSE);

 }

 static void set_clipboard_contents(enum _clipboard_mode mode, gboolean append, gchar *str)
 {
 	gchar	*last_clipboard = clipboard_string;
 	gsize	sz;

 	clipboard_mode = mode;

 	if(append && last_clipboard)
 	{
 		// Append

 		gchar *clipboard_utf	= g_convert_with_fallback(last_clipboard,-1,"UTF-8",CHARSET,"?",NULL,&sz,NULL);
		gchar *concat_utf		= g_strconcat(clipboard_utf,"\n",str,NULL);

 		g_free(clipboard_utf);

		gtk_clipboard_set_text(gtk_widget_get_clipboard(topwindow,GDK_SELECTION_CLIPBOARD),concat_utf ? concat_utf : "",-1);

		clipboard_string = g_convert_with_fallback(concat_utf,-1,CHARSET,"UTF-8","?",NULL,&sz,NULL);

 		g_free(concat_utf);

 	}
 	else
 	{
		clipboard_string = g_convert_with_fallback(str,-1,CHARSET,"UTF-8","?",NULL,&sz,NULL);
		gtk_clipboard_set_text(gtk_widget_get_clipboard(topwindow,GDK_SELECTION_CLIPBOARD),str ? str : "",-1);
 	}

	action_group_set_sensitive(ACTION_GROUP_PASTE,TRUE);

	if(last_clipboard)
		g_free(last_clipboard);

 }

 static void CopyAsText(gboolean append)
 {
 	gchar *str = GetSelection();
	set_clipboard_contents(CLIPBOARD_MODE_TEXT, append, str);
	g_free(str);
 }

 static void CopyAsTable(gboolean append)
 {
	GdkRectangle 	rect;
	gboolean		*cols;
	int				row, col;
	ELEMENT			*el;
	GString			*buffer;

	Trace("%s (append: %s mode: %d)",__FUNCTION__,append ? "Yes" : "No",(int) clipboard_mode);

 	if(get_selected_rectangle(&rect))
 		return;

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

	set_clipboard_contents(CLIPBOARD_MODE_TABLE, append, buffer->str);

	g_string_free(buffer,TRUE);

 }

 PW3270_ACTION( copyastable )
 {
 	CopyAsTable(FALSE);
 }

 PW3270_ACTION( copy )
 {
 	CopyAsText(FALSE);
 }

 PW3270_ACTION( copyasimage )
 {
 	GdkPixmap		* pix;
 	GdkPixbuf		* img;
	GdkRectangle	  rect;
	int				  width;
	int				  height;
	int				  r;
	int				  y;
	cairo_t			* cr;
	int				  baseline;
	int				  baddr;

 	if(get_selected_rectangle(&rect))
 		return;

	Trace("%s begins", __FUNCTION__);

	width  = terminal_font_info.width * rect.width;
	height = terminal_font_info.spacing * rect.height;

	pix = gdk_pixmap_new(GDK_DRAWABLE(get_terminal_pixmap()),width,height,-1);

	cr = gdk_cairo_create(pix);
	cairo_set_scaled_font(cr,terminal_font_info.font);

	baddr = rect.y * terminal_cols;
	baseline = terminal_font_info.ascent;
	y = 0;

	for(r=0;r < rect.height;r++)
	{
		int c;
		int x = 0;
		int addr = baddr + rect.x;

		for(c=0;c < rect.width;c++)
		{
			draw_element(cr,&terminal_font_info,x,y,baseline,addr+c,color);
			x += terminal_font_info.width;
		}

		baddr += terminal_cols;
		y += terminal_font_info.spacing;
		baseline += terminal_font_info.spacing;
	}

	cairo_destroy(cr);

	img = gdk_pixbuf_get_from_drawable(0, pix, gdk_drawable_get_colormap(get_terminal_pixmap()),0, 0, 0, 0, width, height);

	g_object_unref(pix);

	gtk_clipboard_set_image(gtk_widget_get_clipboard(topwindow,GDK_SELECTION_CLIPBOARD),img);

	g_object_unref(img);

	Trace("%s ends", __FUNCTION__);
 }


 PW3270_ACTION( append )
 {
 	if(clipboard_mode == CLIPBOARD_MODE_TABLE)
 		CopyAsTable(TRUE);
 	else
 		CopyAsText(TRUE);
 }



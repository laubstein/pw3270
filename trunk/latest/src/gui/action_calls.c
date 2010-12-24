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
 * Este programa está nomeado como action_calls.c e possui - linhas de código.
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

 #ifdef HAVE_MALLOC_H
	#include <malloc.h>
 #endif

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 PW3270_ACTION( quit )
 {
 	Trace("%s called, disconnecting",__FUNCTION__);
 	screen_updates_enabled = FALSE;
 	action_disconnect(0);
	gtk_main_quit();
 }

 PW3270_ACTION( sethostname )
 {
 	char			*hostname;
 	char			*ptr;
 	gboolean		again		= TRUE;
	char 			buffer[1024];
 	GtkTable		*table		= GTK_TABLE(gtk_table_new(2,4,FALSE));
 	GtkEntry		*host		= GTK_ENTRY(gtk_entry_new());
 	GtkEntry		*port		= GTK_ENTRY(gtk_entry_new());
 	GtkToggleButton	*checkbox	= GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label( _( "Secure connection" ) ));
 	GtkWidget 		*dialog 	= gtk_dialog_new_with_buttons(	_( "Select hostname" ),
																GTK_WINDOW(topwindow),
																GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
																GTK_STOCK_CONNECT,	GTK_RESPONSE_ACCEPT,
																GTK_STOCK_CANCEL,	GTK_RESPONSE_REJECT,
																NULL	);

	gtk_window_set_icon_name(GTK_WINDOW(dialog),GTK_STOCK_HOME);
	gtk_entry_set_max_length(host,0xFF);
	gtk_entry_set_width_chars(host,60);

	gtk_entry_set_max_length(port,6);
	gtk_entry_set_width_chars(port,7);

	gtk_table_attach(table,gtk_label_new( _( "Hostname:" ) ), 0,1,0,1,0,0,5,0);
	gtk_table_attach(table,GTK_WIDGET(host), 1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);

	gtk_table_attach(table,gtk_label_new( _( "Port:" ) ), 2,3,0,1,0,0,5,0);
	gtk_table_attach(table,GTK_WIDGET(port), 3,4,0,1,GTK_FILL,0,0,0);

	gtk_table_attach(table,GTK_WIDGET(checkbox), 1,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);

	gtk_container_set_border_width(GTK_CONTAINER(table),5);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),GTK_WIDGET(table),FALSE,FALSE,2);

	strncpy(hostname = buffer,GetString("Network","Hostname",""),1023);

#ifdef HAVE_LIBSSL
	if(!strncmp(hostname,"L:",2))
	{
		gtk_toggle_button_set_active(checkbox,TRUE);
		hostname += 2;
	}
#else
	gtk_toggle_button_set_active(checkbox,FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(checkbox),FALSE);
	if(!strncmp(hostname,"L:",2))
		hostname += 2;
#endif

	ptr = strchr(hostname,':');
	if(ptr)
	{
		*(ptr++) = 0;
		gtk_entry_set_text(port,ptr);
	}
	else
	{
		gtk_entry_set_text(port,GetString("Network","DefaultPort","23"));
	}

	gtk_entry_set_text(host,hostname);

	gtk_widget_show_all(GTK_WIDGET(table));

 	while(again)
 	{
 		gtk_widget_set_sensitive(dialog,TRUE);
 		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
 		{
		case GTK_RESPONSE_ACCEPT:

			gtk_widget_set_sensitive(dialog,FALSE);

			if(gtk_toggle_button_get_active(checkbox))
				strcpy(buffer,"L:");
			else
				*buffer = 0;

			strncat(buffer,gtk_entry_get_text(host),1023);
			strncat(buffer,":",1023);
			strncat(buffer,gtk_entry_get_text(port),1023);

			if(!host_connect(buffer,1))
			{
				// Connection OK
				again = FALSE;
				SetString("Network","Hostname",buffer);
			}
			break;

		case GTK_RESPONSE_REJECT:
			again = FALSE;
			break;
 		}
 	}

	gtk_widget_destroy(dialog);

 }

 PW3270_ACTION( connect )
 {
    const gchar *host;

 	Trace("%s Connected:%d",__FUNCTION__,PCONNECTED);

 	if(PCONNECTED)
 		return;

 	unselect();

	host = GetString("Network","Hostname",CN);

    if(host == CN)
    {
        action_sethostname(action);
    }
    else
    {
    	int rc;

		action_group_set_sensitive(ACTION_GROUP_ONLINE,FALSE);
		action_group_set_sensitive(ACTION_GROUP_OFFLINE,FALSE);

        gtk_widget_set_sensitive(topwindow,FALSE);
        RunPendingEvents(0);

		rc = host_connect(host,1);

		if(rc && rc != EAGAIN)
		{
			// Connection failed, notify user
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_WARNING,
												GTK_BUTTONS_OK,
												_(  "Negotiation with %s failed" ), host);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Connection error" ) );

			switch(rc)
			{
			case EBUSY:	// System busy (already connected or connecting)
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog) ,"%s", _( "Connection already in progress" ));
				break;

			case ENOTCONN:	// Connection failed
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog) ,"%s", _( "Can't connect" ));
				break;

			case -1:
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog) ,_( "Unexpected error" ));
				break;

			default:
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog) ,_( "The error code was %d (%s)" ), rc, strerror(rc));
			}

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);


		}

		Trace("Topwindow: %p Terminal: %p",topwindow,terminal);

		if(topwindow)
			gtk_widget_set_sensitive(topwindow,TRUE);

		if(terminal)
			gtk_widget_grab_focus(terminal);

    }

	Trace("%s ends",__FUNCTION__);

 }

 PW3270_ACTION( enter )
 {
 	unselect();
 	if(PCONNECTED)
		lib3270_enter();
	else
		action_connect(action);
 }

 PW3270_ACTION( about )
 {
 	static const char *authors[] = {	"Paul Mattes <Paul.Mattes@usa.net>",
										"GTRC",
										"Perry Werneck <perry.werneck@gmail.com>",
										"and others",
										NULL};

	static const char license[] =
	N_( "This program is free software; you can redistribute it and/or "
		"modify it under the terms of the GNU General Public License as "
 		"published by the Free Software Foundation; either version 2 of the "
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License "
		"along with this program; if not, write to the Free Software "
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307 "
		"USA" );

	GdkPixbuf	*logo = NULL;
	gchar		*filename;

	/* Load image logo */
	if(program_logo && g_file_test(program_logo,G_FILE_TEST_IS_REGULAR))
		 logo = gdk_pixbuf_new_from_file(program_logo,NULL);

	if(!logo)
	{
		filename = g_build_filename(program_data,LOGO,NULL);

		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			logo = gdk_pixbuf_new_from_file(filename, NULL);

		g_free(filename);
	}

	/* Build and show about dialog */
 	gtk_show_about_dialog(	GTK_WINDOW(topwindow),
#if GTK_CHECK_VERSION(2,12,0)
							"program-name",    		program_name,
#else
							"name",    				program_name,
#endif
							"logo",					logo,
							"authors", 				authors,
							"license", 				gettext( license ),
#if defined( HAVE_IGEMAC )
							"comments",				_( "3270 Terminal emulator for Gtk-OSX."),
#elif defined( HAVE_LIBGNOME )
							"comments",				_( "3270 Terminal emulator for Gnome."),
#else
							"comments",				_( "3270 Terminal emulator for GTK."),
#endif
							"version", 				program_fullversion,
							"wrap-license",			TRUE,
							NULL
						);

	if(logo)
		gdk_pixbuf_unref(logo);
 }

 PW3270_ACTION( unselect )
 {
	unselect();
 }

 PW3270_ACTION( reselect )
 {
	reselect();
 }

 PW3270_ACTION( disconnect )
 {
 	if(!PCONNECTED)
		return;

	Trace("%s - Disconecting",__FUNCTION__);
	action_group_set_sensitive(ACTION_GROUP_ONLINE,FALSE);
	action_group_set_sensitive(ACTION_GROUP_OFFLINE,FALSE);
	unselect();
	host_disconnect(hSession,0);
 }

 static int SaveText(const char *title, gchar *text)
 {

	GtkWidget *dialog = gtk_file_chooser_dialog_new( gettext(title),
                                                     GTK_WINDOW(topwindow),
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_SAVE,	GTK_RESPONSE_ACCEPT,
                                                     NULL );


	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GError	*error = NULL;
		gchar	*filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if(!g_file_set_contents(filename,text,-1,&error))
		{
			Warning( N_( "Error saving %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}

		g_free(filename);
	}

	gtk_widget_destroy(dialog);
 	g_free(text);
 	return 0;
 }

 PW3270_ACTION( savescreen )
 {
	SaveText(N_( "Save screen contents" ), GetScreenContents(TRUE));
 }

 PW3270_ACTION( saveselected )
 {
	SaveText(N_( "Save selected text" ), GetSelection());
 }

 PW3270_ACTION( saveclipboard )
 {
	SaveText(N_( "Save clipboard contents" ), GetClipboard());
 }

 PW3270_ACTION( dumpscreen )
 {
 	gchar		*ptr;
	GKeyFile	*conf   = GetConf();

	// TODO (perry#1#): Show an error message if offline

	GtkWidget 	*dialog = gtk_file_chooser_dialog_new( _( "Dump screen contents" ),
														 GTK_WINDOW(topwindow),
														 GTK_FILE_CHOOSER_ACTION_SAVE,
														 GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														 GTK_STOCK_SAVE,	GTK_RESPONSE_ACCEPT,
														 NULL );


	ptr = g_key_file_get_string(conf,"uri","ScreenDump",NULL);
	if(ptr)
	{
			gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(dialog),ptr);
			g_free(ptr);
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GError		*error = NULL;
		gchar		*filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		int			sz;
		struct ea	*buffer = copy_device_buffer(&sz);

		ptr = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
		g_key_file_set_string(conf,"uri","ScreenDump",ptr);
		g_free(ptr);

		if(!g_file_set_contents(filename,(gchar *) buffer,sz*sizeof(struct ea),&error))
		{
			Warning( N_( "Error saving %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		g_free(filename);
		free(buffer);
	}

	gtk_widget_destroy(dialog);
 }

 PW3270_ACTION( loadscreendump )
 {
 	gchar		*ptr;
	GKeyFile	*conf   = GetConf();

	// TODO (perry#1#): Show an error message if online

	GtkWidget 	*dialog = gtk_file_chooser_dialog_new( _( "Load screen dump" ),
														GTK_WINDOW(topwindow),
														GTK_FILE_CHOOSER_ACTION_OPEN,
														GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
														NULL );


	ptr = g_key_file_get_string(conf,"uri","ScreenDump",NULL);
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
		struct ea	*buffer	= NULL;
		gchar		*ptr;

		ptr = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
		g_key_file_set_string(conf,"uri","ScreenDump",ptr);
		g_free(ptr);

		if(!g_file_get_contents(filename, (gchar **) ((void *) &buffer), &sz, &error))
		{
			Warning( N_( "Error loading %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		else
		{
			sz /= sizeof(struct ea);
			if(set_device_buffer(buffer,sz))
				Warning( N_( "Can't set device buffer contents" ) );

			gtk_widget_set_sensitive(terminal,TRUE);
			action_group_set_sensitive(ACTION_GROUP_ONLINE,TRUE);
			action_group_set_sensitive(ACTION_GROUP_OFFLINE,TRUE);

			gtk_widget_queue_draw(terminal);
			gtk_widget_grab_focus(terminal);

		}
		g_free(filename);
		g_free(buffer);
	}

	gtk_widget_destroy(dialog);
 }

 PW3270_ACTION(kpadd)
 {
 	Trace("%s - KPAlternative is %s",__FUNCTION__,TOGGLED_KP_ALTERNATIVE ? "Enabled" : "Disabled");
 	if(TOGGLED_KP_ALTERNATIVE)
		lib3270_tab();
	else
		ParseInput("+");

 }

 PW3270_ACTION(kpsubtract)
 {
 	Trace("%s - KPAlternative is %s",__FUNCTION__,TOGGLED_KP_ALTERNATIVE ? "Enabled" : "Disabled");
 	if(TOGGLED_KP_ALTERNATIVE)
		lib3270_backtab();
	else
		ParseInput("-");
 }

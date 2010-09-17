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
 * Este programa está nomeado como actions.c e possui 1088 linhas de código.
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

 #include <lib3270/config.h>
 #include <gdk/gdk.h>

 #include "gui.h"
 #include "fonts.h"
 #include "actions.h"
 #include "uiparser.h"
 #include <gdk/gdkkeysyms.h>
 #include <errno.h>

 #include <globals.h>

 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/toggle.h>
 #include <lib3270/plugins.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void action_Up(GtkWidget *w, gpointer user_data);
 static void action_Down(GtkWidget *w, gpointer user_data);
 static void action_Left(GtkWidget *w, gpointer user_data);
 static void action_Right(GtkWidget *w, gpointer user_data);

 static void action_pfkey(GtkAction *action,gpointer id);
 static void action_pakey(GtkAction *action,gpointer id);

 static void action_PrintScreen(GtkWidget *w, gpointer user_data);

/*---[ Gui toggles ]--------------------------------------------------------------------------------------------*/

 static const struct _gui_toggle_info
 {
    const gchar     *name;
    const gchar     *label;
    const gboolean  def;
 } gui_toggle_info[GUI_TOGGLE_COUNT] =
 {
    { "Bold", 			    N_( "Bold" ),				FALSE	},
    { "KeepSelected", 	    N_( "Keep selected" ),		FALSE	},
    { "Underline",		    N_( "Show Underline" ), 	TRUE	},
    { "AutoConnect",    	N_( "Connect on startup" ),	TRUE	}
 };

 gboolean gui_toggle[GUI_TOGGLE_COUNT] = { 0 };


/*---[ Action tables ]------------------------------------------------------------------------------------------*/

 GtkActionGroup 		**action_group		= NULL;
 static GtkAction 		*scroll_action[]	= { NULL, NULL, NULL, NULL };

 static struct _keyboard_action
 {
	guint			keyval;
	GdkModifierType	state;
	GtkAction		*action;
	GCallback		def;
 } keyboard_action[] =
 {
	{ GDK_Left,				0,					NULL,	G_CALLBACK(action_Left)				},
	{ GDK_Up,				0,					NULL,	G_CALLBACK(action_Up)				},
	{ GDK_Right,			0,					NULL,	G_CALLBACK(action_Right)			},
	{ GDK_Down,				0,					NULL,	G_CALLBACK(action_Down)				},
	{ GDK_Tab,				0,					NULL,	G_CALLBACK(action_NextField)		},
	{ GDK_ISO_Left_Tab,		GDK_SHIFT_MASK,		NULL,	G_CALLBACK(action_PreviousField)	},
	{ GDK_KP_Left,			0,					NULL,	G_CALLBACK(action_Left)				},
	{ GDK_KP_Up,			0,					NULL,	G_CALLBACK(action_Up)				},
	{ GDK_KP_Right,			0,					NULL,	G_CALLBACK(action_Right)			},
	{ GDK_KP_Down,			0,					NULL,	G_CALLBACK(action_Down)				},
	{ GDK_KP_Add,			GDK_NUMLOCK_MASK,	NULL,	G_CALLBACK(action_NextField)		},

	{ GDK_3270_PrintScreen,	0,					NULL,	G_CALLBACK(action_PrintScreen)		},
	{ GDK_Sys_Req,			0,					NULL,	G_CALLBACK(action_SysReq)			},

	{ GDK_Print,			GDK_CONTROL_MASK,	NULL,	G_CALLBACK(action_PrintScreen)		},
	{ GDK_Print,			GDK_SHIFT_MASK,		NULL,	G_CALLBACK(action_SysReq)			},

#ifdef WIN32
	{ GDK_Pause,			0,					NULL,	0									},
#endif
 };

 static struct _pf_action
 {
	GtkAction 	*normal;
	GtkAction	*shift;
 } pf_action[12] =
 {
 	{ NULL,	NULL }, 	// PF01
 	{ NULL,	NULL }, 	// PF02
 	{ NULL,	NULL }, 	// PF03
 	{ NULL,	NULL }, 	// PF04
 	{ NULL,	NULL }, 	// PF05
 	{ NULL,	NULL }, 	// PF06
 	{ NULL,	NULL }, 	// PF07
 	{ NULL,	NULL }, 	// PF08
 	{ NULL,	NULL }, 	// PF09
 	{ NULL,	NULL }, 	// PF10
 	{ NULL,	NULL }, 	// PF11
 	{ NULL,	NULL }, 	// PF12
 };

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 GtkAction * get_action_by_name(const gchar *name)
 {
	int			p;
	GtkAction	*rc = NULL;

	for(p = 0; action_group[p] && !rc; p++)
		rc = gtk_action_group_get_action(action_group[p],name);

	return rc;
 }

 void set_action_sensitive_by_name(const gchar *name,gboolean sensitive)
 {
	int p;

	for(p = 0; action_group[p]; p++)
	{
		GtkAction *act = gtk_action_group_get_action(action_group[p],name);
		if(act)
		{
			Trace("%s: %s(%s)",__FUNCTION__,name,sensitive ? "sensitive" : "insensitive");
			gtk_action_set_sensitive(act,sensitive);
			return;
		}
	}

	Trace("%s: %s isn't available",__FUNCTION__,name);

 }

 static void clear_and_call(GtkAction *action, int (*call)(void))
 {
 	action_ClearSelection();
 	call();
 }

 static void action_Reset(void)
 {
 	clear_and_call(0,lib3270_Reset);
 }

 static void clear_action(void)
 {
 	clear_and_call(0,action_Clear);
 }

 static void erase_input_action(void)
 {
 	clear_and_call(0,action_EraseInput);
 }

 void action_Up(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,action_CursorUp);
 }

 void action_Down(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,action_CursorDown);
 }

 void action_Left(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,action_CursorLeft);
 }

 void action_Right(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,action_CursorRight);
 }

 void DisableNetworkActions(void)
 {
	set_action_group_sensitive_state(ACTION_GROUP_ONLINE,FALSE);
	set_action_group_sensitive_state(ACTION_GROUP_OFFLINE,FALSE);
 }

 static void action_Disconnect(GtkWidget *w, gpointer user_data)
 {
 	Trace("%s Connected:%d Widget: %p",__FUNCTION__,PCONNECTED,w);

 	if(!PCONNECTED)
 		return;

	DisableNetworkActions();
 	action_ClearSelection();
 	host_disconnect(hSession,0);
 }

 void action_Connect(void)
 {
    const gchar *host;

 	Trace("%s Connected:%d",__FUNCTION__,PCONNECTED);

 	if(PCONNECTED)
 		return;

 	action_ClearSelection();

	host = GetString("Network","Hostname",CN);

    if(host == CN)
    {
        action_SetHostname();
    }
    else
    {
    	int rc;

        DisableNetworkActions();
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

 void action_enter(GtkWidget *w, gpointer user_data)
 {
 	action_ClearSelection();
 	if(PCONNECTED)
		action_Enter();
	else
		action_Connect();
 }

 static void action_PrintScreen(GtkWidget *w, gpointer user_data)
 {
	PrintText(PROGRAM_NAME, GetScreenContents(TRUE));
 }

 static void action_PrintSelected(GtkWidget *w, gpointer user_data)
 {
	PrintText(PROGRAM_NAME, GetSelection());
 }

 static void action_PrintClipboard(GtkWidget *w, gpointer user_data)
 {
	PrintText(PROGRAM_NAME, GetClipboard());
 }

 static void action_Quit(void)
 {
 	action_Save();
 	program_quit();
 }

 static void action_About(GtkWidget *w, gpointer user_data)
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
		filename = g_strdup_printf("%s%c%s.%s", program_data, G_DIR_SEPARATOR, PROGRAM_NAME, LOGOEXT);

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
#ifdef HAVE_LIBGNOME
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

 static void toggle_action(GtkToggleAction *action, gpointer id)
 {
 	Trace("%s(%d)",__FUNCTION__,(int) id);
 	set_toggle((int) id,gtk_toggle_action_get_active(action));
 }

 static void toggle_set_action(GtkAction *action,int id)
 {
 	set_toggle(id,TRUE);
 }

 static void toggle_reset_action(GtkAction *action,int id)
 {
 	set_toggle(id,FALSE);
 }

 static void toggle_gui(GtkToggleAction *action, int id)
 {
    gui_toggle[id] = gtk_toggle_action_get_active(action);

	SetBoolean("Toggles",gui_toggle_info[id].name,gui_toggle[id]);

    if(id == GUI_TOGGLE_BOLD)
		action_Redraw();
 }

/**
 * Default script action - Call defined commands 1 by 1
 *
 */
 static void action_script_activated(GtkWidget *widget, GtkWidget *topwindow)
 {
 	const gchar	*filename	= g_object_get_data(G_OBJECT(widget),"script_filename");
 	const gchar	*text	= g_object_get_data(G_OBJECT(widget),"script_text");
	gchar			**line;
	int				ln;

	Trace("Internal script_text: %p",text);

	if(text)
	{
		line = g_strsplit(text,"\n",-1);
	}
	else if(filename)
	{
		// TODO (perry#1#): Read and split filename contents
		return;
	}
	else
	{
		return;
	}

	for(ln = 0; line[ln]; ln++)
	{
		line[ln] = g_strstrip(line[ln]);
		if(*line[ln] && *line[ln] != '#')
			run_script_command_line(line[ln],NULL);
	}

	g_strfreev(line);

 }

 void init_gui_toggles(void)
 {
 	int 		f;
 	gchar 		*name;
 	GtkAction	*action;

 	for(f=0;f<GUI_TOGGLE_COUNT;f++)
 	{
 		gui_toggle[f] = GetBoolean("Toggles", gui_toggle_info[f].name, gui_toggle_info[f].def);
		name = g_strconcat("Toggle",gui_toggle_info[f].name,NULL);
		action = get_action_by_name(name);
		if(action)
			gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),gui_toggle[f]);
		g_free(name);
 	}

 }

 static void action_ToggleGDKDebug(GtkToggleAction *action, gpointer user_data)
 {
	gdk_window_set_debug_updates(gtk_toggle_action_get_active(action));
 }

 static void action_pfkey(GtkAction *action, gpointer id)
 {
	Trace("Running PF %d",(int) id);

	if(!TOGGLED_KEEP_SELECTED)
		action_ClearSelection();
 	action_PFKey((int) id);
 }

 static void action_pakey(GtkAction *action, gpointer id)
 {
	Trace("Running PA %d",(int) id);

	if(!TOGGLED_KEEP_SELECTED)
		action_ClearSelection();
 	action_PAKey((int) id);
 }

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	int f;
 	int	state = event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_ALT_MASK);

	// Check for PF keys
	if(event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(state & (GDK_CONTROL_MASK|GDK_ALT_MASK)))
	{
		GtkAction *action;

		f = (event->keyval - GDK_F1);

		action = (state & GDK_SHIFT_MASK) ? pf_action[f].shift : pf_action[f].normal;

		Trace("PF%02d: %s %s action: %p",f+1,gdk_keyval_name(event->keyval),state & GDK_SHIFT_MASK ? "Shift " : "",action);

		if(action)
			gtk_action_activate(action);
		else
			action_pfkey(NULL, (gpointer) (f + ((state & GDK_SHIFT_MASK) ? 13 : 1)));

		return TRUE;
	}

#ifdef WIN32
	// FIXME (perry#1#): Find a better way!
	if( event->keyval == 0xffffff && event->hardware_keycode == 0x0013)
		event->keyval = GDK_Pause;
#endif

	Trace("Key action 0x%04x: %s %s keycode: 0x%04x",event->keyval,gdk_keyval_name(event->keyval),state & GDK_SHIFT_MASK ? "Shift " : "",event->hardware_keycode);

    // Check for special keyproc actions
	for(f=0; f < G_N_ELEMENTS(keyboard_action);f++)
	{
		if(keyboard_action[f].keyval == event->keyval && state == keyboard_action[f].state)
		{
			if(keyboard_action[f].action)
				gtk_action_activate(keyboard_action[f].action);
			else if(!keyboard_action[f].def)
				return FALSE;
			else
				keyboard_action[f].def();
			return TRUE;
		}
	}

	return FALSE;
 }

 GtkAction * create_action_by_descriptor(const gchar *name, struct action_descriptor *data)
 {
	int			state		= data->attr.key_state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_ALT_MASK);
	int			f;

//	Trace("%s: %d",name,data->ui_type);

	switch(data->ui_type)
	{
	case UI_CALLBACK_TYPE_TOGGLE:
		data->action = GTK_ACTION(gtk_toggle_action_new(name, gettext(data->attr.label ? data->attr.label : data->name), gettext(data->attr.tooltip), data->attr.stock_id));
		break;

	case UI_CALLBACK_TYPE_DEFAULT:
		data->action = gtk_action_new(name, gettext(data->attr.label ? data->attr.label : data->name), gettext(data->attr.tooltip),data->attr.stock_id);
		break;

	case UI_CALLBACK_TYPE_SCROLL:
		if(data->sub >= 0 && data->sub <= G_N_ELEMENTS(scroll_action))
			scroll_action[data->sub] = data->action = gtk_action_new(name, gettext(data->attr.label ? data->attr.label : data->name), gettext(data->attr.tooltip),data->attr.stock_id);
		break;

	case UI_CALLBACK_TYPE_SCRIPT:

		if(data->script.text)
			g_object_set_data_full(G_OBJECT(data->action),"script_text",g_strdup(data->script.text),g_free);

		if(data->callback)
			g_signal_connect(G_OBJECT(data->action),"activate",G_CALLBACK(data->callback),topwindow);
		else
			g_signal_connect(G_OBJECT(data->action),"activate",G_CALLBACK(action_script_activated),topwindow);

		data->callback = 0;
		break;

	default:
		Trace("Invalid action type %d in %s",data->ui_type,data->name);
		return NULL;
	}

//	Trace("%s(%s), callback: %p action: %p",__FUNCTION__,name,data->callback,data->action);

	if(data->callback)
		g_signal_connect(G_OBJECT(data->action),data->ui_type == UI_CALLBACK_TYPE_TOGGLE ? "toggled" : "activate",G_CALLBACK(data->callback),data->user_data);

	// Check for special keyboard action
	for(f=0;f<G_N_ELEMENTS(keyboard_action);f++)
	{
		if(data->attr.key_value == keyboard_action[f].keyval && (state == keyboard_action[f].state))
		{
			keyboard_action[f].action = data->action;
			gtk_action_group_add_action(action_group[data->group],data->action);
			return data->action;
		}
	}

	// Check for PF actions
	if(data->attr.key_value >= GDK_F1 && data->attr.key_value <= GDK_F12 && !(state & (GDK_CONTROL_MASK|GDK_ALT_MASK)))
	{
		f = data->attr.key_value - GDK_F1;

		if(state&GDK_SHIFT_MASK)
			pf_action[f].shift = data->action;
		else
			pf_action[f].normal = data->action;
		gtk_action_group_add_action(action_group[data->group],data->action);
		return data->action;
	}

	// Register action
	if(data->attr.accel)
		gtk_action_group_add_action_with_accel(action_group[data->group],data->action,data->attr.accel);
	else
		gtk_action_group_add_action(action_group[data->group],data->action);

	return data->action;
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

 static void action_LoadScreenDump(void)
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
			set_action_group_sensitive_state(ACTION_GROUP_ONLINE,TRUE);
			set_action_group_sensitive_state(ACTION_GROUP_OFFLINE,TRUE);

			gtk_widget_queue_draw(terminal);
			gtk_widget_grab_focus(terminal);

		}
		g_free(filename);
		g_free(buffer);
	}

	gtk_widget_destroy(dialog);
 }

 static void action_DumpScreen(void)
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

  static void action_SaveScreen(void)
 {
	SaveText(N_( "Save screen contents" ), GetScreenContents(TRUE));
 }

 static void action_SaveSelected(void)
 {
	SaveText(N_( "Save selected text" ), GetSelection());
 }

 static void action_SaveClipboard(void)
 {
	SaveText(N_( "Save clipboard contents" ), GetClipboard());
 }

 void action_SetHostname(void)
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

 LOCAL_EXTERN int get_action_info_by_name(const gchar *key, const gchar **names, const gchar **values, gchar **name, UI_CALLBACK *info)
 {
	static const struct _action
	{
		const gchar	* name;		/**< Name of the action */
		const gchar	* label;	/**< Default action label */
		GCallback	  callback;		/**< Callback for "activated/toggled" signal */
	}
	action[] =
	{
		// Offline actions
		{	"Connect",			N_( "_Connect" ),				G_CALLBACK(action_Connect)			},
		{	"LoadScreenDump",	N_( "Load screen dump" ),		G_CALLBACK(action_LoadScreenDump)	},
		{ 	"SetHostname",		N_( "Set hostname" ),			G_CALLBACK(action_SetHostname)		},

		// Online actions
		{	"Redraw",			N_( "Redraw screen" ),			G_CALLBACK(action_Redraw)			},
		{	"SaveScreen",		N_( "Save screen" ),			G_CALLBACK(action_SaveScreen)		},
		{	"PrintScreen",		N_( "Print" ),					G_CALLBACK(action_PrintScreen)		},
		{	"DumpScreen",		N_( "Dump screen" ),			G_CALLBACK(action_DumpScreen)		},
		{	"Disconnect",		N_( "_Disconnect" ),			G_CALLBACK(action_Disconnect)		},

		// Select actions
		{	"SelectField",		N_( "Select Field" ),			G_CALLBACK(action_SelectField)		},

		{ 	"SelectRight",		N_( "Select Right" ),			G_CALLBACK(action_SelectRight)		},
		{ 	"SelectLeft",		N_( "Select Left" ),			G_CALLBACK(action_SelectLeft)		},
		{ 	"SelectUp",			N_( "Select Up" ),				G_CALLBACK(action_SelectUp)			},
		{ 	"SelectDown",		N_( "Select Down" ),			G_CALLBACK(action_SelectDown)		},

		{	"SelectionRight",	N_( "Selection Right" ),		G_CALLBACK(action_SelectionRight)	},
		{	"SelectionLeft",	N_( "Selection Left" ),			G_CALLBACK(action_SelectionLeft)	},
		{	"SelectionUp",		N_( "Selection Up" ),			G_CALLBACK(action_SelectionUp)		},
		{	"SelectionDown",	N_( "Selection Down" ),			G_CALLBACK(action_SelectionDown)	},

		// Cursor Movement
		{ 	"CursorRight",		N_( "Right" ),					G_CALLBACK(action_Right)			},
		{ 	"CursorLeft",		N_( "Left" ),					G_CALLBACK(action_Left)				},
		{ 	"CursorUp",			N_( "Up" ),						G_CALLBACK(action_Up)				},
		{ 	"CursorDown",		N_( "Down" ),					G_CALLBACK(action_Down)				},

		{	"NextField",		N_( "Next field" ),				G_CALLBACK(action_NextField)		},
		{	"PreviousField",	N_( "Previous field" ),			G_CALLBACK(action_PreviousField)	},

		// Edit actions
		{	"PasteNext",		N_( "Paste next" ),				G_CALLBACK(action_PasteNext)		},
		{	"PasteTextFile",	N_( "Paste text file" ),		G_CALLBACK(action_PasteTextFile)	},
		{	"Reselect",			N_( "Reselect" ),				G_CALLBACK(Reselect)				},
		{	"SelectAll",		N_( "Select all" ),				G_CALLBACK(action_SelectAll)		},
		{	"EraseInput",		N_( "Erase input" ),			G_CALLBACK(erase_input_action)		},
		{	"Clear",			N_( "Clear" ),					G_CALLBACK(clear_action)			},

		{	"Reset",			N_( "Reset" ),					G_CALLBACK(action_Reset)			},
		{	"Escape",			N_( "Escape" ),					G_CALLBACK(action_Reset)			},

		// File-transfer actions
		{	"Download",			N_( "Receive file" ),			G_CALLBACK(action_Download)			},
		{	"Upload",   		N_( "Send file" ),   			G_CALLBACK(action_Upload)			},

		// Selection actions
		{	"Append",			N_( "Add to copy" ),			G_CALLBACK(action_Append)			},
		{	"Unselect",			N_( "Unselect" ),				G_CALLBACK(action_ClearSelection)	},
		{	"Copy",				N_( "Copy" ),					G_CALLBACK(action_Copy)				},
		{	"CopyAsTable",		N_( "Copy as table" ),			G_CALLBACK(action_CopyAsTable)		},
		{	"PrintSelected",	N_( "Print selected" ),			G_CALLBACK(action_PrintSelected)	},
		{	"SaveSelected",		N_( "Save selected" ),			G_CALLBACK(action_SaveSelected)		},

		{	"PrintClipboard",	N_( "Print copy" ),				G_CALLBACK(action_PrintClipboard)	},
		{	"SaveClipboard",	N_( "Save copy" ),				G_CALLBACK(action_SaveClipboard)	},
		{	"Paste",			N_( "Paste" ),					G_CALLBACK(action_Paste)			},

		{	"FileMenu",			N_( "_File" ),					NULL								},
		{	"FTMenu",			N_( "Send/Receive" ),			NULL								},
		{	"NetworkMenu",		N_( "_Network" ),				NULL								},
		{	"HelpMenu",			N_( "Help" ),					NULL								},
		{	"EditMenu",			N_( "_Edit" ),					NULL								},
		{	"OptionsMenu",		N_( "_Options" ),				NULL								},
		{	"SettingsMenu",		N_( "Settings" ),				NULL								},
		{	"ScriptsMenu",		N_( "Scripts" ),				NULL								},
		{	"ViewMenu",			N_( "_View" ),					NULL								},
		{	"InputMethod",		N_( "Input method" ),			NULL								},
		{	"ToolbarMenu",		N_( "Toolbars" ),				NULL								},
		{	"DebugMenu",		N_( "Debug" ),					NULL								},
		{	"FontSettings",		N_( "Select font" ),			NULL								},
		{	"Preferences",		N_( "Preferences" ),			NULL								},
		{	"Network",			N_( "Network" ),				NULL								},
		{	"Properties",		N_( "Properties" ),				NULL								},

		{	"About",			N_( "About" ),					G_CALLBACK(action_About)			},
		{	"Quit",				N_( "_Quit" ),					G_CALLBACK(action_Quit)				},
		{	"SelectColors",		N_( "Colors" ),					G_CALLBACK(action_SelectColors)		},

		{	"Save",				N_( "Save" ),					NULL								},

		{ 	"Return",			N_( "Return" ),					G_CALLBACK(action_enter)			},
		{ 	"Enter",			N_( "Enter" ),					G_CALLBACK(action_enter)			},

		// Lib3270 calls
		{ "EraseEOF",			N_( "Erase to end of field" ),	G_CALLBACK(lib3270_EraseEOF)		},
		{ "EraseEOL",			N_( "Erase to end of line" ),	G_CALLBACK(lib3270_EraseEOL)		},
		{ "Home",				N_( "First Field" ),			G_CALLBACK(action_FirstField)		},
		{ "DeleteWord",			N_( "Delete Word" ),			G_CALLBACK(action_DeleteWord)		},
		{ "EraseField",			N_( "Erase Field" ),			G_CALLBACK(action_DeleteField)		},
		{ "Delete",				N_( "Delete Char" ),			G_CALLBACK(action_Delete)			},
		{ "Erase",				N_( "Backspace" ),				G_CALLBACK(action_Erase)			},
		{ "SysREQ",				N_( "Sys Req" ),				G_CALLBACK(action_SysReq)			},
	};

	static const struct _toggle_info
	{
		const gchar *do_label;
		const gchar *set_label;
		const gchar *reset_label;
	} toggle_info[N_TOGGLES] =
	{
		{ N_( "Monocase" ),					NULL,		NULL				},
		{ N_( "Alt Cursor" ),				NULL,		NULL				},
		{ N_( "Blinking Cursor" ),			NULL,		NULL				},
		{ N_( "Show timing" ),				NULL,		NULL				},
		{ N_( "Track Cursor" ),				NULL,		NULL				},
		{ N_( "DS Trace" ),					NULL,		NULL				},
		{ N_( "Scroll bar" ),				NULL,		NULL				},
		{ N_( "Line Wrap" ),				NULL,		NULL				},
		{ N_( "Blank Fill" ),				NULL,		NULL				},
		{ N_( "Screen Trace" ),				NULL,		NULL				},
		{ N_( "Event Trace" ),				NULL,		NULL				},
		{ N_( "Paste with left margin" ),	NULL,		NULL				},
		{ N_( "Select by rectangles" ),		NULL,		NULL				},
		{ N_( "Cross Hair Cursor" ),		NULL,		NULL				},
		{ N_( "Visible Control chars" ),	NULL,		NULL				},
		{ N_( "Aid wait" ),					NULL,		NULL				},
		{ N_( "Full Screen" ),				NULL,		N_( "Window" )		},
		{ N_( "Auto-Reconnect" ),			NULL,		NULL				},
		{ N_( "Insert" ),					NULL,		N_( "Overwrite")	},
		{ N_( "Keypad" ),					NULL,		NULL				},
		{ N_( "Smart paste" ),				NULL,		NULL				},
	};

	int			f;

	if(name)
		*name = NULL;

	memset(info,0,sizeof(UI_CALLBACK));

	if(!g_ascii_strcasecmp(key,"pfkey"))
	{
		info->type 		= UI_CALLBACK_TYPE_DEFAULT;
		info->label		= "pfkey";
		info->callback	= G_CALLBACK(action_pfkey);
		info->user_data = (gpointer) atoi(get_xml_attribute(names, values, "id"));
		if(name)
			*name = g_strdup_printf("pf%02d",(int) info->user_data);
		return 0;
	}

	if(!g_ascii_strcasecmp(key,"pakey"))
	{
		info->type 		= UI_CALLBACK_TYPE_DEFAULT;
		info->label		= "pakey";
		info->callback	= G_CALLBACK(action_pakey);
		info->user_data = (gpointer) atoi(get_xml_attribute(names, values, "id"));
		if(name)
			*name = g_strdup_printf("pa%02d",(int) info->user_data);
		return 0;
	}

	if(!g_ascii_strcasecmp(key,"toggle"))
	{
		// Check for lib3270 toggles
		const gchar *id = get_xml_attribute(names, values, "id");

		for(f=0;f<N_TOGGLES;f++)
		{
			if(!g_ascii_strcasecmp(id,get_toggle_name(f)))
			{
				info->type 		= UI_CALLBACK_TYPE_TOGGLE;
				info->label		= toggle_info[f].do_label;
				info->callback	= G_CALLBACK(toggle_action);
				info->user_data = (gpointer) f;
				if(name)
					*name = g_strconcat("Toggle",get_toggle_name(f),NULL);
				return 0;
			}
		}

		// Check for GUI toggles
		for(f=0;f<G_N_ELEMENTS(gui_toggle_info);f++)
		{
			if(!g_ascii_strcasecmp(id,gui_toggle_info[f].name))
			{
				info->type 		= UI_CALLBACK_TYPE_TOGGLE;
				info->label		= gui_toggle_info[f].label;
				info->callback	= G_CALLBACK(toggle_gui);
				info->user_data = (gpointer) f;
				if(name)
					*name = g_strconcat("Toggle",gui_toggle_info[f].name,NULL);
				return 0;
			}
		}

		if(!g_ascii_strcasecmp(id,"gdkdebug"))
		{
			info->type 		= UI_CALLBACK_TYPE_TOGGLE;
			info->label		= _( "Debug window updates" );
			info->callback	= G_CALLBACK(action_ToggleGDKDebug);
			info->user_data = (gpointer) f;
			if(name)
				*name = g_strdup("ToggleGDKDebug");
			return 0;
		}

		return EINVAL;
	}

	if(!g_ascii_strcasecmp(key,"toggleset"))
	{
		const gchar *id = get_xml_attribute(names, values, "id");

		for(f=0;f<N_TOGGLES;f++)
		{
			if(!g_ascii_strcasecmp(id,get_toggle_name(f)))
			{
				info->type 		= UI_CALLBACK_TYPE_DEFAULT;
				info->label		= toggle_info[f].set_label ? toggle_info[f].set_label : toggle_info[f].do_label;
				info->callback	= G_CALLBACK(toggle_set_action);
				info->user_data = (gpointer) f;
				if(name)
					*name = g_strconcat("ToggleSet",get_toggle_name(f),NULL);
				return 0;
			}
		}
		return EINVAL;
	}

	if(!g_ascii_strcasecmp(key,"togglereset"))
	{
		const gchar *id = get_xml_attribute(names, values, "id");

		for(f=0;f<N_TOGGLES;f++)
		{
			if(!g_ascii_strcasecmp(id,get_toggle_name(f)))
			{
				info->type 		= UI_CALLBACK_TYPE_DEFAULT;
				info->label		= toggle_info[f].reset_label ? toggle_info[f].reset_label : toggle_info[f].do_label;
				info->callback	= G_CALLBACK(toggle_reset_action);
				info->user_data = (gpointer) f;
				if(name)
					*name = g_strconcat("ToggleReset",get_toggle_name(f),NULL);
				return 0;
			}
		}
		return EINVAL;
	}

	// Search internal actions
	for(f=0;f<G_N_ELEMENTS(action);f++)
	{
		if(!g_ascii_strcasecmp(key,action[f].name))
		{
			info->type = UI_CALLBACK_TYPE_DEFAULT;
			info->label = action[f].label;
			info->callback = action[f].callback;
			return 0;
		}
	}

	// Search for plugin actions
	if(strchr(key,'.'))
	{
		gchar	*plugin_name = g_strdup(key);
		gchar	*entry_name	 = strchr(plugin_name,'.');
		GModule *plugin;

		*(entry_name++) = 0;

		plugin = get_plugin_by_name(plugin_name);
		if(plugin)
		{
			gpointer cbk;

			if(get_symbol_by_name(plugin,&cbk,"action_%s_activated",entry_name))
			{
				info->type 		= UI_CALLBACK_TYPE_DEFAULT;
				info->callback	= (GCallback) cbk;
				info->user_data	= (gpointer) topwindow;

			}
			else if(get_symbol_by_name(plugin,&cbk,"action_%s_toggled",entry_name))
			{
				info->type		= UI_CALLBACK_TYPE_TOGGLE;
				info->callback	= (GCallback) cbk;
				info->user_data	= (gpointer) topwindow;

			}
		}

		g_free(plugin_name);
		if(info->callback)
			return 0;
	}

	// Check if it's an external program action
	if(!g_ascii_strncasecmp(key,"call",4) && strlen(key) > 5 && g_ascii_isspace(*(key+4)))
	{

	}

	return ENOENT;
 }

 gboolean mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
 {
	if(query_3270_terminal_status() != STATUS_CODE_BLANK || event->direction < 0 || event->direction > G_N_ELEMENTS(scroll_action))
		return 0;

	Trace("Scroll: %d Action: %p",event->direction,scroll_action[event->direction]);

	if(scroll_action[event->direction])
		gtk_action_activate(scroll_action[event->direction]);

 	return 0;
 }


 void set_action_group_sensitive_state(int id, gboolean status)
 {
	gtk_action_group_set_sensitive(action_group[id],status);
 }

 static void set_ft_action_state(int state)
 {
	gtk_action_group_set_sensitive(action_group[ACTION_GROUP_FT],state);
 }

 void init_actions(GtkWidget *widget)
 {
	static const gchar	*group_name[]	= { "default", "online", "offline", "selection", "clipboard", "paste", "filetransfer" };
 	GtkActionGroup		**group			= g_malloc0((G_N_ELEMENTS(group_name)+1)*sizeof(GtkActionGroup *));
	int					f;

	for(f=0;f<G_N_ELEMENTS(group_name);f++)
	{
		group[f] = gtk_action_group_new(group_name[f]);
		gtk_action_group_set_translation_domain(group[f], PACKAGE_NAME);
	}

	action_group = group;

#ifdef DEBUG
	if(f != ACTION_GROUP_MAX)
	{
		Trace("Unexpected action_group size, found %d, expecting %d",f,ACTION_GROUP_MAX);
		exit(-1);
	}
#endif

	g_object_set_data_full(G_OBJECT(widget),"ActionGroups",group,g_free);

#ifdef 	X3270_FT
	set_ft_action_state(0);
	register_schange(ST_3270_MODE, set_ft_action_state);
#else
	gtk_action_group_set_sensitive(ft_actions,FALSE);
#endif

 }


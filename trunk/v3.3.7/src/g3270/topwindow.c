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
 #include <gdk/gdkkeysyms.h>
 #include <lib3270/config.h>

 #include <globals.h>
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/toggle.h>
 #include <lib3270/hostc.h>
 #include <lib3270/plugins.h>

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkWidget	*topwindow 	= NULL;
 GList 		*main_icon	= NULL;
 GdkCursor	*wCursor[CURSOR_MODE_G3270];

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
 	action_Save();
 	gtk_main_quit();
    return FALSE;
 }

 static void destroy( GtkWidget *widget, gpointer   data )
 {
 	topwindow = NULL;
	g3270_quit();
 }

 static void set_widget_flags(GtkWidget *widget, gpointer data)
 {
 	if(!widget)
		return;

	GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_DEFAULT);

	if(GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),set_widget_flags,0);

 }

 static gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	// http://developer.gnome.org/doc/API/2.0/gtk/GtkWidget.html#GtkWidget-key-press-event
    if(IS_FUNCTION_KEY(event))
		return(PFKey(GetFunctionKey(event)));
    return FALSE;
 }

 static gboolean key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {

    if(IS_FUNCTION_KEY(event))
    {
    	GetFunctionKey(event);
    	return TRUE;
    }

 	return 0;
 }

 static void activate_font(GtkCheckMenuItem *item, gchar *text)
 {
	if(gtk_check_menu_item_get_active(item))
	{
		gchar *vlr	= strdup(text);
		char *ptr	= strchr(vlr,',');

		if(ptr)
		{
			*(ptr++) = 0;
			SetString("Terminal","FontSizes",ptr);
		}
		else
		{
			SetString("Terminal","FontSizes",NULL);
		}

		SetString("Terminal","Font",vlr);

		g_free(vlr);

		FontChanged();
	}
 }

 static void LoadSystemFonts(GtkWidget *widget, GtkWidget *menu_item, const gchar *selected)
 {
 	// Stolen from http://svn.gnome.org/svn/gtk+/trunk/gtk/gtkfontsel.c
	PangoFontFamily **families;
	gint 			n_families, i;

 	GtkWidget		*menu	= gtk_menu_new();
 	GSList 			*group	= NULL;

	pango_context_list_families(gtk_widget_get_pango_context(widget),&families, &n_families);

	Trace("Font families: %d",n_families);

	for(i=0; i<n_families; i++)
    {
    	if(pango_font_family_is_monospace(families[i]))
    	{
			const gchar 	*name = pango_font_family_get_name (families[i]);
			GtkWidget		*item = gtk_radio_menu_item_new_with_label(group,name);

//			Trace("Adding font %s",name);

			// FIXME (perry#2#): the user_data isn't being freed!
			group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
			g_signal_connect(G_OBJECT(item),"toggled",G_CALLBACK(activate_font),g_strdup(name));
			gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

			if(!strcmp(name,selected))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),TRUE);

    	}
    }

	gtk_widget_show_all(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item),menu);

 }

 static void LoadFontMenu(GtkWidget *widget, GtkWidget *menu_item)
 {
	gchar			*filename	= NULL;
	GError			*error		= NULL;
	gchar			*text		= NULL;
	const gchar	*selected	= GetString("Terminal","Font","Courier");
	gchar			**ln;


	Trace("Selected font: \"%s\"",selected);

	if(!menu_item)
		return;

	filename = FindSystemConfigFile("fonts.conf");
	if(!filename)
	{
		LoadSystemFonts(widget, menu_item, selected);
		return;
	}

	if(!g_file_get_contents(filename,&text,NULL,&error))
	{
		PopupAnError( N_( "Error loading %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
		g_error_free(error);
		g_free(filename);
		g_free(text);
		return;
	}

	if(*filename)
	{
		ln = g_strsplit(text,"\n",0);

		if(ln)
		{
		 	GtkWidget	*menu	= gtk_menu_new();
		 	int 		f;
		 	GSList 		*group	= NULL;

		 	for(f=0;ln[f];f++)
		 	{
		 		ln[f] = g_strstrip(ln[f]);

		 		if(*ln[f])
		 		{
		 			GtkWidget 	*item;
		 			gchar		*arg	= g_strdup(ln[f]);
		 			char		*ptr	= strchr(ln[f],',');

		 			if(ptr)
						*ptr = 0;

					item = gtk_radio_menu_item_new_with_label(group,ln[f]);

		 			// FIXME (perry#2#): the user_data isn't being freed!
		 			group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
					g_signal_connect(G_OBJECT(item),"toggled",G_CALLBACK(activate_font),arg);
					gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

					if(!strcmp(arg,selected))
						gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),TRUE);
		 		}
		 	}

			gtk_widget_show_all(menu);
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item),menu);
			g_strfreev(ln);
		}

	}

	g_free(text);
	g_free(filename);
 }

 GdkPixbuf * LoadLogo(void)
 {
 	// TODO (perry#5#): Make it configurable.
	GdkPixbuf	*pix;
	gchar 		*file;

	file = FindSystemConfigFile(PACKAGE_NAME ".png");
	if(file)
	{
		Trace("Loading %s",file);
		pix = gdk_pixbuf_new_from_file(file, NULL);
		g_free(file);
		return pix;
	}

	file = FindSystemConfigFile(PACKAGE_NAME ".jpg");
	if(file)
	{
		Trace("Loading %s",file);
		pix = gdk_pixbuf_new_from_file(file, NULL);
		g_free(file);
		return pix;
	}

	return NULL;
 }

 int CreateTopWindow(void)
 {
#if defined(_WIN32)
 	static int 			cr[CURSOR_MODE_G3270] = { 	GDK_ARROW,
														GDK_WATCH,
														GDK_X_CURSOR,

														GDK_TOP_LEFT_CORNER, 		// Top-left
														GDK_TOP_RIGHT_CORNER,		// Top-right
														GDK_TREK,					// Top
														GDK_BOTTOM_LEFT_CORNER,		// Bottom-left
														GDK_BOTTOM_RIGHT_CORNER,	// Bottom-right
														GDK_BOTTOM_TEE,				// Bottom
														GDK_LEFT_SIDE,				// Left
														GDK_RIGHT_PTR,				// Right
														GDK_SIZING				// Inside

													};
#else
 	static int 			cr[CURSOR_MODE_G3270] = { 	GDK_XTERM,
														GDK_WATCH,
														GDK_X_CURSOR,

														GDK_TOP_LEFT_CORNER, 		// Top-left
														GDK_TOP_RIGHT_CORNER,		// Top-right
														GDK_TOP_SIDE,				// Top
														GDK_BOTTOM_LEFT_CORNER,		// Bottom-left
														GDK_BOTTOM_RIGHT_CORNER,	// Bottom-right
														GDK_BOTTOM_SIDE,			// Bottom
														GDK_LEFT_SIDE,				// Left
														GDK_RIGHT_SIDE,				// Right
														GDK_FLEUR					// Inside

													};
#endif

 	GtkWidget				*vbox;
 	GtkWidget				*hbox;
 	int						f;
	GtkUIManager			*ui_manager;
	GdkPixbuf				*pix;

	topwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	pix = LoadLogo();

	if(pix)
	{
		main_icon = g_list_append(main_icon, pix);
		gtk_window_set_icon_list(GTK_WINDOW(topwindow),main_icon);
	}

	for(f=0;f<CURSOR_MODE_G3270;f++)
		wCursor[f] = gdk_cursor_new(cr[f]);

	g_signal_connect(G_OBJECT(topwindow),	"delete_event", 		G_CALLBACK(delete_event),			0);
	g_signal_connect(G_OBJECT(topwindow),	"destroy", 				G_CALLBACK(destroy),				0);
    g_signal_connect(G_OBJECT(topwindow),	"key-press-event",		G_CALLBACK(key_press_event),		0);
    g_signal_connect(G_OBJECT(topwindow), 	"key-release-event",	G_CALLBACK(key_release_event),		0);

    vbox = gtk_vbox_new(FALSE,0);

	/* Create terminal window */
	if(!CreateTerminalWindow())
		return -1;

	/* Create UI elements */
	ui_manager = LoadApplicationUI(topwindow);
	if(ui_manager)
	{
		static const char *toolbar[] = { "/MainMenubar", "/MainToolbar" };

		static const struct _popup
		{
			const char 	*name;
			GtkWidget		**widget;
		} popup[] =
		{
			{ "/DefaultPopup",		&DefaultPopup	},
			{ "/SelectionPopup",	&SelectionPopup	}
		};

		int f;

		gtk_window_add_accel_group(GTK_WINDOW(topwindow),gtk_ui_manager_get_accel_group(ui_manager));

		for(f=0;f < G_N_ELEMENTS(toolbar);f++)
		{
			GtkWidget *widget = gtk_ui_manager_get_widget(ui_manager, toolbar[f]);
			if(widget)
				gtk_box_pack_start(GTK_BOX(vbox),widget,FALSE,FALSE,0);
		}

		set_widget_flags(gtk_ui_manager_get_widget(ui_manager,"/MainToolbar"),0);

		for(f=0;f < G_N_ELEMENTS(popup);f++)
		{
			*popup[f].widget =  gtk_ui_manager_get_widget(ui_manager, popup[f].name);
			if(*popup[f].widget)
			{
				g_object_ref(*popup[f].widget);
			}
		}

		LoadFontMenu(topwindow,gtk_ui_manager_get_widget(ui_manager,"/MainMenubar/SettingsMenu/FontSettings"));
		g_object_unref(ui_manager);
	}

	gtk_widget_show_all(vbox);
    gtk_container_add(GTK_CONTAINER(topwindow),vbox);

    hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox), terminal, TRUE, TRUE, 0);
	gtk_widget_show_all(hbox);

	keypad = CreateKeypadWindow();
	set_widget_flags(keypad,0);
	gtk_box_pack_end(GTK_BOX(hbox), keypad, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

#ifdef PACKAGE_NAME
	gtk_window_set_role(GTK_WINDOW(topwindow), PACKAGE_NAME "_TOP" );
#else
	gtk_window_set_role(GTK_WINDOW(topwindow), "G3270_TOP" );
#endif

	action_ClearSelection();
	ClearClipboard();

	gtk_action_group_set_sensitive(online_actions,FALSE);
	gtk_action_group_set_sensitive(offline_actions,TRUE);
	gtk_action_group_set_sensitive(clipboard_actions,FALSE);
	gtk_action_set_sensitive(gtk_action_group_get_action(online_actions,"Reselect"),FALSE);

	gtk_window_set_default_size(GTK_WINDOW(topwindow),590,430);
	settitle(GetString("TopWindow","Title",""));

	action_Restore();

	gtk_window_set_position(GTK_WINDOW(topwindow),GTK_WIN_POS_CENTER);

	return 0;
 }

 void g3270_quit(void)
 {
 	action_internal(Disconnect_action, IA_DEFAULT, CN, CN);
	gtk_main_quit();
 }

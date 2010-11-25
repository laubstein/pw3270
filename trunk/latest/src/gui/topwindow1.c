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
 * Este programa está nomeado como topwindow1.c e possui - linhas de código.
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
 #include "uiparser1.h"

 #include <gdk/gdkkeysyms.h>
 #include <lib3270/config.h>

 #include <globals.h>
 #include <lib3270/kybdc.h>
 #include <lib3270/toggle.h>
 #include <lib3270/plugins.h>

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkWidget	*topwindow 		= NULL;
 GList 		*main_icon		= NULL;
 gchar		*program_logo	= NULL;

#ifdef MOUSE_POINTER_CHANGE
 GdkCursor	*wCursor[CURSOR_MODE_3270];
#endif

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static void destroy( GtkWidget *widget, gpointer   data )
 {
 	Trace("%s called - Destroying top-window %p",__FUNCTION__,topwindow);
 	topwindow = NULL;
	action_quit(0);
 	Trace("%s","Ok");
 }

 static gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	// http://developer.gnome.org/doc/API/2.0/gtk/GtkWidget.html#GtkWidget-key-press-event
	return check_key_action(widget,event);
 }

 GtkWidget * widget_from_action_name(const gchar *name)
 {
 	GtkAction *action = get_action_by_name(name);

	if(action)
		return g_object_get_data(G_OBJECT(action),"ui_widget");

	return NULL;
 }

 static void LoadInputMethods(void)
 {
	GtkWidget *topmenu = widget_from_action_name("InputMethod");

	if(topmenu && input_method)
	{
		GtkWidget *menu	= gtk_menu_new();
		gtk_im_multicontext_append_menuitems((GtkIMMulticontext *) input_method,GTK_MENU_SHELL(menu));
		gtk_widget_show_all(menu);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(topmenu),menu);
	}
#ifdef DEBUG
	else
	{
		Trace("*** %s","No fontselect menu");
	}
#endif
 }

 static void LoadFontMenu(void)
 {
	gchar		*selected;
	GtkWidget	*menu = widget_from_action_name("fontselect");

	if(menu)
	{
		selected = GetString("Terminal","Font","Courier");
		load_font_menu(topwindow, menu, selected);
		g_free(selected);
	}
#ifdef DEBUG
	else
	{
		Trace("*** %s","No fontselect menu");
	}
#endif

	return;
 }

 GdkPixbuf * LoadLogo(void)
 {
	GdkPixbuf	*pix = NULL;
	gchar		*filename;

	if(program_logo && g_file_test(program_logo,G_FILE_TEST_IS_REGULAR))
		return gdk_pixbuf_new_from_file(program_logo,NULL);

	filename = g_strdup_printf("%s%c%s.%s", program_data, G_DIR_SEPARATOR, PROGRAM_NAME, LOGOEXT);

	if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		pix = gdk_pixbuf_new_from_file(filename, NULL);

	Trace("pixbuf(%s): %p",filename,pix);
	g_free(filename);

	return pix;
 }

 static void selection_owner_changed(GtkClipboard *clipboard, GdkEventOwnerChange *event, gpointer user_data)
 {
// 	Trace("%s called reason: %d owner: %p",__FUNCTION__,(int) event->reason, (void *) event->owner);
 	if(terminal && !GTK_WIDGET_HAS_FOCUS(terminal))
		unselect();

	if(event->owner)
		check_clipboard_contents();
 }

 int CreateTopWindow(void)
 {
#ifdef MOUSE_POINTER_CHANGE

	#ifdef WIN32

		static const gchar *cr[CURSOR_MODE_3270] = {	"arrow",
														"wait",
														"arrow",

														"sizenwse",	// Top-left
														"sizenesw",	// Top-right
														"sizens",	// Top
														"sizenesw",	// Bottom-left
														"sizenwse",	// Bottom-right
														"sizens",	// Bottom
														"sizewe",	// Left
														"sizewe",	// Right
														"sizeall"	// Inside

													};

	#else

		static int 		cr[CURSOR_MODE_3270] = { 	GDK_XTERM,
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

 	int						f;

#endif

	GtkWidget				*toolbar;
 	GtkWidget				*vbox;
 	GtkWidget				*hbox;
#ifdef HAVE_GTK_STATUS_BAR
 	GtkWidget				*box;
 	GtkWidget				*widget;
#endif // HAVE_GTK_STATUS_BAR
	GtkUIManager			*manager;
	GdkPixbuf				*pix;
	gchar 					*ptr;

	topwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_object_set_data(G_OBJECT(topwindow),"pw3270_config",	(gpointer) GetConf());
	g_object_set_data(G_OBJECT(topwindow),"pw3270_dpath",	(gpointer) program_data);

	init_actions();

	pix = LoadLogo();

	if(pix)
	{
		main_icon = g_list_append(main_icon, pix);
		gtk_window_set_icon_list(GTK_WINDOW(topwindow),main_icon);

#if defined( HAVE_IGEMAC )
		gtk_osxapplication_set_dock_icon_pixbuf(osxapp,pix);
#endif

	}

#ifdef MOUSE_POINTER_CHANGE

	#ifdef WIN32
		for(f=0;f<CURSOR_MODE_3270;f++)
			wCursor[f] = gdk_cursor_new_from_name(gdk_display_get_default(), cr[f]);
	#else
		for(f=0;f<CURSOR_MODE_3270;f++)
			wCursor[f] = gdk_cursor_new(cr[f]);
	#endif

#endif

//	g_signal_connect(G_OBJECT(topwindow),	"delete_event", 		G_CALLBACK(delete_event),			0);
	g_signal_connect(G_OBJECT(topwindow),	"destroy", 				G_CALLBACK(destroy),				0);
    g_signal_connect(G_OBJECT(topwindow),	"key-press-event",		G_CALLBACK(key_press_event),		0);
//    g_signal_connect(G_OBJECT(topwindow), 	"key-release-event",	G_CALLBACK(key_release_event),		0);

	/* Create terminal window */
	terminal = gtk_label_new("Teste");
	if(!CreateTerminalWindow())
		return -1;

	/* Create UI elements */
	toolbar = gtk_vbox_new(FALSE,0);
    vbox = gtk_vbox_new(FALSE,0);
    hbox = gtk_hbox_new(FALSE,0);

	manager = load_application_ui(topwindow, GTK_BOX(toolbar), GTK_BOX(vbox), GTK_BOX(hbox));
	if(manager)
	{
		static const struct _popup
		{
			const char 	*name;
			GtkWidget		**widget;
		} popup[] =
		{
			{ "defaultpopup",	&DefaultPopup	},
			{ "selectionpopup",	&SelectionPopup	}
		};

		int f;

		for(f=0;f < G_N_ELEMENTS(popup);f++)
		{
			*popup[f].widget = widget_from_action_name(popup[f].name);
			if(*popup[f].widget)
			{
				g_object_ref(*popup[f].widget);
			}
		}

		LoadFontMenu();
		LoadInputMethods();

		g_object_unref(manager);
	}

	gtk_box_pack_start(GTK_BOX(vbox), terminal, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), vbox,TRUE,TRUE,0);

	gtk_box_pack_start(GTK_BOX(toolbar), hbox, TRUE, TRUE, 0);

#ifdef HAVE_GTK_STATUS_BAR
	box = gtk_vbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(box),toolbar,TRUE,TRUE,0);

	widget =  gtk_statusbar_new();
	gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(widget),TRUE);

	gtk_box_pack_end(GTK_BOX(box),widget,FALSE,FALSE,0);

	gtk_widget_show(widget);
	gtk_widget_show(box);
    gtk_container_add(GTK_CONTAINER(topwindow),box);
#else
    gtk_container_add(GTK_CONTAINER(topwindow),toolbar);
#endif // HAVE_GTK_STATUS_BAR

    gtk_container_add(GTK_CONTAINER(topwindow),toolbar);

	{
		static const gchar *name[ACTION_ID_MAX] = { "CopyAsTable", "CopyAsImage", "PasteNext", "Unselect", "Reselect" };
		int f;

		for(f=0;f<ACTION_ID_MAX;f++)
		{
			GtkAction *a = get_action_by_name(name[f]);
			if(a)
				action_by_id[f] = a;
		}
	}

	gtk_action_set_sensitive(action_by_id[ACTION_RESELECT],FALSE);

	gtk_widget_show(terminal);
	gtk_widget_show(toolbar);
	gtk_widget_show(hbox);
	gtk_widget_show(vbox);

	gtk_window_set_role(GTK_WINDOW(topwindow), PACKAGE_NAME "_TOP" );

	unselect();
	ClearClipboard();

	action_group_set_sensitive(ACTION_GROUP_ONLINE,FALSE);
	action_group_set_sensitive(ACTION_GROUP_OFFLINE,TRUE);
	action_group_set_sensitive(ACTION_GROUP_CLIPBOARD,FALSE);
	action_group_set_sensitive(ACTION_GROUP_PASTE,FALSE);

	gtk_action_set_sensitive(action_by_id[ACTION_RESELECT],FALSE);
	gtk_action_set_sensitive(action_by_id[ACTION_PASTENEXT],FALSE);

	g_signal_connect(G_OBJECT(gtk_widget_get_clipboard(topwindow,GDK_SELECTION_CLIPBOARD)),"owner-change",G_CALLBACK(selection_owner_changed),0);

	gtk_window_set_default_size(GTK_WINDOW(topwindow),590,430);
	ptr = GetString("TopWindow","Title","");
	settitle(ptr);
	g_free(ptr);

	action_restore(0);

	gtk_window_set_position(GTK_WINDOW(topwindow),GTK_WIN_POS_CENTER);

	return 0;
 }


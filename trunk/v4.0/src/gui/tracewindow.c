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
 * Este programa está nomeado como tracewindow.c e possui - linhas de código.
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

 struct _console;
 #define HCONSOLE struct _console *

 #include "gui.h"
 #include <stdarg.h>
 #include <string.h>

/*---[ Structs ]----------------------------------------------------------------------------------*/

 struct _console
 {
 	unsigned short	sz;
 	gboolean		waiting;
 	GtkWidget		*window;
 	GtkWidget 		*entry;
 	GtkWidget 		*button;
 	GtkTextBuffer	*text;
	GtkAdjustment	*scroll;
	gchar			*title;
	gchar			*label;
 };

/*---[ Prototipes ]-------------------------------------------------------------------------------*/

/*---[ Globals ]----------------------------------------------------------------------------------*/

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static void entry_ok(GtkButton *button,HCONSOLE hwnd)
 {
 	hwnd->waiting = FALSE;
 }

 HCONSOLE gui_console_window_new(const char *title, const char *label)
 {
	HCONSOLE hwnd = (HCONSOLE) g_malloc0(sizeof(struct _console));

	hwnd->sz 	= sizeof(struct _console);
	hwnd->title = g_strdup(title  ? title : _( "Console" ));
	hwnd->label = g_strdup(label ? label : _( "Command:" ));

	return hwnd;
 }

 void gui_console_window_delete(HCONSOLE hwnd)
 {
 	hwnd->sz = 0;
 	if(hwnd->window)
		g_object_unref(hwnd->window);

 	g_free(hwnd->title);
 	g_free(hwnd->label);

 	g_free(hwnd);
 }

 static void destroy(GtkWidget *widget,HCONSOLE hwnd)
 {
 	Trace("Console \"%s\" destroyed (%p)",hwnd->title,hwnd);
 	hwnd->window = 0;
 }

 static GtkWidget * console_window_get(HCONSOLE hwnd)
 {
 	GtkWidget *widget;
 	GtkWidget *view;
 	GtkWidget *vbox;
 	GtkWidget *hbox;

	if(hwnd->window)
		return hwnd->window;

 	Trace("Console \"%s\" created (%p)",hwnd->title,hwnd);

	// Create a new trace window
	hwnd->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(hwnd->window),"destroy",G_CALLBACK(destroy),hwnd);
//	g_object_set_data(G_OBJECT(hwnd->window),"window_data",hwnd);

	gtk_window_set_transient_for(GTK_WINDOW(hwnd->window),GTK_WINDOW(topwindow));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(hwnd->window),TRUE);

	gtk_window_set_title(GTK_WINDOW(hwnd->window),hwnd->title);

	vbox = gtk_vbox_new(FALSE,2);

	// Create text box
	widget = gtk_scrolled_window_new(NULL, NULL);
	hwnd->scroll = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widget));
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	view = gtk_text_view_new();

 	hwnd->text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),view);

	gtk_box_pack_start(GTK_BOX(vbox),widget,TRUE,TRUE,0);

	// Entry line
	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new( hwnd->label ),FALSE,TRUE,4);

	hwnd->entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),hwnd->entry,TRUE,TRUE,4);
	gtk_widget_set_sensitive(hwnd->entry,FALSE);
	g_signal_connect(G_OBJECT(hwnd->entry),"activate",G_CALLBACK(entry_ok),hwnd);

	hwnd->button = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_box_pack_end(GTK_BOX(hbox),hwnd->button,FALSE,FALSE,4);
	gtk_widget_set_sensitive(hwnd->button,FALSE);
	g_signal_connect(G_OBJECT(hwnd->button),"clicked",G_CALLBACK(entry_ok),hwnd);
	gtk_button_set_focus_on_click(GTK_BUTTON(hwnd->button),FALSE);

	gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	// Show dialog
	g_signal_connect(G_OBJECT(hwnd->window),"activate-default",G_CALLBACK(entry_ok),hwnd);
	gtk_container_add(GTK_CONTAINER(hwnd->window),vbox);
	gtk_window_set_default_size(GTK_WINDOW(hwnd->window),590,430);
	gtk_widget_show_all(hwnd->window);

	return hwnd->window;
 }

 int gui_console_window_append(HCONSOLE hwnd, const char *fmt, va_list args)
 {
 	GtkTextIter itr;
 	gchar		*msg;

	if(!console_window_get(hwnd))
		return EINVAL;

	gtk_text_buffer_get_end_iter(hwnd->text,&itr);

	msg = g_strdup_vprintf(fmt,args);

	gtk_text_buffer_insert(hwnd->text,&itr,msg,strlen(msg));
	g_free(msg);

	gtk_text_buffer_get_end_iter(hwnd->text,&itr);

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 14
	gtk_adjustment_set_value(hwnd->scroll,gtk_adjustment_get_upper(hwnd->scroll));
#else
	gtk_adjustment_set_value(hwnd->scroll,(GTK_ADJUSTMENT(hwnd->scroll))->upper);
#endif

	return 0;
 }

 char * gui_console_window_wait_for_user_entry(HCONSOLE hwnd)
 {

	if(!console_window_get(hwnd))
	{
		errno = EINVAL;
		return NULL;
	}

 	hwnd->waiting = TRUE;

	gtk_window_present(GTK_WINDOW(hwnd->window));

	gtk_widget_set_sensitive(hwnd->entry,TRUE);
	gtk_widget_set_sensitive(hwnd->button,TRUE);
	gtk_widget_grab_focus(hwnd->entry);

	while(hwnd->waiting && hwnd->window)
	{
		RunPendingEvents(1);
	}

	if(hwnd->window)
	{
		char *ret = strdup(gtk_entry_get_text(GTK_ENTRY(hwnd->entry)));
		gtk_entry_set_text(GTK_ENTRY(hwnd->entry),"");
		gtk_widget_set_sensitive(hwnd->entry,FALSE);
		gtk_widget_set_sensitive(hwnd->button,FALSE);
		return ret;
	}

	errno = ENOENT;
	return NULL;
 }



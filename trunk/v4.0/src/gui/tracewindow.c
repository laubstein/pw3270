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

 #include "gui.h"
 #include <stdarg.h>

/*---[ Structs ]----------------------------------------------------------------------------------*/

 struct trace_window_data
 {
 	gboolean		waiting;
 	GtkWidget 		*entry;
 	GtkWidget 		*button;
 	GtkTextBuffer	*text;
	GtkAdjustment	*scroll;

 };

/*---[ Prototipes ]-------------------------------------------------------------------------------*/

 static struct trace_window_data * getTraceWindow(void);

/*---[ Globals ]----------------------------------------------------------------------------------*/

 static GtkWidget	*trace_window = NULL;

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static void destroy_trace(GtkWidget *widget,gpointer data)
 {
 	Trace("%s","Trace window destroyed");
 	trace_window = 0;
 }

 void entry_ok(GtkButton *button,struct trace_window_data *data)
 {
 	data->waiting = FALSE;
 }

 static struct trace_window_data * getTraceWindow(void)
 {
 	struct trace_window_data *data;

 	GtkWidget *widget;
 	GtkWidget *view;
 	GtkWidget *vbox;
 	GtkWidget *hbox;

 	if(trace_window)
		return (struct trace_window_data *) g_object_get_data(G_OBJECT(trace_window),"window_data");

	// Create a new trace window
	trace_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(trace_window),"destroy",G_CALLBACK(destroy_trace),0);

	data = g_malloc0(sizeof(struct trace_window_data));
	g_object_set_data_full(G_OBJECT(trace_window),"window_data",data,g_free);

	gtk_window_set_transient_for(GTK_WINDOW(trace_window),GTK_WINDOW(topwindow));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(trace_window),TRUE);

	gtk_window_set_title(GTK_WINDOW(trace_window),_( "Console" ));

	vbox = gtk_vbox_new(FALSE,2);

	// Create text box
	widget = gtk_scrolled_window_new(NULL, NULL);
	data->scroll = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widget));
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	view = gtk_text_view_new();

 	data->text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),view);

	gtk_box_pack_start(GTK_BOX(vbox),widget,TRUE,TRUE,0);

	// Entry line
	hbox = gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new( _( "Command:" ) ),FALSE,TRUE,4);

	data->entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(hbox),data->entry,TRUE,TRUE,4);
	gtk_widget_set_sensitive(data->entry,FALSE);
	g_signal_connect(G_OBJECT(data->entry),"activate",G_CALLBACK(entry_ok),data);

	data->button = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_box_pack_end(GTK_BOX(hbox),data->button,FALSE,FALSE,4);
	gtk_widget_set_sensitive(data->button,FALSE);
	g_signal_connect(G_OBJECT(data->button),"clicked",G_CALLBACK(entry_ok),data);
	gtk_button_set_focus_on_click(GTK_BUTTON(data->button),FALSE);

	gtk_box_pack_end(GTK_BOX(vbox),hbox,FALSE,TRUE,0);

	// Show dialog
	g_signal_connect(G_OBJECT(trace_window),"activate-default",G_CALLBACK(entry_ok),data);
	gtk_container_add(GTK_CONTAINER(trace_window),vbox);
	gtk_window_set_default_size(GTK_WINDOW(trace_window),590,430);
	gtk_widget_show_all(trace_window);

	return data;
 }

 void PrintConsole(const gchar *fmt, ...)
 {
 	struct trace_window_data *data = getTraceWindow();
 	GtkTextIter itr;
 	gchar		*msg;
    va_list 	args;

	if(!data)
		return;

	gtk_text_buffer_get_end_iter(data->text,&itr);

    va_start(args, fmt);
	msg = g_strdup_vprintf(fmt,args);
    va_end(args);
	gtk_text_buffer_insert(data->text,&itr,msg,strlen(msg));
	g_free(msg);

	gtk_text_buffer_get_end_iter(data->text,&itr);
	gtk_text_buffer_insert(data->text,&itr,"\n",1);

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 14
	gtk_adjustment_set_value(data->scroll,gtk_adjustment_get_upper(data->scroll));
#else
	gtk_adjustment_set_value(data->scroll,(GTK_ADJUSTMENT(data->scroll))->upper);
#endif
 }

/*
 static void gettracecommand(PRXSTRING str)
 {
 	struct trace_window_data *data = getTraceWindow();

	if(!data)
	{
		RetString(str,NULL);
		return;
	}

 	data->waiting = TRUE;

	gtk_window_present(GTK_WINDOW(trace_window));

	gtk_widget_set_sensitive(data->entry,TRUE);
	gtk_widget_set_sensitive(data->button,TRUE);

	gtk_widget_grab_focus(data->entry);

	while(trace_window && data->waiting)
	{
		RunPendingEvents(1);
	}

	if(trace_window)
	{
		RetString(str,gtk_entry_get_text(GTK_ENTRY(data->entry)));
		gtk_entry_set_text(GTK_ENTRY(data->entry),"");
		gtk_widget_set_sensitive(data->entry,FALSE);
		gtk_widget_set_sensitive(data->button,FALSE);
		return;
	}
	RetString(str,NULL);
 }
*/



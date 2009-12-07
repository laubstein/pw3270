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
 * Este programa está nomeado como print.c e possui 265 linhas de código.
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

 #include <lib3270/config.h>
 #include <globals.h>
 #include <errno.h>


/*---[ Statics ]------------------------------------------------------------------------------------------------*/


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

#ifdef GTK_PRINT_OPERATION

 static int doPrint(GtkPrintOperation *prt, GtkPrintContext *context,cairo_t *cr,gint page)
 {
 	gchar					**text;
	PangoFontDescription	*FontDescr	= (PangoFontDescription *) g_object_get_data(G_OBJECT(prt),"3270FontDescr");
	PangoLayout				*FontLayout	= (PangoLayout *) g_object_get_data(G_OBJECT(prt),"3270FontLayout");
	int						pg			= 0;
	gdouble					maxHeight 	= gtk_print_context_get_height(context);
	gdouble					current		= 0;
	gdouble 				pos;

	if(!FontDescr)
	{
		FontDescr = pango_font_description_from_string(g_object_get_data(G_OBJECT(prt),"3270FontName"));
		g_object_set_data_full(G_OBJECT(prt),"3270FontDescr",FontDescr,(void (*)(gpointer)) pango_font_description_free);
	}

	if(!FontLayout)
	{
		Trace("Creating FontLayout to context %p",context);
		FontLayout = gtk_print_context_create_pango_layout(context);
		g_object_set_data_full(G_OBJECT(prt),"3270FontLayout",FontLayout,(void (*)(gpointer)) g_object_unref);
		pango_layout_set_font_description(FontLayout,FontDescr);
	}

	for(text = g_object_get_data(G_OBJECT(prt),"3270Text");*text;text++)
	{
		gdouble width, height;
		PangoRectangle	rect;

		pango_layout_set_text(FontLayout,*text,-1);

		pango_layout_get_extents(FontLayout,NULL,&rect);
		width = rect.width / PANGO_SCALE;
		height = rect.height / PANGO_SCALE;

		pos = current;

		if( (current+height) > maxHeight)
		{
			pg++;
			current = 0;
		}
		else
		{
			current += height;
		}

		if(cr && page == pg)
		{
			cairo_move_to(cr,0,pos);
			pango_cairo_show_layout(cr,FontLayout);
		}
	}

	return pg+1;
 }

 static void draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint pg, gpointer user_data)
 {
	cairo_t *cr = gtk_print_context_get_cairo_context(context);
	doPrint(prt,context,cr,pg);
 }

 static void begin_print(GtkPrintOperation *prt, GtkPrintContext *context, gpointer user_data)
 {
		int pages = doPrint(prt,context,NULL,-1);
		if(pages <= 0)
		{
			gtk_print_operation_cancel(prt);
			return;
		}
		gtk_print_operation_set_n_pages(prt,pages);
 }


#ifdef HAVE_PRINT_FONT_DIALOG
/*---[ Begin font selection dialog ]------------------------------------------------------------------------------*/

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *font_dialog, gpointer user_data)
 {
 	gchar *font = gtk_font_selection_get_font_name(GTK_FONT_SELECTION(font_dialog));
	if(font)
		g_object_set_data_full(G_OBJECT(prt),"3270FontName",font,g_free);
 }

 static void load_font(GtkWidget *widget, GtkPrintOperation *prt)
 {
	gtk_font_selection_set_font_name(GTK_FONT_SELECTION(widget),g_object_get_data(G_OBJECT(prt),"3270FontName"));
 }

 static GObject * create_custom_widget(GtkPrintOperation *prt, gpointer user_data)
 {
 	GtkWidget *font_dialog =  gtk_font_selection_new();
    g_signal_connect(font_dialog, "realize", G_CALLBACK(load_font), prt);
 	return G_OBJECT(font_dialog);
 }

/*---[ End Font Selection Dialog ]--------------------------------------------------------------------------------*/

#endif

#if ! GTK_CHECK_VERSION(2,12,0)

 static void SavePrintSetting(const gchar *key, const gchar *value, GKeyFile *cfg)
 {
 	 g_key_file_set_string(cfg, "PrintSettings", key, value);
 }
#endif

 static void print_done(GtkPrintOperation *prt, GtkPrintOperationResult result, gpointer user_data)
 {
	GKeyFile 				*conf		= GetConf();
#ifdef HAVE_PRINT_FONT_DIALOG
	gchar					*ptr;
#endif
	GtkPrintSettings		*settings	= gtk_print_operation_get_print_settings(prt);
#ifdef HAVE_PRINT_FONT_DIALOG
	PangoFontDescription	*FontDescr	= (PangoFontDescription *) g_object_get_data(G_OBJECT(prt),"3270FontDescr");
#endif
	GtkPageSetup			*setup		= gtk_print_operation_get_default_page_setup(prt);


	if(!conf)
		return;

	Trace("Settings: %p Conf: %p",settings,conf);
#if GTK_CHECK_VERSION(2,12,0)
	gtk_print_settings_to_key_file(settings,conf,NULL);
	gtk_page_setup_to_key_file(setup,conf,NULL);
#else
	gtk_print_settings_foreach(settings,(GtkPrintSettingsFunc) SavePrintSetting,conf);
#endif


#ifdef HAVE_PRINT_FONT_DIALOG
	if(FontDescr)
	{
		ptr = pango_font_description_to_string(FontDescr);
		if(ptr)
		{
			g_key_file_set_string(conf,"Print","Font",ptr);
			g_free(ptr);
		}
	}
#endif

 }

 int PrintText(const char *name, gchar *text)
 {
	GKeyFile			*conf	= GetConf();
 	GtkPrintOperation	*prt;

 	if(!text)
		return -EINVAL;

 	prt = gtk_print_operation_new();

 	if(!prt)
 		return -1;

	// Set job parameters
	g_object_set_data_full(G_OBJECT(prt),"3270Text",g_strsplit(g_strchomp(text),"\n",-1),(void (*)(gpointer)) g_strfreev);
	g_object_set_data_full(G_OBJECT(prt),"3270FontName",g_strdup(GetString("Print","Font","Courier 10")),g_free);

	// Configure print operation
	gtk_print_operation_set_job_name(prt,name);
	gtk_print_operation_set_allow_async(prt,0);
	gtk_print_operation_set_show_progress(prt,1);

	gtk_print_operation_set_custom_tab_label(prt,_( "Font" ));
	g_signal_connect(prt, "begin-print",    		G_CALLBACK(begin_print), 			0);
    g_signal_connect(prt, "draw-page",      		G_CALLBACK(draw_page),   			0);
#ifdef HAVE_PRINT_FONT_DIALOG
	g_signal_connect(prt, "create-custom-widget",   G_CALLBACK(create_custom_widget),	0);
	g_signal_connect(prt, "custom-widget-apply",   	G_CALLBACK(custom_widget_apply),	0);
#endif
    g_signal_connect(prt, "done",      				G_CALLBACK(print_done),	 			0);

	if(conf)
	{
#if GTK_CHECK_VERSION(2,12,0)
		gchar *ptr = g_key_file_get_string(conf,"Print Settings","output-uri",NULL);
		if(ptr)
		{
			gchar *uri = NULL;

			switch(*(ptr++))
			{
			case '$':
				if(g_str_has_prefix(ptr,"home/"))
					uri = g_strdup_printf("file:///%s/%s",g_get_home_dir(),ptr+5);
#if GTK_CHECK_VERSION(2,14,0)
				else if(g_str_has_prefix(ptr,"documents/"))
					uri = g_strdup_printf("file:///%s/%s",g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS),ptr+10);
				else if(g_str_has_prefix(ptr,"desktop/"))
					uri = g_strdup_printf("file:///%s/%s",g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP),ptr+8);
#endif
				break;

			case '~':
				uri = g_strdup_printf("file:///%s/%s",g_get_home_dir(),ptr);
				break;

			}

			if(uri)
			{
				g_key_file_set_string(conf,"Print Settings","output-uri",uri);
				g_free(uri);
			}
		}

		gtk_print_operation_set_print_settings(prt,gtk_print_settings_new_from_key_file(conf,NULL,NULL));
		gtk_print_operation_set_default_page_setup(prt,gtk_page_setup_new_from_key_file(conf,NULL,NULL));
#else
		settings = gtk_print_settings_new();
		if(settings)
		{
			gchar 				**list;
			GtkPrintSettings	*settings;
			int 				f;

			list = g_key_file_get_keys(conf,"PrintSettings",NULL,NULL);
			if(list)
			{
				for(f=0;list[f];f++)
					gtk_print_settings_set(settings,list[f],g_key_file_get_string(conf,"PrintSettings",list[f],NULL));
				g_strfreev(list);
			}
		}
		gtk_print_operation_set_print_settings(prt,settings);
		gtk_print_operation_set_default_page_setup(prt,gtk_page_setup_new());
#endif
	}
	else
	{
		gtk_print_operation_set_default_page_setup(prt,gtk_page_setup_new());
		gtk_print_operation_set_print_settings(prt,gtk_print_settings_new());
	}

	// Run Print dialog
	gtk_print_operation_run(prt,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(topwindow),NULL);

    g_object_unref(prt);

    return 0;
 }

#else // GTK_PRINT_OPERATION

 int PrintText(const char *name, gchar *text)
 {
	gchar *command = GetString("Print", "Command", "lpr");
	RunExternalProgramWithText(command,text);
	g_free(command);
	return 0;
 }

#endif // GTK_PRINT_OPERATION

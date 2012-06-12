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
 #include "actions.h"

 #include <lib3270/config.h>
 #include <globals.h>
 #include <errno.h>


/*---[ Statics ]------------------------------------------------------------------------------------------------*/


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

#ifdef GTK_PRINT_OPERATION

 static int doPrint(GtkPrintOperation *prt, GtkPrintContext *context,cairo_t *cr,gint page)
 {
	gdouble					maxHeight	= gtk_print_context_get_height(context);
	PangoFontDescription	*desc;
	gint					pg			= 0;
	gdouble					row			= 0;

	gchar					**text;
	gdouble					text_height;


	desc = pango_font_description_from_string(g_object_get_data(G_OBJECT(prt),"3270FontName"));

	for(text = g_object_get_data(G_OBJECT(prt),"3270Text");*text;text++)
	{
		gint		layout_height;
		PangoLayout	*layout			= gtk_print_context_create_pango_layout(context);

		pango_layout_set_font_description(layout, desc);
		pango_layout_set_text(layout, *text, -1);
		pango_layout_get_size(layout, NULL, &layout_height);
		text_height = ((gdouble)layout_height) / PANGO_SCALE;

		if((row+text_height) >= maxHeight)
		{
			row = 0;
			pg++;
		}

		row += text_height;

		if(cr && page == pg)
		{
			cairo_move_to(cr,0,row);
			pango_cairo_show_layout(cr, layout);
		}

		g_object_unref(layout);
	}

	pango_font_description_free(desc);

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
 	gchar *font = NULL;

#if GTK_CHECK_VERSION(2,20,0)
	if(gtk_widget_get_realized(font_dialog))
#else
	if(GTK_WIDGET_REALIZED(font_dialog))
#endif

	font = gtk_font_selection_get_font_name(GTK_FONT_SELECTION(font_dialog));

	Trace("Selected font: \"%s\"",font);

	if(font)
		g_object_set_data_full(G_OBJECT(prt),"3270FontName",font,g_free);


 }


 static void load_font(GtkWidget *widget, GtkPrintOperation *prt)
 {
 	gchar *fontname = g_object_get_data(G_OBJECT(prt),"3270FontName");

	gtk_font_selection_set_font_name(GTK_FONT_SELECTION(widget),fontname);

 	Trace("Font: \"%s\" size: %d",fontname,gtk_font_selection_get_size(GTK_FONT_SELECTION(widget)));

 	if(!gtk_font_selection_get_size(GTK_FONT_SELECTION(widget)))
	{
		// Font size is 0, set it to 10
		gchar *ptr = g_strdup_printf("%s 10",fontname);
		gtk_font_selection_set_font_name(GTK_FONT_SELECTION(widget),ptr);
		g_free(ptr);
	}

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

	GtkPrintSettings		*settings	= gtk_print_operation_get_print_settings(prt);

#if GTK_CHECK_VERSION(2,12,0)
	GtkPageSetup			*setup		= gtk_print_operation_get_default_page_setup(prt);
#endif

	if(!conf)
		return;
	Trace("Settings: %p Conf: %p page_setup: %p",settings,conf,setup);
#if GTK_CHECK_VERSION(2,12,0)
	gtk_print_settings_to_key_file(settings,conf,NULL);
	gtk_page_setup_to_key_file(setup,conf,NULL);
#else
	gtk_print_settings_foreach(settings,(GtkPrintSettingsFunc) SavePrintSetting,conf);
#endif

	g_key_file_set_string(conf,"Print","fontname",g_object_get_data(G_OBJECT(prt),"3270FontName"));

 }

 int PrintText(const char *name, gchar *text)
 {
	GKeyFile			*conf	= GetConf();
 	GtkPrintOperation	*prt;
 	const gchar		*font;
	GtkPageSetup 		*page_setup = NULL;
	GtkPrintSettings 	*print_settings = NULL;

 	if(!text)
		return -EINVAL;

 	prt = gtk_print_operation_new();

 	if(!prt)
 		return -1;

	// Set job parameters
	g_object_set_data_full(G_OBJECT(prt),"3270Text",g_strsplit(g_strchomp(text),"\n",-1),(void (*)(gpointer)) g_strfreev);

	// Set print
	font = GetString("Print","fontname","");
	if(!*font)
		font = GetString("Terminal","Font","Courier new 10");

	g_object_set_data_full(G_OBJECT(prt),"3270FontName",g_strdup(font),g_free);

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

		print_settings = gtk_print_settings_new_from_key_file(conf,NULL,NULL);
		page_setup = gtk_page_setup_new_from_key_file(conf,NULL,NULL);

		if(!page_setup)
			page_setup = gtk_page_setup_new();

#else // GTK_CHECK_VERSION(2,12,0)
		print_settings = gtk_print_settings_new();
		if(print_settings)
		{
			gchar 				**list;
			int 				f;

			list = g_key_file_get_keys(conf,"PrintSettings",NULL,NULL);
			if(list)
			{
				for(f=0;list[f];f++)
					gtk_print_settings_set(print_settings,list[f],g_key_file_get_string(conf,"PrintSettings",list[f],NULL));
				g_strfreev(list);
			}
		}
		page_setup = gtk_page_setup_new();
#endif
	}
	else
	{
		page_setup = gtk_page_setup_new();
		gtk_print_operation_set_print_settings(prt,gtk_print_settings_new());
	}

	Trace("page_setup: %p print_settings: %p",page_setup,print_settings);
	gtk_print_operation_set_print_settings(prt,print_settings);
	gtk_print_operation_set_default_page_setup(prt,page_setup);


	// Run Print dialog
	gtk_print_operation_run(prt,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(topwindow),NULL);

    g_object_unref(prt);

    return 0;
 }

#else // GTK_PRINT_OPERATION

 static void print_finished(GPid pid,gint status,gchar *tempfile)
 {
 	Trace("Process %d ended with status %d",(int) pid, status);
 	remove(tempfile);
 	g_free(tempfile);
 }

 int PrintText(const char *name, gchar *str)
 {
	gchar	*cmd		= GetString("Print", "Command", "lpr");
	GError	*error		= NULL;
	gchar	*filename	= NULL;
	GPid 	pid			= 0;
	gchar	*argv[3];
	gchar	tmpname[20];

	Trace("Running comand %s\n%s",cmd,str);

	do
	{
		g_free(filename);
		g_snprintf(tmpname,19,"%08lx.tmp",rand() ^ ((unsigned long) time(0)));
		filename = g_build_filename(g_get_tmp_dir(),tmpname,NULL);
	} while(g_file_test(filename,G_FILE_TEST_EXISTS));

	Trace("Temporary file: %s",filename);

	if(!g_file_set_contents(filename,str,-1,&error))
	{
		if(error)
		{
			Warning( N_( "Can't create temporary file:\n%s" ), error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		remove(filename);
		g_free(filename);
		g_error_free(error);
		g_free(cmd);
		return -1;
	}

	argv[0] = (gchar *) cmd;
	argv[1] = filename;
	argv[2] = NULL;

	Trace("Spawning %s %s",cmd,filename);

	error = NULL;

	if(!g_spawn_async(	NULL,											// const gchar *working_directory,
						argv,											// gchar **argv,
						NULL,											// gchar **envp,
						G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD,	// GSpawnFlags flags,
						NULL,											// GSpawnChildSetupFunc child_setup,
						NULL,											// gpointer user_data,
						&pid,											// GPid *child_pid,
						&error ))										// GError **error);
	{
		if(error)
		{
			Warning( N_( "Error spawning %s\n%s" ), argv[0], error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		remove(filename);
		g_free(filename);
		g_free(cmd);
		return -1;
	}

	g_free(cmd);

	Trace("pid %d",(int) pid);

	g_child_watch_add(pid,(GChildWatchFunc) print_finished,filename);

	return 0;
 }

#endif // GTK_PRINT_OPERATION

 PW3270_ACTION( printscreen )
 {
	PrintText(PROGRAM_NAME, GetScreenContents(TRUE));
 }

 PW3270_ACTION( printselected )
 {
	PrintText(PROGRAM_NAME, GetSelection());
 }

 PW3270_ACTION( printclipboard )
 {
	PrintText(PROGRAM_NAME, GetClipboard());
 }

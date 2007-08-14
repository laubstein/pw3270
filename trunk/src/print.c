

 #include "g3270.h"

/*---[ Structs ]--------------------------------------------------------------*/

 typedef struct _printinfo
 {
	gchar 					*text;
	GtkPageSetup			*setup;
	GtkPrintSettings		*settings;
	PangoFontDescription	*FontDescr;
	PangoContext			*FontContext;
	PangoFontMap			*FontMap;
	PangoFont				*Font;
	PangoLayout				*FontLayout;
	gdouble					left;
 } PRINTINFO;

/*---[ Constants ]------------------------------------------------------------*/

/*---[ Implement ]------------------------------------------------------------*/

#if defined(DEBUG) && defined(__GTK_PRINT_OPERATION_H__) && defined(__G_KEY_FILE_H__)

 static void begin_print(GtkPrintOperation *prt, GtkPrintContext *context, PRINTINFO *cfg)
 {
 	gchar		buffer[4096];
 	gchar		*ptr;
	int 		width;
	int			height;
	int			pages = 1;
	char		*tok;
	char		*text;
	gdouble     maxHeight 	= gtk_print_context_get_height(context);
	gdouble		current		= 0;
	int			maxWidth	= 0;

 	CHKPoint();

	if(!cfg->text)
		gtk_print_operation_cancel(prt);

	if(!cfg->FontDescr)
	{
		// Get the current font for the printing operation
		DBGMessage("Font isn't set, reading from configuration");
		strncpy(buffer,"Courier New 10",4095);
		if(main_configuration)
		{
			ptr = g_key_file_get_string(main_configuration,"Fonts","Printer",NULL);
			if(ptr)
			{
				strncpy(buffer,ptr,4095);
				g_free(ptr);
			}
		}
		cfg->FontDescr = pango_font_description_from_string(buffer);
		DBGPrintf("%s = %p",buffer,cfg->FontDescr);
	}

	cfg->FontContext	= gtk_print_context_create_pango_context(context);
	cfg->FontMap		= gtk_print_context_get_pango_fontmap(context);
	cfg->Font			= pango_font_map_load_font(cfg->FontMap,cfg->FontContext,cfg->FontDescr);
	cfg->FontLayout		= gtk_print_context_create_pango_layout(context);

	pango_layout_set_font_description(cfg->FontLayout,cfg->FontDescr);

	// Simulate print to get the right number of pages.
	text = g_convert(cfg->text, -1,  "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
	if(!text)
	{
		gtk_print_operation_cancel(prt);
		return;
	}

	tok = ptr = text;
	while(*ptr)
	{
		if(*ptr == '\n')
		{
			*(ptr++) = 0;
			pango_layout_set_text(cfg->FontLayout,tok,-1);
			pango_layout_get_pixel_size(cfg->FontLayout,&width,&height);

			if(width > maxWidth)
				maxWidth = width;

			DBGPrintf("%dx%d: %s",width,height,tok);

			if( (current+ ((gdouble) height)) > maxHeight)
			{
				pages++;
				current = 0;
				DBGMessage("-- NEW PAGE --");
			}
			else
			{
				current += ((gdouble) height);
			}
			tok = ptr;
		}
		else
		{
			ptr++;
		}
	}
	g_free(text);

	DBGTrace(pages);

	cfg->left = (gtk_print_context_get_width(context)/2) - ((gdouble) (maxWidth >> 1));

	gtk_print_operation_set_n_pages(prt,pages);

 }

 static void draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint page_nr, PRINTINFO *cfg)
 {
	char	*tok;
	char	*ptr;
	char	*text;
	int		pg			= 0;
	gdouble	maxHeight 	= gtk_print_context_get_height(context);
	gdouble	current		= 0;
	gdouble pos;
	int 	width;
	int		height;
 	cairo_t *cr			= gtk_print_context_get_cairo_context(context);

	DBGTrace(page_nr);

	text = g_convert(cfg->text, -1,  "UTF-8", "ISO-8859-1", NULL, NULL, NULL);
	if(!text)
	{
		gtk_print_operation_cancel(prt);
		return;
	}

	tok = ptr = text;
	while(*ptr)
	{
		if(*ptr == '\n')
		{
			pos = current;

			*(ptr++) = 0;
			pango_layout_set_text(cfg->FontLayout,tok,-1);
			pango_layout_get_pixel_size(cfg->FontLayout,&width,&height);

			if( (current+ ((gdouble) height)) > maxHeight)
			{
				pg++;
				current = 0;
			}
			else
			{
				current += ((gdouble) height);
			}

			if(pg == page_nr)
			{
				DBGTrace(pos);
				cairo_move_to(cr,cfg->left,pos);
				pango_cairo_show_layout(cr,cfg->FontLayout);
				CHKPoint();
			}

			tok = ptr;
		}
		else
		{
			ptr++;
		}
	}
	g_free(text);


 }

 static void print_done(GtkPrintOperation *prt, GtkPrintOperationResult result, PRINTINFO *cfg)
 {
 	gchar *ptr;

 	DBGMessage("Print operation done");

	if(main_configuration && result == GTK_PRINT_OPERATION_RESULT_APPLY)
	{
		DBGMessage("Saving print settings");

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12

    	if(main_configuration)
    	{
    		if(cfg->settings)
				gtk_print_settings_to_key_file(cfg->settings,main_configuration,"PrintSettings");

			if(cfg->setup)
				gtk_page_setup_to_key_file(cfg->setup,NULL,NULL);

    	}
#endif

		if(cfg->FontDescr)
		{
			ptr = pango_font_description_to_string(cfg->FontDescr);
			if(ptr)
			{
				g_key_file_set_string(main_configuration,"Fonts","Printer",ptr);
				g_free(ptr);
			}
		}

	}

 }

 static void load_font(GtkWidget *widget, PRINTINFO *cfg)
 {
 	gchar *ptr;

	if(main_configuration)
	{
		ptr = g_key_file_get_string(main_configuration,"Fonts","Printer",NULL);
		if(ptr)
		{
			gtk_font_selection_set_font_name(GTK_FONT_SELECTION(widget),ptr);
			g_free(ptr);
		}
	}
 }

 static GObject * create_custom_widget(GtkPrintOperation *prt, PRINTINFO *cfg)
 {
 	GtkWidget *font_dialog =  gtk_font_selection_new();

    g_signal_connect(font_dialog, "realize", G_CALLBACK(load_font), cfg);

 	return G_OBJECT(font_dialog);
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *font_dialog, PRINTINFO *cfg)
 {
 	gchar *font;

 	if(main_configuration)
 	{
 		font = gtk_font_selection_get_font_name(GTK_FONT_SELECTION(font_dialog));
 		if(font)
 		{
			cfg->FontDescr = pango_font_description_from_string(font);
			DBGPrintf("%s = %p",font,cfg->FontDescr);
			g_key_file_set_string(main_configuration,"Fonts","Printer",font);
 			g_free(font);
 		}
 	}
 }

 #define DESTROY(x) if(x) { g_object_unref(x); x = 0; }

 static void ReleaseInfo(PRINTINFO *cfg)
 {
 	CHKPoint();
	DESTROY(cfg->setup);

 	CHKPoint();
	DESTROY(cfg->settings);

// FIXME (perry#1#): Why it segfaults at this point?
// 	CHKPoint();
//	DESTROY(cfg->FontDescr);

 	CHKPoint();
	DESTROY(cfg->FontContext);

 	CHKPoint();
	DESTROY(cfg->Font);

 	CHKPoint();
	DESTROY(cfg->FontMap);

 	CHKPoint();
	DESTROY(cfg->FontLayout);

 	if(cfg->text)
 		g_free(cfg->text);

	CHKPoint();
	g_free(cfg);
	DBGMessage("Print Job released");
 }

 static GtkPrintOperation * NewPrintOperation(const char *name, gchar *text)
 {
 	GtkPrintOperation	*prt;
 	PRINTINFO 			*cfg;

	gdk_lock();
	prt = gtk_print_operation_new();
	gdk_unlock();

	if(!prt)
		return 0;

	gdk_lock();

	gtk_print_operation_set_job_name(prt,name);

 	cfg = g_malloc(sizeof(PRINTINFO));
 	memset(cfg,0,sizeof(PRINTINFO));

	cfg->text = text;

 	g_object_set_data_full(G_OBJECT(prt),"JobInfo",(gpointer) cfg,(void (*)(gpointer)) ReleaseInfo);

	g_signal_connect(prt, "begin-print",    		G_CALLBACK(begin_print), 			cfg);
    g_signal_connect(prt, "draw-page",      		G_CALLBACK(draw_page),   			cfg);
    g_signal_connect(prt, "done",      				G_CALLBACK(print_done),	 			cfg);
    g_signal_connect(prt, "create-custom-widget",   G_CALLBACK(create_custom_widget),	cfg);
    g_signal_connect(prt, "custom-widget-apply",   	G_CALLBACK(custom_widget_apply),	cfg);

	gtk_print_operation_set_allow_async(prt,0);
	gtk_print_operation_set_show_progress(prt,1);

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
	if(main_configuration)
	{
		cfg->settings = gtk_print_settings_new_from_key_file(main_configuration,"PrintSettings",NULL);
		cfg->setup = gtk_page_setup_new_from_key_file(main_configuration,NULL,NULL);
	}
#endif

	if(!cfg->settings)
		cfg->settings = gtk_print_settings_new();

	if(!cfg->setup)
		cfg->setup = gtk_page_setup_new();

	gtk_print_operation_set_default_page_setup(prt,cfg->setup);

	DBGTracex(cfg->settings);
	gtk_print_operation_set_print_settings(prt,cfg->settings);

	gtk_print_operation_set_custom_tab_label(prt,_("Font"));

	gdk_unlock();

 	return prt;
 }

 void action_print(GtkWidget *w, gpointer data)
 {
 	GtkPrintOperation 	*prt =  NewPrintOperation("3270 Screen",CopyTerminalContents(0,0,-1,-1,0));

 	if(!prt)
 		return;

	// Run Print dialog
	gdk_lock();
	gtk_print_operation_run(prt,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(top_window),NULL);
	gdk_unlock();

    DESTROY(prt);
 }

 void action_print_copy(GtkWidget *w, gpointer data)
 {
    action_exec_with_copy(w,data ? data : PRINT_COMMAND);
 }

 void action_print_selection(GtkWidget *w, gpointer data)
 {
    action_exec_with_selection(w,data ? data : PRINT_COMMAND);
 }

#else

 void action_print(GtkWidget *w, gpointer data)
 {
 	action_exec_with_screen(w,data ? data : PRINT_COMMAND);
 }

 void action_print_copy(GtkWidget *w, gpointer data)
 {
    action_exec_with_copy(w,data ? data : PRINT_COMMAND);
 }

 void action_print_selection(GtkWidget *w, gpointer data)
 {
    action_exec_with_selection(w,data ? data : PRINT_COMMAND);
 }


#endif

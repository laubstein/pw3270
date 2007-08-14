

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

	// Get the current font for the printing operation
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

 #define DESTROY(x) if(x) { g_object_unref(x); x = 0; }

 static void ReleaseInfo(PRINTINFO *cfg)
 {
 	char *ptr;

 	CHKPoint();
	DESTROY(cfg->setup);

 	CHKPoint();
	DESTROY(cfg->settings);

	if(main_configuration && cfg->FontDescr)
	{
		ptr = pango_font_description_to_string(cfg->FontDescr);
		if(ptr)
		{
			g_key_file_set_string(main_configuration,"Fonts","Printer",ptr);
			g_free(ptr);
		}
	}


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

 static PRINTINFO * AllocInfo(GtkPrintOperation *prt)
 {
 	PRINTINFO 	*cfg;

	if(!prt)
		return 0;

 	cfg = g_malloc(sizeof(PRINTINFO));
 	memset(cfg,0,sizeof(PRINTINFO));

 	g_object_set_data_full(G_OBJECT(prt),"JobInfo",(gpointer) cfg,(void (*)(gpointer)) ReleaseInfo);

	g_signal_connect(prt, "begin-print",    G_CALLBACK(begin_print), cfg);
    g_signal_connect(prt, "draw-page",      G_CALLBACK(draw_page),   cfg);

	gtk_print_operation_set_allow_async(prt,0);
	gtk_print_operation_set_show_progress(prt,1);

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
	if(main_configuration)
		cfg->settings = gtk_print_settings_new_from_key_file(main_configuration,"PrintSettings",NULL);
#endif

	if(!cfg->settings)
		cfg->settings = gtk_print_settings_new();

	DBGTracex(cfg->settings);

 	return cfg;
 }

 void action_print(GtkWidget *w, gpointer data)
 {
 	GtkPrintOperation 	*prt = gtk_print_operation_new();
 	PRINTINFO			*cfg = AllocInfo(prt);;

 	if(!cfg)
 		return;

	// Set screen to print
	gtk_print_operation_set_job_name(prt,"3270 Screen");
	cfg->text = CopyTerminalContents(0,0,-1,-1,0);

	// Run Print dialog
	gtk_print_operation_run(prt,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(top_window),NULL);

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
    if(cfg->settings && main_configuration)
		gtk_print_settings_to_key_file(cfg->settings,main_configuration,"PrintSettings");
#endif

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

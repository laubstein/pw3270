

 #include "g3270.h"

/*---[ Structs ]--------------------------------------------------------------*/

 typedef struct _printinfo
 {
 	gchar 				*text;
 	GtkPrintSettings	*settings;
 } PRINTINFO;

/*---[ Lock/Unlock ]----------------------------------------------------------*/

/*---[ Implement ]------------------------------------------------------------*/

#if defined(DEBUG) && defined(__GTK_PRINT_OPERATION_H__) && defined(__G_KEY_FILE_H__)

 static void begin_print(GtkPrintOperation *prt, GtkPrintContext *context, PRINTINFO *cfg)
 {
 	CHKPoint();

	if(!prt->text)
		gtk_print_operation_cancel(prt);

	// Set rigth font for the print operation

	// http://developer.gnome.org/doc/API/2.0/pango/pango-Fonts.html

 }

 static void draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint page_nr, PRINTINFO *cfg)
 {
 	CHKPoint();
	gtk_print_operation_cancel(prt);

 }

 #define RELEASE(x) if(x) { g_object_unref(x); x = 0; }

 static void ReleaseInfo(PRINTINFO *cfg)
 {
	RELEASE(cfg->settings);

 	if(cfg->text)
 		g_free(cfg->text);

	g_free(cfg);
	DBGMessage("Print Job released");
 }

 static PRINTINFO * AllocInfo(GtkPrintOperation *prt)
 {
 	PRINTINFO *cfg;
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

    RELEASE(prt);
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

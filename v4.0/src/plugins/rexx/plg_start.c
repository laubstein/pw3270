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
 * Este programa está nomeado como main.c e possui 351 linhas de código.
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

/*
 * References:
 *
 * http://www.howzatt.demon.co.uk/articles/29dec94.html
 * http://www-01.ibm.com/support/docview.wss?rs=22&context=SS8PLL&dc=DB520&dc=DB560&uid=swg21140958&loc=en_US&cs=UTF-8&lang=en&rss=ct22other
 *
 */

 #define BUILDING_AS_PLUGIN 1

 #define INCL_RXSYSEXIT
 #define INCL_RXARI
 #include "rx3270.h"

 #include <gmodule.h>

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

 RexxExitHandler SysExit_SIO;	// Handler for screen I/O
 static struct trace_window_data * getTraceWindow(void);

/*---[ Globals ]----------------------------------------------------------------------------------*/

 static RXSYSEXIT ExitArray[] =
 {
   { "SysExit_SIO",	RXSIO 		},
   { NULL, 			RXENDLST 	}
 };


 GtkWidget 	*program_window	= NULL;
 GtkWidget	*trace_window = NULL;

 static SCRIPT_STATE script_state = SCRIPT_STATE_NONE;

/*---[ Implement ]--------------------------------------------------------------------------------*/

 #include "calls.h"

  int call_rexx(const gchar *filename, const gchar *arg)
 {
	LONG      			return_code;                 	// interpreter return code
	RXSTRING  			argv;           	          	// program argument string
	RXSTRING  			retstr;                      	// program return value
	RXSTRING			prg[2];							// Program data
	SHORT     			rc		= 0;                   	// converted return code
	GError				*error	= NULL;
	gsize				sz;

	if(script_state != SCRIPT_STATE_NONE)
	{
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(program_window),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "Can't start script" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "System busy" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",_( "Please, try again in a few moments" ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

		return EBUSY;
	}

	script_state = status_script(SCRIPT_STATE_RUNNING);

	// build the argument string
	memset(&argv,0,sizeof(argv));
	MAKERXSTRING(argv, arg, strlen(arg));

	// set up default return
	memset(&retstr,0,sizeof(retstr));

	Trace("%s","Running pending events");
	RunPendingEvents(0);

	/* Preload script file contents */
	Trace("Loading %s",filename);

	memset(prg,0,2*sizeof(RXSTRING));

	if(!g_file_get_contents(filename, (gchar **) (&prg[0].strptr), &sz, &error))
	{
		gchar *name = g_path_get_basename(filename);
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(program_window),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_OK,
													_(  "Can't load %s" ), name);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "File error" ) );
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s", error->message ? error->message : N_( "Unexpected error" ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
        g_free(name);

		g_error_free(error);
		return -1;
	}

	prg[0].strlength = strlen(prg[0].strptr);

	return_code = RexxStart(	1,				// argument count
								&argv,			// argument array
								NULL,			// REXX procedure name
								prg,			// program
								PACKAGE_NAME,	// default address name
								RXCOMMAND,		// calling as a subcommand
								ExitArray,		// EXITs for this call
								&rc,			// converted return code
								&retstr);		// returned result

	Trace("RexxStart(%s): %d",filename,(int) return_code);

	g_free(prg[0].strptr);

	// process return value
	Trace("Return value: \"%s\"",retstr.strptr);

	if(RXSTRPTR(prg[1]))
		RexxFreeMemory(RXSTRPTR(prg[1]));

	if(RXSTRPTR(retstr))
		RexxFreeMemory(RXSTRPTR(retstr));

	Trace("Call of \"%s\" ends (rc=%d return_code=%d)",filename,rc,(int) return_code);

	// Check script state
	if(script_state == SCRIPT_STATE_HALTED)
	{
		// Interrupted by user, no dialog
		rc = ECANCELED;
	}
 	else if(rc)
 	{
		gchar *name = g_path_get_basename(filename);
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(program_window),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_OK,
													_(  "script %s failed" ), name);

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Rexx script failed" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),_( "Return code was %d" ), rc);

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
        g_free(name);
 	}
 	else if(return_code)
 	{
		gchar *name = g_path_get_basename(filename);
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(program_window),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_OK,
													_( "Script %s aborted" ),
													name );

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Rexx script error" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),_( "Return code was %d" ), (int) return_code );

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
        g_free(name);
 	}

	script_state = status_script(SCRIPT_STATE_NONE);

	Trace("%s exits with rc=%d",__FUNCTION__,rc);
	return (int) rc;
 }

 void pw3270_plugin_startup(GtkWidget *topwindow, const gchar *script)
 {
	int	 f;

	program_window = topwindow;

	// Register Exit calls
	RexxRegisterExitExe( "SysExit_SIO", (PFN) SysExit_SIO, NULL );

 	// Load rexx calls
	for(f=0; f < G_N_ELEMENTS(rexx_exported_calls); f++)
		RexxRegisterFunctionExe((char *) rexx_exported_calls[f].name,rexx_exported_calls[f].call);

 	// Check for startup script
 	if(!(script && g_str_has_suffix(script,".rex")))
		return;

 	Trace("Calling %s",script);
	call_rexx(script,"");
 }

 static void activate_script(GtkMenuItem *menuitem, const gchar *path)
 {
 	Trace("\n-- \"%s\" --\n",path);
	call_rexx(path,path);
 	Trace("\n-- \"%s\" --\n",path);
 }

 void AddPluginUI(GtkUIManager *ui, const gchar *program_data)
 {
	gchar			*path;
	gchar			*filename;
 	GDir			*dir;
 	const gchar 	*name;
 	int				qtd		= 0;
 	GtkWidget 		*top	= gtk_ui_manager_get_widget(ui,"/MainMenubar/ScriptsMenu/RexxScripts");
 	GtkWidget		*menu;

	Trace("Rexx scripts menu: %p",top);

 	if(!top)
 		return;

	path = g_build_filename(program_data,"rexx",NULL);

    dir = g_dir_open(path,0,NULL);

    if(!dir)
    {
   		gtk_widget_hide(top);
    	g_free(path);
		return;
    }

	menu = gtk_menu_new();
	name = g_dir_read_name(dir);

	while(name)
	{
		filename = g_build_filename(path,name,NULL);

		if(g_str_has_suffix(filename,"rex"))
		{
 			GtkWidget 	*item = gtk_menu_item_new_with_label(name);
 			Trace("Appending script %s (item: %p)",name,item);

 			g_object_set_data_full(G_OBJECT(item),"filename",filename,g_free);
			g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(activate_script),filename);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
			qtd++;
		}
		else
		{
			g_free(filename);
		}

		name = g_dir_read_name(dir);
	}
	g_dir_close(dir);

	if(qtd)
		gtk_widget_show_all(menu);
	else
   		gtk_widget_hide(top);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(top),menu);

	g_free(path);

 }

 static void destroy_trace(GtkWidget *widget,gpointer data)
 {
 	Trace("%s","Rexx trace destroyed");
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

	if(script_state == SCRIPT_STATE_HALTED)
		return NULL;

	// Create a new trace window
	trace_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(trace_window),"destroy",G_CALLBACK(destroy_trace),0);

	data = g_malloc0(sizeof(struct trace_window_data));
	g_object_set_data_full(G_OBJECT(trace_window),"window_data",data,g_free);

	gtk_window_set_transient_for(GTK_WINDOW(trace_window),GTK_WINDOW(program_window));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(trace_window),TRUE);

	gtk_window_set_title(GTK_WINDOW(trace_window),_( "Rexx trace" ));

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

 static void showtracemessage(const gchar *msg)
 {
 	struct trace_window_data *data = getTraceWindow();
 	GtkTextIter itr;

	if(!data)
		return;

	gtk_text_buffer_get_end_iter(data->text,&itr);
	gtk_text_buffer_insert(data->text,&itr,msg,strlen(msg));

	gtk_text_buffer_get_end_iter(data->text,&itr);
	gtk_text_buffer_insert(data->text,&itr,"\n",1);

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 14
	gtk_adjustment_set_value(data->scroll,gtk_adjustment_get_upper(data->scroll));
#else
	gtk_adjustment_set_value(data->scroll,(GTK_ADJUSTMENT(data->scroll))->upper);
#endif
 }

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

 LONG APIENTRY SysExit_SIO(LONG ExitNumber, LONG  Subfunction, PEXIT ParmBlock)
 {
 	LONG		retcode = RXEXIT_HANDLED;
 	GtkWidget	*dialog;

 	Trace("%s call with ExitNumber: %d Subfunction: %d",__FUNCTION__,(int) ExitNumber, (int) Subfunction);

	switch(Subfunction)
	{
	case RXSIOSAY:	// SAY a line to STDOUT
		 dialog = gtk_message_dialog_new(	GTK_WINDOW(program_window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_INFO,
											GTK_BUTTONS_OK_CANCEL,
											"%s", (((RXSIOSAY_PARM *) ParmBlock)->rxsio_string).strptr );

		if(trace_window)
			showtracemessage((((RXSIOSAY_PARM *) ParmBlock)->rxsio_string).strptr);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Script message" ) );

        if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
        {
			if(RaiseHaltSignal() != RXARI_OK)
				retcode = RXEXIT_RAISE_ERROR;
        }
        gtk_widget_destroy(dialog);
		break;

	case RXSIOTRC:	// Trace output
		showtracemessage((((RXSIOTRC_PARM *) ParmBlock)->rxsio_string).strptr);
		break;

	case RXSIOTRD:	// Read from char stream
		gettracecommand( & (((RXSIOTRD_PARM *) ParmBlock)->rxsiotrd_retc) );
		break;

	case RXSIODTR:	// DEBUG read from char stream
		gettracecommand( & (((RXSIODTR_PARM *) ParmBlock)->rxsiodtr_retc) );
		break;

	case RXSIOTLL:	// Return linelength(N/A OS/2)
		return RXEXIT_NOT_HANDLED;
	}

	return retcode;
 }

 ULONG RetConvertedString(PRXSTRING Retstr, const char *value)
 {
 	if(!value)
 	{
 		// Empty string doesn't need to be converted
 		strcpy(Retstr->strptr,"");
 	}
 	else
 	{
 		// Convert received string to UTF-8
 		gchar *str = g_convert(value, -1, REXX_CHARSET, CHARSET, NULL, NULL, NULL);

		if(strlen(str) > (RXAUTOBUFLEN-1))
		{
			Retstr->strptr = RexxAllocateMemory(strlen(str)+1);
			strcpy(Retstr->strptr,str);
		}
		else
		{
			g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%s",str);
		}

		g_free(str);
 	}

    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 int IsHalted(void)
 {
 	return script_state != SCRIPT_STATE_RUNNING;
 }

 ULONG RaiseHaltSignal(void)
 {
	script_state = SCRIPT_STATE_HALTED;

#ifdef WIN32
	return RexxSetHalt(getpid(),GetCurrentThreadId());
#else
	return RexxSetHalt(getpid(),pthread_self());
#endif

 }

 ULONG APIENTRY rx3270QueryRunMode(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	strncpy(Retstr->strptr,"PLUGIN",RXAUTOBUFLEN-1);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

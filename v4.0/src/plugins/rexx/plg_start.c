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

 #define INCL_RXSYSEXIT
 #include "rx3270.h"

 #include <gmodule.h>

// http://www.howzatt.demon.co.uk/articles/29dec94.html

/*---[ Prototipes ]-------------------------------------------------------------------------------*/

 RexxExitHandler SysExit_SIO;	// Handler for screen I/O

 static RXSYSEXIT ExitArray[] =
 {
   { "SysExit_SIO",	RXSIO 		},
   { NULL, 			RXENDLST 	}
 };

/*---[ Structs ]----------------------------------------------------------------------------------*/

 struct blinker
 {
 	gboolean enabled;
 	gboolean status;
 };

/*---[ Globals ]----------------------------------------------------------------------------------*/

 GtkWidget 	*program_window	= NULL;
 GtkWidget	*trace_window = NULL;

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static gboolean do_blink(struct blinker *blink)
 {
 	if(!blink->enabled)
 	{
		status_script(FALSE);
		return FALSE;
 	}

	status_script(blink->status);
 	blink->status = !blink->status;

	return TRUE;
 }

 int call_rexx(const gchar *prg, const gchar *arg)
 {
	LONG      			return_code;                 	// interpreter return code
	RXSTRING  			argv;           	          	// program argument string
	RXSTRING  			retstr;                      	// program return value
	SHORT     			rc		= 0;                   	// converted return code
	CHAR      			return_buffer[RXAUTOBUFLEN];	// returned buffer
	struct blinker		*blink;

	blink = g_malloc0(sizeof(struct blinker));

	blink->enabled = TRUE;
	status_script(TRUE);

	g_timeout_add_full(G_PRIORITY_DEFAULT, (guint) 600, (GSourceFunc) do_blink, blink, g_free);

	// build the argument string
	memset(&argv,0,sizeof(argv));
	MAKERXSTRING(argv, arg, strlen(arg));

	// set up default return
	*return_buffer = 0;
	memset(&retstr,0,sizeof(retstr));
	MAKERXSTRING(retstr, return_buffer, sizeof(RXAUTOBUFLEN));

	Trace("%s","Running pending events");
	RunPendingEvents(0);

	Trace("Starting %s",prg);

	return_code = RexxStart(	1,				// argument count
								&argv,			// argument array
								(char *) prg,	// REXX procedure name
								NULL,			// use disk version
								"",				// default address name
								RXCOMMAND,		// calling as a subcommand
								ExitArray,		// EXITs for this call
								&rc,			// converted return code
								&retstr);		// returned result

	Trace("RexxStart(%s): %d",prg,(int) return_code);

	// process return value
	Trace("Return value: \"%s\"",retstr.strptr);

	blink->enabled = FALSE;

	if(RXSTRPTR(retstr) && RXSTRPTR(retstr) != return_buffer)
	{
		Trace("Releasing %p (expected %p)",RXSTRPTR(retstr),return_buffer);
		RexxFreeMemory(RXSTRPTR(retstr));
	}

	Trace("Call of \"%s\" ends",prg);

 	if(rc)
 	{
		gchar *name = g_path_get_basename(prg);
		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_OK,
													_(  "script %s failed" ), name);

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Script failed" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),_( "Return code was %d" ), rc);

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
        g_free(name);
 	}
 	else if(return_code)
 	{
		gchar *name = g_path_get_basename(prg);
		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_OK,
													_( "Can't start \"%s\"" ),
													name );

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Can't start script" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),_( "Return code was %d" ), (int) return_code );

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
        g_free(name);
 	}

	return (int) rc;
 }

 void pw3270_plugin_startup(GtkWidget *topwindow, const gchar *script)
 {
	int	 f;

	program_window = topwindow;

	// Register Exit calls
	RexxRegisterExitExe( "SysExit_SIO", (PFN) SysExit_SIO, NULL );

 	// Load common functions
 	Trace("Loading %d common calls",rexx_common_calls_count);
	for(f=0;f < rexx_common_calls_count; f++)
		RexxRegisterFunctionExe((char *) rexx_common_calls[f].name,rexx_common_calls[f].call);

 	// Load plugin functions
 	Trace("Loading %d plugin calls",rexx_plugin_calls_count);
	for(f=0;f < rexx_plugin_calls_count; f++)
		RexxRegisterFunctionExe((char *) rexx_plugin_calls[f].name,rexx_plugin_calls[f].call);

/*
	// Disable standalone functions
 	Trace("Disabling %d standalone calls",rexx_standalone_calls_count);
	for(f=0;f < rexx_standalone_calls_count; f++)
		RexxRegisterFunctionExe((char *) rexx_standalone_calls[f].name,(PFN) rx3270Dunno);
*/

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

 void AddPluginUI(GtkUIManager *ui)
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

#if defined( DEBUG )
	path = g_build_filename("..","..","rexx",NULL);
#elif defined( DATAROOTDIR )
	path = g_build_filename(DATAROOTDIR,PACKAGE_NAME,"rexx",NULL);
#else
	path = g_build_filename(".","rexx",NULL);
#endif

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
 			GtkWidget *item = gtk_menu_item_new_with_label(name);
 			Trace("Appending script %s (item: %p)",name,item);
			g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(activate_script),g_strdup(filename));
			gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
			qtd++;
		}

		g_free(filename);
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

 static GtkWidget * getTraceWindow(void)
 {
 	GtkWidget *widget;
 	GtkWidget *view;

 	if(trace_window)
		return trace_window;

	// Create a new trace window
	trace_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(trace_window),"destroy",G_CALLBACK(destroy_trace),0);
	gtk_window_set_transient_for(GTK_WINDOW(trace_window),GTK_WINDOW(program_window));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(trace_window),TRUE);
	gtk_window_set_title(GTK_WINDOW(trace_window),_( "Rexx trace" ));

	// Create text box
	widget = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	view = gtk_text_view_new();
	g_object_set_data(G_OBJECT(trace_window),"TextBuffer",gtk_text_view_get_buffer(GTK_TEXT_VIEW(view)));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),view);

	gtk_container_add(GTK_CONTAINER(trace_window),widget);

	// Show dialog


	Trace("Trace window %p created",trace_window);
	gtk_window_set_default_size(GTK_WINDOW(trace_window),590,430);
	gtk_widget_show_all(trace_window);
	return trace_window;
 }

 static void showtracemessage(const gchar *msg)
 {
 	GtkWidget *widget = getTraceWindow();
 	GtkTextBuffer *buffer;
 	GtkTextIter itr;

 	Trace("%s (Tracewindow: %p)",msg,widget);

 	if(!widget)
		return;

	buffer = (GtkTextBuffer *) g_object_get_data(G_OBJECT(widget),"TextBuffer");

	Trace("Textbuffer: %p",buffer);

	if(!buffer)
		return;

	gtk_text_buffer_get_end_iter(buffer,&itr);
	gtk_text_buffer_insert(buffer,&itr,msg,strlen(msg));

	gtk_text_buffer_get_end_iter(buffer,&itr);
	gtk_text_buffer_insert(buffer,&itr,"\n",1);

	Trace("%s","Texto incluido no buffer");

 }

 LONG APIENTRY SysExit_SIO(LONG ExitNumber, LONG  Subfunction, PEXIT ParmBlock)
 {
 	GtkWidget *dialog;

 	Trace("%s call with ExitNumber: %d Subfunction: %d",__FUNCTION__,(int) ExitNumber, (int) Subfunction);

	switch(Subfunction)
	{
	case RXSIOSAY:	// SAY a line to STDOUT
		 dialog = gtk_message_dialog_new(	GTK_WINDOW(program_window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_INFO,
											GTK_BUTTONS_OK,
											"%s", (((RXSIOSAY_PARM *) ParmBlock)->rxsio_string).strptr );

		if(trace_window)
			showtracemessage((((RXSIOSAY_PARM *) ParmBlock)->rxsio_string).strptr);

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Rexx message" ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
		break;

	case RXSIOTRC:	// Trace output
		showtracemessage((((RXSIOTRC_PARM *) ParmBlock)->rxsio_string).strptr);
		break;

	case RXSIOTRD:	// Read from char stream
		return RXEXIT_NOT_HANDLED;

	case RXSIODTR:	// DEBUG read from char stream
		// TODO (perry#1#): Get line from trace window.
		return RXEXIT_NOT_HANDLED;

	case RXSIOTLL:	// Return linelength(N/A OS/2)
		return RXEXIT_NOT_HANDLED;
	}

	return RXEXIT_HANDLED;
 }

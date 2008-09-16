/*
 * Modifications and original code Copyright 1993, 1994, 1995, 1996,
 *    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *   All Rights Reserved.  GTRC hereby grants public use of this software.
 *   Derivative works based on this software must incorporate this copyright
 *   notice.
 *
 * c3270 and wc3270 are distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

 /* include the REXX stuff */
 #define RXFUNC_BADCALL 40
// #define INCL_REXXSAA	/* Complete Rexx support */
// #define INCL_RXSUBCOM	/* Rexx subcommand handler support */
// #define INCL_RXSHV		/* Rexx shared variable pool support */
 #define INCL_RXFUNC	/* Rexx external function support */
// #define INCL_RXSYSEXIT	/* Rexx system exit support */
// #define INCL_RXARI		/* Rexx asynchronous Trace/Halt support */
 #include <rexx.h>

 /* include the glib stuff */
 #include <gmodule.h>
 #include <glib/gi18n.h>

 /* include the "C" stuff */
 #include <string.h>
 #include <sys/time.h>                   /* System time-related data types */
 #include <stdlib.h>

 /* include the lib3270 stuff */
 #define G3270_MODULE_NAME "rexx"
 #include <lib3270/config.h>
 #include <lib3270/api.h>
 #include <lib3270/plugins.h>
 #include <lib3270/localdefs.h>
 #include <lib3270/statusc.h>
 #include <lib3270/toggle.h>

 #define CONFIG_GROUP "Rexx"

/*---[ Structs ]----------------------------------------------------------------------------------*/

 struct blinker
 {
 	gboolean enabled;
 	gboolean status;
 };

/*---[ Statics ]----------------------------------------------------------------------------------*/

 static GtkWidget 		*topwindow 			= NULL;
 static GtkActionGroup *action_groups[ACTION_GROUP_MAX];

/*---[ Rexx entry points ]------------------------------------------------------------------------*/

 static ULONG RetValue(PRXSTRING Retstr, int value)
 {
	g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%d",value);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 static ULONG RetString(PRXSTRING Retstr, const char *value)
 {
 	if(!value)
 	{
 		strcpy(Retstr->strptr,"");
 	}
 	else if(strlen(value) > (RXAUTOBUFLEN-1))
 	{
 		Retstr->strptr = RexxAllocateMemory(strlen(value)+1);
 		strcpy(Retstr->strptr,value);
 	}
 	else
 	{
		g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%s",value);
 	}

    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 ULONG APIENTRY rx3270Version(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	strncpy(Retstr->strptr,PACKAGE_VERSION,RXAUTOBUFLEN-1);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 ULONG APIENTRY rx3270Actions(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		GtkAction	*action = NULL;
		int			p;

 		Trace("Searching action %s",Argv[f].strptr);

		for(p = 0; p < ACTION_GROUP_MAX && !action; p++)
			action = gtk_action_group_get_action(action_groups[p],Argv[f].strptr);

		if(!action)
			return RXFUNC_BADCALL;

 		Trace("Running action %s: %p",Argv[f].strptr,action);

 		gtk_action_activate(action);

 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270ToggleON(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			set_toggle(toggle,TRUE);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270ToggleOFF(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			set_toggle(toggle,FALSE);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Toggle(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			do_toggle(toggle);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Toggled(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	int toggle;

	if(Argc != 1)
		return RXFUNC_BADCALL;

	toggle = get_toggle_by_name(Argv[0].strptr);

	if(toggle < 0)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,Toggled(toggle));
 }

 ULONG APIENTRY rx3270Log(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	Log("%s",Argv[0].strptr);

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Popup(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	static const struct _msgtype
 	{
 		const gchar 	*name;
 		GtkMessageType type;
 	} msgtype[] =
 	{
		{ "INFO",		GTK_MESSAGE_INFO		},
		{ "WARNING",	GTK_MESSAGE_WARNING		},
		{ "QUESTION",	GTK_MESSAGE_QUESTION	},
		{ "ERROR",		GTK_MESSAGE_ERROR		}
 	};

	int				f;
 	GtkMessageType	type = GTK_MESSAGE_INFO;
 	GtkWidget		*dialog;
 	int				rc = 0;

	Trace("Argc: %d",(int) Argc);
	if(Argc < 1 )
		return RXFUNC_BADCALL;

	if(Argc > 1 )
	{
		type = GTK_MESSAGE_OTHER;

		Trace("Popup type: \"%s\"",Argv[1].strptr);

		for(f = 0; f < G_N_ELEMENTS(msgtype) && type == GTK_MESSAGE_OTHER; f++)
		{
			if(!g_ascii_strncasecmp(msgtype[f].name,Argv[1].strptr,strlen(msgtype[f].name)))
			{
				type = msgtype[f].type;
				Trace("Messagetype: %s (%d)",msgtype[f].name,(int) type);
			}
		}
	}


 	dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										type,
										GTK_BUTTONS_CLOSE,
										"%s",Argv[0].strptr );

	rc = gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);
	return RetValue(Retstr,rc);
 }

 ULONG APIENTRY rx3270TerminalInput(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	Input_String((const unsigned char *) Argv[0].strptr);

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270QueryCState(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	#define DECLARE_XLAT_STATE( x ) { x, #x }

 	static const struct _xlat_state
 	{
 		enum cstate	state;
 		const gchar	*ret;
 	} xlat_state[] =
 	{
			DECLARE_XLAT_STATE( NOT_CONNECTED 		),
			DECLARE_XLAT_STATE( RESOLVING			),
			DECLARE_XLAT_STATE( PENDING				),
			DECLARE_XLAT_STATE( CONNECTED_INITIAL	),
			DECLARE_XLAT_STATE( CONNECTED_ANSI		),
			DECLARE_XLAT_STATE( CONNECTED_3270		),
			DECLARE_XLAT_STATE( CONNECTED_INITIAL_E	),
			DECLARE_XLAT_STATE( CONNECTED_NVT		),
			DECLARE_XLAT_STATE( CONNECTED_SSCP		),
			DECLARE_XLAT_STATE( CONNECTED_TN3270E	)
 	};

 	int				f;
 	enum cstate 	state = QueryCstate();

	if(Argc != 0)
		return RXFUNC_BADCALL;

	for(f=0;f < G_N_ELEMENTS(xlat_state); f++)
	{
		if(state == xlat_state[f].state)
		{
			return RetString(Retstr,xlat_state[f].ret);
		}
	}

	return RetString(Retstr,"UNEXPECTED");
 }

 ULONG APIENTRY rx3270UpdateScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc)
		return RXFUNC_BADCALL;

	while(gtk_events_pending())
		gtk_main_iteration();

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Sleep(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	clock_t end = clock();

	switch(Argc)
	{
	case 0:
		end += CLOCKS_PER_SEC;
		break;

	case 1:
		end += (atoi(Argv[0].strptr) * CLOCKS_PER_SEC);
		break;

	default:
		return RXFUNC_BADCALL;
	}

	while(clock() < end)
		gtk_main_iteration();

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270FindFieldAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,find_field_attribute(atoi(Argv[0].strptr)));
 }

 ULONG APIENTRY rx3270FindFieldLength(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,find_field_length(atoi(Argv[0].strptr)));
 }

 ULONG APIENTRY rx3270MoveCursor(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	cursor_move(atoi(Argv[0].strptr));

	return RetValue(Retstr,0);
 }

 #define EXPORT_REXX_FUNCTION(x) { #x, (PFN) x }

 static const struct _entrypoint
 {
 	const char 	*name;
 	PFN				call;
 } entrypoint[] =
 {
	EXPORT_REXX_FUNCTION( rx3270Version				),
	EXPORT_REXX_FUNCTION( rx3270Actions				),
	EXPORT_REXX_FUNCTION( rx3270ToggleON			),
	EXPORT_REXX_FUNCTION( rx3270ToggleOFF			),
	EXPORT_REXX_FUNCTION( rx3270Toggle				),
	EXPORT_REXX_FUNCTION( rx3270Toggled				),
	EXPORT_REXX_FUNCTION( rx3270Log					),
	EXPORT_REXX_FUNCTION( rx3270QueryCState			),
	EXPORT_REXX_FUNCTION( rx3270Popup 				),
	EXPORT_REXX_FUNCTION( rx3270UpdateScreen		),
	EXPORT_REXX_FUNCTION( rx3270Sleep				),
	EXPORT_REXX_FUNCTION( rx3270FindFieldAttribute	),
	EXPORT_REXX_FUNCTION( rx3270FindFieldLength		),
	EXPORT_REXX_FUNCTION( rx3270MoveCursor			)
 };

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

 static void call_rexx(const gchar *prg, const gchar *arg)
 {
	LONG      			return_code;                 // interpreter return code
	RXSTRING  			argv[1];                     // program argument string
	RXSTRING  			retstr;                      // program return value
	SHORT     			rc;                          // converted return code
	CHAR      			return_buffer[250];          // returned buffer
	struct blinker		*blink	= g_malloc0(sizeof(struct blinker));

	blink->enabled = TRUE;
	status_script(TRUE);

	g_timeout_add_full(G_PRIORITY_DEFAULT, (guint) 600, (GSourceFunc) do_blink, blink, g_free);

	// build the argument string
	MAKERXSTRING(argv[0], arg, strlen(arg));

	// set up default return
	MAKERXSTRING(retstr, return_buffer, sizeof(return_buffer));

	while(gtk_events_pending())
		gtk_main_iteration();


	return_code = RexxStart(	1,				// one argument
								argv,			// argument array
								(char *) prg,	// REXX procedure name
								NULL,			// use disk version
								"Editor",		// default address name
								RXCOMMAND,		// calling as a subcommand
								NULL,			// no exits used
								&rc,			// converted return code
								&retstr);		// returned result

	Trace("RexxStart(%s): %d",prg,(int) return_code);

	// process return value
	while(!RexxDidRexxTerminate())
		gtk_main_iteration();

	blink->enabled = FALSE;

	// need to return storage?
	if(RXSTRPTR(retstr) != return_buffer)
		RexxFreeMemory(RXSTRPTR(retstr));

 }

 /*
  * If a module contains a function named g_module_check_init() it is called automatically
  * when the module is loaded. It is passed the GModule structure and should return NULL
  * on success or a string describing the initialization error.
  */
 const gchar * g_module_check_init(GModule *module)
 {
 	const gchar	*name = g_module_name(module);
	int				f;

 	Trace("Rexx module %p loaded (%s)",module,name);

	memset(action_groups,0,sizeof(GtkActionGroup *) *ACTION_GROUP_MAX);

	for(f=0;f < G_N_ELEMENTS(entrypoint); f++)
		RexxRegisterFunctionExe((char *) entrypoint[f].name,entrypoint[f].call);

 	return NULL;
 }

 /*
  * If a module contains a function named g_module_unload() it is called automatically
  * when the module is unloaded. It is passed the GModule structure.
  */
 void g_module_unload(GModule *module)
 {
 	Trace("Rexx module %p unloaded",module);
 }

 static void RunRexxScript(GtkAction *action, GKeyFile *conf)
 {
	gchar 		*ptr;
	GtkWidget 	*dialog = gtk_file_chooser_dialog_new(	_( "Select Rexx script to run" ),
														 GTK_WINDOW(topwindow),
														 GTK_FILE_CHOOSER_ACTION_OPEN,
														 GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														 GTK_STOCK_EXECUTE,	GTK_RESPONSE_ACCEPT,
														 NULL );


	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),FALSE);

	ptr = g_key_file_get_string(conf,CONFIG_GROUP,"lasturi",NULL);
	if(ptr)
	{
		gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(dialog),ptr);
		g_free(ptr);
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_hide(dialog);
		g_key_file_set_string(conf,CONFIG_GROUP,"lasturi",gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog)));
		call_rexx(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)),"");
	}

	gtk_widget_destroy(dialog);

 }

 void LoadCustomActions(GtkUIManager *ui, GtkActionGroup **groups, guint n_actions, GKeyFile *conf)
 {
 	static const struct _action_info
 	{
 		const gchar *name;
 		const gchar *label;
 		const gchar *tooltip;
 		void (*call)(GtkAction *action, GKeyFile *conf);
 	} action_info[] =
 	{
 		{ "RunRexxScript", N_( "Run rexx script" ), NULL, RunRexxScript }
 	};

	int f;

 	if(n_actions != ACTION_GROUP_MAX)
 	{
 		Log( "Unexpected number of actions (expected: %d received: %d)",ACTION_GROUP_MAX,n_actions);
 		return;
 	}

	Trace("Loading %d rexx actions",G_N_ELEMENTS(action_info));

	for(f=0;f < ACTION_GROUP_MAX;f++)
		action_groups[f] = groups[f];

	for(f=0;f<G_N_ELEMENTS(action_info);f++)
	{
		GtkAction *action = gtk_action_new(	action_info[f].name,
											gettext(action_info[f].label),
											gettext(action_info[f].tooltip),
											NULL );

		g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(action_info[f].call),(gpointer) conf);
		gtk_action_group_add_action(groups[0],action);
	}
 }


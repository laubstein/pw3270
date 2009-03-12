/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe.
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
 * Este programa está nomeado como main.c e possui 286 linhas de código.
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


 #include "rx3270.h"

 #include <gmodule.h>


/*---[ Structs ]----------------------------------------------------------------------------------*/

 struct blinker
 {
 	gboolean enabled;
 	gboolean status;
 };

/*---[ Globals ]----------------------------------------------------------------------------------*/

 GtkWidget 	*g3270_topwindow	= NULL;

/*---[ Rexx entry points ]------------------------------------------------------------------------*/

 #define EXPORT_REXX_FUNCTION(x) 		{ #x, (PFN) x }

 struct entrypoint
 {
 	const char 	*name;
 	PFN				call;
 };

 static const struct entrypoint common_entrypoint[] =
 {
	EXPORT_REXX_FUNCTION( rx3270Version				    ),
	EXPORT_REXX_FUNCTION( rx3270Connect				    ),
	EXPORT_REXX_FUNCTION( rx3270Disconnect				),
	EXPORT_REXX_FUNCTION( rx3270QueryScreenAttribute    ),
	EXPORT_REXX_FUNCTION( rx3270ToggleON			    ),
	EXPORT_REXX_FUNCTION( rx3270ToggleOFF			    ),
	EXPORT_REXX_FUNCTION( rx3270Toggle				    ),
	EXPORT_REXX_FUNCTION( rx3270Toggled				    ),
	EXPORT_REXX_FUNCTION( rx3270Log					    ),
	EXPORT_REXX_FUNCTION( rx3270QueryCState			    ),
	EXPORT_REXX_FUNCTION( rx3270UpdateScreen		    ),
	EXPORT_REXX_FUNCTION( rx3270Sleep				    ),
	EXPORT_REXX_FUNCTION( rx3270InputString			    ),
	EXPORT_REXX_FUNCTION( rx3270FindFieldAttribute	    ),
	EXPORT_REXX_FUNCTION( rx3270FindFieldLength		    ),
	EXPORT_REXX_FUNCTION( rx3270MoveCursor			    ),
	EXPORT_REXX_FUNCTION( rx3270GetCursorPosition	    ),
	EXPORT_REXX_FUNCTION( rx3270ReadScreen			    ),
	EXPORT_REXX_FUNCTION( rx3270SendPFKey			    ),
	EXPORT_REXX_FUNCTION( rx3270WaitForChanges		    ),
	EXPORT_REXX_FUNCTION( rx3270SendENTERKey			),
	EXPORT_REXX_FUNCTION( rx3270WaitForString			),
 };

 static const struct entrypoint plugin_entrypoint[] =
 {
	EXPORT_REXX_FUNCTION( rx3270Popup 				    ),
	EXPORT_REXX_FUNCTION( rx3270Actions				    ),
	EXPORT_REXX_FUNCTION( rx3270Quit					),
	EXPORT_REXX_FUNCTION( rx3270SetVisible				),
 };

 static const struct entrypoint standalone_entrypoint[] =
 {
	EXPORT_REXX_FUNCTION( rx3270LoadFuncs				),
	EXPORT_REXX_FUNCTION( rx3270Init				    )
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

 int call_rexx(const gchar *prg, const gchar *arg)
 {
	LONG      			return_code;                 	// interpreter return code
	RXSTRING  			argv;           	          	// program argument string
	RXSTRING  			retstr;                      	// program return value
	SHORT     			rc		= 0;                   	// converted return code
	CHAR      			return_buffer[RXAUTOBUFLEN];	// returned buffer
	struct blinker		*blink	= g_malloc0(sizeof(struct blinker));

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
	return_code = RexxStart(	1,				// No argument
								&argv,			// argument array
								(char *) prg,	// REXX procedure name
								NULL,			// use disk version
								"",				// default address name
								RXCOMMAND,		// calling as a subcommand
								NULL,			// no exits used
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

	return (int) rc;
 }

 /*
  * If a module contains a function named g_module_check_init() it is called automatically
  * when the module is loaded. It is passed the GModule structure and should return NULL
  * on success or a string describing the initialization error.
  */ /*
 const gchar * g_module_check_init(GModule *module)
 {
 	return NULL;
 }
*/

 void g3270_plugin_startup(GtkWidget *topwindow, const gchar *script)
 {
	int	 f;

	g3270_topwindow = topwindow;

 	// Load common functions
 	Trace("Loading %d common calls",G_N_ELEMENTS(common_entrypoint));
	for(f=0;f < G_N_ELEMENTS(common_entrypoint); f++)
		RexxRegisterFunctionExe((char *) common_entrypoint[f].name,common_entrypoint[f].call);

 	// Load plugin functions
 	Trace("Loading %d plugin calls",G_N_ELEMENTS(plugin_entrypoint));
	for(f=0;f < G_N_ELEMENTS(plugin_entrypoint); f++)
		RexxRegisterFunctionExe((char *) plugin_entrypoint[f].name,plugin_entrypoint[f].call);

	// Disable standalone functions
 	Trace("Disabling %d standalone calls",G_N_ELEMENTS(standalone_entrypoint));
	for(f=0;f < G_N_ELEMENTS(standalone_entrypoint); f++)
		RexxRegisterFunctionExe((char *) standalone_entrypoint[f].name,(PFN) rx3270Dunno);


 	// Check for startup script
 	if(!(script && g_str_has_suffix(script,".rex")))
		return;

 	Trace("Calling %s",script);
	call_rexx(script,"");
 }



/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270LoadFuncs                                    */
/*                                                                            */
/* Description: Register all functions in this library.                       */
/*                                                                            */
/* Rexx Args:   None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
ULONG APIENTRY rx3270LoadFuncs(PSZ Name, LONG Argc, RXSTRING Argv[], PSZ Queuename, PRXSTRING Retstr)
{
	int	 f;

	g3270_topwindow = 0;

 	// Load common functions
 	Trace("Loading %d common calls",G_N_ELEMENTS(common_entrypoint));
	for(f=0;f < G_N_ELEMENTS(common_entrypoint); f++)
		RexxRegisterFunctionExe((char *) common_entrypoint[f].name,common_entrypoint[f].call);

 	// Disable plugin functions
 	Trace("Disabing %d plugin calls",G_N_ELEMENTS(plugin_entrypoint));
	for(f=0;f < G_N_ELEMENTS(plugin_entrypoint); f++)
		RexxRegisterFunctionExe((char *) plugin_entrypoint[f].name,(PFN) rx3270Dunno);

	// Load standalone functions
 	Trace("Loading %d standalone calls",G_N_ELEMENTS(standalone_entrypoint));
	for(f=0;f < G_N_ELEMENTS(standalone_entrypoint); f++)
		RexxRegisterFunctionExe((char *) standalone_entrypoint[f].name,standalone_entrypoint[f].call);

	return RetValue(Retstr,-1);
}

 static void activate_script(GtkMenuItem *menuitem, const gchar *path)
 {
 	Trace("-- \"%s\" --",path);
	call_rexx(path,path);
 	Trace("-- \"%s\" --",path);
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

 ULONG RetValue(PRXSTRING Retstr, int value)
 {
	g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%d",value);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 ULONG RetString(PRXSTRING Retstr, const char *value)
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



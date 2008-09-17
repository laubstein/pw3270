/*
* Copyright 2008, Banco do Brasil S.A.
*
* This file is part of g3270
*
* This program file is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program in a file named COPYING; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
*
* Authors:
*
* Perry Werneck<perry.werneck@gmail.com>
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

 GtkWidget 	*g3270_topwindow = NULL;

/*---[ Rexx entry points ]------------------------------------------------------------------------*/


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
	EXPORT_REXX_FUNCTION( rx3270InputString			),
	EXPORT_REXX_FUNCTION( rx3270FindFieldAttribute	),
	EXPORT_REXX_FUNCTION( rx3270FindFieldLength		),
	EXPORT_REXX_FUNCTION( rx3270MoveCursor			),
	EXPORT_REXX_FUNCTION( rx3270ReadScreen			),
	EXPORT_REXX_FUNCTION( rx3270SendPFKey			)
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

	Trace("Waiting for events for %s",prg);
	while(gtk_events_pending())
		gtk_main_iteration();

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
	/*
	while(!RexxDidRexxTerminate())
	{
		gtk_main_iteration();
	}
	*/

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
  */
 const gchar * g_module_check_init(GModule *module)
 {
 	const gchar	*name = g_module_name(module);
	int				f;

 	Trace("Rexx module %p loaded (%s)",module,name);

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


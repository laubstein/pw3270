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
 * Este programa está nomeado como actions.c e possui 158 linhas de código.
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

/*---[ Implement ]--------------------------------------------------------------------------------*/

 PW3270_PLUGIN_ENTRY void script_rexx_activated(GtkWidget *widget, GtkWidget *window)
 {
 	struct rexx_translated_data
 	{
		size_t  strlength;
		char	strptr[1];
 	};

 	struct rexx_translated_data	*rexx_data = g_object_get_data(G_OBJECT(widget),"rexx_translated_data");

	LONG      		return_code;                 	// interpreter return code
#ifdef CONSTRXSTRING
	CONSTRXSTRING	argv;           	          	// program argument string
#else
	RXSTRING		argv;           	          	// program argument string
#endif
	RXSTRING		retstr;                      	// program return value
	RXSTRING		prg[2];							// Program data
	gchar			*name;
	short			rc;
 	const gchar 	*filename	= g_object_get_data(G_OBJECT(widget),"script_filename");

	// Clear structues
	memset(&retstr,0,sizeof(retstr));
	memset(prg,0,2*sizeof(RXSTRING));

	// Lock engine and load script
	if(lock_rexx_script_engine(window))
		return;

	if(load_rexx_script(filename,prg))
		return;

	// Get script arguments
	argv.strptr = g_object_get_data(G_OBJECT(widget),"script_arguments");
	if(!argv.strptr)
		argv.strptr = "";
	argv.strlength = strlen(argv.strptr);

	// Check for translated image and save it in prg
	if(rexx_data)
	{
		Trace("Using translated data with %d bytes",rexx_data->strlength);
		prg[1].strptr		= rexx_data->strptr;
		prg[1].strlength	= rexx_data->strlength;
	}

	RunPendingEvents(0);

	// Run rexx script
	Trace("Starting script \"%s\"",filename);
	return_code = RexxStart(	1,					// argument count
								&argv,				// argument array
								NULL,				// REXX procedure name
								prg,				// program
								PACKAGE_NAME,		// default address name
								RXCOMMAND,			// calling as a subcommand
								rexx_exit_array,	// EXITs for this call
								&rc,				// converted return code
								&retstr);			// returned result

	g_free(prg->strptr);

	Trace("RexxStart exits with %d and translated code in %p",(int) return_code,prg[1].strptr);

	if(prg[1].strptr)
	{
		if(!rexx_data)
		{
			Trace("Saving translated data with %d bytes",(int) prg[1].strlength);
			rexx_data = g_malloc0(sizeof(struct rexx_translated_data)+prg[1].strlength);
			rexx_data->strlength = prg[1].strlength;
			memcpy(rexx_data->strptr,prg[1].strptr,prg[1].strlength);
			g_object_set_data_full(G_OBJECT(widget),"rexx_translated_data",rexx_data,g_free);
		}

		if(prg[1].strptr != rexx_data->strptr)
		{
			RexxFreeMemory(prg[1].strptr);
		}

	}

	Trace("Retstr exits with %p",retstr.strptr);
	if(retstr.strptr)
		RexxFreeMemory(retstr.strptr);

	Trace("rc=%d return_code=%d",rc,(int) return_code);

	// Check script state
	name = g_path_get_basename(filename);

	rc = unlock_rexx_script_engine(window, name, rc, return_code);

	g_free(name);

 }

 PW3270_PLUGIN_ENTRY int load_menu_scripts(GtkMenu *menu, GtkWidget *program_window)
 {
	gchar			*path = g_build_filename((const gchar *) g_object_get_data(G_OBJECT(program_window),"pw3270_dpath"),"rexx",NULL);
	const gchar	*name;
 	GDir			*dir;

    dir = g_dir_open(path,0,NULL);

	Trace("\n\n\n%s: %p",path,dir);
    if(!dir)
    {
    	g_free(path);
		return ENOENT;
    }

	name = g_dir_read_name(dir);

	while(name)
	{
		gchar *filename = g_build_filename(path,name,NULL);

		if(g_str_has_suffix(filename,"rex"))
		{
 			GtkWidget 	*item = gtk_menu_item_new_with_label(name);
 			Trace("Appending script %s (item: %p menu: %p)",name,item,menu);

 			g_object_set_data_full(G_OBJECT(item),"script_filename",filename,g_free);
			g_signal_connect(G_OBJECT(item),"activate",G_CALLBACK(script_rexx_activated),program_window);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
			gtk_widget_show(item);
		}
		else
		{
			g_free(filename);
		}

		name = g_dir_read_name(dir);
	}
	g_dir_close(dir);

	gtk_widget_show_all(GTK_WIDGET(menu));

	return 0;
 }

 PW3270_PLUGIN_ENTRY void action_select_activated(GtkAction *action, GtkWidget *program_window)
 {
	gchar 		*ptr;
 	GKeyFile 	*conf	= (GKeyFile *) g_object_get_data(G_OBJECT(program_window),"pw3270_config");
	GtkWidget 	*dialog = gtk_file_chooser_dialog_new(	_( "Select Rexx script to run" ),
														GTK_WINDOW(program_window),
														GTK_FILE_CHOOSER_ACTION_OPEN,
														GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														GTK_STOCK_EXECUTE,	GTK_RESPONSE_ACCEPT,
														NULL );


	Trace("%s starts",__FUNCTION__);

	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),FALSE);

	ptr = g_key_file_get_string(conf,"uri","RexxScript",NULL);
	if(ptr)
	{
		gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(dialog),ptr);
		g_free(ptr);
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		g_key_file_set_string(conf,"uri","RexxScript",uri);
		g_free(uri);

		Trace("Destroying dialog %p",dialog);
		gtk_widget_destroy(dialog);

		call_rexx(filename,"");

		g_free(filename);
	}
	else
	{
		gtk_widget_destroy(dialog);
	}

	Trace("%s ends",__FUNCTION__);
 }

 RexxReturnCode REXXENTRY rx3270Actions(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;
 	GtkActionGroup **action_group = (GtkActionGroup **) g_object_get_data(G_OBJECT(program_window),"ActionGroups");

	if(!action_group)
		return RetValue(Retstr,EINVAL);

 	for(f=0;f<Argc;f++)
 	{
 		GtkAction	*action = NULL;
		int			p;

 		Trace("Searching action %s",Argv[f].strptr);

		for(p = 0; action_group[p] && !action; p++)
			action = gtk_action_group_get_action(action_group[p],Argv[f].strptr);

		if(!action)
			return RetValue(Retstr,ENOENT);

 		Trace("Running action %s: %p",Argv[f].strptr,action);

 		gtk_action_activate(action);

 	}

	return RetValue(Retstr,0);
 }


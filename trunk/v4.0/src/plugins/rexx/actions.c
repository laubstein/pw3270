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

/*---[ Statics ]----------------------------------------------------------------------------------*/

 static GtkActionGroup *action_groups[ACTION_GROUP_MAX] = { 0 };

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static void RunExternalRexx(GtkAction *action, GKeyFile *conf)
 {
	gchar 		*ptr;
	GtkWidget 	*dialog = gtk_file_chooser_dialog_new(	_( "Select Rexx script to run" ),
														GTK_WINDOW(program_window),
														GTK_FILE_CHOOSER_ACTION_OPEN,
														GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														GTK_STOCK_EXECUTE,	GTK_RESPONSE_ACCEPT,
														NULL );


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
		gtk_widget_hide(dialog);
		g_key_file_set_string(conf,"uri","RexxScript",uri);
		call_rexx(filename,"");
		g_free(uri);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);

 }

 void G3270Action_Rexx(GtkAction *action, gpointer cmd)
 {
 	gchar *script 	= g_strdup(cmd);
 	gchar *arg		= strchr(script,' ');

	if(arg)
		*(arg++) = 0;
	else
		arg = "";

	call_rexx(script,g_strstrip(arg));

	g_free(script);
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
 		{ "RunExternalRexx",	N_( "External rexx script" ), 	NULL, 	RunExternalRexx	},
 		{ "RexxScripts",		N_( "Rexx scripts" ),			NULL,	NULL			},
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
											dgettext(GETTEXT_PACKAGE,action_info[f].label),
											dgettext(GETTEXT_PACKAGE,action_info[f].tooltip),
											NULL );

		if(action_info[f].call)
			g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(action_info[f].call),(gpointer) conf);
		gtk_action_group_add_action(groups[0],action);
	}
 }

 ULONG APIENTRY rx3270Actions(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

	if(!action_groups[0])
		return RetValue(Retstr,EINVAL);


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



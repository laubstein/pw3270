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

/*---[ Statics ]----------------------------------------------------------------------------------*/

 static GtkActionGroup *action_groups[ACTION_GROUP_MAX] = { 0 };

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static void RunExternalRexx(GtkAction *action, GKeyFile *conf)
 {
	gchar 		*ptr;
	GtkWidget 	*dialog = gtk_file_chooser_dialog_new(	_( "Select Rexx script to run" ),
														GTK_WINDOW(g3270_topwindow),
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



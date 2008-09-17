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

/*---[ Implement ]--------------------------------------------------------------------------------*/

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


 	dialog = gtk_message_dialog_new(	GTK_WINDOW(g3270_topwindow),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										type,
										GTK_BUTTONS_CLOSE,
										"%s",Argv[0].strptr );

	rc = gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);
	return RetValue(Retstr,rc);
 }



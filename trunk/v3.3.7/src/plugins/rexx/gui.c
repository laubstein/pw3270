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
 * Este programa está nomeado como @@FILENAME@@ e possui @@LINES@@ linhas de código.
 * 
 * Contatos: 
 * 
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
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



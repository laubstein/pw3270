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
 * Este programa está nomeado como gui.c e possui 88 linhas de código.
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

 static GtkWidget *getWidget(LONG Argc, RXSTRING Argv[])
 {
 	GtkWidget *widget = NULL;

 	if(Argc == 0)
		return NULL;

	if(strncmp(Argv[0].strptr,"0x",2))
		return NULL;

	if(sscanf(Argv[0].strptr, "%p", &widget) != 1)
		return NULL;

	if(!widget)
		return NULL;

	if(!GTK_IS_WIDGET(widget))
		return NULL;

	return widget;
 }

 #define DECLARE_WIDGET_ARG( widget ) GtkWidget *widget = getWidget(Argc,Argv); if(!widget) return RXFUNC_BADCALL;

 static ULONG RetGtkResponse(PRXSTRING Retstr, GtkResponseType type)
 {
 	static const struct _rsptype
 	{
 		GtkResponseType type;
 		const gchar 	*ret;
 	} rsptype[] =
 	{
		{ GTK_RESPONSE_NONE,			"NONE"		},
		{ GTK_RESPONSE_REJECT,			"REJECT"	},
		{ GTK_RESPONSE_ACCEPT,			"ACCEPT"	},
		{ GTK_RESPONSE_DELETE_EVENT,	"DELETED"	},
		{ GTK_RESPONSE_OK,				"OK"		},
		{ GTK_RESPONSE_CANCEL,			"CANCEL"	},
		{ GTK_RESPONSE_CLOSE,			"CLOSE"		},
		{ GTK_RESPONSE_YES,				"YES"		},
		{ GTK_RESPONSE_NO,				"NO"		},
		{ GTK_RESPONSE_APPLY,			"APPLY"		},
		{ GTK_RESPONSE_HELP,			"HELP"		},
 	};

 	int f;

 	for(f=0;f<G_N_ELEMENTS(rsptype);f++)
 	{
 		if(rsptype[f].type == type)
			return RetString(Retstr,rsptype[f].ret);
 	}

	return RetValue(Retstr,(int) type);
 }

/*---[ Implement ]--------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270Popup			                              */
/*                                                                            */
/* Description: Show Popup window                                             */
/*                                                                            */
/* Rexx Args:   Text for error popup                                          */
/*                                                                            */
/* Rexx Args:   Message Type ( "INFO", "WARNING", "QUESTION", "ERROR"         */
/*				Text for popup           							  		  */
/*              string                                       				  */
/*                                                                            */
/* Returns:	    Dialog result                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
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
 	GtkMessageType	type = GTK_MESSAGE_ERROR;
 	GtkWidget		*dialog;
 	const gchar	*text = "";
 	int				rc = 0;

	Trace("Argc: %d",(int) Argc);

	switch(Argc)
	{
	case 1:
		text = Argv[0].strptr;
		break;

	case 2:

		text = Argv[1].strptr;
		type = GTK_MESSAGE_OTHER;

		for(f = 0; f < G_N_ELEMENTS(msgtype) && type == GTK_MESSAGE_OTHER; f++)
		{
			if(!g_ascii_strncasecmp(msgtype[f].name,Argv[0].strptr,strlen(msgtype[f].name)))
			{
				type = msgtype[f].type;
				Trace("Messagetype: %s (%d)",msgtype[f].name,(int) type);
			}
		}
		break;

	default:
		return RXFUNC_BADCALL;

	}

 	dialog = gtk_message_dialog_new(	GTK_WINDOW(program_window),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										type,
										GTK_BUTTONS_CLOSE,
										"%s",Argv[0].strptr );

	rc = gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);
	return RetValue(Retstr,rc);
 }


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270runDialog                                    */
/*                                                                            */
/* Description: Run Dialog box                                                */
/*                                                                            */
/* Rexx Args:   Dialog handle                                                 */
/*                                                                            */
/* Returns:	    Dialog result                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270runDialog(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	DECLARE_WIDGET_ARG( widget );
	return RetGtkResponse(Retstr,gtk_dialog_run(GTK_DIALOG(widget)));
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270DestroyDialog                                */
/*                                                                            */
/* Description: Destroy dialog widget                                         */
/*                                                                            */
/* Rexx Args:   Dialog handle                                                 */
/*                                                                            */
/* Returns:	    0 if ok, or error code                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270DestroyDialog(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	GtkWidget *widget = NULL;

 	if(Argc == 0)
		return RetValue(Retstr,EINVAL);

	if(strncmp(Argv[0].strptr,"0x",2))
		return RetValue(Retstr,EINVAL);

	if(sscanf(Argv[0].strptr, "%p", &widget) != 1)
		return RetValue(Retstr,EINVAL);

	if(!widget)
		return RetValue(Retstr,EINVAL);

	if(!GTK_IS_WIDGET(widget))
		return RetValue(Retstr,EINVAL);

	Trace("Widget %p destroyed",widget);
	gtk_widget_destroy(widget);

	return RetValue(Retstr,0);

 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270FileChooserNew                               */
/*                                                                            */
/* Description: Create a new file chooser dialog                              */
/*                                                                            */
/* Rexx Args:   Dialog type (OPEN/SAVE)                                       */
/*				Dialog title												  */
/*																			  */
/* Returns:	    Widget handle                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270FileChooserNew(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	GtkWidget *widget;

 	if(Argc != 2)
		return RXFUNC_BADCALL;


	if(!strcmp(Argv[0].strptr,"OPEN"))
	{
		widget = gtk_file_chooser_dialog_new( Argv[1].strptr,
											  GTK_WINDOW(program_window),
											  GTK_FILE_CHOOSER_ACTION_OPEN,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
										 	  NULL);

	}
	else if(!strcmp(Argv[0].strptr,"SAVE"))
	{
		widget = gtk_file_chooser_dialog_new( Argv[1].strptr,
											  GTK_WINDOW(program_window),
											  GTK_FILE_CHOOSER_ACTION_SAVE,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
											  NULL);
	}
	else
	{
		return RXFUNC_BADCALL;
	}


	ReturnPointer(widget);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270FileChooserGetFilename                       */
/*                                                                            */
/* Description: Gets the filename for the currently selected file in the file selector.  */
/*                                                                            */
/* Rexx Args:   File Chooser handle                                           */
/*																			  */
/* Returns:	    Selected file                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270FileChooserGetFilename(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	DECLARE_WIDGET_ARG( widget );
	return RetString(Retstr,gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget)));
 }

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
 * Este programa está nomeado como dialogs.c e possui 88 linhas de código.
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

/*---[ Implement ]--------------------------------------------------------------------------------*/

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
 	CHECK_SINGLE_WIDGET_ARG(widget);
	return RetGtkResponse(Retstr,gtk_dialog_run(GTK_DIALOG(widget)));
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270SetWindowSize                                */
/*                                                                            */
/* Description: Set window size                                       	      */
/*                                                                            */
/* Rexx Args:   Window handle                                                 */
/*              Default width                                                 */
/*              Default height                                                */
/*                                                                            */
/* Returns:	    none                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270SetWindowSize(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	GtkWidget *widget;

	if(Argc != 3)
		return RXFUNC_BADCALL;

	GET_WIDGET_ARG(widget,0);

	Trace("Window %p size will be %dx%d",widget,atoi(Argv[1].strptr),atoi(Argv[2].strptr));
	gtk_window_resize(GTK_WINDOW(widget),atoi(Argv[1].strptr),atoi(Argv[2].strptr));

	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270SetDialogTitle                               */
/*                                                                            */
/* Description: Set dialog title                                              */
/*                                                                            */
/* Rexx Args:   Dialog handle                                                 */
/*              String with dialog title                                      */
/*                                                                            */
/* Returns:	    None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270SetDialogTitle(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	GtkWidget *widget;

	if(Argc != 2)
		return RXFUNC_BADCALL;

	GET_WIDGET_ARG(widget,0);

	gtk_window_set_title(GTK_WINDOW(widget),Argv[0].strptr);

	return RetValue(Retstr,EINVAL);
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
	GtkWidget *widget = getWidget(0,Argv);

	Trace("%s widget: %p",__FUNCTION__,widget);

	if(!widget)
		return RetValue(Retstr,EINVAL);

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

	Trace("%s called with %d arguments",__FUNCTION__,(int) Argc);

 	if(Argc < 2)
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


	Trace("%s: %p",__FUNCTION__,widget);

	if(Argc == 3)
		gtk_window_set_title(GTK_WINDOW(widget),Argv[2].strptr);

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
 	ULONG rc;
 	gchar *filename;
	CHECK_SINGLE_WIDGET_ARG(widget);

	filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
	rc = RetString(Retstr,filename);
	g_free(filename);
	return rc;
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270MessageDialogNew                             */
/*                                                                            */
/* Description: Create a new message dialog                                   */
/*                                                                            */
/* Rexx Args:   Text for error popup                                          */
/*                                                                            */
/* Rexx Args:   Message Type ( "INFO", "WARNING", "QUESTION", "ERROR"         */
/*				Text for popup           							  		  */
/*                                                                            */
/* Returns:	    Dialog handle                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270MessageDialogNew(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	GtkMessageType	type = GTK_MESSAGE_ERROR;
 	GtkWidget		*dialog;
 	const gchar	*text = "";

	Trace("Argc: %d",(int) Argc);

	switch(Argc)
	{
	case 1:
		text = Argv[0].strptr;
		break;

	case 2:
		text = Argv[1].strptr;
		type = getMessageDialogType(Argv[0].strptr);
		break;

	default:
		return RXFUNC_BADCALL;

	}

 	dialog = gtk_message_dialog_new(	GTK_WINDOW(program_window),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										type,
										GTK_BUTTONS_CLOSE,
										"%s",text );

	ReturnPointer(dialog);
 }


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270ProgressDialogNew                            */
/*                                                                            */
/* Description: Create a new progress dialog                                  */
/*                                                                            */
/* Rexx Args:   Dialog title                                                  */
/*              Dialog message                                                */
/*                                                                            */
/* Returns:	    Dialog handle                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/

 void cancel_clicked(GtkButton *button, gpointer user_data)
 {
 	RaiseHaltSignal();
 }

 ULONG APIENTRY rx3270ProgressDialogNew(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	GtkWidget		*dialog;
 	GtkWidget		*widget;
 	GtkWidget		*box;
 	const gchar	*text  = "";
 	const gchar	*title = _( "Processing data..." );

	switch(Argc)
	{
	case 0:
		break;

	case 1:
		text = Argv[0].strptr;
		break;

	case 2:
		title = Argv[0].strptr;
		text = Argv[1].strptr;
		break;

	default:
		return RXFUNC_BADCALL;

	}

	dialog = gtk_dialog_new();
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
	gtk_window_set_title(GTK_WINDOW(dialog),_( "Please wait" ) );
	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(program_window));
	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog),TRUE);

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 14
	box = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
#else
	box = (GTK_DIALOG(dialog))->vbox;
#endif

	/* Cria label com o sub-titulo */
	widget = gtk_label_new(title);
	g_object_set_data(G_OBJECT(dialog),"LabelWidget",widget);
	gtk_box_pack_start(GTK_BOX(box),widget,TRUE,TRUE,5);

	/* Cria barra de progresso */
	widget = gtk_progress_bar_new();

	if(text && *text)
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget),text);

	g_object_set_data(G_OBJECT(dialog),"ProgressBarWidget",widget);
	gtk_box_pack_start(GTK_BOX(box),widget,TRUE,TRUE,5);

	gtk_widget_show_all(box);

	/* Cria botao "cancelar" */
#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 14
	box = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
#else
	box = (GTK_DIALOG(dialog))->action_area;
#endif
	widget = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(cancel_clicked),(gpointer) dialog);

	gtk_box_pack_end(GTK_BOX(box),widget,FALSE,FALSE,5);

	gtk_widget_show_all(box);
 	ReturnPointer(dialog);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270ProgressDialogSetCurrent                     */
/*                                                                            */
/* Description: Create a new progress dialog                                  */
/*                                                                            */
/* Rexx Args:   Dialog handle                                                 */
/*              Progress bar position                                         */
/*                                                                            */
/* Returns:	    none                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270ProgressDialogSetCurrent(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	GtkWidget 	*widget;
 	gdouble		total;
 	gdouble		current;

	if(Argc != 2)
		return RXFUNC_BADCALL;

	GET_WIDGET_ARG(widget,0);

	total   = (gdouble) ( (int) g_object_get_data(G_OBJECT(widget),"Total"));
	current = (gdouble) atoi(Argv[1].strptr);

	if(total < 1 || current < 1)
		return RetValue(Retstr,EINVAL);

	widget = g_object_get_data(G_OBJECT(widget),"ProgressBarWidget");
	if(!widget)
		return RetValue(Retstr,ENOENT);

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widget),(current/total) );

	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270ProgressDialogSetTotal                       */
/*                                                                            */
/* Description: Set number of elements in progress bar                        */
/*                                                                            */
/* Rexx Args:   Dialog handle                                                 */
/*              Progress bar position                                         */
/*                                                                            */
/* Returns:	    none                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270ProgressDialogSetTotal(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	GtkWidget *widget;

	if(Argc != 2)
		return RXFUNC_BADCALL;

	GET_WIDGET_ARG(widget,0);

	g_object_set_data(G_OBJECT(widget),"Total",(gpointer) atoi(Argv[1].strptr));

	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270ProgressDialogSetText                        */
/*                                                                            */
/* Description: Causes the given text to appear superimposed on the progress bar. */
/*                                                                            */
/* Rexx Args:   Dialog handle                                                 */
/*              Text to show                                                  */
/*                                                                            */
/* Returns:	    none                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270ProgressDialogSetText(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	GtkWidget *widget;

	if(Argc != 2)
		return RXFUNC_BADCALL;

	GET_WIDGET_ARG(widget,0);

	widget = (GtkWidget *) g_object_get_data(G_OBJECT(widget),"ProgressBarWidget");
	Trace("Progressbar: %p",widget);
	if(!widget)
		return RetValue(Retstr,EINVAL);

	Trace("%s: text=\"%s\" is_progress_bar: %s",__FUNCTION__,Argv[1].strptr,GTK_IS_PROGRESS_BAR(widget) ? "Yes" : "No");

	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widget),Argv[1].strptr);
	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270ProgressDialogPulse                          */
/*                                                                            */
/* Description: Indicates that some progress is made, but you don't know how much. */
/*                                                                            */
/* Rexx Args:   Dialog handle                                                 */
/*                                                                            */
/* Returns:	    none                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270ProgressDialogPulse(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	CHECK_SINGLE_WIDGET_ARG(widget);

	widget = (GtkWidget *) g_object_get_data(G_OBJECT(widget),"ProgressBarWidget");
	Trace("Progressbar: %p",widget);
	if(!widget)
		return RetValue(Retstr,EINVAL);

	gtk_progress_bar_pulse(GTK_PROGRESS_BAR(widget));
	return RetValue(Retstr,0);
 }

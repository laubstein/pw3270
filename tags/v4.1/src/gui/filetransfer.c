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
 * Este programa está nomeado como filetransfer.c e possui 570 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 * Agradecimento:
 *
 * Roberto Soares 			(a_r_soares@hotmail.com)
 *
 */

 #include <lib3270/config.h>
 #include <string.h>
 #include <stdlib.h>

 #include "gui.h"

#if defined(X3270_FT)

// #define SHOW_FT_PROGRESS 1

/*---[ Defines ]------------------------------------------------------------------------------------------------*/

 struct ftdialog
 {
	unsigned int	 	flags;
	const gchar 		*group_name;
	GtkWidget			*file[2];
	GtkWidget			*entry[5];
	GtkWidget			*ready;
	int					value[5];
 };

 struct ftprogress_dialog
 {
	GtkWidget		*window;
	GtkWidget		*info[3];
#ifdef SHOW_FT_PROGRESS
#endif
	GtkWidget		*progress_bar;
	GtkWidget		*close_button;
	GtkWidget		*cancel_button;
	unsigned long	total;
 };

/*---[ Statics ]------------------------------------------------------------------------------------------------*/

 static void ft_begin(unsigned short flags, const char *local, const char *remote);
 static void ft_complete(const char *errmsg,unsigned long length,double kbytes_sec,const char *mode);
 static void ft_setlength(unsigned long length);
 static void ft_update(unsigned long length,double kbytes_sec);
 static void ft_running(int is_cut);
 static void ft_aborting(void);

 static const	struct filetransfer_callbacks ft_callbacs =
 {
	sizeof(struct filetransfer_callbacks),

	ft_begin,
	ft_complete,
	ft_setlength,
	ft_update,
	ft_running,
	ft_aborting

 };

 static struct ftprogress_dialog *progress_dialog = NULL;


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

/**
 * Open file tranfer status dialog.
 *
 */

/*
http://www.suggestsoft.com/images/medieval-software/medieval-bluetooth-obex-file-transfer.gif

--Informations----------------------------------------
|
| From:		xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx
|
| To:		xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx
|
| Status:	xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx
------------------------------------------------------

--Progress----------------------------------------------
|
| Total: 	xxx.xxx.xxx 		Current:	xxx.xxx.xxx
|
| Started:	xx:xx:xx			ETA: 		xx:xx:xx
|
| xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
--------------------------------------------------------

												[Cancel]
*/

 static void destroy( GtkWidget *widget, struct ftprogress_dialog *dialog )
 {
 	progress_dialog = NULL;
 	g_free(dialog);
 	Trace("Progress dialog %p destroyed",dialog);
 }

 static void close_dialog(GtkButton *button,GtkWidget *dialog)
 {
 	Trace("Closing dialog %p",dialog);
 	gtk_widget_destroy(dialog);
 }

 static void cancel_ft(GtkButton *button,struct ftprogress_dialog *dialog)
 {
 	// TODO (perry#2#): Try with not force first, if it fails with EBUSY show dialog asking if the user is really sure of aborting transfer
	CancelFileTransfer(TRUE);
 }

 int create_ft_progress_dialog(void)
 {
 	static const gchar *info[] = { N_( "From:" ), N_( "To:" ), N_( "Status:" ) };

#ifdef SHOW_FT_PROGRESS
 	static const gchar *progress[] = { N_( "Total:" ), N_( "Current:" ), N_( "Started:" ), N_( "ETA:" ) };
#endif

	int f;

 	GtkWidget *frame;
 	GtkWidget *table;

#ifdef SHOW_FT_PROGRESS
 	GtkWidget *widget;
#endif

 	GtkWidget *box;

	if(progress_dialog)
		return EBUSY;

	progress_dialog	= g_malloc0(sizeof(struct ftprogress_dialog));

	progress_dialog->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
//	gtk_window_set_default_size(GTK_WINDOW(progress_dialog->window),600,150);

	g_signal_connect(G_OBJECT(progress_dialog->window),"destroy",G_CALLBACK(destroy),progress_dialog);

	gtk_window_set_title(GTK_WINDOW(progress_dialog->window),_( "File transfer") );
	gtk_window_set_resizable(GTK_WINDOW(progress_dialog->window),FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(progress_dialog->window),GTK_WINDOW(topwindow));
	gtk_window_set_modal(GTK_WINDOW(progress_dialog->window),TRUE);
	gtk_window_set_position(GTK_WINDOW(progress_dialog->window),GTK_WIN_POS_CENTER_ON_PARENT);

	// Box
 	box	= gtk_vbox_new(FALSE,2);

	// File transfer info
	frame = gtk_frame_new( _( "Informations" ) );
	gtk_container_set_border_width(GTK_CONTAINER(frame),3);
#ifdef SHOW_FT_PROGRESS
	table = gtk_table_new(3,2,FALSE);
#else
	table = gtk_table_new(4,2,FALSE);
#endif
	gtk_container_set_border_width(GTK_CONTAINER(table),6);

	for(f=0;f<G_N_ELEMENTS(progress_dialog->info);f++)
	{
		GtkWidget *widget = gtk_label_new(gettext(info[f]));
		gtk_misc_set_alignment(GTK_MISC(widget),0,.5);
		gtk_table_attach(GTK_TABLE(table),widget,0,1,f,f+1,GTK_FILL,GTK_FILL,2,2);

		progress_dialog->info[f] = gtk_entry_new();
		gtk_entry_set_width_chars(GTK_ENTRY(progress_dialog->info[f]),70);
	//	gtk_entry_set_has_frame(GTK_ENTRY(progress_dialog->info[f]),FALSE);
		gtk_editable_set_editable(GTK_EDITABLE(progress_dialog->info[f]),FALSE);
		gtk_table_attach(GTK_TABLE(table),progress_dialog->info[f],1,2,f,f+1,GTK_FILL|GTK_EXPAND,GTK_FILL|GTK_EXPAND,2,2);

	}

#ifndef SHOW_FT_PROGRESS
	progress_dialog->progress_bar = gtk_progress_bar_new();
	gtk_table_attach(GTK_TABLE(table),progress_dialog->progress_bar,0,4,f,f+1,GTK_FILL,GTK_FILL,2,2);
#endif

	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_box_pack_start(GTK_BOX(box),frame,TRUE,TRUE,2);

#ifdef SHOW_FT_PROGRESS
	// Progress info

	frame = gtk_frame_new( _( "Progress" ) );
	gtk_container_set_border_width(GTK_CONTAINER(frame),3);

	table = gtk_table_new(3,4,FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table),6);

	for(f=0;f<2;f++)
	{
		// Left box
		widget = gtk_label_new(gettext(progress[f<<1]));
		gtk_misc_set_alignment(GTK_MISC(widget),0,.5);
		gtk_table_attach(GTK_TABLE(table),widget,0,1,f,f+1,GTK_FILL,GTK_FILL,2,2);

		widget = gtk_entry_new();
		gtk_editable_set_editable(GTK_EDITABLE(widget),FALSE);
		gtk_table_attach(GTK_TABLE(table),widget,1,2,f,f+1,GTK_EXPAND,GTK_FILL,2,2);

		// Right box
		widget = gtk_label_new(gettext(progress[(f<<1)+1]));
		gtk_misc_set_alignment(GTK_MISC(widget),0,.5);
		gtk_table_attach(GTK_TABLE(table),widget,2,3,f,f+1,GTK_FILL,GTK_FILL,2,2);

		widget = gtk_entry_new();
		gtk_editable_set_editable(GTK_EDITABLE(widget),FALSE);
		gtk_table_attach(GTK_TABLE(table),widget,3,4,f,f+1,GTK_EXPAND,GTK_FILL,2,2);

	}

	progress_dialog->progress_bar = gtk_progress_bar_new();
	gtk_table_attach(GTK_TABLE(table),progress_dialog->progress_bar,0,4,f,f+1,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,2,2);

	gtk_container_add(GTK_CONTAINER(frame),table);
	gtk_box_pack_start(GTK_BOX(box),frame,TRUE,TRUE,2);

#endif

	// Buttons
	frame = gtk_hbox_new(FALSE,2);
	progress_dialog->close_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
	progress_dialog->cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

#ifndef DEBUG
	gtk_widget_set_sensitive(progress_dialog->cancel_button,FALSE);
	gtk_widget_set_sensitive(progress_dialog->close_button,FALSE);
#endif

	g_signal_connect(G_OBJECT(progress_dialog->cancel_button),"clicked",G_CALLBACK(cancel_ft),progress_dialog);
	g_signal_connect(G_OBJECT(progress_dialog->close_button),"clicked",G_CALLBACK(close_dialog),progress_dialog->window);

	gtk_box_pack_end(GTK_BOX(frame),progress_dialog->cancel_button,FALSE,FALSE,2);
	gtk_box_pack_end(GTK_BOX(frame),progress_dialog->close_button,FALSE,FALSE,2);

	gtk_box_pack_end(GTK_BOX(box),frame,TRUE,TRUE,2);

	// Finish dialog
	gtk_container_add(GTK_CONTAINER(progress_dialog->window),box);
	gtk_widget_show_all(progress_dialog->window);

	return 0;
 }


/**
 * Register file transfer callbacks
 *
 */
 int initft(void)
 {
 	return RegisterFTCallbacks(&ft_callbacs);
 }


/**
 * File selection dialog.
 *
 * @param button
 * @param info		Current file-transfer dialog info.
 *
 */
 static void browse_file(GtkButton *button,struct ftdialog *info)
 {
 	gchar		*ptr;
	GKeyFile	*conf   = GetConf();
	gboolean	recv	= (info->flags & FT_FLAG_RECEIVE) != 0;

	GtkWidget 	*dialog = gtk_file_chooser_dialog_new(	recv ? _( "Select file to receive" ) : _( "Select file to send" ),
														GTK_WINDOW(topwindow),
														GTK_FILE_CHOOSER_ACTION_OPEN,
														GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														recv ? GTK_STOCK_SAVE : GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
														NULL );


	ptr = g_key_file_get_string(conf,info->group_name,"uri",NULL);
	if(ptr)
	{
			gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(dialog),ptr);
			g_free(ptr);
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		g_key_file_set_string(conf,info->group_name,"uri",uri);
		gtk_entry_set_text(GTK_ENTRY(info->file[0]),filename);
		g_free(uri);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);
 }

 static void updatefields(struct ftdialog *info)
 {
 	gboolean enable;

 	// recfm fields
 	enable = (info->flags & FT_RECORD_FORMAT_MASK) != 0;
 	if(info->entry[0])
		gtk_widget_set_sensitive(info->entry[0],enable);
 	if(info->entry[1])
		gtk_widget_set_sensitive(info->entry[1],enable);

 	// Unit fields
 	enable = (info->flags & FT_ALLOCATION_UNITS_MASK) != 0;
 	if(info->entry[2])
		gtk_widget_set_sensitive(info->entry[2],enable);

 	if(info->entry[3])
		gtk_widget_set_sensitive(info->entry[3],enable);

 }

 static void toggle_flag(GtkToggleButton *button, gpointer user_data)
 {
 	struct ftdialog	*info = (struct ftdialog *) g_object_get_data(G_OBJECT(button),"info");

 	if(gtk_toggle_button_get_active(button))
		info->flags |= ((unsigned int) user_data);
	else
		info->flags &= ~((unsigned int) user_data);

	Trace("Flags changed to %04x",info->flags);
	updatefields(info);
 }

 static void check_filenames(GtkEditable *editable,struct ftdialog *info)
 {
 	int 			f;
 	const gchar	*ptr;

 	for(f=0;f<G_N_ELEMENTS(info->file);f++)
 	{
 		ptr = gtk_entry_get_text(GTK_ENTRY(info->file[f]));
 		if(!(ptr && *ptr))
 		{
 			gtk_widget_set_sensitive(info->ready,FALSE);
 			return;
 		}
 	}
	gtk_widget_set_sensitive(info->ready,TRUE);
 }

 static int ftdialog(gboolean receive)
 {
	static const struct _ftoptions
	{
		unsigned int	 	flag;
		const gchar		*label;
	} ft_options[] 			=		{	{	FT_FLAG_ASCII,			N_( "Text file" )						},
										{	FT_FLAG_TSO,			N_( "Host is TSO" )						},
										{	FT_FLAG_CRLF,			N_( "Add/Remove CR at end of line" )	},
										{	FT_FLAG_APPEND,			N_( "Append" )							},
										{   FT_FLAG_REMAP_ASCII,	N_( "Remap ASCII Characters" )			}
									};

	static const gchar *label[] = {	N_( "Local file name:" 	),
										N_( "Host file name:"	),
									};

	static const gchar *recfm[] = {	N_( "Default" 			),
										N_( "Fixed"				),
										N_( "Variable"			),
										N_( "Undefined"			)
									};

	static const gchar *extra[] =	{	N_( "LRECL:" 			),
										N_( "BLKSIZE:" 			),
										N_( "Primary space:"	),
										N_( "Secondary space:"	)
									};

	static const gchar *unit[] = {	N_( "Default" 			),
										N_( "Tracks"			),
										N_( "Cylinders"			),
										N_( "Avblock"			)
									};

	static const struct _opt
	{
		const gchar *key;
		const gchar *def;
	} opt[] =						{	{ "LRECL", 			"" 			},
										{ "BLKSIZE", 		""			},
										{ "PRIMARY_SPC", 	"" 			},
										{ "SECONDARY_SPC",	"" 			},
										{ "DFT",			"4096"		}
									};

	int rc;
	int f;

	GKeyFile			*conf   = GetConf();

	GtkWidget			*topbox;
	GtkWidget 			*table;
	GtkWidget 			*widget;
	GtkWidget 			*dialog;
	GtkWidget			*frame;
	GtkWidget			*box;
	GtkWidget			*hbox;
	GtkWidget			*vbox;
	GSList				*group;
	gchar				*ptr;

	int					row;
	int					col;
	struct ftdialog	info;
	unsigned int		flag;

	/* Set initial values */
	memset(&info,0,sizeof(info));
	info.group_name	= receive ? "FileReceive" : "FileSend";
	if(conf)
		info.flags = g_key_file_get_integer(conf,info.group_name,"flags",NULL);

	if(receive)
		info.flags |= FT_FLAG_RECEIVE;
	else
		info.flags &= ~FT_FLAG_RECEIVE;

	/* Create dialog */

	dialog  = gtk_dialog_new_with_buttons(	receive ?  _( "Receive file from host" ) : _( "Send file to host" ), \
													GTK_WINDOW(topwindow), \
													GTK_DIALOG_DESTROY_WITH_PARENT,
													NULL );


	info.ready = gtk_dialog_add_button(GTK_DIALOG(dialog),	receive ? GTK_STOCK_SAVE : GTK_STOCK_OPEN,
															GTK_RESPONSE_ACCEPT);

	gtk_widget_set_sensitive(info.ready,FALSE);

	gtk_dialog_add_button(GTK_DIALOG(dialog),	GTK_STOCK_CANCEL,
												GTK_RESPONSE_REJECT);

	topbox = gtk_vbox_new(FALSE,2);

	/* Add file names */
	table = gtk_table_new(G_N_ELEMENTS(label),5,FALSE);
	for(f=0;f < G_N_ELEMENTS(label);f++)
	{
		widget = gtk_label_new(gettext(label[f]));
		gtk_misc_set_alignment(GTK_MISC(widget),0,.5);
		gtk_table_attach(GTK_TABLE(table),widget,0,1,f,f+1,GTK_FILL,GTK_FILL,2,2);

		info.file[f] = gtk_entry_new_with_max_length(0x0100);
		g_signal_connect(G_OBJECT(info.file[f]),"changed",G_CALLBACK(check_filenames),&info);

		gtk_entry_set_width_chars(GTK_ENTRY(info.file[f]),40);
		gtk_table_attach(GTK_TABLE(table),info.file[f],1,3,f,f+1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,GTK_EXPAND|GTK_SHRINK|GTK_FILL,2,2);
	}

	widget = gtk_button_new_with_label( _( "Browse" ) );
	gtk_table_attach(GTK_TABLE(table),widget,3,4,0,1,0,0,2,2);
	g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(browse_file),&info);

	gtk_box_pack_start(GTK_BOX(topbox),table,TRUE,TRUE,0);

	/* Transfer options */
	frame = gtk_frame_new( _( "Transfer options" ) );
	box   = gtk_table_new(3,2,FALSE);

	row=0;
	col=0;
	for(f=0;f < G_N_ELEMENTS(ft_options);f++)
	{
		widget = gtk_check_button_new_with_label( gettext(ft_options[f].label));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(ft_options[f].flag & info.flags) != 0);
		g_object_set_data(G_OBJECT(widget),"info",(gpointer) &info);
		g_signal_connect(G_OBJECT(widget),"toggled", G_CALLBACK(toggle_flag),(gpointer) ft_options[f].flag);

		gtk_table_attach(GTK_TABLE(box),widget,col,col+1,row,row+1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,GTK_EXPAND|GTK_SHRINK|GTK_FILL,2,2);
		if(col++ > 0)
		{
			row++;
			col=0;
		}
	}
	gtk_container_add(GTK_CONTAINER(frame),box);
	gtk_box_pack_start(GTK_BOX(topbox),frame,TRUE,TRUE,0);

	row = G_N_ELEMENTS(label)+1;
	if(!receive)
	{
		/* Formats */
		hbox = gtk_hbox_new(TRUE,2);

		/* Record */
		frame	= gtk_frame_new( _( "Record format" ) );
		vbox 	= gtk_vbox_new(TRUE,2);
		group	= NULL;
		for(f=0;f < G_N_ELEMENTS(recfm);f++)
		{
			flag = (f&0x03) << 8;

			widget = gtk_radio_button_new_with_label(group,gettext(recfm[f]));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(info.flags & FT_RECORD_FORMAT_MASK) == flag);

			g_object_set_data(G_OBJECT(widget),"info",(gpointer) &info);
			g_signal_connect(G_OBJECT(widget),"toggled", G_CALLBACK(toggle_flag),(gpointer) flag);

			group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
			gtk_box_pack_start(GTK_BOX(vbox),widget,TRUE,TRUE,0);
		}
		gtk_container_add(GTK_CONTAINER(frame),vbox);
		gtk_box_pack_start(GTK_BOX(hbox),frame,TRUE,TRUE,0);

		/* Unit */
		frame	= gtk_frame_new( _( "Space allocation units" ) );
		vbox 	= gtk_vbox_new(TRUE,2);
		group	= NULL;
		for(f=0;f < G_N_ELEMENTS(unit);f++)
		{
			flag = (f&0x03) << 12;

			widget = gtk_radio_button_new_with_label(group,gettext(unit[f]));
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),(info.flags & FT_ALLOCATION_UNITS_MASK) == flag);
			g_object_set_data(G_OBJECT(widget),"info",(gpointer) &info);
			g_signal_connect(G_OBJECT(widget),"toggled", G_CALLBACK(toggle_flag),(gpointer) flag);

			group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
			gtk_box_pack_start(GTK_BOX(vbox),widget,TRUE,TRUE,0);
		}
		gtk_container_add(GTK_CONTAINER(frame),vbox);
		gtk_box_pack_end(GTK_BOX(hbox),frame,TRUE,TRUE,0);

		/* Add record options */
		gtk_box_pack_start(GTK_BOX(topbox),hbox,TRUE,TRUE,0);

		/* Add LRECL, BLKSIZE, primary and secondary spaces */
		hbox = gtk_table_new(3,4,FALSE);

		for(f=0;f<2;f++)
		{
			// Left
			widget = gtk_label_new( gettext( extra[f] )  );
			gtk_misc_set_alignment(GTK_MISC(widget),0,.5);
			gtk_table_attach(GTK_TABLE(hbox),widget,0,1,f,f+1,GTK_FILL,GTK_FILL,2,2);
			info.entry[f] = gtk_entry_new_with_max_length(10);
			gtk_entry_set_width_chars(GTK_ENTRY(info.entry[f]),10);
			gtk_table_attach(GTK_TABLE(hbox),info.entry[f],1,2,f,f+1,GTK_FILL|GTK_EXPAND,GTK_FILL|GTK_EXPAND,2,2);

			// Right
			widget = gtk_label_new( gettext( extra[f+2] )  );
			gtk_misc_set_alignment(GTK_MISC(widget),0,.5);
			gtk_table_attach(GTK_TABLE(hbox),widget,2,3,f,f+1,GTK_FILL,GTK_FILL,2,2);
			info.entry[f+2] = gtk_entry_new_with_max_length(10);
			gtk_entry_set_width_chars(GTK_ENTRY(info.entry[f+2]),10);
			gtk_table_attach(GTK_TABLE(hbox),info.entry[f+2],3,4,f,f+1,GTK_FILL|GTK_EXPAND,GTK_FILL|GTK_EXPAND,2,2);
		}

		gtk_box_pack_start(GTK_BOX(topbox),hbox,TRUE,TRUE,0);

	}
	/* Add dft option */
	hbox = gtk_hbox_new(FALSE,2);
	widget = gtk_label_new( _( "DFT Buffer size:" ) );
	gtk_misc_set_alignment(GTK_MISC(widget),0,.5);
	gtk_box_pack_start(GTK_BOX(hbox),widget,FALSE,FALSE,0);
	info.entry[4] = gtk_entry_new_with_max_length(10);
	gtk_entry_set_width_chars(GTK_ENTRY(info.entry[4]),10);
	gtk_box_pack_start(GTK_BOX(hbox),info.entry[4],FALSE,FALSE,0);

	gtk_box_pack_start(GTK_BOX(topbox),hbox,TRUE,TRUE,0);

	if(conf)
	{
		/* Load options */

		for(f=0;f<5;f++)
		{
			if(info.entry[f])
			{
				ptr = g_key_file_get_string(conf,info.group_name,opt[f].key,NULL);
				if(ptr)
					gtk_entry_set_text(GTK_ENTRY(info.entry[f]),ptr);
				else
					gtk_entry_set_text(GTK_ENTRY(info.entry[f]),opt[f].def);
			}
		}
	}

	/* Run dialog */
	updatefields(&info);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),topbox);
	gtk_widget_show_all(dialog);
	rc = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);

	if(rc == GTK_RESPONSE_ACCEPT)
	{
		if(conf)
		{
			/* Save options */
			for(f=0;f<5;f++)
			{
				if(info.entry[f])
					g_key_file_set_string(conf,info.group_name,opt[f].key,gtk_entry_get_text(GTK_ENTRY(info.entry[f])));
			}
			g_key_file_set_integer(conf,info.group_name,"flags",info.flags);
		}

		/* Get option values */
		for(f=0;f<5;f++)
		{
			if(info.entry[f] && GTK_WIDGET_SENSITIVE(info.entry[f]))
				ptr = (gchar *) gtk_entry_get_text(GTK_ENTRY(info.entry[f]));
			else
				ptr = "";

			if(ptr && *ptr)
				info.value[f] = atoi(ptr);
			else
				info.value[f] = -1;
		}

		/* Begin transfer */
		create_ft_progress_dialog();

		rc = BeginFileTransfer(		info.flags,
									gtk_entry_get_text(GTK_ENTRY(info.file[0])),
									gtk_entry_get_text(GTK_ENTRY(info.file[1])),
									info.value[0],
									info.value[1],
									info.value[2],
									info.value[3],
									info.value[4]
								);

		Trace("BeginFileTransfer(%s): %d",gtk_entry_get_text(GTK_ENTRY(info.file[0])),rc);

		if(rc)
		{
			// Can't transfer file, notify user
			// FIXME (perry#1#): Probably unnecessary since a more detailed message should be sent to progress dialog.

			widget =  gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_ERROR,
												GTK_BUTTONS_CLOSE,
												receive ? _( "Can't receive \"%s\"" ) : _( "Can't send \"%s\"" ),
												gtk_entry_get_text(GTK_ENTRY(info.file[0]))
											);

			gtk_window_set_title(GTK_WINDOW(widget),_( "File transfer failed" ) );
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(widget),"%s (rc=%d)",rc < 0 ? "Unexpected" : g_strerror(rc),rc);
			gtk_dialog_run(GTK_DIALOG(widget));
			gtk_widget_destroy(widget);
		}

	}
	else
	{
		rc = -1;
	}

	gtk_widget_destroy(dialog);

	return rc;

 }

 static void ft_begin(unsigned short flags, const char *local, const char *remote)
 {
	Trace("%s Flags: %04x local: \"%s\" remote: \"%s\"",__FUNCTION__,flags,local,remote);

	if(progress_dialog)
	{
		if(flags & FT_FLAG_RECEIVE)
		{
			gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[1]),local);
			gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[0]),remote);

			gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[2]),_( "Receiving file"));
		}
		else
		{
			gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[0]),local);
			gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[1]),remote);

			gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[2]),_( "Sending file"));
		}
	}
 }

 static void ft_complete(const char *errmsg,unsigned long length,double kbytes_sec,const char *mode)
 {
	Trace("%s: msg: \"%s\"",__FUNCTION__,errmsg);

	if(progress_dialog)
	{
		if(errmsg)
		{
			gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[2]),gettext(errmsg));
		}
		else
		{
			gchar *msg = g_strdup_printf( _( "Transfer complete, %ld bytes transferred %.2lg Kbytes/sec in %s mode" ),
												length,kbytes_sec,mode );

			gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[2]),msg);
			g_free(msg);
		}

		gtk_widget_set_sensitive(progress_dialog->cancel_button,FALSE);
		gtk_widget_set_sensitive(progress_dialog->close_button,TRUE);
	}
 }

 static void ft_setlength(unsigned long length)
 {
	Trace("%s: length=%ld",__FUNCTION__,length);

	if(progress_dialog)
		progress_dialog->total = length;
 }

 static void ft_update(unsigned long length,double kbytes_sec)
 {
	Trace("%s: update_length=%ld speed=%.2lg",__FUNCTION__,length,kbytes_sec);
	if(progress_dialog)
	{
//		if(progress_dialog->total)
//		else
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress_dialog->progress_bar));
	}
 }

 static void ft_running(int is_cut)
 {
	Trace("%s: is_cut: %d",__FUNCTION__,is_cut);
	if(progress_dialog)
	{
		gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[2]),_( "Running transfer"));
	}
 }

 static void ft_aborting(void)
 {
	Trace("%s",__FUNCTION__);

	if(progress_dialog)
	{
		gtk_entry_set_text(GTK_ENTRY(progress_dialog->info[2]),_( "Aborting..."));
	}
 }


#else

 /* No file transfer - Implement "dunno" action */

 static int ftdialog(gboolean receive)
 {
 	return EINVAL;
 }


#endif // X3270_FT

 void action_Download(void)
 {
 	ftdialog(TRUE);
 }

 void action_Upload(void)
 {
 	ftdialog(FALSE);
 }



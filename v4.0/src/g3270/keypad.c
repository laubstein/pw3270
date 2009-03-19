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
 * Este programa está nomeado como keypad.c e possui 177 linhas de código.
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

 #include "g3270.h"

 #include <lib3270/config.h>
 #include <globals.h>

 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/toggle.h>
// #include <lib3270/hostc.h>

/*---[ Internals ]----------------------------------------------------------------------------------------------*/

 GtkWidget *keypad = NULL;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static void set_visible(int visible, int reason)
 {
 	if(visible)
		gtk_widget_show(keypad);
	else
		gtk_widget_hide(keypad);

 }

 static void pf_button(GtkButton *button, int key)
 {
 	PFKey(key);
 }

 static void pa_button(GtkButton *button, int key)
 {
 	action_PAKey(key);
 }

 static void clear_and_call(GtkButton *button, int (*call)(void))
 {
 	action_ClearSelection();
 	if(call)
		call();
 }

 static GtkWidget * image_button(const gchar *stock, int (*call)(void))
 {
 	GtkWidget *widget = gtk_button_new();
 	gtk_container_add(GTK_CONTAINER(widget),gtk_image_new_from_stock(stock,GTK_ICON_SIZE_SMALL_TOOLBAR));

	g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(clear_and_call),call);
	return widget;
 }

 GtkWidget *CreateKeypadWindow(void)
 {

	#define SMALL_BUTTON(stock,call,col,row)		widget = image_button(stock,call); \
													gtk_table_attach_defaults(GTK_TABLE(table),widget,col,col+2,row,row+1);

	#define LARGE_BUTTON(stock,call,col,row)		widget = image_button(stock,call); \
													gtk_table_attach_defaults(GTK_TABLE(table),widget,(3*col),(3*col)+3,row,row+1);

	#define LARGE_TEXT_BUTTON(label,call,col,row)	widget = gtk_button_new_with_label(label); \
													g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(clear_and_call),call); \
													gtk_table_attach_defaults(GTK_TABLE(table),widget,(3*col),(3*col)+3,row,row+1);

 	int			row 	= 0;
 	int			col		= 0;
 	int			f;
 	GtkWidget	*table 	= gtk_table_new(4,6,FALSE);
 	GtkWidget	*widget;
 	GtkWidget	*vbox 	= gtk_vbox_new(FALSE,0);
 	gchar		label[10];

	gtk_container_set_border_width(GTK_CONTAINER(table),3);

	/* Create PF Buttons */
	for(f=0;f<12;f++)
	{
		g_snprintf(label,9,"PF%d",f+1);
		widget = gtk_button_new_with_label(label);

		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(pf_button),(gpointer) (f+1));

		gtk_table_attach_defaults(GTK_TABLE(table),widget,col,col+2,row,row+1);
		col += 2;
		if(col > 5)
		{
			col = 0;
			row++;
		}
	}

	/* Create movement buttons */
	SMALL_BUTTON(GTK_STOCK_GO_UP,action_CursorUp,2,4);
	SMALL_BUTTON(GTK_STOCK_GO_BACK,action_CursorLeft,0,5);
	SMALL_BUTTON(GTK_STOCK_GOTO_TOP,action_FirstField,2,5);
	SMALL_BUTTON(GTK_STOCK_GO_FORWARD,action_CursorRight,4,5);
	SMALL_BUTTON(GTK_STOCK_GO_DOWN,action_CursorDown,2,6);

	/* Create PA Buttons */
	for(f=0;f<3;f++)
	{
		g_snprintf(label,9,"PA%d",f+1);
		widget = gtk_button_new_with_label(label);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(pa_button),(gpointer) (f+1));
		gtk_table_attach_defaults(GTK_TABLE(table),widget,(f<<1),(f<<1)+2,7,8);
	}

	/* Create extra buttons */
	LARGE_BUTTON(GTK_STOCK_GOTO_FIRST,action_PreviousField,0,8);
	LARGE_BUTTON(GTK_STOCK_GOTO_LAST,action_NextField,1,8);
	LARGE_TEXT_BUTTON( _( "Clear" ), action_ClearFields, 0, 9);
	LARGE_TEXT_BUTTON( _( "Reset" ), action_Reset, 1, 9);
	LARGE_TEXT_BUTTON( _( "Erase\nEOF" ), action_EraseEOF, 0, 10);
	LARGE_TEXT_BUTTON( _( "Erase\nInput" ), action_EraseInput, 1, 10);


	/* "Enter" Button */
	widget = image_button(GTK_STOCK_OK,action_Enter);
	gtk_table_attach_defaults(GTK_TABLE(table),widget,0,6,11,12);

	/* Buttons ok, add table */
	gtk_box_pack_start(GTK_BOX(vbox),table,FALSE,FALSE,0);
	gtk_widget_show_all(vbox);

	keypad = gtk_handle_box_new();

	gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad),GTK_POS_LEFT);
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(keypad),GTK_SHADOW_ETCHED_IN);

    gtk_container_add(GTK_CONTAINER(keypad),vbox);

	if(Toggled(KEYPAD))
		gtk_widget_show(keypad);

	register_tchange(KEYPAD,set_visible);

	gtk_widget_set_name(keypad,"KeyPAD");
 	return keypad;
 }


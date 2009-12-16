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
 * Este programa está nomeado como keypad.c e possui 172 linhas de código.
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

 #include "gui.h"
 #include "keypad.h"

 #include <lib3270/config.h>
 #include <globals.h>

 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/toggle.h>
// #include <lib3270/hostc.h>

/*---[ Internals ]----------------------------------------------------------------------------------------------*/

 GtkWidget *keypad = NULL;

/*---[ Implement common calls ]---------------------------------------------------------------------------------*/

 static void pf_button(GtkButton *button, int key)
 {
 	PFKey(key);
 }

 static void pa_button(GtkButton *button, int key)
 {
 	action_PAKey(key);
 }

 static void set_button_action(GtkWidget *widget, const gchar *action_name)
 {
#if GTK_CHECK_VERSION(2,16,0)
	/* Find action by name, associate it with the button */
	GtkAction *action = get_action_by_name(action_name);

	Trace("Action(\"%s\")=%p",action_name,action);

	if(action)
	{
		gtk_activatable_set_related_action(GTK_ACTIVATABLE(widget),action);
	}
	else
	{
		Log("Error loading keypad: action \"%s\" is undefined",action_name);
		gtk_widget_set_sensitive(widget,FALSE);
	}
#else
	/* Find action callback by name, associate it with "clicked" signal */
	GCallback callback = get_action_callback_by_name(action_name);

	Trace("Action(\"%s\")=%p",action_name,callback);

	if(callback)
	{
		g_signal_connect(G_OBJECT(widget),"clicked",callback,0);
	}
	else
	{
		Log("Error loading keypad: action \"%s\" is undefined",action_name);
		gtk_widget_set_sensitive(widget,FALSE);
	}
#endif

 }

/*---[ Implement Configurable keypad ]--------------------------------------------------------------------------*/




/*---[ Implement Fixed Keypad ]---------------------------------------------------------------------------------*/

void SetKeypadSensitive(gboolean state)
{
	if(keypad)
		gtk_widget_set_sensitive(keypad,state);
}

 static void set_visible(int visible, int reason)
 {
 	if(visible)
		gtk_widget_show(keypad);
	else
		gtk_widget_hide(keypad);

 }

 static GtkWidget * image_button(const gchar *stock, const gchar *action_name)
 {
 	GtkWidget *widget = gtk_button_new();
 	gtk_container_add(GTK_CONTAINER(widget),gtk_image_new_from_stock(stock,GTK_ICON_SIZE_SMALL_TOOLBAR));

	set_button_action(widget,action_name);
	return widget;
 }

 GtkWidget *CreateKeypadWidget(void)
 {

	#define SMALL_BUTTON(stock,action,col,row)		widget = image_button(stock,action); \
													gtk_table_attach_defaults(GTK_TABLE(table),widget,col,col+2,row,row+1);

	#define LARGE_BUTTON(stock,action,col,row)		widget = image_button(stock,action); \
													gtk_table_attach_defaults(GTK_TABLE(table),widget,(3*col),(3*col)+3,row,row+1);

	#define LARGE_TEXT_BUTTON(label,action,col,row)	widget = gtk_button_new_with_label(label); \
													set_button_action(widget,action); \
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
	SMALL_BUTTON(GTK_STOCK_GO_UP,"CursorUp",2,4);
	SMALL_BUTTON(GTK_STOCK_GO_BACK,"CursorLeft",0,5);
	SMALL_BUTTON(GTK_STOCK_GOTO_TOP,"Home",2,5);
	SMALL_BUTTON(GTK_STOCK_GO_FORWARD,"CursorRight",4,5);
	SMALL_BUTTON(GTK_STOCK_GO_DOWN,"CursorDown",2,6);

	/* Create PA Buttons */
	for(f=0;f<3;f++)
	{
		g_snprintf(label,9,"PA%d",f+1);
		widget = gtk_button_new_with_label(label);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(pa_button),(gpointer) (f+1));
		gtk_table_attach_defaults(GTK_TABLE(table),widget,(f<<1),(f<<1)+2,7,8);
	}

	/* Create extra buttons */
	LARGE_BUTTON(GTK_STOCK_GOTO_FIRST,"PreviousField",0,8);
	LARGE_BUTTON(GTK_STOCK_GOTO_LAST,"NextField",1,8);
	LARGE_TEXT_BUTTON( _( "Clear" ), "ClearFields", 0, 9);
	LARGE_TEXT_BUTTON( _( "Reset" ), "Reset", 1, 9);
	LARGE_TEXT_BUTTON( _( "Erase\nEOF" ), "EraseEOF", 0, 10);
	LARGE_TEXT_BUTTON( _( "Erase\nInput" ), "EraseInput", 1, 10);


	/* "Enter" Button */
	widget = image_button(GTK_STOCK_OK,"Enter");
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


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

 #include <lib3270/config.h>
 #include <globals.h>

 #include "g3270.h"
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/toggle.h>
 #include <lib3270/hostc.h>

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

 static void pf_button(GtkButton *button, gchar *key)
 {
	action_internal(PF_action, IA_DEFAULT, key, CN);
 }

 static void pa_button(GtkButton *button, gchar *key)
 {
	action_internal(PA_action, IA_DEFAULT, key, CN);
 }

 static void clear_and_call(GtkButton *button, XtActionProc call)
 {
 	action_ClearSelection();
 	if(call)
		action_internal(call, IA_DEFAULT, CN, CN);
 }

 static GtkWidget * image_button(const gchar *stock, XtActionProc call)
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

		g_snprintf(label,9,"%d",f+1);

		// FIXME (perry#2#): Add a closure function to "g_free" the allocated string.
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(pf_button),g_strdup(label));

		gtk_table_attach_defaults(GTK_TABLE(table),widget,col,col+2,row,row+1);
		col += 2;
		if(col > 5)
		{
			col = 0;
			row++;
		}
	}

	/* Create movement buttons */
	SMALL_BUTTON(GTK_STOCK_GO_UP,Up_action,2,4);
	SMALL_BUTTON(GTK_STOCK_GO_BACK,Left_action,0,5);
	SMALL_BUTTON(GTK_STOCK_GOTO_TOP,Home_action,2,5);
	SMALL_BUTTON(GTK_STOCK_GO_FORWARD,Right_action,4,5);
	SMALL_BUTTON(GTK_STOCK_GO_DOWN,Down_action,2,6);

	/* Create PA Buttons */
	for(f=0;f<3;f++)
	{
		g_snprintf(label,9,"PA%d",f+1);
		widget = gtk_button_new_with_label(label);

		g_snprintf(label,9,"%d",f+1);

		// FIXME (perry#2#): Add a closure function to "g_free" the allocated string.
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(pa_button),g_strdup(label));

		gtk_table_attach_defaults(GTK_TABLE(table),widget,(f<<1),(f<<1)+2,7,8);
	}

	/* Create extra buttons */
	LARGE_BUTTON(GTK_STOCK_GOTO_FIRST,BackTab_action,0,8);
	LARGE_BUTTON(GTK_STOCK_GOTO_LAST,Tab_action,1,8);
	LARGE_TEXT_BUTTON( _( "Clear" ), Clear_action, 0, 9);
	LARGE_TEXT_BUTTON( _( "Reset" ), Reset_action, 1, 9);
	LARGE_TEXT_BUTTON( _( "Erase\nEOF" ), EraseEOF_action, 0, 10);
	LARGE_TEXT_BUTTON( _( "Erase\nInput" ), EraseInput_action, 1, 10);


	/* "Enter" Button */
	widget = image_button(GTK_STOCK_OK,Enter_action);
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


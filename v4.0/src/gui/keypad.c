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

 #include <stdlib.h>
 // #include <globals.h>

 // #include <lib3270/kybdc.h>
// #include <lib3270/actionsc.h>
// #include <lib3270/toggle.h>
// #include <lib3270/hostc.h>

/*---[ Internals ]----------------------------------------------------------------------------------------------*/

#ifdef ENABLE_LOADABLE_KEYPAD

 struct build_data
 {
 	struct keypad *first_keypad;
 	struct keypad *last_keypad;

 };

#endif

 static const char *widget_name_prefix = "keypad_";

/*---[ Implement common calls ]---------------------------------------------------------------------------------*/

 static void pf_button(GtkButton *button, int key)
 {
 	PFKey(key);
 }

 static void pa_button(GtkButton *button, int key)
 {
 	action_PAKey(key);
 }

 static void set_sensitive(GtkWidget *widget, gpointer state)
 {
	if(g_str_has_prefix(gtk_widget_get_name(widget),widget_name_prefix))
		gtk_widget_set_sensitive(widget,state == 0 ? FALSE : TRUE);
	else if(GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),set_sensitive,state);

 }

 void SetKeypadSensitive(gboolean state)
 {
	gtk_container_foreach(GTK_CONTAINER(topwindow),set_sensitive,(gpointer) ( state ? -1 : 0 ));
 }

 static void set_button_action(GtkWidget *widget, const gchar *action_name)
 {
	/* Find action callback by name, associate it with "clicked" signal */

	#warning Ajustar ao novo processamento de acoes

	/*

	GCallback callback = get_action_callback_by_name(action_name);

//	Trace("Action(\"%s\")=%p",action_name,callback);

	if(callback)
	{
		g_signal_connect(G_OBJECT(widget),"clicked",callback,0);
		return;

	}

	if(g_str_has_prefix(action_name,"pf") || g_str_has_prefix(action_name,"PF"))
	{
		// Connect with pfkey
		int key = atoi(action_name+2);
		if(key > 0 && key < 25)
		{
			g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(pf_button),(gpointer) key);
			return;
		}
	}
	else if(g_str_has_prefix(action_name,"pa") || g_str_has_prefix(action_name,"PA"))
	{
		// Connect with pakey
		int key = atoi(action_name+2);
		if(key > 0 && key < 4)
		{
			g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(pa_button),(gpointer) key);
			return;
		}
	}

	Trace("\n\nError loading keypad: action \"%s\" is undefined",action_name);
	Log("Error loading keypad: action \"%s\" is undefined",action_name);
*/
	gtk_widget_set_sensitive(widget,FALSE);


 }

 void keypad_set_flags(GtkWidget *widget)
 {
 	if(!widget)
		return;

	GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_DEFAULT);

	if(GTK_IS_CONTAINER(widget))
		gtk_container_foreach(GTK_CONTAINER(widget),(GtkCallback) keypad_set_flags,0);

 }

 static void toggle_toolbar(GtkCheckMenuItem *item, GtkWidget *keypad)
 {
 	gboolean state = gtk_check_menu_item_get_active(item);
 	if(state)
		gtk_widget_show(keypad);
 	else
		gtk_widget_hide(keypad);

 	SetBoolean("Toggles", gtk_widget_get_name(keypad), state);
 }

 void configure_toolbar(GtkWidget *toolbar, GtkWidget *parent, const gchar *label)
 {
 	GtkWidget *menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(parent));
	gboolean isVisible = GetBoolean("Toggles", gtk_widget_get_name(toolbar), TRUE);

	if(!menu)
	{
		menu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(parent),menu);
	}

	Trace("Submenu: %p",menu);
	if(menu)
	{
		GtkWidget *item = gtk_check_menu_item_new_with_label(gettext(label));
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),isVisible);
		gtk_widget_show(item);
		g_signal_connect(G_OBJECT(item), "toggled", G_CALLBACK(toggle_toolbar), toolbar);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
		gtk_widget_show(menu);
	}

	if(isVisible)
		gtk_widget_show(toolbar);
	else
		gtk_widget_hide(toolbar);

	gtk_widget_show(parent);

 }

 #define APPEND_CHILD_ELEMENT(parent,first,last,child) if( parent->last) { parent->last->next = child; parent->last = child; } else { parent->first = parent->last = child; }

 static const gchar * get_attribute(const gchar **names, const gchar **values, const gchar *key)
 {
	while(*names && strcmp(*names,key))
	{
		names++;
		values++;
	}

 	return *names ? *values : "";
 }

 static void element_start(GMarkupParseContext *context,const gchar *element_name,const gchar **attribute_names,const gchar **attribute_values, gpointer user_data, GError **error)
 {
 	struct build_data *data = ((struct build_data *) user_data);

//	Trace("Starting \"%s\"",element_name);

	if(!strcmp(element_name,"keypad"))
	{
		// Alocate a new keypad
		const gchar *name 	= get_xml_attribute(attribute_names,attribute_values,"name");
		const gchar *label = get_xml_attribute(attribute_names,attribute_values,"label");
		const gchar *position = get_xml_attribute(attribute_names,attribute_values,"position");

		if(*name)
		{
			struct keypad *keypad;

			if(!(label && *label))
				label = name;

			keypad = g_malloc0(sizeof(struct keypad)+strlen(name)+strlen(label)+2);

			strcpy(keypad->name,name);

			keypad->label = keypad->name+(strlen(keypad->name)+1);
			strcpy(keypad->label,label);

			if(!g_strncasecmp(position,"top",3))
				keypad->position = KEYPAD_POSITION_TOP;
			else if(!g_strncasecmp(position,"left",4))
				keypad->position = KEYPAD_POSITION_LEFT;
			else if(!g_strncasecmp(position,"bottom",6))
				keypad->position = KEYPAD_POSITION_BOTTOM;
			else
				keypad->position = KEYPAD_POSITION_RIGHT;

//			Trace("\n\n\nLoading keypad \"%s\" (\"%s\") first: %p last: %p",keypad->name,keypad->label,data->first_keypad,data->last_keypad);

			APPEND_CHILD_ELEMENT(data,first_keypad,last_keypad,keypad);
		}

	}
	else if(!strcmp(element_name,"row") && data->last_keypad)
	{
		struct keypad		*keypad = data->last_keypad;
		struct keypad_row 	*row 	= g_malloc0(sizeof(struct keypad_row));

		keypad->rows++;

		APPEND_CHILD_ELEMENT(keypad,first_row,last_row,row);

	}
	else if(!strcmp(element_name,"keypadbutton") && data->last_keypad && data->last_keypad->last_row)
	{
		struct keypad			*keypad = data->last_keypad;
		struct keypad_row 		*row	= keypad->last_row;
		struct keypad_button 	*button	= g_malloc0(sizeof(struct keypad_button));
		const	gchar 			*action	= get_attribute(attribute_names,attribute_values,"action");
		const	gchar 			*label	= get_attribute(attribute_names,attribute_values,"label");
		const	gchar 			*icon	= get_attribute(attribute_names,attribute_values,"icon");

		if(++row->cols > keypad->cols)
			keypad->cols = row->cols;

		// Create action button
		if(*action)
		{
			if(*icon)
			{
				gchar *ptr = g_strdup_printf("gtk-%s",icon);
				button->widget = gtk_button_new();
				gtk_container_add(GTK_CONTAINER(button->widget),gtk_image_new_from_stock(ptr,GTK_ICON_SIZE_SMALL_TOOLBAR));
				g_free(ptr);
			}
			else if(*label)
			{
				gchar *ptr = g_strcompress(label);
				button->widget = gtk_button_new_with_label(gettext(ptr));
				g_free(ptr);
			}
			else
			{
				button->widget = gtk_button_new_with_label( gettext(action) );
			}

			gtk_button_set_focus_on_click(GTK_BUTTON(button->widget),FALSE);
			gtk_button_set_relief(GTK_BUTTON(button->widget),GTK_RELIEF_HALF);
			set_button_action(button->widget, action);
		}

		// Append in list
		APPEND_CHILD_ELEMENT(row,first_button,last_button,button);
//		Trace("Keypad %p tem %dx%d",keypad,keypad->rows,keypad->cols);

	}


 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error)
 {

 }

 static void element_text(GMarkupParseContext *context,const gchar *text,gsize text_len, gpointer user_data, GError **error)
 {
 }

 static void element_passthrough(GMarkupParseContext *context,const gchar *passthrough_text, gsize text_len,  gpointer user_data,GError **error)
 {
 }

 static void element_error(GMarkupParseContext *context,GError              *error,gpointer             user_data)
 {
 }


 static const GMarkupParser parser =
 {
	element_start,
	element_end,
	element_text,
	element_passthrough,
	element_error,
 };

 void keypad_free(struct keypad *keypad)
 {
 	if(!keypad)
		return;

	while(keypad->first_row)
	{
		struct keypad_row *next_row = keypad->first_row->next;
		while(keypad->first_row->first_button)
		{
			struct keypad_button *next_button = keypad->first_row->first_button->next;
			g_free(keypad->first_row->first_button);
			keypad->first_row->first_button = next_button;
		}
		g_free(keypad->first_row);
		keypad->first_row = next_row;
	}
	g_free(keypad);
 }

 void keypad_pack(GtkBox *box, GtkWidget *menu, struct keypad *keypad, enum KEYPAD_POSITION position)
 {
	while(keypad)
	{
		if(keypad->position == position && keypad->widget)
		{
//			Trace("\n\n\nInserting keypad %s (%s) into menu %p",gtk_widget_get_name(keypad->widget),keypad->label,menu);

			gtk_box_pack_start(GTK_BOX(box), keypad->widget, FALSE, FALSE, 0);

			if(menu)
				configure_toolbar(keypad->widget,menu,keypad->label);
		}
		keypad = keypad->next;
	}
 }

 static GtkWidget * build_keypad(struct keypad *keypad)
 {
 	int			row = 0;
 	int 		col = 0;
 	GtkWidget	*table;
 	GtkWidget	*widget;
 	GtkWidget	*box;
 	gchar		*name;

 	keypad->cols <<= 1;

	table = gtk_table_new(keypad->rows,keypad->cols,FALSE);

	while(keypad->first_row)
	{
		struct keypad_row *next_row = keypad->first_row->next;
		int step = keypad->cols / keypad->first_row->cols;

//		Trace("Colunas na linha: %d  Colunas na tabela: %d   Step: %d",keypad->first_row->cols,keypad->cols,step);

		col = 0;
		while(keypad->first_row->first_button)
		{
			struct keypad_button *next_button = keypad->first_row->first_button->next;

			if(keypad->first_row->first_button->widget)
				gtk_table_attach(GTK_TABLE(table),keypad->first_row->first_button->widget,col,col+step,row,row+1,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

			col += step;

			g_free(keypad->first_row->first_button);
			keypad->first_row->first_button = next_button;
		}

		row++;

		g_free(keypad->first_row);
		keypad->first_row = next_row;
	}

	widget = gtk_handle_box_new();

	switch(keypad->position)
	{
	case KEYPAD_POSITION_LEFT:
		box 	= gtk_vbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(widget),GTK_POS_RIGHT);
		break;

	case KEYPAD_POSITION_TOP:
		box 	= gtk_vbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(widget),GTK_POS_LEFT);
		break;

	case KEYPAD_POSITION_BOTTOM:
		box 	= gtk_vbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(widget),GTK_POS_LEFT);
		break;

	default:	// KEYPAD_POSITION_RIGHT
		box 	= gtk_vbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(widget),GTK_POS_LEFT);

	}

	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(widget),GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(widget),box);

	name = g_strdup_printf("%s%s",widget_name_prefix,keypad->name);
	gtk_widget_set_name(widget,name);
	g_free(name);

	keypad_set_flags(widget);

	gtk_widget_show_all(widget);
	return widget;
 }

 struct keypad * keypad_load(void)
 {
 	struct build_data 	info;
 	struct keypad		*keypad;

 	GMarkupParseContext *context;

	GError  			*error		= NULL;
 	gchar				*contents	= NULL;
 	gchar				*filename	=  g_build_filename(program_data,"keypad.xml",NULL);

	//
	// Parse XML file creating data structures
	//
	Trace("Loading \"%s\"",filename);
	if(!g_file_get_contents(filename,&contents,NULL,&error))
	{
		GtkWidget *dialog;

		// Can't save configuration data, notify user
		dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Can't load %s" ), filename);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Error loading keypad" ) );

		if(error && error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);

		g_free(filename);
		return NULL;
	}

 	memset(&info,0,sizeof(info));

	context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT,&info,NULL);

	if(!g_markup_parse_context_parse(context,contents,strlen(contents),&error))
	{
		GtkWidget *dialog;

		// Can't save configuration data, notify user
		dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Can't parse keypad description" ) );

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Error loading keypad" ) );

		if(error && error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);

		Log("Error loading \"%s\"",filename);
	}
#ifdef DEBUG
	else
	{
		Trace("Carregou %s",filename);
	}
#endif

	g_free(filename);

	g_markup_parse_context_free(context);
	g_free(contents);

	// Build keypad widgets
	for(keypad = info.first_keypad;keypad;keypad = keypad->next)
		keypad->widget = build_keypad(keypad);

	// And return keypad list
	return info.first_keypad;
 }


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
 * Este programa está nomeado como uiparser.c e possui - linhas de código.
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

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <libintl.h>
 #include <glib/gi18n.h>

 #include <gtk/gtk.h>
 #include <string.h>

 #include "gui.h"
 #include "uiparser.h"

 #define APPEND_ELEMENT_IN_LIST(first,last,el)	el->previous  = last; \
												last = el; \
												if(el->previous) el->previous->next = el; else first = el;

 #define REPLACE_ATTRIBUTE(field,value) if(field) g_free(field); field = g_strdup(value);

/*---[ Structs ]--------------------------------------------------------------------------------------------*/

 struct ui_element
 {
 	GtkUIManagerItemType		  type;

 	gchar						* path;
 	gchar						* label;
 	gchar						* accel;

	GtkToggleAction				* view_action;

 	struct ui_element			* previous;
 	struct ui_element			* next;

	guint 				  		  merge_id;

 	gchar						  name[1];

 };

 struct keypad_col
 {
	GtkWidget				*widget;
 };

 struct keypad_row
 {
	unsigned short			allocated_cols;
 	unsigned short			used_cols;
	struct keypad_col		*col;
 };

 struct keypad_descriptor
 {
	struct ui_attributes	attr;

	unsigned short			rows;
	unsigned short			cols;

	enum KEYPAD_POSITION	position;

	unsigned short			allocated_rows;
	unsigned short			used_rows;
	struct keypad_row 		*row;

	GtkToggleAction			*view_action;
	GtkWidget				*widget;

 	gchar					name[1];
 };

 struct parse_data
 {
	GHashTable 					* actions;
	GHashTable 					* keypads;

	GtkBox						* box[2];

	GtkWidget					* window;

	GtkUIManager				* manager;
	guint 						  merge_id;

	GtkActionGroup				**group;
	GtkWidget					**keypad_widget;
	int							  index;

	gchar						* path;
	struct keypad_descriptor	* current_keypad;
	struct action_descriptor	* current_action;

	const gchar				* filename;

	struct ui_element			* first_element;
	struct ui_element			* last_element;

 };

/*---[ Implement ]------------------------------------------------------------------------------------------*/

 static GQuark xml_parse_error(void)
 {
 	return g_quark_from_static_string(PACKAGE_NAME "-xml-error");
 }

 const gchar * get_xml_attribute(const gchar **names, const gchar **values, const gchar *key)
 {
	while(*names && strcmp(*names,key))
	{
		names++;
		values++;
	}

 	return *names ? *values : "";
 }

 static struct action_descriptor * action_descriptor_check(struct parse_data *info, const gchar *action_name)
 {
 	// TODO (perry#1#): Convert action_name to lowercase first
	struct action_descriptor *data	= g_hash_table_lookup(info->actions,action_name);

	if(!data)
	{
		// Allocate a new action
		data = g_malloc0(sizeof(struct action_descriptor)+strlen(action_name));
		strcpy(data->name,action_name);
		g_hash_table_insert(info->actions,g_strdup(data->name),data);
	}

	info->current_action = data;

	return data;
 }

 static void load_ui_attributes(struct parse_data *info, struct ui_attributes *attr,const gchar **names, const gchar **values)
 {
	const struct _descr
	{
		const gchar	*name;
		gchar 		**value;
	} descr[] =
	{
		{ "label", 		&attr->label		},
		{ "tooltip", 	&attr->tooltip		},
	};

	int f;
	const char *ptr;

	for(f=0;f<G_N_ELEMENTS(descr);f++)
	{
		const gchar *ptr = get_xml_attribute(names,values,descr[f].name);
		if(ptr && *ptr)
		{
			REPLACE_ATTRIBUTE(*descr[f].value,ptr);
		}
	}

	// Load gtk stock icon attribute
	ptr = get_xml_attribute(names,values,"icon");
	if(ptr && *ptr)
	{
		if(attr->stock_id)
			g_free(attr->stock_id);
		attr->stock_id = g_strdup_printf("gtk-%s",ptr);
	}

	ptr = get_xml_attribute(names,values,"key");
	if(ptr && *ptr)
	{
		attr->key_value = 0;
		gtk_accelerator_parse(ptr,&attr->key_value,&attr->key_state);

		if(attr->key_value)
		{
			REPLACE_ATTRIBUTE(attr->accel,ptr);
		}
		else
		{
			GtkWidget *dialog;

			// Can't parse UI definition, notify user
			dialog = gtk_message_dialog_new(	GTK_WINDOW(info->window),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_WARNING,
												GTK_BUTTONS_OK,
												_(  "Error parsing %s" ), info->filename);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't parse UI" ) );
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), _( "Can't parse accelerator \"%s\"" ),ptr);
			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);
		}

	}

 }

 static void load_action_attributes(struct parse_data *info, struct action_descriptor *data, const gchar **names, const gchar **values)
 {
 	const gchar					* attr;

	// Create/Update action label
	load_ui_attributes(info,&data->attr,names,values);

	// Check for group
	attr = get_xml_attribute(names,values,"group");
	if(*attr && info->group)
	{
		int f;

		for(f=0;info->group[f];f++)
		{
			if(!g_strcasecmp(attr,gtk_action_group_get_name(info->group[f])))
				data->group = f;
		}
	}

 }

/**
 * Allocates a new keypad descriptor from xml attributes
 *
 */
 static struct keypad_descriptor * get_keypad_descriptor(struct parse_data *info, const gchar **names, const gchar **values)
 {
 	static const gchar *position[] = { "top", "left", "bottom", "right" };

 	const gchar 			 	*keypad_name = get_xml_attribute(names,values,"name");
 	struct keypad_descriptor	*ret;
 	int							f;
 	const						gchar *attr;

 	if(!(keypad_name && *keypad_name))
		return NULL;

 	ret = (struct keypad_descriptor *) g_hash_table_lookup(info->keypads,keypad_name);
 	if(ret)
 	{
		load_ui_attributes(info,&ret->attr,names,values);
		return ret;
 	}

	ret = g_malloc0(sizeof(struct keypad_descriptor)+strlen(keypad_name));
	strcpy(ret->name,keypad_name);

	load_ui_attributes(info,&ret->attr,names,values);
	if(!ret->attr.label)
		ret->attr.label = g_strdup("Keypad");

	g_hash_table_insert(info->keypads,g_strdup(ret->name),ret);

	attr = get_xml_attribute(names,values,"position");
	if(attr && *attr)
	{
		for(f=0;f<G_N_ELEMENTS(position);f++)
		{
			if(!g_strcasecmp(attr,position[f]))
				ret->position = f;
		}
	}
	else
	{
		ret->position = KEYPAD_POSITION_RIGHT;
	}


	Trace("Keypad \"%s\" created",ret->name);

 	return ret;
 }

/**
 * Allocates a new action descriptor from xml attributes
 *
 */
 static struct action_descriptor * get_action_descriptor(struct parse_data *info, const gchar **names, const gchar **values)
 {

	struct action_descriptor	* data			= NULL;
 	const gchar				* action_name	= get_xml_attribute(names,values,"name");
 	const gchar				* callback_name	= get_xml_attribute(names,values,"action");
 	gchar						* ptr;
	UI_CALLBACK					  cbk;

	// Get callback info
	if(callback_name && *callback_name)
		get_action_info_by_name(callback_name,names,values,&ptr,&cbk);
	else
		get_action_info_by_name(action_name,names,values,&ptr,&cbk);

	if(action_name && *action_name)
		data = action_descriptor_check(info, action_name);
	else
		data = action_descriptor_check(info, ptr ? ptr : callback_name);

	if(ptr)
		g_free(ptr);

//	Trace("Action %s: callback=%p %p key=%s (%04x:%04x)",data->name,data->callback,data->user_data,data->accelerator,data->key_value,data->key_state);

	load_action_attributes(info,data,names,values);

	data->ui_type	= cbk.type;

	if(!data->callback)
		data->callback	= cbk.callback;

	data->user_data	= cbk.user_data;

	if(cbk.label && !data->attr.label)
		data->attr.label = g_strdup(cbk.label);

	return data;
 }

 static void action_descriptor_free(struct action_descriptor *data)
 {
 	#define UNLOAD_ATTRIBUTE(x) g_free(data->attr.x);

	UNLOAD_ATTRIBUTE(label);
	UNLOAD_ATTRIBUTE(tooltip);
	UNLOAD_ATTRIBUTE(stock_id);
	UNLOAD_ATTRIBUTE(accel);
	UNLOAD_ATTRIBUTE(text);

	g_free(data->script.text);
//	g_free(data->script.filename);

	g_free(data);
 }

 static void keypad_descriptor_free(struct keypad_descriptor *data)
 {
 	int f;
 	#define UNLOAD_ATTRIBUTE(x) g_free(data->attr.x);

	Trace("Releasing descriptor for keypad %s (%dx%d)",data->name,data->rows,data->cols);

	UNLOAD_ATTRIBUTE(label);
	UNLOAD_ATTRIBUTE(tooltip);
	UNLOAD_ATTRIBUTE(stock_id);
	UNLOAD_ATTRIBUTE(accel);
	UNLOAD_ATTRIBUTE(text);

	// Release keypad rows
	for(f=0;f < data->used_rows;f++)
	{
		if(data->row[f].col)
			g_free(data->row[f].col);
	}
	g_free(data->row);

	g_free(data);
 }

 static struct ui_element * ui_element_append(struct parse_data *info, GtkUIManagerItemType type, const gchar *name)
 {
	struct ui_element	*element;
	gchar				*path 	= (info->path && *info->path) ? info->path : "/";

 	if(name && *name)
 	{
		// Check if element already exists
		for(element = info->first_element;element;element = element->next)
		{
			if( (strcmp(element->name,name) == 0) && (strcmp(element->path,path) == 0))
				return element;
		}

		// Not found, create a new one
		element = g_malloc0(sizeof(struct ui_element)+strlen(name)+strlen(path)+2);
		strcpy(element->name,name);

 	}
 	else
 	{
 		// Create an unnamed element
 		static int	  id	= 0;
 		gchar		* ptr	= g_strdup_printf("ui%08x",id++);

		element = g_malloc0(sizeof(struct ui_element)+strlen(ptr)+strlen(path)+2);
		strcpy(element->name,ptr);

		g_free(ptr);

 	}

	element->type = type;
	element->merge_id = info->merge_id;

	element->path = element->name+strlen(element->name)+1;
	strcpy(element->path,path);

	APPEND_ELEMENT_IN_LIST(info->first_element,info->last_element,element);

	return element;
 }

 static void add_ui_element(struct parse_data *info, GtkUIManagerItemType type, const gchar **names,const gchar **values)
 {
 	const gchar 		*name	= get_xml_attribute(names,values,"name");
	struct ui_element	*el		= ui_element_append(info, type, name);
	const gchar			*ptr;

	if(!el)
		return;

	ptr = get_xml_attribute(names,values,"label");
	if(ptr && *ptr)
		el->label = g_strdup(ptr);

	ptr = get_xml_attribute(names,values,"key");
	if(ptr && *ptr)
		el->accel = g_strdup(ptr);
 }

 static void add_ui_action(struct parse_data *info, GtkUIManagerItemType type, const gchar **names,const gchar **values)
 {
	struct action_descriptor *entry = get_action_descriptor(info, names, values);

	entry->callback_type = type;

	if(!entry)
		return;

	ui_element_append(info, type, entry->name);

 }

 static void add_scroll_element(struct parse_data *info, const gchar **names,const gchar **values)
 {
 	static const struct _direction
 	{
 		GdkScrollDirection	id;
 		const gchar 		*name;
 	} direction[] =
 	{
		{ GDK_SCROLL_UP,	"up"	},
		{ GDK_SCROLL_DOWN,	"down"	},
		{ GDK_SCROLL_LEFT,	"left"	},
		{ GDK_SCROLL_RIGHT,	"right"	}
 	};

 	int 						  dir 	= -1;
	UI_CALLBACK					  cbk;
	gchar						* ptr;
	struct action_descriptor	* data	= NULL;

	const gchar	*attr = get_xml_attribute(names,values,"direction");

	if(attr && *attr)
	{
		int f;

		for(f=0;f<G_N_ELEMENTS(direction) && dir == -1;f++)
		{
			if(!g_strcasecmp(direction[f].name,attr))
				dir = (int) direction[f].id;

		}
	}

 	if(dir < 0)
		return;

	ptr = g_strconcat("Scroll",direction[dir].name,NULL);
	data = action_descriptor_check(info, ptr);
	g_free(ptr);

	data->ui_type	= UI_CALLBACK_TYPE_SCROLL;
	data->sub		= dir;

	ptr = NULL;
	get_action_info_by_name(get_xml_attribute(names,values,"action"),names,values,&ptr,&cbk);
	if(ptr)
		g_free(ptr);

	load_action_attributes(info, data, names, values);

	data->callback	= cbk.callback;
	data->user_data	= cbk.user_data;

 }

 static void create_keypad_row(struct keypad_descriptor *keypad,const gchar **names, const gchar **values)
 {

	if(!keypad->row)
	{
		keypad->used_rows = 0;
		keypad->allocated_rows = 3;
		keypad->row = g_malloc(sizeof(struct keypad_row)*keypad->allocated_rows);
	}
	else if(keypad->used_rows >= keypad->allocated_rows)
	{
		keypad->allocated_rows <<= 1;
		keypad->row = g_realloc(keypad->row,sizeof(struct keypad_row)*keypad->allocated_rows);
	}

	memset(keypad->row+keypad->used_rows,0,sizeof(struct keypad_row));

	if(++keypad->used_rows > keypad->rows)
		keypad->rows = keypad->used_rows;
 }

 static void create_keypad_button(struct keypad_descriptor *keypad,const gchar **names, const gchar **values)
 {
	const gchar			*name;
 	struct keypad_row	*row = keypad->row+(keypad->used_rows-1);

	if(!row->col)
	{
		row->used_cols = 0;
		row->allocated_cols = keypad->cols ? keypad->cols : 3;
		row->col = g_malloc(sizeof(struct keypad_col)*row->allocated_cols);
	}
	else if(row->used_cols >= row->allocated_cols)
	{
		row->allocated_cols <<= 1;
		row->col = g_realloc(row->col,sizeof(struct keypad_col)*row->allocated_cols);
	}

	memset(row->col+row->used_cols,0,sizeof(struct keypad_col));

	name = get_xml_attribute(names,values,"name");

	if(!(name && *name))
		name = get_xml_attribute(names,values,"action");

//	Trace("Row %d Col %d Name: %s",keypad->used_rows-1,row->used_cols,name);

	if(name && *name)
	{
		UI_CALLBACK info;

		if(!get_action_info_by_name(name,names,values, 0,&info))
		{
			// Create keypad button
			GtkWidget		*widget = NULL;
			const gchar		*attr;

			if(info.type == UI_CALLBACK_TYPE_TOGGLE)
				widget = gtk_toggle_button_new();
			else
				widget = gtk_button_new();

			attr = get_xml_attribute(names,values,"label");
			if(attr && *attr)
			{
				gtk_button_set_label(GTK_BUTTON(widget),gettext(g_strcompress(attr)));
			}
			else
			{
				attr = get_xml_attribute(names,values,"icon");
				if(attr && *attr)
				{
					gchar *ptr = g_strconcat("gtk-",attr,NULL);
					gtk_container_add(GTK_CONTAINER(widget),gtk_image_new_from_stock(ptr,GTK_ICON_SIZE_SMALL_TOOLBAR));
					g_free(ptr);

				}
			}

			if(info.callback)
				g_signal_connect(G_OBJECT(widget),info.type == UI_CALLBACK_TYPE_TOGGLE ? "toggled" : "clicked",info.callback,info.user_data);
			else
				gtk_widget_set_sensitive(widget,FALSE);

			gtk_button_set_alignment(GTK_BUTTON(widget),0.5,0.5);
			gtk_button_set_focus_on_click(GTK_BUTTON(widget),FALSE);
			GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_FOCUS);
			GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_DEFAULT);

//			Trace("Widget %p row: %p, col: %p (%d)",widget,row,row->col+row->used_cols,row->used_cols);

			(row->col+row->used_cols)->widget = widget;
		}

	}

	if(++row->used_cols > keypad->cols)
		keypad->cols = row->used_cols;

 }

 static void create_script_element(struct action_script *script,const gchar **names, const gchar **values)
 {
 	const gchar *ptr = get_xml_attribute(names,values,"platform");

	if(ptr && *ptr)
	{
#ifdef linux
		if(g_ascii_strncasecmp(ptr,"linux",5))
		{
			script->enabled = FALSE;
			return;
		}
#endif

#ifdef WIN32
		if(g_ascii_strncasecmp(ptr,"win",3))
		{
			script->enabled = FALSE;
			return;
		}
#endif
	}

	/* Get filename */
/*
	ptr = get_xml_attribute(names,values,"filename");
	if(ptr)
	{
		if(script->filename)
			g_free(script->filename);
		script->filename = g_strdup(ptr);
	}
*/
 }

 static void element_start(GMarkupParseContext *context,const gchar *element_name,const gchar **names,const gchar **values, gpointer user_data, GError **error)
 {
 	const gchar	*name = get_xml_attribute(names,values,"name");
 	gchar 			*path = ((struct parse_data *) user_data)->path;

	if(!*name)
		name = get_xml_attribute(names,values,"action");

	if(!g_ascii_strcasecmp(element_name,"menuitem"))
	{
		add_ui_action((struct parse_data *) user_data, GTK_UI_MANAGER_MENUITEM, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"toolitem"))
	{
		add_ui_action((struct parse_data *) user_data, GTK_UI_MANAGER_TOOLITEM, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"accelerator"))
	{
		add_ui_action((struct parse_data *) user_data, GTK_UI_MANAGER_ACCELERATOR, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"menubar"))
	{
		add_ui_element((struct parse_data *) user_data, GTK_UI_MANAGER_MENUBAR, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"menu"))
	{
		add_ui_action((struct parse_data *) user_data, GTK_UI_MANAGER_MENU, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"toolbar"))
	{
		add_ui_element((struct parse_data *) user_data, GTK_UI_MANAGER_TOOLBAR, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"popup"))
	{
		add_ui_action((struct parse_data *) user_data, GTK_UI_MANAGER_POPUP, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"separator"))
	{
		ui_element_append((struct parse_data *) user_data, GTK_UI_MANAGER_SEPARATOR, get_xml_attribute(names,values,"name"));
	}
	else if(!g_ascii_strcasecmp(element_name,"scroll"))
	{
		add_scroll_element((struct parse_data *) user_data, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"keypad"))
	{
		((struct parse_data *) user_data)->current_keypad = get_keypad_descriptor((struct parse_data *) user_data, names, values);
	}
	else if(!g_ascii_strcasecmp(element_name,"row"))
	{
		if(!((struct parse_data *) user_data)->current_keypad)
			g_set_error(error,xml_parse_error(),EINVAL,_("<row> element should be used only inside <keypad>"));
		else
			create_keypad_row( ((struct parse_data *) user_data)->current_keypad,names,values);
	}
	else if(!g_ascii_strcasecmp(element_name,"button"))
	{
		if(!( ((struct parse_data *) user_data)->current_keypad && (((struct parse_data *) user_data)->current_keypad->used_rows)) )
			g_set_error(error,xml_parse_error(),EINVAL,_("<button> element should be used only inside <keypad><row>"));
		else
			create_keypad_button( ((struct parse_data *) user_data)->current_keypad,names,values);

	}
	else if(!g_ascii_strcasecmp(element_name,"script"))
	{
		struct action_descriptor *data = ((struct parse_data *) user_data)->current_action;

		if(!data)
		{
			g_set_error(error,xml_parse_error(),EINVAL,_("<script> element should be used only inside another active element"));
		}
		else
		{
			create_script_element( &data->script, names, values);
			data->ui_type = UI_CALLBACK_TYPE_SCRIPT;

			if(!(name && *name))
				name = data->name;

			if(!name)
			{
				g_set_error(error,xml_parse_error(),EINVAL,_("<script> element requires a name"));
			}
			else if(data->script.enabled);
			{
				int f;

#ifdef HAVE_PLUGINS
				const gchar *ptr = get_xml_attribute(names,values,"language");
				if(ptr && *ptr)
					data->script.enabled = get_symbol_by_name(NULL, (gpointer) &data->callback, "pw3270_call_%s_script", ptr);
#endif
				if(data->script.enabled)
				{
					data->action = gtk_action_new(name, gettext(data->attr.label ? data->attr.label : data->name), gettext(data->attr.tooltip),data->attr.stock_id);
					Trace("Action %s: %p",name,data->action);

					for(f=0;names[f];f++)
					{
						gchar *ptr = g_strdup_printf("script_%s",names[f]);
						Trace("%s=%s",ptr,values[f]);
						g_object_set_data_full(G_OBJECT(data->action),ptr,g_strdup(values[f]),g_free);
						g_free(ptr);
					}
				}

			}

		}
	}

/*
  GTK_UI_MANAGER_PLACEHOLDER       = 1 << 3,
  GTK_UI_MANAGER_POPUP_WITH_ACCELS = 1 << 9
*/

	if(path)
		((struct parse_data *) user_data)->path = g_strdup_printf("%s/%s",path,name);
	else
		((struct parse_data *) user_data)->path = g_strdup_printf("%s",name);

	g_free(path);

 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error)
 {
 	gchar *path = ((struct parse_data *) user_data)->path;

 	if(path)
 	{
 		gchar *ptr = g_strrstr(path,"/");
 		if(ptr)
			*ptr = 0;
 	}
 }

 static void element_text(GMarkupParseContext *context,const gchar *text,gsize text_len, gpointer user_data, GError **error)
 {
 	struct action_descriptor	*action = ((struct parse_data *) user_data)->current_action;
 	gchar 						*str = g_malloc0(text_len+1);

 	memcpy(str,text,text_len);

 	g_strstrip(str);

	if(!*str)
	{
		g_free(str);
		return;
	}

	if(!action)
	{
		g_set_error(error,xml_parse_error(),EINVAL,_("Expecting active element"));
		g_free(str);
		return;
	}

	if(!action->script.enabled)
	{
		g_free(str);
		return;
	}


	if(action->script.text)
		g_free(action->script.text);

	action->script.text = str;
	return;

 }

 static void element_passthrough(GMarkupParseContext *context,const gchar *passthrough_text, gsize text_len,  gpointer user_data,GError **error)
 {
 }

 static void element_error(GMarkupParseContext *context,GError *error,gpointer user_data)
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

 static int append_from_xml(struct parse_data *info, const gchar *filename)
 {
 	GMarkupParseContext *context;
 	gchar				*contents = NULL;
	GError 				*error = NULL;

	//
	// Parse XML file creating data structures
	//
	if(!g_file_get_contents(filename,&contents,NULL,&error))
	{
		GtkWidget *dialog;

		// Can't parse UI definition, notify user
		dialog = gtk_message_dialog_new(	GTK_WINDOW(info->window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Can't load %s" ), filename);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't parse UI" ) );

		if(error && error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);
		return 0;
	}

	Trace("Loading %s",filename);
	context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT,info,NULL);

	if(!g_markup_parse_context_parse(context,contents,strlen(contents),&error))
	{
		GtkWidget *dialog;

		// Can't parse UI definition, notify user
		dialog = gtk_message_dialog_new(	GTK_WINDOW(info->window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Can't parse ui in %s" ), filename);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't parse UI" ) );

		if(error && error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);
	}

	g_markup_parse_context_free(context);

	// Release file contents and build data
	Trace("%s: Releasing file contents",__FUNCTION__);
	g_free(contents);
	return 0;
 }

 static int append_from_act(struct parse_data *info, const gchar *filename)
 {
	GKeyFile *conf = g_key_file_new();

	if(g_key_file_load_from_file(conf,filename,G_KEY_FILE_NONE,NULL))
	{
		int f,p;
		gchar **group = g_key_file_get_groups(conf,NULL);

		for(f=0;group[f];f++)
		{
			static const gchar *names[] =	{	"name",
												"action",
												"label",
												"tooltip",
												"icon",
												"key",
												"id",
												NULL
											};
			const gchar *values[]		=	{	group[f],
												g_key_file_get_string(conf,group[f],"action",NULL),
												g_key_file_get_string(conf,group[f],"label",NULL),
												g_key_file_get_string(conf,group[f],"tooltip",NULL),
												g_key_file_get_string(conf,group[f],"stock-id",NULL),
												g_key_file_get_string(conf,group[f],"accelerator",NULL),
												g_key_file_get_string(conf,group[f],"value",NULL),
											};

			get_action_descriptor(info, names, values);

			// Release memory
			for(p=1;p<G_N_ELEMENTS(values);p++)
			{
				if(values[p])
					g_free((gchar *) values[p]);
			}

		}
		g_strfreev(group);
	}

	g_key_file_free(conf);

	return 0;
 }

 static void element_toggle_show(GtkToggleAction *action, GtkWidget *widget)
 {
 	gboolean show = gtk_toggle_action_get_active(action);

#if GTK_CHECK_VERSION(2,18,0)

 	if(gtk_widget_get_visible(widget) != show)
 	{
		gtk_widget_set_visible(widget,show);
		SetBoolean("UIToggles",gtk_widget_get_name(widget),show);
 	}

#else

	SetBoolean("UIToggles",gtk_widget_get_name(widget),show);

	if(show)
		gtk_widget_show(widget);
	else
		gtk_widget_hide(widget);

#endif

 }

 static void create_view_actions(struct parse_data *info, GtkUIManagerItemType type)
 {
 	struct ui_element *element;

 	for(element = info->first_element;element;element = element->next)
	{
		if(element->type == type)
		{
			gchar *action_name	= g_strconcat("Show",element->name,NULL);

			element->view_action = gtk_toggle_action_new(action_name,gettext(element->label),NULL,NULL);

			if(element->accel)
				gtk_action_group_add_action_with_accel(info->group[0],GTK_ACTION(element->view_action),element->accel);
			else
				gtk_action_group_add_action(info->group[0],GTK_ACTION(element->view_action));

			g_free(action_name);
		}
	}
 }

 static void pack_view_actions(struct ui_element *element, struct parse_data *info, GtkUIManagerItemType type, GtkBox *box)
 {
	while(element)
	{
		if(element->view_action && element->type == type)
		{
			gchar		*path	= g_strconcat(element->path,"/",element->name,NULL);
			GtkWidget	*widget	= gtk_ui_manager_get_widget(info->manager,path);

			if(!widget && path[1] == '/')
				widget = gtk_ui_manager_get_widget(info->manager,path+1);

			Trace("%s: %p (label: %s)",path,widget,element->label);
			if(widget)
			{
				gboolean visible = GetBoolean("UIToggles",gtk_widget_get_name(widget),TRUE);

				gtk_box_pack_start(box,widget,FALSE,FALSE,0);

				gtk_toggle_action_set_active(element->view_action,visible);

#if GTK_CHECK_VERSION(2,18,0)
				gtk_widget_set_visible(widget,visible);
#else
				if(visible)
					gtk_widget_show(widget);
				else
					gtk_widget_hide(widget);
#endif
				g_signal_connect(G_OBJECT(element->view_action),"toggled",G_CALLBACK(element_toggle_show),widget);
			}

			g_free(path);
		}
		element = element->next;
	}
 }

 struct add_action_data
 {
	gchar 			*path;
	GtkUIManager	*manager;
	guint			merge_id;

 };

 void add_keypad_view_action(const gchar *key, struct keypad_descriptor *element, struct add_action_data *data)
 {
 	Trace("%s: %s %p",__FUNCTION__,element->name,element->view_action);
 	if(element->view_action)
 	{
		const gchar *name = gtk_action_get_name(GTK_ACTION(element->view_action));

#if GTK_CHECK_VERSION(2,18,0)
		const gchar *label = gtk_action_get_label(GTK_ACTION(element->view_action));
#else
		const gchar *label = NULL;
		g_object_get(G_OBJECT(element->view_action),"label",&label,NULL);
#endif

		Trace("Incluindo %s/%s",data->path,name);

		if(label && *label)
			gtk_ui_manager_add_ui(data->manager,data->merge_id,data->path,name,name,GTK_UI_MANAGER_MENUITEM,FALSE);
 	}
 }

 static void build_ui_manager(struct parse_data *info)
 {
	struct add_action_data data;
	struct ui_element * view = NULL;
	struct ui_element * element;

	data.manager = info->manager;

	for(element = info->first_element;element;element = element->next)
	{
		gtk_ui_manager_add_ui(info->manager,element->merge_id,element->path,element->name,element->name,element->type,FALSE);

		if(element->type == GTK_UI_MANAGER_MENU && !g_ascii_strcasecmp(element->name,"view"))
			view = element;
	}

	gtk_ui_manager_ensure_update(info->manager);

	Trace("View menu: %p",view);

	if(!view)
		return;

	// Add menus & toolbars
	data.path = g_strconcat(view->path,"/",view->name,NULL);
	data.merge_id = gtk_ui_manager_new_merge_id(info->manager);

	for(element = info->first_element;element;element = element->next)
	{
		if(element->view_action)
		{
			const gchar *name = gtk_action_get_name(GTK_ACTION(element->view_action));

#if GTK_CHECK_VERSION(2,18,0)
			const gchar *label = gtk_action_get_label(GTK_ACTION(element->view_action));
#else
			const gchar *label = NULL;
			g_object_get(G_OBJECT(element->view_action),"label",&label,NULL);
#endif

			Trace("Incluindo %s/%s",data.path,name);
			if(label && *label)
				gtk_ui_manager_add_ui(data.manager,data.merge_id,data.path,name,name,GTK_UI_MANAGER_MENUITEM,FALSE);
		}
	}

	// Add keypads
	g_hash_table_foreach(info->keypads, (GHFunc) add_keypad_view_action, &data);

	// Release memory & update
	g_free(data.path);
	gtk_ui_manager_ensure_update(info->manager);
 }

 static void create_keypad_by_descriptor(const gchar *name, struct keypad_descriptor *keypad, struct parse_data *info)
 {
	gchar *action_name = g_strconcat("Keypad",keypad->name,NULL);

	keypad->view_action = gtk_toggle_action_new(action_name,	gettext(keypad->attr.label),
																gettext(keypad->attr.tooltip),
																gettext(keypad->attr.stock_id));

	if(keypad->attr.accel)
		gtk_action_group_add_action_with_accel(info->group[0],GTK_ACTION(keypad->view_action),keypad->attr.accel);
	else
		gtk_action_group_add_action(info->group[0],GTK_ACTION(keypad->view_action));

	Trace("Action(%s): %p",action_name,keypad->view_action);
	g_free(action_name);
 }

 void pack_keypad_widget(const gchar *key, struct keypad_descriptor *keypad, struct parse_data *info)
 {
 	int			width	= keypad->cols << 1;
	GtkWidget	*table	= gtk_table_new(keypad->rows,width,FALSE);
	GtkWidget	*box;
	int			r;
	int			c;
//	int			f;

	GTK_WIDGET_UNSET_FLAGS(table,GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(table,GTK_CAN_DEFAULT);

	for(r=0;r < keypad->used_rows;r++)
	{
		struct keypad_row *row = keypad->row+r;
		int step = width / (row->used_cols);
		int pos  = 0;

		for(c = 0; c < row->used_cols;c++)
		{
			struct keypad_col *col = row->col+c;

//			Trace("Widget %p at %d,%d",col->widget,r,pos);

			if(col->widget)
			{
				// Add widget in table
				gtk_table_attach(GTK_TABLE(table),col->widget,pos,pos+step,r,r+1,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);
			}
			pos += step;
		}
	}

	keypad->widget = gtk_handle_box_new();

	switch(keypad->position)
	{
	case KEYPAD_POSITION_LEFT:
		box 	= gtk_vbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->widget),GTK_POS_RIGHT);
		gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(keypad->widget),GTK_POS_LEFT);
		break;

	case KEYPAD_POSITION_TOP:
		box 	= gtk_vbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->widget),GTK_POS_BOTTOM);
		gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(keypad->widget),GTK_POS_TOP);
		break;

	case KEYPAD_POSITION_BOTTOM:
		box 	= gtk_vbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->widget),GTK_POS_TOP);
		gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(keypad->widget),GTK_POS_BOTTOM);
		break;

	default:	// KEYPAD_POSITION_RIGHT
		box 	= gtk_vbox_new(FALSE,0);
		gtk_box_pack_start(GTK_BOX(box),table,FALSE,FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->widget),GTK_POS_LEFT);
		gtk_handle_box_set_snap_edge(GTK_HANDLE_BOX(keypad->widget),GTK_POS_RIGHT);

	}

	gtk_widget_show_all(box);

	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(keypad->widget),GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(keypad->widget),box);

	GTK_WIDGET_UNSET_FLAGS(keypad->widget,GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(keypad->widget,GTK_CAN_DEFAULT);

	gtk_widget_set_name(keypad->widget,keypad->name);

	Trace("Packing keypad (%s): %p",keypad->name,keypad->widget);

	gtk_widget_show_all(keypad->widget);

	if(keypad->position & 0x02)
		gtk_box_pack_end(info->box[keypad->position & 0x01],keypad->widget,FALSE,FALSE,0);
	else
		gtk_box_pack_start(info->box[keypad->position & 0x01],keypad->widget,FALSE,FALSE,0);

	if(keypad->view_action)
	{
		gboolean visible = GetBoolean("UIToggles",gtk_widget_get_name(keypad->widget),TRUE);

		gtk_toggle_action_set_active(keypad->view_action,visible);

#if GTK_CHECK_VERSION(2,18,0)

		gtk_widget_set_visible(keypad->widget,visible);

#else

		if(visible)
			gtk_widget_show(keypad->widget);
		else
			gtk_widget_hide(keypad->widget);

#endif

		g_signal_connect(G_OBJECT(keypad->view_action),"toggled",G_CALLBACK(element_toggle_show),keypad->widget);
	}

	// Pack keypad widget
	Trace("Keypad(%d): %p",info->index,keypad->widget);
	info->keypad_widget[info->index++] = keypad->widget;
 }

 GtkUIManager * load_application_ui(GtkWidget *window, GtkBox *toolbar, GtkBox *vbox, GtkBox *hbox)
 {

	GDir				* dir;
	gchar				* path;
	int					  f;
	struct ui_element	* element;
 	struct parse_data 	  info;
	GError 				* error = NULL;
//	GList				* list;
//	GtkWidget			**keypad;
//	int					  sz;

	path = g_build_filename(program_data,"ui",NULL);

 	memset(&info,0,sizeof(info));

 	info.manager	= gtk_ui_manager_new(); // http://library.gnome.org/devel/gtk/stable/GtkUIManager.html
 	info.window		= window;
 	info.actions	= g_hash_table_new_full(g_str_hash,g_str_equal,(GDestroyNotify) g_free,(GDestroyNotify) action_descriptor_free);
 	info.keypads	= g_hash_table_new_full(g_str_hash,g_str_equal,(GDestroyNotify) g_free,(GDestroyNotify) keypad_descriptor_free);
	info.group		= (GtkActionGroup **) g_object_get_data(G_OBJECT(window),"ActionGroups");
	info.box[0]		= vbox;
	info.box[1]		= hbox;

	// Insert action groups in the UI Manager
	if(info.group)
	{
		for(f=0;info.group[f]; f++)
			gtk_ui_manager_insert_action_group(info.manager,info.group[f], 0);
	}

	// Load control files
	dir = g_dir_open(path,0,&error);
	if(!dir)
	{
		GtkWidget *dialog;

		// Can't save configuration data, notify user
		dialog = gtk_message_dialog_new(	GTK_WINDOW(info.window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Can't find UI definitions in %s" ), path);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't load UI" ) );

		if(error && error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);
		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);

	}
	else
    {
		const gchar *name = g_dir_read_name(dir);
		while(name)
		{
				info.filename = g_build_filename(path,name,NULL);

				if(g_str_has_suffix(info.filename,"xml"))
				{
					info.merge_id = gtk_ui_manager_new_merge_id(info.manager);
					append_from_xml(&info, info.filename);
				}
				else if(g_str_has_suffix(info.filename,"act"))
				{
					append_from_act(&info, info.filename);
				}

				if(info.path)
				{
					g_free(info.path);
					info.path = NULL;
				}

				g_free((gchar *) info.filename);
				name = g_dir_read_name(dir);
		}
		g_dir_close(dir);

    }

	// Create actions
	g_hash_table_foreach(info.actions,(GHFunc) create_action_by_descriptor,&info);
	g_hash_table_foreach(info.keypads,(GHFunc) create_keypad_by_descriptor,&info);
	create_view_actions(&info, GTK_UI_MANAGER_TOOLBAR);
	create_view_actions(&info, GTK_UI_MANAGER_MENUBAR);

	// Build UI
	build_ui_manager(&info);

	for(element = info.first_element;element;element = element->next)
	{
		// UI widgets in the action
		gchar						*path	= g_strconcat(element->path,"/",element->name,NULL);
		GtkWidget					*widget	= gtk_ui_manager_get_widget(info.manager,path);
		struct action_descriptor	*data	= g_hash_table_lookup(info.actions,element->name);

		if(!widget && path[1] == '/')
			widget = gtk_ui_manager_get_widget(info.manager,path+1);

//			Trace("%s = %p",path,widget);
		if(data && widget && data->action)
			g_object_set_data(G_OBJECT(data->action),"ui_widget",(gpointer) widget);

		g_free(path);

		if(widget && element->type == GTK_UI_MANAGER_MENUITEM && strchr(element->name,'.'))
		{
			// Populate plugin menu
			GModule *plugin;
			gchar *plugin_name		= g_strdup(element->name);
			gchar *entry_name		= strchr(plugin_name,'.');
			int (*populate)(GtkMenu *menu, GtkWidget *program_window);
			*(entry_name)++ = 0;

			plugin = get_plugin_by_name(plugin_name);

			if(plugin && get_symbol_by_name(plugin, (gpointer *) &populate, "load_menu_%s",entry_name))
			{
				GtkWidget *menu	= gtk_menu_new();
				Trace("Populating menu %p",menu);
				populate(GTK_MENU(menu),topwindow);
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget),menu);
				gtk_widget_show_all(widget);
			}

			g_free(plugin_name);

		}
	}

	// Pack menubar(s)
	pack_view_actions(info.first_element, &info, GTK_UI_MANAGER_MENUBAR, toolbar);

	// Pack toolbar(s)
	pack_view_actions(info.first_element, &info, GTK_UI_MANAGER_TOOLBAR, toolbar);

	// Pack & save keypads
	info.index = 0;
	Trace("Number of keypads: %d",g_hash_table_size(info.keypads));
	info.keypad_widget = g_malloc0(sizeof(GtkWidget *) * (g_hash_table_size(info.keypads)+1));
	g_hash_table_foreach(info.keypads,(GHFunc) pack_keypad_widget, &info);

	g_object_set_data(G_OBJECT(window),"ui_keypad",(gpointer) info.keypad_widget);

	// Release memory
	while(info.first_element)
	{
		info.last_element = info.first_element->next;

		if(info.first_element->label)
			g_free(info.first_element->label);

		if(info.first_element->accel)
			g_free(info.first_element->accel);

		g_free(info.first_element);
		info.first_element = info.last_element;
	}
	info.first_element = info.last_element = 0;

	g_hash_table_destroy(info.keypads);
	g_hash_table_destroy(info.actions);

	g_free(path);

	gtk_window_add_accel_group(GTK_WINDOW(window),gtk_ui_manager_get_accel_group(info.manager));

	// And return the created ui manager
	return info.manager;
 }

 void keypad_set_sensitive(GtkWidget *window, gboolean state)
 {
	GtkWidget **keypad = g_object_get_data(G_OBJECT(window),"ui_keypad");

	if(keypad)
	{
		int f;
		for(f=0;keypad[f];f++)
		{
			Trace("Keypad %s is now %s",gtk_widget_get_name(keypad[f]),state ? "sensitive" : "insensitive");
			gtk_widget_set_sensitive(gtk_bin_get_child(GTK_BIN(keypad[f])),state);

			// TODO (perry#1#): If detached invalidate toplevel.
		}
	}
 }


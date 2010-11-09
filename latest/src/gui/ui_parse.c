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
 * Este programa está nomeado como ui_parse.c e possui - linhas de código.
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

#include <libintl.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "gui.h"
#include "actions.h"
#include "ui_parse.h"

/*---[ Defines ]------------------------------------------------------------------------------------------*/

 #define ERROR_DOMAIN g_quark_from_static_string("uiparser")

 typedef struct _ui_element
 {
 	struct _ui_element	* parent;	/**< The parent element */
 	GtkWidget			* widget;	/**< The element Widget */
 	GtkAction			* action;	/**< The associated action */
 	const gchar		* name;		/**< The element name */

 } UI_ELEMENT;

 #ifdef MAC_INTEGRATION
	/**
	 * Name of the <menuitem> elements that will be moved for mac application menu.
	 */
	static const gchar * app_menu_item[] = { "about", "settings" };
 #endif

 typedef struct _parser_state
 {
 	GtkWidget		* window;									/**< Toplevel window */

 	UI_ELEMENT		* current;									/**< Active element */

	GHashTable	 	* action;									/**< List of active actions */

	GHashTable	 	* menubar;									/**< List of active menu bars */
	GHashTable	 	* menu;										/**< List of active menus */

	GHashTable	 	* toolbar;									/**< List of active toolbars */
	GHashTable 		* tool;										/**< List of active toolbar items */

	GtkAccelGroup	* accel_group;								/**< Accelerators */


#ifdef MAC_INTEGRATION
	GtkMenuShell	* top_menu;									/**< Mac OSX top menu */
	GtkMenuItem		* quit_menu;								/**< Quit menu widget */
	GtkMenuItem		* app_menu[G_N_ELEMENTS(app_menu_item)];	/**< Menu item elements on the mac application menu */
#endif // MAC_INTEGRATION

	int				  ignore;									/**< Non 0 if subtree is disabled */

 } PARSER_STATE;


/*---[ Statics ]------------------------------------------------------------------------------------------*/

 const gchar * get_xml_attribute(const gchar **names, const gchar **values, const gchar *key)
 {
	while(*names && g_strcasecmp(*names,key))
	{
		names++;
		values++;
	}

 	return *names ? *values : NULL;
 }

 static void action_quit(GtkAction *action, GtkWidget *window)
 {
	Trace("%s: Removing window %p",__FUNCTION__,window);
	gtk_widget_destroy(window);
 }

 static GtkAction * create_action(const gchar *element_name, const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	static const struct _rule
 	{
		gboolean	  toggle;
 		const gchar	* name;
		int			  (*setup)(GtkAction *, const gchar *, gboolean, const gchar **, const gchar **, GError **);

 	} rule[] =
 	{
		{ TRUE,		"toggle",			action_setup_toggle			},

		{ TRUE,		"toggleset",		action_setup_toggleset		},
		{ TRUE,		"togglereset",		action_setup_togglereset	},

		{ FALSE,	"pfkey",			action_setup_pfkey			},
		{ FALSE,	"pakey",			action_setup_pakey			},

 	};

	static const gchar *attr[] = { "key", "description" }; /**< Common attributes */

	int				  (*setup)(GtkAction *, const gchar *, gboolean, const gchar **, const gchar **, GError **) = action_setup_default;

 	int				  id			= -1;
	gchar			* name;
 	const gchar		* action_name	= get_xml_attribute(names,values,"action");
	GtkAction		* ret;
 	const gchar		* ptr;
 	int				  f;

	// No action, do nothing
	if(!action_name)
		return NULL;

 	for(f=0;f<G_N_ELEMENTS(rule) && id == -1;f++)
 	{
 		if(!g_strcasecmp(rule[f].name,action_name))
		{
			setup = rule[f].setup;
			id = f;
		}
 	}

	// Search if it was already created
	name = g_ascii_strdown(element_name,-1);
 	ret	 = g_hash_table_lookup(state->action,name);

 	if(!ret)
 	{
		// The action isn't in the table, create a new one
		gchar		* stock		= NULL;
		gboolean	  toggle	= FALSE;
		const gchar * icon		= get_xml_attribute(names,values,"icon");
		const gchar * label		= get_xml_attribute(names,values,"label");

		if(icon)
			stock = g_strdup_printf("gtk-%s",icon);

		if(label)
			label = gettext(label);
		else if(!stock)
			label = name;

		if(id >= 0)
			toggle = rule[id].toggle;

		if(toggle)
			ret = GTK_ACTION(gtk_toggle_action_new(name, label, get_xml_attribute(names,values,"tooltip"), stock));
		else
			ret = gtk_action_new(name, label, get_xml_attribute(names,values,"tooltip"), stock);

		g_free(stock);

		// Connect standard actions
		Trace("Name: %s toggle: %s",action_name,toggle ? "Yes" : "No");
		
		if(!(g_strcasecmp(action_name,"quit") || toggle))
		{
			g_signal_connect(G_OBJECT(ret),"activate",G_CALLBACK(action_quit),state->window);
		}
		else
		{
			if(setup(ret,action_name,TRUE,names,values,error))
			{
				// Action setup failed.
				if(!*error)
					*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Invalid or unknown action: %s"), action_name);

				g_object_unref(ret);
				g_free(name);
				return NULL;
			}
		}

		gtk_action_set_accel_group(ret,state->accel_group);

		g_hash_table_insert(state->action,(gpointer) name, ret);

 	}
 	else
 	{
 		// Already created, just update
		setup(ret,action_name,FALSE,names,values,error);
		g_free(name);
 	}

	// Set common attributes
	for(f=0;f<G_N_ELEMENTS(attr);f++)
	{
		const gchar *a = get_xml_attribute(names,values,attr[f]);
		if(a)
			g_object_set_data_full(G_OBJECT(ret),attr[f],g_strdup(a),g_free);
	}

	// Set action group
	ptr =  get_xml_attribute(names,values,"group");
	if(ptr)
	{
	 	GtkActionGroup *group = NULL;

		for(f=0;action_group_name[f] && !group; f++)
		{
			if(!g_strcasecmp(ptr,action_group_name[f]))
				group = action_group[f];
		}

		if(group)
		{
			g_object_set_data(G_OBJECT(ret),"group",group);
		}
		else
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Invalid action group \"%s\" on action %s"), ptr, gtk_action_get_name(ret));
 			return NULL;
		}
	}

	return ret;

 }

 static UI_ELEMENT *create_element(size_t sz, const gchar *element_name, const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	UI_ELEMENT *ret = NULL;

	ret = g_malloc0(sz+strlen(element_name)+1);
	ret->name = ((gchar *) ret)+sz;
	strcpy((char *) ret->name,element_name);

	ret->action = create_action(element_name,names,values,state,error);

 	return ret;
 }

 static UI_ELEMENT * start_dunno(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
	return NULL;
 }

 static UI_ELEMENT * start_menubar(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	UI_ELEMENT	*el;
 	const gchar	*name = get_xml_attribute(names,values,"name");

	if(!name)
	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Can't accept unnamed menubar"));
		return NULL;
	}

 	el = (UI_ELEMENT *) g_hash_table_lookup(state->menubar,name);

 	if(!el)
 	{
 		// New menubar
		if((el = create_element(sizeof(UI_ELEMENT),name,names,values,state,error)) == NULL)
			return NULL;

		el->widget = gtk_menu_bar_new();

#ifdef MAC_INTEGRATION
		{
			const gchar *ptr = get_xml_attribute(names,values,"topmenu");
			if(ptr && !g_strcasecmp("yes",ptr))
				state->top_menu = GTK_MENU_SHELL(el->widget);
		}
#endif // MAC_INTEGRATION

		g_hash_table_insert(state->menubar,(gpointer) el->name,el);

//		Trace("Menu created: %s widget=%p",el->name,el->widget);
 	}

	return el;

 }

#ifdef MAC_INTEGRATION
 static void check_for_app_menu(const gchar *name, GtkWidget *widget, PARSER_STATE *state)
 {
	int f;

	for(f=0;f<G_N_ELEMENTS(app_menu_item);f++)
	{
		if(!g_strcasecmp(name,app_menu_item[f]))
		{
			state->app_menu[f] = GTK_MENU_ITEM(widget);
			return;
		}
	}
 }
#endif // MAC_INTEGRATION


 static UI_ELEMENT * start_menu(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	UI_ELEMENT		*el;
 	const gchar		*name = get_xml_attribute(names,values,"name");
 	GtkMenuShell	*menu;

	if(!name)
		name = get_xml_attribute(names,values,"action");

 	if(!name)
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Can't accept unnamed menu"));
		return NULL;
 	}

 	if(!state->current)
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Menu \"%s\" is invalid at this context"), name);
		return NULL;
 	}

 	if(GTK_IS_MENU_SHELL(state->current->widget))
 	{
 		menu = GTK_MENU_SHELL(state->current->widget);
 	}
 	else if(GTK_IS_MENU_ITEM(state->current->widget))
 	{
		menu = GTK_MENU_SHELL(gtk_menu_item_get_submenu(GTK_MENU_ITEM(state->current->widget)));
		if(!menu)
		{
			menu = GTK_MENU_SHELL(gtk_menu_new());
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(GTK_MENU_ITEM(state->current->widget)),GTK_WIDGET(menu));
			gtk_widget_show(GTK_WIDGET(menu));
		}
 	}
 	else
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Menu \"%s\" isn't inside a menubar or menu"), name);
		return NULL;
 	}

 	el = (UI_ELEMENT *) g_hash_table_lookup(state->menu,name);

 	if(!el)
 	{
 		// New menubar
		if((el = create_element(sizeof(UI_ELEMENT),name,names,values,state,error)) == NULL)
			return NULL;

		if(el->action)
		{
			el->widget = gtk_action_create_menu_item(el->action);
		}
		else
		{
			const gchar *label = get_xml_attribute(names,values,"label");
			el->widget = gtk_menu_item_new_with_mnemonic(gettext(label ? label : el->name));
		}

		gtk_menu_shell_append(menu,el->widget);

		g_hash_table_insert(state->menu,(gpointer) el->name,el);

#ifdef MAC_INTEGRATION
		check_for_app_menu(el->name,el->widget,state);
#else
		gtk_widget_show(el->widget);
#endif // MAC_INTEGRATION

 	}

	return el;

 }

 static UI_ELEMENT * start_menuitem(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	gchar			*temp = NULL;
 	UI_ELEMENT		*el;
 	const gchar		*name = get_xml_attribute(names,values,"name");

	if(!name)
	{
		name = get_xml_attribute(names,values,"action");

		if(!strncasecmp(name,"toggle",6))
			name = (const gchar *) (temp = g_strdup_printf("%s:%s",name,get_xml_attribute(names,values,"id")));
	}

 	if(!(state->current && GTK_IS_MENU_ITEM(state->current->widget)))
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Menuitem \"%s\" isn't inside a <menu>"), name);
	 	g_free(temp);
		return NULL;
 	}

 	el = (UI_ELEMENT *) g_hash_table_lookup(state->menu,name);

 	if(!el)
 	{
 		// New menuitem
 		GtkWidget *menu;

		if((el = create_element(sizeof(UI_ELEMENT),name,names,values,state,error)) == NULL)
		{
		 	g_free(temp);
			return NULL;
		}

		if(el->action)
		{
			el->widget = gtk_action_create_menu_item(el->action);
		}
		else
		{
			const gchar *label = get_xml_attribute(names,values,"label");
			el->widget = gtk_menu_item_new_with_mnemonic(gettext(label ? label : el->name));
		}

		menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(state->current->widget));
		if(!menu)
		{
			menu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(GTK_MENU_ITEM(state->current->widget)),menu);
		}

		gtk_menu_shell_append((GtkMenuShell *) menu, el->widget);

		g_hash_table_insert(state->menu,(gpointer) el->name,el);

#ifdef MAC_INTEGRATION
		check_for_app_menu(el->name,el->widget,state);
		if(!g_strcasecmp(el->name,"quit"))
			state->quit_menu = GTK_MENU_ITEM(el->widget);
#else
		gtk_widget_show(el->widget);
#endif // MAC_INTEGRATION

 	}

	g_free(temp);
	return el;
 }

 static UI_ELEMENT * start_separator(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
	if(!(state->current && state->current->widget))
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Separator is invalid at this context (no parent)"));
		return NULL;
 	}

	if(GTK_IS_MENU_ITEM(state->current->widget))
 	{
		// New menuitem
		GtkWidget	*menu	= gtk_menu_item_get_submenu(GTK_MENU_ITEM(state->current->widget));
		GtkWidget	*widget = gtk_separator_menu_item_new();

		if(!menu)
		{
			menu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(GTK_MENU_ITEM(state->current->widget)),menu);
		}

		gtk_menu_shell_append((GtkMenuShell *) menu, widget);
 	}
	else if(GTK_IS_TOOLBAR(state->current->widget))
 	{
		GtkWidget *widget = GTK_WIDGET(gtk_separator_tool_item_new());
		gtk_widget_show(widget);
		gtk_toolbar_insert(GTK_TOOLBAR(state->current->widget),GTK_TOOL_ITEM(widget),-1);
	}
 	else
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Separator is invalid at this context (unexpected parent)"));
 	}

	return NULL;
 }

 static UI_ELEMENT * start_toolbar(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	UI_ELEMENT		*el;
 	const gchar	*name = get_xml_attribute(names,values,"name");

	if(!name)
	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Can't accept unnamed toolbar"));
		return NULL;
	}

	if(get_xml_attribute(names,values,"action"))
	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Invalid attribute \"action\" on toolbar %s"),name);
		return NULL;
	}

 	el = (UI_ELEMENT *) g_hash_table_lookup(state->toolbar,name);

 	if(!el)
 	{
 		// New toolbar
		if((el = create_element(sizeof(UI_ELEMENT),name,names,values,state,error)) == NULL)
			return NULL;

		el->widget = gtk_toolbar_new();
		gtk_widget_show(el->widget);

		g_hash_table_insert(state->toolbar,(gpointer) el->name,el);

 	}

	return el;
 }

 static UI_ELEMENT * start_toolitem(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	gchar			*temp = NULL;
 	UI_ELEMENT		*el;
 	const gchar	*name = get_xml_attribute(names,values,"name");

	if(!name)
	{
		name = get_xml_attribute(names,values,"action");

		if(!strncasecmp(name,"toggle",6))
			name = (const gchar *) (temp = g_strdup_printf("%s:%s",name,get_xml_attribute(names,values,"id")));
	}

 	if(!(state->current && GTK_IS_TOOLBAR(state->current->widget)))
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Toolitem \"%s\" isn't inside a <toolbar>"), name);
	 	g_free(temp);
		return NULL;
 	}

 	el = (UI_ELEMENT *) g_hash_table_lookup(state->tool,name);

 	if(!el)
 	{
 		// New toolbar item
		const gchar *stock_id = get_xml_attribute(names,values,"stock-id");
		const gchar *label = get_xml_attribute(names,values,"label");

		if((el = create_element(sizeof(UI_ELEMENT),name,names,values,state,error)) == NULL)
		{
		 	g_free(temp);
			return NULL;
		}

		if(el->action)
		{
			el->widget = gtk_action_create_tool_item(el->action);
		}
		else
		{
			if(stock_id)
			{
				el->widget = GTK_WIDGET(gtk_tool_button_new_from_stock(stock_id));
				if(label)
					gtk_tool_button_set_label(GTK_TOOL_BUTTON(el->widget),label);
			}
			else
			{
				el->widget = GTK_WIDGET(gtk_tool_button_new(NULL,label));
			}

		}

		g_hash_table_insert(state->tool,(gpointer) el->name,el);

//		Trace("Tool item created: %s widget=%p",el->name,el->widget);
 	}

	gtk_toolbar_insert(GTK_TOOLBAR(state->current->widget),GTK_TOOL_ITEM(el->widget),-1);

	g_free(temp);
	return el;
 }

 static UI_ELEMENT * skip_block(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
	state->ignore++;
	return NULL;
 }

 static UI_ELEMENT * start_accelerator(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	const gchar	*name = get_xml_attribute(names,values,"name");

	if(!name)
		name = get_xml_attribute(names,values,"action");

	if(!name)
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s", _( "Can't parse unnamed accelerator"));
		return NULL;
 	}

 	if(state->current)
 	{
	 	*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Accelerator \"%s\" is invalid in this context"), name);
		return NULL;
 	}

	create_action(name, names, values, state, error);

	return NULL;
 }

 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, PARSER_STATE *state, GError **error)
 {
 	static const struct _table
 	{
 		const gchar *name;
		UI_ELEMENT * (*create)(const gchar **names,const gchar **values, PARSER_STATE *state, GError **error);
 	} table[] =
 	{
			// Menubar
			{	"menubar", 		start_menubar		},
			{	"menu", 		start_menu			},
			{	"menuitem",		start_menuitem		},

			// Toolbar
			{	"toolbar", 		start_toolbar		},
			{	"toolitem", 	start_toolitem		},

			// Misc
			{	"popup",		skip_block			},
			{	"separator",	start_separator		},
			{	"ui", 			start_dunno			},
			{ 	"accelerator",	start_accelerator	},

			// OS dependant

#if !defined( WIN32 )
			{	"windows",		skip_block			},
#else
			{	"windows", 		start_dunno			},
#endif // !WIN32

#if !defined( __APPLE__ )
			{	"apple",		skip_block			},
#else
			{	"apple", 		start_dunno			},
#endif // !WIN32

#if !defined ( linux )
			{	"linux",		skip_block			},
#else
			{	"linux", 		start_dunno			},
#endif // !linux

 	};

 	int f;

	if(state->ignore > 0)
	{
	//	Trace("Ignoring \"%s\" level=%d",element_name,state->ignore);
		state->ignore++;
		return;
	}

// 	Trace("%s: %s(%s)",__FUNCTION__,element_name,get_xml_attribute(names,values,"name"));

 	for(f=0;f<G_N_ELEMENTS(table);f++)
 	{
 		if(!g_strcasecmp(element_name,table[f].name))
 		{
 			UI_ELEMENT *el = table[f].create(names,values,state,error);
 			if(el)
 			{
				el->parent = state->current;
				state->current = el;
 			}
 			return;
 		}
 	}

 	*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Unknown element \"%s\"."),element_name);
 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, PARSER_STATE *state, GError **error)
 {
	if(state->ignore > 0)
		state->ignore--;

	if(!g_strcasecmp(element_name,"separator"))
		return;

 	if(state->current)
	{
		if(state->current->widget && state->current->name)
			gtk_widget_set_name(state->current->widget,state->current->name);
		state->current = (state->current->parent);
	}
 }

/*
 static void element_text(GMarkupParseContext *context,const gchar *text,gsize text_len, PARSER_STATE *state, GError **error)
 {

 }

 static void element_passthrough(GMarkupParseContext *context,const gchar *passthrough_text, gsize text_len, PARSER_STATE *state, GError **error)
 {

 }

 static void element_error(GMarkupParseContext *context,GError *error, PARSER_STATE *state)
 {

 }
*/

 static int parse_xml_definition(const gchar *text, PARSER_STATE *state, GError **error)
 {
	static const GMarkupParser parser =
	{
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
			element_start,

		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
			element_end,

//		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **))
//			element_text,
//		NULL,

//		(void (*)(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len,  gpointer user_data,GError **error))
//			element_passthrough,
//		NULL,

//		(void (*)(GMarkupParseContext *, GError *, gpointer))
//			element_error,
//		NULL
	};

	int rc = -1;
 	GMarkupParseContext * context;

	context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT,state,NULL);

	if(g_markup_parse_context_parse(context,text,strlen(text),error))
		rc = 0;

	g_markup_parse_context_free(context);

	return rc;
 }

 static int search_path(const gchar *path, PARSER_STATE *state)
 {
 	GError			*error	= NULL;
	GDir			*dir	= g_dir_open(path,0,&error);
	const gchar 	*name;

	if(!dir)
	{
		// Can't open ui folder
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(	GTK_WINDOW(state->window),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Can't parse ui files in %s" ), path);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't parse UI" ) );

		if(error && error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);
		return -1;
	}

	name = g_dir_read_name(dir);
	while(name)
	{
		gchar *filename = g_build_filename(path,name,NULL);

		if(g_file_test(filename,G_FILE_TEST_IS_DIR))
		{
			search_path(filename, state);
		}
		else if(g_str_has_suffix(filename,".xml"))
		{
			// Uncompressed XML file, load it.
			gchar *text	= NULL;

			if(g_file_get_contents(filename,&text,NULL,&error))
			{
				// File loaded, parse it
				if(parse_xml_definition(text, state, &error))
				{
					GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(state->window),
																GTK_DIALOG_DESTROY_WITH_PARENT,
																GTK_MESSAGE_WARNING,
																GTK_BUTTONS_OK,
																_(  "Can't parse ui in %s" ), filename);

					gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't parse UI" ) );

					if(error && error->message)
						gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

					g_error_free(error);
					error = NULL;

					gtk_dialog_run(GTK_DIALOG (dialog));
					gtk_widget_destroy(dialog);
				}
			}
			else
			{
				// Can't load xml file
				GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(state->window),
															GTK_DIALOG_DESTROY_WITH_PARENT,
															GTK_MESSAGE_WARNING,
															GTK_BUTTONS_OK,
															_(  "Can't load %s" ), filename);

				gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't parse UI" ) );

				if(error && error->message)
					gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

				g_error_free(error);
				error = NULL;

				gtk_dialog_run(GTK_DIALOG (dialog));
				gtk_widget_destroy(dialog);
			}

			g_free(text);

		}
		// TODO (perry#4#): Uncompress and load .xml.gz files.

		g_free(filename);
		name = g_dir_read_name(dir);
	}

	g_dir_close(dir);
	return 0;
 }

 static void pack_menubar(gpointer key,UI_ELEMENT *el, GtkWidget *box)
 {
	Trace("Packing menubar \"%s\" - %p",el->name, el->widget);
	gtk_widget_show_all(el->widget);
	gtk_box_pack_start(GTK_BOX(box),el->widget,FALSE,FALSE,0);
 }

 static void pack_toolbar(gpointer key,UI_ELEMENT *el, GtkWidget *box)
 {
	Trace("Packing toolbar \"%s\"",el->name);
	gtk_box_pack_start(GTK_BOX(box),el->widget,FALSE,FALSE,0);
 }

 static void sync_action_state(gpointer key, GtkAction *action, PARSER_STATE state)
 {
#if GTK_CHECK_VERSION(2,16,0)
	GSList			*child	= gtk_action_get_proxies(action);
#endif

	GtkActionGroup	*group	= (GtkActionGroup *) g_object_get_data(G_OBJECT(action),"group");

	gtk_action_group_add_action_with_accel(group ? group : action_group[0], action, g_object_get_data(G_OBJECT(action),"key"));
	gtk_action_connect_accelerator(action);

	// Update proxy widgets
#if GTK_CHECK_VERSION(2,16,0)
	while(child)
	{
		gtk_activatable_sync_action_properties(GTK_ACTIVATABLE(child->data),action);
		child = child->next;
	}
#endif
 }

/*---[ External Call ]------------------------------------------------------------------------------------*/

 GtkWidget * create_window_from_ui_files(const gchar *path, GtkWidget *app_widget)
 {
	PARSER_STATE 	  state;
	GtkWidget		* hbox;
	GtkWidget		* vbox;

	// Initialize control data
	memset(&state,0,sizeof(state));

	state.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	state.accel_group = gtk_accel_group_new();

	state.menubar = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) 0, g_free);
	state.toolbar = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) 0, g_free);
	state.tool	  = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) 0, g_free);
	state.menu	  = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) 0, g_free);

	state.action  = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free, g_object_unref);

	// Parse
	search_path(path, &state);

	// Create containers
	vbox = gtk_vbox_new(FALSE,0);
	hbox = gtk_hbox_new(FALSE,0);
	gtk_widget_show(hbox);
	gtk_widget_show(vbox);

	// Menubar(s)
	g_hash_table_foreach(state.menubar,(GHFunc) pack_menubar, vbox);

	// Top toolbars & keypads
	g_hash_table_foreach(state.toolbar,(GHFunc) pack_toolbar, vbox);

	// Left toolbars & keypads

	// Center widget
	gtk_widget_show(app_widget);
	gtk_box_pack_start(GTK_BOX(hbox),app_widget,TRUE,TRUE,0);

	// Right toolbars & keypads


	// Pack left/center/right widget
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	// Bottom toolbars & keypads


	// Put widgets on mainwindow
	gtk_container_add(GTK_CONTAINER(state.window),vbox);

	// Update action group & accelerators
	g_hash_table_foreach(state.action,(GHFunc) sync_action_state, &state);


#ifdef MAC_INTEGRATION
	if(state.top_menu)
	{
		// Set application menu
		int f;

		Trace("%s: Top menu is %p",__FUNCTION__,state.top_menu);
		gtk_widget_hide(GTK_WIDGET(state.top_menu));
		gtk_osxapplication_set_menu_bar(osxapp,state.top_menu);

		for(f=0;f<G_N_ELEMENTS(app_menu_item);f++)
		{
			if(state.app_menu[f])
			{
				GtkOSXApplicationMenuGroup *grp = gtk_osxapplication_add_app_menu_group(osxapp);
				gtk_osxapplication_add_app_menu_item(osxapp,grp,state.app_menu[f]);
			}
		}

		if(state.quit_menu)
			gtk_widget_hide(GTK_WIDGET(state.quit_menu));

	}
#endif // MAC_INTEGRATION


	// Set accelerators
	Trace("%s: Setting accelerators",__FUNCTION__);
	gtk_window_add_accel_group(GTK_WINDOW(state.window),state.accel_group);

	// Release control data
	Trace("Releasing %p",state.accel_group);
	g_object_unref(state.accel_group);

	Trace("Releasing %p",state.menubar);
	g_hash_table_unref(state.menubar);

	Trace("Releasing %p",state.toolbar);
	g_hash_table_unref(state.toolbar);

	Trace("Releasing %p",state.tool);
	g_hash_table_unref(state.tool);

	Trace("Releasing %p",state.menu);
	g_hash_table_unref(state.menu);

	Trace("Releasing %p",state.action);
	g_hash_table_unref(state.action);

	Trace("Window: %p",state.window);

	return state.window;
 }


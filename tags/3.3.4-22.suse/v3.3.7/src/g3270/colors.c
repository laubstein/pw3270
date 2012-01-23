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

 #include "config.h"
 #include "g3270.h"
 #include <ctype.h>
 #include <string.h>

/*---[ Statics ]------------------------------------------------------------------------------------------------*/

#define ALTERNATIVE_COLORS 1

#ifdef ALTERNATIVE_COLORS

 	static const char *DefaultColors =	"#000000,"			// TERMINAL_COLOR_BACKGROUND
											"#7890F0,"			// TERMINAL_COLOR_BLUE
											"#FF0000,"			// TERMINAL_COLOR_RED
											"#FF00FF,"			// TERMINAL_COLOR_PINK
											"#00FF00,"			// TERMINAL_COLOR_GREEN
											"#00FFFF,"			// TERMINAL_COLOR_TURQUOISE
											"#FFFF00,"			// TERMINAL_COLOR_YELLOW
											"#FFFFFF,"			// TERMINAL_COLOR_WHITE
											"#000000,"			// TERMINAL_COLOR_BLACK
											"#000080,"			// TERMINAL_COLOR_DARK_BLUE
											"#FFA200,"			// TERMINAL_COLOR_ORANGE
											"#800080,"			// TERMINAL_COLOR_PURPLE
											"#008000,"			// TERMINAL_COLOR_DARK_GREEN
											"#008080,"			// TERMINAL_COLOR_DARK_TURQUOISE
											"#A0A000,"			// TERMINAL_COLOR_MUSTARD
											"#C0C0C0,"			// TERMINAL_COLOR_GRAY

											"#00FF00,"			// TERMINAL_COLOR_FIELD_DEFAULT
											"#FF0000,"			// TERMINAL_COLOR_FIELD_INTENSIFIED
											"#00FFFF,"			// TERMINAL_COLOR_FIELD_PROTECTED
											"#FFFFFF,"			// TERMINAL_COLOR_FIELD_PROTECTED_INTENSIFIED

											"#FFFFFF,"			// TERMINAL_COLOR_SELECTED_BG
											"#000000,"			// TERMINAL_COLOR_SELECTED_FG,

											"#00FF00," 			// TERMINAL_COLOR_CURSOR
											"#00FF00," 			// TERMINAL_COLOR_CROSS_HAIR

											"#000000,"	 		// TERMINAL_COLOR_OIA_BACKGROUND
											"#00FF00,"			// TERMINAL_COLOR_OIA
											"#7890F0,"			// TERMINAL_COLOR_OIA_SEPARATOR
											"#FFFFFF,"			// TERMINAL_COLOR_OIA_STATUS_OK
											"#FF0000";			// TERMINAL_COLOR_OIA_STATUS_INVALID


#else

 	static const char *DefaultColors =	"black,"			// TERMINAL_COLOR_BACKGROUND
											"blue,"				// TERMINAL_COLOR_BLUE
											"red,"				// TERMINAL_COLOR_RED
											"pink,"				// TERMINAL_COLOR_PINK
											"green,"			// TERMINAL_COLOR_GREEN
											"turquoise,"		// TERMINAL_COLOR_TURQUOISE
											"yellow,"			// TERMINAL_COLOR_YELLOW
											"white,"			// TERMINAL_COLOR_WHITE
											"black,"			// TERMINAL_COLOR_BLACK
											"DeepSkyBlue,"		// TERMINAL_COLOR_DARK_BLUE
											"orange,"			// TERMINAL_COLOR_ORANGE
											"DeepSkyBlue,"		// TERMINAL_COLOR_PURPLE
											"PaleGreen,"		// TERMINAL_COLOR_DARK_GREEN
											"PaleTurquoise,"	// TERMINAL_COLOR_DARK_TURQUOISE
											"grey,"				// TERMINAL_COLOR_MUSTARD
											"white,"			// TERMINAL_COLOR_GRAY

											"green1,"			// TERMINAL_COLOR_FIELD_DEFAULT
											"red,"				// TERMINAL_COLOR_FIELD_INTENSIFIED
											"DeepSkyBlue,"		// TERMINAL_COLOR_FIELD_PROTECTED
											"white,"			// TERMINAL_COLOR_FIELD_PROTECTED_INTENSIFIED

											"white,"			// TERMINAL_COLOR_SELECTED_BG
											"black,"			// TERMINAL_COLOR_SELECTED_FG,

											"LimeGreen," 		// TERMINAL_COLOR_CURSOR
											"LimeGreen," 		// TERMINAL_COLOR_CROSS_HAIR

											"black,"	 		// TERMINAL_COLOR_OIA_BACKGROUND
											"LimeGreen,"		// TERMINAL_COLOR_OIA
											"#7890F0,"			// TERMINAL_COLOR_OIA_SEPARATOR
											"white,"			// TERMINAL_COLOR_OIA_STATUS_OK
											"red";				// TERMINAL_COLOR_OIA_STATUS_INVALID

#endif


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 int SaveColors(void)
 {
	int 	f;
	gchar	clr[4096];
	gchar	*ptr;

 	for(f=0;f < TERMINAL_COLOR_COUNT;f++)
 	{
 		if(f > 0)
			g_strlcat(clr,",",4095);

		ptr = gdk_color_to_string(color+f);
		g_strlcat(clr,ptr,4095);
		g_free(ptr);

 	}

	SetString("Terminal","Colors",clr);

 	return 0;
 }

 int LoadColors(void)
 {

 	int 	f;
 	char	*buffer	= GetString("Terminal","Colors",DefaultColors);
 	char	*ptr	= strtok(buffer,",");

 	for(f=0;ptr && f < TERMINAL_COLOR_COUNT;f++)
 	{
 		if(ptr)
 		{
			gdk_color_parse(ptr,color+f);
			ptr = strtok(NULL,",");
 		}
		else
		{
			gdk_color_parse("LimeGreen",color+f);
		}
		gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),color+f,TRUE,TRUE);
 	}

 	g_free(buffer);

 	return 0;
 }

 static void color_changed(GtkColorSelection *widget, gpointer *user_data)
 {
 	int id = (int) g_object_get_data(G_OBJECT(widget),"selected");

	Trace("Color(%d) changed",id);

	if(id < 0 || id >= TERMINAL_COLOR_COUNT)
		return;

	gtk_color_selection_get_current_color(widget,color+id);
	gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),color+id,TRUE,TRUE);

	// Redraw screen
	action_Redraw();

 }

 static void row_activated(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *column, GtkWidget *widget)
 {
	GtkTreeIter		iter;
	GtkTreeModel	*model	= gtk_tree_view_get_model(view);
	GValue			value	= { 0, };
	int				id;

	if(!gtk_tree_model_get_iter(model,&iter,path))
	{
		gtk_widget_set_sensitive(widget,0);
		return;
	}

	gtk_tree_model_get_value(model,&iter,1,&value);

	id = g_value_get_int(&value);
	g_object_set_data(G_OBJECT(widget),"selected",(gpointer) id);

	if(id < 0 || id >= TERMINAL_COLOR_COUNT)
	{
		gtk_widget_set_sensitive(widget,0);
		return;
	}

	gtk_color_selection_set_previous_color(GTK_COLOR_SELECTION(widget),color+id);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(widget),color+id);

	gtk_widget_set_sensitive(widget,1);

 }

 void action_SelectColors(void)
 {
 	static const struct _node
 	{
 		int			id;
 		const char	*text;
 	} node[] =
 	{
 		{ TERMINAL_COLOR_BACKGROUND,		N_( "Terminal" )			},
 		{ TERMINAL_COLOR_FIELD_DEFAULT,		N_( "Base attributes" )		},
 		{ TERMINAL_COLOR_SELECTED_BG,		N_( "Selected text" )		},
 		{ TERMINAL_COLOR_CURSOR,			N_( "Cursor" )				},
 		{ TERMINAL_COLOR_OIA_BACKGROUND,	N_( "OIA" )					}

 	};

	static const gchar *color_name[TERMINAL_COLOR_COUNT] =
	{
		N_( "Background" ),					// TERMINAL_COLOR_BACKGROUND
		N_( "Blue" ),						// TERMINAL_COLOR_BLUE
		N_( "Red" ),						// TERMINAL_COLOR_RED
		N_( "Pink" ),						// TERMINAL_COLOR_PINK
		N_( "Green" ),						// TERMINAL_COLOR_GREEN
		N_( "Turquoise" ),					// TERMINAL_COLOR_TURQUOISE
		N_( "Yellow" ),						// TERMINAL_COLOR_YELLOW
		N_( "White" ),						// TERMINAL_COLOR_WHITE
		N_( "Black" ),						// TERMINAL_COLOR_BLACK
		N_( "Dark Blue" ),					// TERMINAL_COLOR_DARK_BLUE
		N_( "Orange" ),						// TERMINAL_COLOR_ORANGE
		N_( "Purple" ),						// TERMINAL_COLOR_PURPLE
		N_( "Dark Green" ),					// TERMINAL_COLOR_DARK_GREEN
		N_( "Turquoise" ),					// TERMINAL_COLOR_DARK_TURQUOISE
		N_( "Mustard" ),					// TERMINAL_COLOR_MUSTARD
		N_( "Gray" ),						// TERMINAL_COLOR_GRAY

		N_( "Normal/Unprotected" ),			// TERMINAL_COLOR_FIELD_DEFAULT
		N_( "Intensified/Unprotected" ),	// TERMINAL_COLOR_FIELD_INTENSIFIED
		N_( "Normal/Protected" ),			// TERMINAL_COLOR_FIELD_PROTECTED
		N_( "Intensified/Protected" ),		// TERMINAL_COLOR_FIELD_PROTECTED_INTENSIFIED

		N_( "Background" ),					// TERMINAL_COLOR_SELECTED_BG
		N_( "Foreground" ),					// TERMINAL_COLOR_SELECTED_FG

		N_( "Normal" ),						// TERMINAL_COLOR_CURSOR
		N_( "Cross-hair" ),					// TERMINAL_COLOR_CROSS_HAIR

		// Oia Colors
		N_( "Background" ),					// TERMINAL_COLOR_OIA_BACKGROUND
		N_( "Foreground" ),					// TERMINAL_COLOR_OIA
		N_( "Separator" ),					// TERMINAL_COLOR_OIA_SEPARATOR
		N_( "Normal status" ),				// TERMINAL_COLOR_OIA_STATUS_OK
		N_( "Locked status" ),				// TERMINAL_COLOR_OIA_STATUS_INVALID

	};

	GtkWidget		*dialog;
	GtkWidget		*widget;
	GtkWidget		*color;
	GtkWidget		*box;
	GtkTreeModel	*model;
	GtkWidget		*frame;
	GtkTreeIter		iter;
	GtkTreeIter		parent;
	int				title = 0;
	int				f;

	dialog = gtk_dialog_new_with_buttons (	_( "Color setup" ),
											GTK_WINDOW(topwindow),
											GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
											_( "Restore defaults" ),	1,
											GTK_STOCK_OK,     			GTK_RESPONSE_ACCEPT,
											GTK_STOCK_CANCEL, 			GTK_RESPONSE_REJECT,
											NULL);

	box = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(box),GetInt("ColorSetup","PanedPosition",120));

	// Color selection box
	color = gtk_color_selection_new();
	g_object_set_data(G_OBJECT(color),"selected",(gpointer) -1);
	g_signal_connect(G_OBJECT(color), "color-changed", G_CALLBACK(color_changed), 0);

	gtk_widget_set_sensitive(color,0);
	gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(color),FALSE);
	gtk_paned_add2(GTK_PANED(box),color);

	// Color List box
 	model = (GtkTreeModel *) gtk_tree_store_new(2,G_TYPE_STRING,G_TYPE_INT);

	widget = gtk_tree_view_new_with_model(model);
	g_signal_connect(G_OBJECT(widget), "row-activated", G_CALLBACK(row_activated), (gpointer) color);

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(widget),FALSE);
	gtk_tree_view_insert_column_with_attributes(	GTK_TREE_VIEW(widget),
													-1,
													"Color",gtk_cell_renderer_text_new(),"text",
													0, NULL );

	for(f=0;f<TERMINAL_COLOR_COUNT;f++)
	{
		if(f == node[title].id)
		{
			gtk_tree_store_append((GtkTreeStore *) model,&parent,NULL);
			gtk_tree_store_set((GtkTreeStore *) model, &parent, 0, gettext(node[title].text), 1, TERMINAL_COLOR_COUNT, -1);
			title++;
		}
		gtk_tree_store_append((GtkTreeStore *) model,&iter,&parent);
		gtk_tree_store_set((GtkTreeStore *) model, &iter, 0, gettext(color_name[f]), 1, f, -1);
	}

	frame = gtk_scrolled_window_new(NULL,NULL);
	gtk_container_add(GTK_CONTAINER(frame),widget);
	gtk_paned_add1(GTK_PANED(box),frame);

	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),box);
	gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(dialog)->vbox));

	// Run dialog
	RestoreWindowSize("ColorSetup", dialog);

	switch(gtk_dialog_run(GTK_DIALOG(dialog)))
	{
	case 1:					// Restore default colors
		SetString("Terminal","Colors",NULL);
		LoadColors();
		break;

	case GTK_RESPONSE_ACCEPT:	// Save selected colors
		SaveColors();
		break;

	case GTK_RESPONSE_REJECT:	// Reload colors from configuration file
		LoadColors();
		break;
	}

	SaveWindowSize("ColorSetup",dialog);
	SetInt("ColorSetup","PanedPosition",gtk_paned_get_position(GTK_PANED(box)));

	gtk_widget_destroy(dialog);
	action_Redraw();

 }



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
 * Este programa está nomeado como colors.c e possui 574 linhas de código.
 * 
 * Contatos: 
 * 
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */


 #include <lib3270/config.h>
 #include "g3270.h"
 #include <ctype.h>
 #include <string.h>

/*---[ Statics ]------------------------------------------------------------------------------------------------*/

 	static const struct _color_profile
 	{
 		const gchar *name;
 		const gchar *colors;
	}
	color_profile[] =
	{
		{	N_( "Default Color set 1" ),	"#000000,"			// TERMINAL_COLOR_BACKGROUND
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

											"#404040,"			// TERMINAL_COLOR_SELECTED_BG
											"#FFFFFF,"			// TERMINAL_COLOR_SELECTED_FG,
											"#FFFF00,"			// TERMINAL_COLOR_SELECTED_BORDER

											"#00FF00," 			// TERMINAL_COLOR_CURSOR
											"#00FF00," 			// TERMINAL_COLOR_CROSS_HAIR

											"#000000,"	 		// TERMINAL_COLOR_OIA_BACKGROUND
											"#00FF00,"			// TERMINAL_COLOR_OIA
											"#7890F0,"			// TERMINAL_COLOR_OIA_SEPARATOR
											"#FFFFFF,"			// TERMINAL_COLOR_OIA_STATUS_OK
											"#FF0000"			// TERMINAL_COLOR_OIA_STATUS_INVALID
		},
		{	N_( "Default Color Set 2" ),	"black,"			// TERMINAL_COLOR_BACKGROUND
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

											"DimGray,"			// TERMINAL_COLOR_SELECTED_BG
											"white,"			// TERMINAL_COLOR_SELECTED_FG,
											"yellow,"			// TERMINAL_COLOR_SELECTED_BORDER


											"LimeGreen," 		// TERMINAL_COLOR_CURSOR
											"LimeGreen," 		// TERMINAL_COLOR_CROSS_HAIR

											"black,"	 		// TERMINAL_COLOR_OIA_BACKGROUND
											"LimeGreen,"		// TERMINAL_COLOR_OIA
											"#7890F0,"			// TERMINAL_COLOR_OIA_SEPARATOR
											"white,"			// TERMINAL_COLOR_OIA_STATUS_OK
											"red"				// TERMINAL_COLOR_OIA_STATUS_INVALID
		}

	};


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static int SaveColors(GtkComboBox *combo)
 {
	int 			f;
	gchar			clr[4096];
	GtkTreeIter 	iter;

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
	gchar			*ptr;
#else
	int				sz;
#endif

	*clr = 0;

 	for(f=0;f < TERMINAL_COLOR_COUNT;f++)
 	{
 		if(f > 0)
			g_strlcat(clr,",",4095);

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION >= 12
		ptr = gdk_color_to_string(color+f);
		g_strlcat(clr,ptr,4095);
		g_free(ptr);
#else
		sz = strlen(clr);
		g_snprintf(clr+sz,4094-sz,"#%04x%04x%04x",(color+f)->red,(color+f)->green,(color+f)->blue);
#endif

 	}

	SetString("Terminal","Colors",clr);

 	if(gtk_combo_box_get_active_iter(combo,&iter))
 	{
		GValue		value	= { 0, };

		gtk_tree_model_get_value(gtk_combo_box_get_model(combo),&iter,1,&value);
		SetString("Terminal","ColorScheme",g_value_get_string(&value));
 	}

 	return 0;
 }


 static void parsecolorentry(int id, const gchar *str)
 {
	gdk_color_parse(str,color+id);
	gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),color+id,TRUE,TRUE);
 }

 static void ParseColorList(char *colors)
 {
 	int		f;
 	gchar	**clr = g_strsplit(colors,",",TERMINAL_COLOR_COUNT+1);

	for(f=0;clr[f] && f < TERMINAL_COLOR_COUNT;f++);
	Trace("%d colors in %s (max=%d)",f,colors,TERMINAL_COLOR_COUNT);

	switch(f)
	{
	case 29: // Release 1 colors
		for(f=0;f < TERMINAL_COLOR_SELECTED_BORDER;f++)
			parsecolorentry(f,clr[f]);

		parsecolorentry(TERMINAL_COLOR_SELECTED_BORDER,clr[TERMINAL_COLOR_SELECTED_BG]);

		for(f=TERMINAL_COLOR_SELECTED_BORDER+1;f < TERMINAL_COLOR_COUNT;f++)
			parsecolorentry(f,clr[f-1]);

		break;

	case TERMINAL_COLOR_COUNT:	// Complete string
		for(f=0;f < TERMINAL_COLOR_COUNT;f++)
			parsecolorentry(f,clr[f]);
		break;

	default: // Unexpected, parse only standard base colors

		Trace("Unexpected color count in %s, loading base colors",colors);

		for(f=0;f < TERMINAL_COLOR_FIELD_DEFAULT;f++)
			parsecolorentry(f,clr[f]);

		parsecolorentry(TERMINAL_COLOR_FIELD_DEFAULT,				clr[TERMINAL_COLOR_GREEN]);
		parsecolorentry(TERMINAL_COLOR_FIELD_INTENSIFIED,			clr[TERMINAL_COLOR_RED]);
		parsecolorentry(TERMINAL_COLOR_FIELD_PROTECTED,				clr[TERMINAL_COLOR_DARK_BLUE]);
		parsecolorentry(TERMINAL_COLOR_FIELD_PROTECTED_INTENSIFIED,	clr[TERMINAL_COLOR_WHITE]);

		parsecolorentry(TERMINAL_COLOR_SELECTED_BG,					clr[TERMINAL_COLOR_WHITE]);
		parsecolorentry(TERMINAL_COLOR_SELECTED_FG,					clr[TERMINAL_COLOR_BLACK]);

		for(f=TERMINAL_COLOR_CURSOR;f < TERMINAL_COLOR_COUNT;f++)
			parsecolorentry(f,clr[TERMINAL_COLOR_GREEN]);

	}

	g_strfreev(clr);

/*

 	int 	f		= 0;
 	char	buffer	= g_strdup(str);
 	char	*ptr;


 	for(ptr = strtok(buffer,",");ptr;ptr =
		f++;

	strcpy(buffer,str);


	if(f == TERMINAL_COLOR_COUNT)
	{
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
	}

	g_free(buffer);
*/

 }

 int LoadColors(void)
 {
 	char	*buffer	= GetString("Terminal","Colors",color_profile[0].colors);
 	ParseColorList(buffer);
 	g_free(buffer);
 	return 0;
 }

 static void color_changed(GtkColorSelection *widget, gpointer *user_data)
 {
 	int			id		= (int) g_object_get_data(G_OBJECT(widget),"selected");
 	GtkComboBox	*combo	= (GtkComboBox *) g_object_get_data(G_OBJECT(widget),"combo");

	Trace("Color(%d) changed",id);

	if(id < 0 || id >= TERMINAL_COLOR_COUNT)
		return;

    LoadImages(terminal->window, terminal->style->fg_gc[GTK_WIDGET_STATE(terminal)]);

	gtk_color_selection_get_current_color(widget,color+id);
	gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),color+id,TRUE,TRUE);

	if(gtk_combo_box_get_active(combo))
	{
		gtk_combo_box_set_active(combo,0);
	}
	else
	{
		ReloadPixmaps();
		action_Redraw();
	}

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

 static void activate_scheme(GtkComboBox *widget, gpointer user_data)
 {
	GValue			value	= { 0, };
 	GtkTreeIter 	iter;
 	const gchar	*vlr;
 	char			*ptr;

 	if(!gtk_combo_box_get_active_iter(widget,&iter))
			return;

#ifdef DEBUG
	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);
	Trace("Color scheme changed to %s",g_value_get_string(&value));
#endif

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,2,&value);
	vlr = g_value_get_string(&value);
	if(vlr)
	{
		Trace("Mudando cores para %s",vlr);
		ptr = g_strdup(vlr);
		ParseColorList(ptr);
		g_free(ptr);
		ReloadPixmaps();
		action_Redraw();
	}
 }

 void action_SelectColors(void)
 {
 	static const gchar *custom = N_( "Custom colors" );

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
		N_( "Border" ),						// TERMINAL_COLOR_SELECTED_FG

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
	GtkWidget		*combo;
	GtkWidget		*color;
	GtkWidget		*box;
	GtkTreeModel	*model;
	GtkWidget		*frame;
	GtkTreeIter		iter;
	GtkTreeIter		parent;
	GtkCellRenderer *rend;
	gchar			*file;
	const gchar	*scheme	= GetString("Terminal","ColorScheme",color_profile->name);
	int				title 	= 0;
	int				f;

	dialog = gtk_dialog_new_with_buttons (	_( "Color setup" ),
											GTK_WINDOW(topwindow),
											GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
											NULL );

	gtk_window_set_icon_name(GTK_WINDOW(dialog),GTK_STOCK_SELECT_COLOR);

	box = gtk_hpaned_new();
	gtk_paned_set_position(GTK_PANED(box),GetInt("ColorSetup","PanedPosition",120));

	// Buttons
 	model = (GtkTreeModel *) gtk_list_store_new(3,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);

	combo = gtk_combo_box_new_with_model(model);

	rend = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), rend, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), rend, "text", 0, NULL);

	gtk_list_store_append((GtkListStore *) model,&iter);
	gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(custom), 1, custom, 2, NULL, -1);
	parent = iter;

 	for(f=0;f<G_N_ELEMENTS(color_profile);f++)
 	{
		gtk_list_store_append((GtkListStore *) model,&iter);
		gtk_list_store_set((GtkListStore *) model, &iter,	0, gettext(color_profile[f].name),
															1, color_profile[f].name,
															2, color_profile[f].colors,
															-1 );

		if(!strcmp(scheme,color_profile[f].name))
			parent = iter;
 	}

 	file = FindSystemConfigFile("colors.conf");
 	if(file)
 	{
		gchar 		**group;
		GKeyFile	*conf = g_key_file_new();

		g_key_file_load_from_file(conf,file,G_KEY_FILE_NONE,NULL);
 		g_free(file);

		group = g_key_file_get_groups(conf,NULL);

		for(f=0;group[f];f++)
		{
			gchar *str = g_strjoin( ",",	g_key_file_get_string(conf,group[f],"Terminal",NULL),
											g_key_file_get_string(conf,group[f],"BaseAttributes",NULL),
											g_key_file_get_string(conf,group[f],"SelectedText",NULL),
											g_key_file_get_string(conf,group[f],"Cursor",NULL),
											g_key_file_get_string(conf,group[f],"OIA",NULL),
											NULL
								);

			Trace("Colors(%s): \"%s\"",group[f],str);

			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter,	0, g_key_file_get_locale_string(conf,group[f],"Label",NULL,NULL),
																1, group[f],
																2, str,
																-1 );
			if(!strcmp(scheme,group[f]))
				parent = iter;

			g_free(str);
		}

		g_strfreev(group);
		g_key_file_free(conf);

 	}

	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(activate_scheme),0);
	gtk_combo_box_set_active_iter(GTK_COMBO_BOX(combo),&parent);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),combo,FALSE,FALSE,0);

	gtk_dialog_add_action_widget(GTK_DIALOG(dialog),gtk_button_new_from_stock(GTK_STOCK_OK),GTK_RESPONSE_ACCEPT);
	gtk_dialog_add_action_widget(GTK_DIALOG(dialog),gtk_button_new_from_stock(GTK_STOCK_CANCEL),GTK_RESPONSE_REJECT);

	// Color selection box
	color = gtk_color_selection_new();
	g_object_set_data(G_OBJECT(color),"selected",(gpointer) -1);
	g_signal_connect(G_OBJECT(color), "color-changed", G_CALLBACK(color_changed), 0);

	g_object_set_data(G_OBJECT(color),"combo",(gpointer) combo);

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
	case GTK_RESPONSE_ACCEPT:	// Save selected colors
		SaveColors(GTK_COMBO_BOX(combo));
		break;

	case GTK_RESPONSE_REJECT:	// Reload colors from configuration file
		LoadColors();
		break;

	}

	SaveWindowSize("ColorSetup",dialog);
	SetInt("ColorSetup","PanedPosition",gtk_paned_get_position(GTK_PANED(box)));

	gtk_widget_destroy(dialog);
	ReloadPixmaps();
	action_Redraw();

 }




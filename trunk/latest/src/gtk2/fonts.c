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
 * Este programa está nomeado como fonts.c e possui - linhas de código.
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
#include "oia.h"
#include "fonts.h"
#include "actions.h"

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 struct _font_list
 {
 	double 		size;
 	gint			ascent;
 	gint			descent;
 	gint			width;
 	gint			height;
	cairo_matrix_t	matrix;
 } *font_list = NULL;

 double *font_size = NULL;

 PW3270_FONT_INFO terminal_font_info = { 0 };

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

/**
 * Update font & coordinates for screen elements.
 *
 * @param info	Font description data.
 * @param width	Widget width.
 * @param height	Widget height.
 *
 * @return TRUE if the image needs update
 */
 gboolean update_terminal_font_size(gint width, gint height)
 {
 	int size = -1;
 	int f;

 	for(f=0;font_list[f].size;f++)
 	{
		if( ((font_list[f].height*(screen->rows+1))+2) < height && (font_list[f].width*screen->cols) < width )
			size = f;
 	}

 	if(size < 0)
		size = 0;

//	terminal_font_info.size		= font_list[size].size;
	terminal_font_info.matrix	= &font_list[size].matrix;
	terminal_font_info.descent	= font_list[size].descent;
	terminal_font_info.ascent	= font_list[size].ascent;

	if(terminal_font_info.width != font_list[size].width || terminal_font_info.height != font_list[size].height)
	{
		terminal_font_info.width	= font_list[size].width;

		terminal_font_info.height 	=
		terminal_font_info.spacing = font_list[size].height;

		Trace("Font size changed to %dx%d",terminal_font_info.width,terminal_font_info.height);

		if(terminal_font_info.font)
		{
			cairo_scaled_font_destroy(terminal_font_info.font);
			terminal_font_info.font = NULL;
		}

		oia_release_pixmaps();

	}

	// Adjust line spacing

	terminal_font_info.spacing = height / (view.rows+2);

//	Trace("Spacing: %d  height: %d",terminal_font_info.spacing, terminal_font_info.height);

	if(terminal_font_info.spacing < terminal_font_info.height)
		terminal_font_info.spacing = terminal_font_info.height;

	// Center image
	view.left = (width >> 1) - ((view.cols * terminal_font_info.width) >> 1);
	if(view.left < 0)
		view.left = 0;

	view.top = (height >> 1) - (((view.rows+1) * terminal_font_info.spacing) >> 1);
	if(view.top < 0)
		view.top = 0;

	return TRUE;
 }

/**
 * Release font info.
 *
 */
 void release_font_info(PW3270_FONT_INFO *info)
 {
	if(info->face)
	{
		cairo_font_face_destroy(info->face);
		info->face = NULL;
	}

	if(info->font)
	{
		cairo_scaled_font_destroy(info->font);
		info->font = NULL;
	}

 }

/**
 * Load terminal font info
 *
 */
 void update_font_info(cairo_t *cr, const gchar *fontname, PW3270_FONT_INFO *info)
 {
 	int		f;
 	int		pos = 0;
    double width = -1;
    double height = -1;

	release_font_info(info);

	info->face = cairo_toy_font_face_create(fontname,CAIRO_FONT_SLANT_NORMAL,TOGGLED_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);

	/* Load font sizes */
	cairo_set_font_face(cr,info->face);

 	for(f=0;font_size[f];f++)
 	{
		cairo_font_extents_t sz;

		cairo_set_font_size(cr,font_size[f]);
		cairo_font_extents(cr,&sz);

		if(width != sz.max_x_advance || height != sz.height)
		{
			// Font size changed

			cairo_get_font_matrix(cr,&font_list[pos].matrix);

			font_list[pos].size 	= font_size[f];
			font_list[pos].width  	= (int) sz.max_x_advance;
			font_list[pos].height 	= (int) sz.height;
			font_list[pos].ascent 	= (int) sz.ascent;
			font_list[pos].descent	= (int) sz.descent;

/*
			Trace("%s size=%d xx=%d yx=%d xy=%d yy=%d x0=%d y0=%d w=%d h=%d+%d",
										fontname,
										(int) font_list[pos].size ,
										(int) font_list[pos].matrix.xx ,
										(int) font_list[pos].matrix.yx ,
										(int) font_list[pos].matrix.xy ,
										(int) font_list[pos].matrix.yy ,
										(int) font_list[pos].matrix.x0 ,
										(int) font_list[pos].matrix.y0,
										(int) font_list[pos].width,
										(int) font_list[pos].ascent,
										(int) font_list[pos].descent);
*/

			width 	= sz.max_x_advance;
			height	= sz.height;

			pos = pos+1;
		}
 	}

	memset(font_list+pos,0,sizeof(struct _font_list));

	info->width		= font_list->width;

	info->spacing 	=
	info->height	= font_list->height;

	info->descent	= font_list->descent;
	info->ascent	= font_list->ascent;
//	info->size		= font_list->size;
	info->matrix	= &font_list->matrix;

 	Trace("Minimum terminal size is %dx%d",screen->cols*font_list->width, (screen->rows+1)*font_list->height);

 }


/**
 * Load font sizes.
 *
 */
 void load_font_sizes(void)
 {
 	static const gchar *default_sizes = "6,7,8,9,10,11,12,13,14,16,18,20,22,24,26,28,32,36,40,48,56,64,72";

 	gchar 				*list = GetString("Terminal","FontSizes",default_sizes);
 	gchar 				**vlr;
 	gint				sz;
 	int					f;

	Trace("%s",__FUNCTION__);

	memset(&terminal_font_info,0,sizeof(terminal_font_info));

	vlr = g_strsplit(*list == '*' ? default_sizes : list,",",-1);

	sz = g_strv_length(vlr);

	font_size = g_malloc0((sz+1) * sizeof(double));
	font_list = g_malloc0((sz+1) * sizeof(struct _font_list));

	for(f=0;f<sz;f++)
		font_size[f] = g_ascii_strtod(vlr[f],NULL);

	g_strfreev(vlr);
	g_free(list);

 }

 static void activate_font(GtkCheckMenuItem *item, gchar *text)
 {
	if(gtk_check_menu_item_get_active(item))
	{
		gchar *vlr	= strdup(text);
		char *ptr	= strchr(vlr,',');

		if(ptr)
		{
			*(ptr++) = 0;
			SetString("Terminal","FontSizes",ptr);
		}
		else
		{
			SetString("Terminal","FontSizes",NULL);
		}

		SetString("Terminal","Font",vlr);

		g_free(vlr);

		vlr = g_object_get_data(G_OBJECT(item),"bold");

		if(vlr)
			gui_toogle_set_active(GUI_TOGGLE_BOLD,((g_strcasecmp(vlr,"yes") == 0) ? TRUE : FALSE));

		action_redraw(0);
	}
 }

 static void load_system_monospaced_fonts(GtkWidget *topmenu, GtkWidget *menu, const gchar *selected)
 {
	// Stolen from http://svn.gnome.org/svn/gtk+/trunk/gtk/gtkfontsel.c
	PangoFontFamily **families;
	gint 			n_families, i;
 	GSList 			*group	= NULL;

	pango_context_list_families(gtk_widget_get_pango_context(topmenu),&families, &n_families);

	for(i=0; i<n_families; i++)
    {
    	if(pango_font_family_is_monospace(families[i]))
    	{
			const gchar 	*name = pango_font_family_get_name (families[i]);
			GtkWidget		*item = gtk_radio_menu_item_new_with_label(group,name);
			gchar			*ptr;

			group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
			ptr = g_strdup(name);
			g_object_set_data_full(G_OBJECT(item),"fontname",ptr,g_free);
			g_signal_connect(G_OBJECT(item),"toggled",G_CALLBACK(activate_font),ptr);

			gtk_widget_show(item);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

			if(!strcmp(name,selected))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),TRUE);

    	}
    }

	g_free(families);

 }

#ifdef HAVE_FONTLIST_XML
 struct parse_fontlist
 {
	GtkWidget		*menu;
	GtkWidget		*item;
	GSList 			*group;
	const gchar	*selected;
	gchar			*element;
 };

 static void fontmenu_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names, const gchar **values, struct parse_fontlist *info, GError **error)
 {
	if(!g_strcasecmp(element_name,"fontlist"))
	{

	}
	else if(!g_strcasecmp(element_name,"font"))
	{
		int	 f;

		info->item  = gtk_radio_menu_item_new(info->group);
		info->group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(info->item));

		for(f=0;names[f];f++)
		{
			if(g_strcasecmp(names[f],"label"))
			{
				g_object_set_data_full(G_OBJECT(info->item),names[f],g_strdup(values[f]),g_free);
			}
			else
			{
#if GTK_CHECK_VERSION(2,16,0)
				gtk_menu_item_set_label(GTK_MENU_ITEM(info->item),values[f]);
#else
				GtkWidget *label = gtk_label_new(values[f]);

				gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
				gtk_container_add(GTK_CONTAINER(info->item),label);
#endif
				gtk_widget_show_all(info->item);

			}
		}

		gtk_menu_shell_append(GTK_MENU_SHELL(info->menu),info->item);

	}
	else
	{
		if(info->element)
			g_free(info->element);
		info->element = g_strdup(element_name);

	}

 }

 static void fontmenu_end(GMarkupParseContext *context, const gchar *element_name, struct parse_fontlist *info, GError **error)
 {
 	if(!g_strcasecmp(element_name,"font") && info->item)
 	{
 		gchar *name = g_object_get_data(G_OBJECT(info->item),"name");
 		gchar *bold = g_object_get_data(G_OBJECT(info->item),"bold");

 		if(name)
 		{
			if(!g_strcasecmp(info->selected,name))
			{
				gboolean fBold = FALSE;

				if(bold)
					fBold = g_strcasecmp(bold,"yes") == 0 ? TRUE : FALSE;

				if(gui_toggle_state[GUI_TOGGLE_BOLD] == fBold)
					gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(info->item),TRUE);

			}
			g_signal_connect(G_OBJECT(info->item),"toggled",G_CALLBACK(activate_font),name);
 		}
 	}

	if(info->element)
	{
		g_free(info->element);
		info->element = 0;
	}

 }

 static void fontmenu_text(GMarkupParseContext *context, const gchar *text, gsize sz, struct parse_fontlist *info, GError **error)
 {
 	if(!(info->element && info->item))
		return;

	g_object_set_data_full(G_OBJECT(info->item),info->element,g_strdup(text),g_free);
	g_free(info->element);
	info->element = 0;
 }

#endif

/**
 * Load system's or predefined list of terminal fonts.
 *
 * Check for fontlist.xml in program's data folder if it exists, load it on
 * supplied menu, if not populate the supplied menu with the fonts defined
 * in fontlist.xml
 *
 * @param topmenu	Font menu
 * @param selected	Selected font.
 *
 */
 void load_font_menu(GtkWidget *topmenu, const gchar *selected)
 {
 	GtkWidget	*menu	= gtk_menu_new();

#ifdef HAVE_FONTLIST_XML
	gchar *filename = g_build_filename(program_data,"fontlist.xml",NULL);

	if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
	{
		gchar 	*text  = NULL;
		GError	*error = NULL;

		if(g_file_get_contents(filename,&text,NULL,&error))
		{
			static const GMarkupParser parser =
			{
				(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))	fontmenu_start,
				(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))										fontmenu_end,
				(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **))								fontmenu_text,
				NULL,
				NULL
			};

			struct parse_fontlist	info;
			GMarkupParseContext 	*context;

			Log("Loading fonts from %s",filename);

			memset(&info,0,sizeof(info));
			info.menu		= menu;
			info.selected	= selected;

			context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT,&info,NULL);

			gui_toggle_state[GUI_TOGGLE_BOLD] = (gui_toggle_state[GUI_TOGGLE_BOLD] != 0) ? TRUE : FALSE;
			gui_toogle_set_visible(GUI_TOGGLE_BOLD,FALSE);

			g_markup_parse_context_parse(context,text,strlen(text),&error);
			g_markup_parse_context_free(context);

			if(info.element)
				g_free(info.element);

		}

		if(error)
		{
			Log("Error parsing %s: %s",filename,error->message);
			g_error_free(error);
		}

	}
	else
	{
		load_system_monospaced_fonts(topmenu,menu,selected);
	}

	g_free(filename);

#else

	load_system_monospaced_fonts(topmenu,menu,selected);

#endif

	gtk_widget_show_all(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(topmenu),menu);
 }


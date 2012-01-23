
 #include <errno.h>
 #include "g3270.h"

/*---[ Structs ]--------------------------------------------------------------*/

	#pragma pack(1)

	typedef struct _clrconfig
	{
		guint16 red;
		guint16 green;
		guint16 blue;
	} CLRCONFIG;

    static const struct _clrinfo
	{
		unsigned short sz;
		GdkColor 		 *itn;
		const char		 *name;
		const char		 *def;
	} clrinfo[] =
	{
		{
			TERMINAL_COLORS,
			terminal_cmap,
			"Terminal",
			"black,#00FFFF,red,pink,green1,turquoise,yellow,white,black,DeepSkyBlue,orange,DeepSkyBlue,PaleGreen,PaleTurquoise,grey,white"
		},
		{
			FIELD_COLORS,
			field_cmap,
			"Fields",
			"green,red,#00FFFF,white"
		},
		{
			CURSOR_COLORS,
			cursor_cmap,
			"Cursor",
			"white,white,LimeGreen,LimeGreen"
		},
		{
			SELECTION_COLORS,
			selection_cmap,
			"Selection",
			"#000020,yellow"
		},
		{
			STATUS_COLORS,
			status_cmap,
			"Status",
			"black,#7890F0,white,LimeGreen,red,white,yellow,green,LimeGreen,LimeGreen,LimeGreen,LimeGreen,LimeGreen"
		}
	};

	#pragma pack()


/*---[ Terminal colors ]------------------------------------------------------*/

 GdkColor	terminal_cmap[TERMINAL_COLORS];
 GdkColor	field_cmap[FIELD_COLORS];
 GdkColor	cursor_cmap[CURSOR_COLORS];
 GdkColor	status_cmap[STATUS_COLORS];
 GdkColor	selection_cmap[SELECTION_COLORS];

/*---[ Implement ]------------------------------------------------------------*/

 void LoadTerminalColors(FILE *cfg)
 {
	int			c,p;
	char		buffer[4096];
	char		*ptr;
	char		*tok;
	GdkColor	*clr;

	for(c=0;c<(sizeof(clrinfo)/sizeof(struct _clrinfo));c++)
	{
		// Check for entry in configuration file
		*buffer = 0;

#ifdef __G_KEY_FILE_H__
    	if(main_configuration)
    	{
    		ptr = g_key_file_get_string(main_configuration,"Colors",clrinfo[c].name,NULL);

    		if(ptr)
    		{
    			strncpy(buffer,ptr,4095);
				g_free(ptr);

    			for(ptr=buffer;*ptr;ptr++)
				{
					if(*ptr == ';')
						*ptr = ',';
				}
    		}
    	}
#endif

		if(!*buffer)
		{
			// No predefined color, try the default ones
			snprintf(buffer,4095,"%s3270",clrinfo[c].name);
			ptr = getenv(buffer);
			if(ptr)
				strncpy(buffer,ptr,4095);
			else
				strncpy(buffer,clrinfo[c].def,4095);
		}

		// Load colors in buffer
		clr = clrinfo[c].itn;

		ptr=strtok_r(buffer,",",&tok);
		for(p=0;p<clrinfo[c].sz;p++)
		{
			if(ptr)
			{
				gdk_color_parse(ptr,clr);
				ptr = strtok_r(0,",",&tok);
			}
			else
			{
				gdk_color_parse("green",clr);
			}
			gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),clr,TRUE,TRUE);
			clr++;
		}

	}
 }


 void SaveTerminalColors(FILE *arq)
 {
#ifndef __G_KEY_FILE_H__

	int f,p;
	unsigned char tag;
	CLRCONFIG		rec;

	for(f=0;f<(sizeof(clrinfo)/sizeof(struct _clrinfo));f++)
	{
		tag = 'c';
		fwrite(&tag,sizeof(tag),1,arq);
		tag = f;
		fwrite(&tag,sizeof(tag),1,arq);

		fwrite(&clrinfo[f].sz,sizeof(clrinfo[f].sz),1,arq);

		for(p=0;p<clrinfo[f].sz;p++)
		{
			rec.red   = (clrinfo[f].itn +p)->red;
			rec.green = (clrinfo[f].itn +p)->green;
			rec.blue  = (clrinfo[f].itn +p)->blue;
			fwrite(&rec,sizeof(rec),1,arq);
		}
	}

#endif
 }

 #define NUM_COLOR_TABLES 5

 typedef struct _colorlist
 {
		 char 				*name;
		 int				conf;
		 char 				**title;
		 int 				max;
		 GdkColor 			*saved;
		 GdkColor 			*tbl;
 } COLORLIST;

 typedef struct _colordata
 {
	 GdkColor	*current;
	 COLORLIST	*clr;

	 int		selected;
	 char		changed[sizeof(clrinfo)/sizeof(struct _clrinfo)];

	 GdkColor	terminal_cmap[TERMINAL_COLORS];
	 GdkColor	field_cmap[FIELD_COLORS];
	 GdkColor	cursor_cmap[CURSOR_COLORS];
	 GdkColor	status_cmap[STATUS_COLORS];
	 GdkColor	selection_cmap[SELECTION_COLORS];

 } COLORDATA;

#ifdef ENABLE_COLOR_SELECTION

 static void InsertInTree(GtkTreeStore *store, GtkTreeIter *parent, const char *str)
 {
	 GtkTreeIter iter;
	 memset(&iter,0,sizeof(iter));
	 gtk_tree_store_append(store,&iter,parent);
	 gtk_tree_store_set(store, &iter, 0, str, -1);
 }

 static void color_changed(GtkColorSelection *colorselection, COLORDATA *last)
 {
	 if(last->current)
	 {
	 	 last->changed[last->selected] = 1;
		 gtk_color_selection_get_current_color(colorselection,last->current);
		 gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),last->current,TRUE,TRUE);
		 LoadImages(terminal->window,terminal->style->fg_gc[GTK_WIDGET_STATE(terminal)]);
		 RedrawTerminalContents();
	 }
 }

 static void row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, GtkWidget *color)
 {
	 gchar 			*str  = gtk_tree_path_to_string(path);
	 gchar 			*ptr  = strchr((char *) str, ':');
	 COLORDATA		*last = g_object_get_data(G_OBJECT(color),"info");
	 COLORLIST		*clr;
	 unsigned int idx;
	 unsigned int el;

	 if(ptr)
	 {
		 *(ptr++) = 0;
		 idx = atoi(ptr);
		 el  = atoi(str);

		 DBGPrintf("el=%d idx=%d",el,idx);

		 if(el < NUM_COLOR_TABLES)
		 {
			 clr = last->clr+el;
			 if(idx < clr->max)
			 {
				 last->current = clr->tbl+idx;
				 last->selected = el;
				 gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(color),last->current);
				 gtk_color_selection_set_previous_color(GTK_COLOR_SELECTION(color),clr->saved + idx);
			 }
		 }
	 }

	 g_free(str);
 }

 void action_set_colors(GtkWidget *w, gpointer data)
 {
	 char *FieldColors[FIELD_COLORS] =
		 {	_( "Normal, Unprotected" ),
			_( "Intensified, Unprotected" ),
			_( "Normal, Protected" ),
			_( "Intensified, Protected" )
		 };

	 char *GColors[TERMINAL_COLORS] =
		 {	_( "Background" ),
			_( "Blue" ),
			_( "Red"  ),
			_( "Pink" ),
			_( "Green" ),
			_( "Turquoise" ),
			_( "Yellow" ),
			_( "White" ),
			_( "Black" ),
			_( "Dark Blue" ),
			_( "Orange" ),
			_( "Purple" ),
			_( "Dark Green" ),
			_( "Dark Turquoise" ),
			_( "Mustard" ),
			_( "Gray" )

		 };


    char *SColors[STATUS_COLORS] =
		 {	_( "Background" ),
			_( "Separator" ),
			_( "Cursor position" ),
			_( "LU Name" ),
			_( "Error message" ),
			_( "Clock" ),
			_( "Warning Message" ),
			_( "Message" ),
			_( "Toggle" ),
			_( "SSL mark" ),
			_( "Connection mark" ),
			_( "Key mark" ),
			_( "Connection icons" )
		  };

	 char *cursors[CURSOR_COLORS] =
		  {  _( "Normal" ),
			 _( "Insert" ),
			 _( "Cross Hair/Normal" ),
			 _( "Cross Hair/Insert" )
		  };


	 char *selec[SELECTION_COLORS] =
		  {  _( "Contents" ),
			 _( "Border" )
		  };

	 COLORDATA			last;
	 COLORLIST			clr[NUM_COLOR_TABLES] =
	 {
		 { _( "Base colors" ),	 	1,	FieldColors, 	FIELD_COLORS,		last.field_cmap,		field_cmap		},
		 { _( "Graphics colors"),	0,	GColors,		TERMINAL_COLORS,	last.terminal_cmap,		terminal_cmap	},
		 { _( "OIA" ),				4,	SColors,		STATUS_COLORS,		last.status_cmap,		status_cmap		},
		 { _( "Cursors" ),			2,	cursors,		CURSOR_COLORS,		last.cursor_cmap,		cursor_cmap		},
		 { _( "Selection box" ),	3,	selec,			SELECTION_COLORS,	last.selection_cmap,	selection_cmap 	}
	 };

	 GtkWidget			*widget;
	 GtkWidget			*hbox;
	 GtkWidget			*color;
	 GtkWidget			*view;
	 GtkCellRenderer	*renderer;
	 GtkTreeStore  		*store;
	 GtkTreeIter 		iter;
	 GtkWidget			*frame;
	 int 				f;
	 int				p;
#ifdef __G_KEY_FILE_H__
	 int c;
	 char buffer[4096];
#endif

	 memset(&last,0,sizeof(last));
	 last.clr = clr;
	 for(f=0;f<NUM_COLOR_TABLES;f++)
		 memcpy(clr[f].saved,clr[f].tbl,sizeof(GdkColor)*clr[f].max);

     widget = gtk_dialog_new_with_buttons (	_( "Configuração de cores" ),
                                            GTK_WINDOW(top_window),
                                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                            NULL);

	 // Color selection dialog
	 color = gtk_color_selection_new();
	 gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(color),FALSE);

	 g_object_set_data(G_OBJECT(color),"info",&last);

	 // Colors tree
	 view = gtk_tree_view_new();
	 gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view),FALSE);
	 gtk_tree_view_set_enable_tree_lines(GTK_TREE_VIEW(view),TRUE);

	 renderer = gtk_cell_renderer_text_new();
	 gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),-1,"Name",renderer,"text", 0,NULL);

	 store = gtk_tree_store_new(1, G_TYPE_STRING);
	 memset(&iter,0,sizeof(iter));

	 // Add colors
 	 for(f=0;f<NUM_COLOR_TABLES;f++)
	 {
		 gtk_tree_store_append(store,&iter,NULL);
		 DBGPrintf("Inserting \"%s\" (%d itens)",clr[f].name,clr[f].max);
		 gtk_tree_store_set(store, &iter, 0, clr[f].name, -1);
		 for(p=0;p<clr[f].max;p++)
		 {
			 DBGPrintf("Inserting %d.%d %s/%s",f,p,clr[f].name,clr[f].title[p]);
			 InsertInTree(store,&iter,clr[f].title[p]);
		 }

	 }

	 gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));

	 g_signal_connect(G_OBJECT(view),  "row-activated", G_CALLBACK(row_activated), (gpointer) color);
	 g_signal_connect(G_OBJECT(color), "color-changed", G_CALLBACK(color_changed), (gpointer) &last);

     g_object_unref(store);
	 // Boxes
	 hbox  = gtk_hpaned_new();

	 frame = gtk_scrolled_window_new(NULL,NULL);
	 gtk_container_add(GTK_CONTAINER(frame),view);
	 gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(frame),GTK_SHADOW_ETCHED_IN);
	 gtk_paned_add1(GTK_PANED(hbox),frame);

	 gtk_paned_add2(GTK_PANED(hbox),color);

	 gtk_container_add(GTK_CONTAINER(GTK_DIALOG(widget)->vbox),hbox);
	 gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(widget)->vbox));

	 switch(gtk_dialog_run(GTK_DIALOG(widget)))
	 {
	 case GTK_RESPONSE_ACCEPT:
	 	// Save selected colors
#ifdef __G_KEY_FILE_H__
		if(main_configuration)
		{
			for(c=0;c<(sizeof(clrinfo)/sizeof(struct _clrinfo));c++)
			{
				if(last.changed[c])
				{
					*buffer = 0;
					for(f=0;f<clr[c].max;f++)
					{
						sprintf(buffer+strlen(buffer),"#%04x%04x%04x;",
															(clr[c].tbl+f)->red,
															(clr[c].tbl+f)->green,
															(clr[c].tbl+f)->blue
												);

					}
					DBGPrintf("%s=%s",clrinfo[clr[c].conf].name,buffer);
					g_key_file_set_string(main_configuration,"Colors",clrinfo[clr[c].conf].name,buffer);
				}
			}
		}
#endif
		break;

	 case GTK_RESPONSE_REJECT:
		// Restore color using the values saved in "last"
		for(f=0;f<NUM_COLOR_TABLES;f++)
		{
		 memcpy(clr[f].tbl,clr[f].saved,sizeof(GdkColor)*clr[f].max);
		 for(p=0;p<clr[f].max;p++)
			 gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),clr[f].tbl+p,TRUE,TRUE);
		}
		LoadImages(terminal->window,terminal->style->fg_gc[GTK_WIDGET_STATE(terminal)]);
		RedrawTerminalContents();
		break;

	 default:
		Log("Unexpected response from color selection dialog");
	 }

     gtk_widget_destroy (widget);


     RedrawTerminalContents();

 }

#endif
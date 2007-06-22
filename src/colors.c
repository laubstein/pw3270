
 #include <errno.h>
 #include "g3270.h"

/*---[ Terminal colors ]------------------------------------------------------*/

 GdkColor	terminal_cmap[TERMINAL_COLORS];
 GdkColor	field_cmap[FIELD_COLORS];
 GdkColor	cursor_cmap[CURSOR_COLORS];
 GdkColor	status_cmap[STATUS_COLORS];
 GdkColor	selection_cmap[SELECTION_COLORS];

/*---[ Implement ]------------------------------------------------------------*/

 static int GetColorDescription(const char *id, char *buffer)
 {
    static const struct _colors
    {
    	const char *name;
    	const char *def;
    } colors[] =
    {
		{ "Terminal",	"black,#00FFFF,red,pink,green1,turquoise,yellow,white,black,DeepSkyBlue,orange,DeepSkyBlue,PaleGreen,PaleTurquoise,grey,white" },
		{ "Fields",		"green,red,#00FFFF,white" },
		{ "Cursor",		"white,white,LimeGreen,LimeGreen" },
		{ "Selection",	"#000020,yellow" },
		{ "Status",		"black,#7890F0,white,LimeGreen,red,white,yellow,green,LimeGreen,LimeGreen,LimeGreen,LimeGreen,LimeGreen" }
    };
    char	key[40];
    char	*ptr;
    int 	f;

	/* Check for environment variable */
	snprintf(key,39,"%s3270",id);
	ptr = getenv(key);
	if(ptr)
	{
   		strncpy(buffer,ptr,4095);
		return 0;
	}

    // TODO (perry#1#): Search configuration file for the definition.

    /* Search for the default colors */
    for(f=0;f< (sizeof(colors)/sizeof(struct _colors));f++)
    {
    	if(!strcmp(id,colors[f].name))
    	{
    		strncpy(buffer,colors[f].def,4095);
    		return 0;
    	}
    }

 	return -1;
 }

 static int LoadColors(GdkColor *clr, int qtd, const char *id)
 {
 	char buffer[4096];
	char *ptr;
 	char *tok;
 	int	 f;
 	int	 rc = 0;

    memset(clr,0,sizeof(GdkColor)*qtd);

    f = 0;
    if(GetColorDescription(id,buffer))
    {
    	Log("Cant'find color definition for \"%s\"",id);
    	return ENOENT;
    }

    for(ptr=strtok_r(buffer,",",&tok);ptr && f < qtd;ptr = strtok_r(0,",",&tok))
    {
    	gdk_color_parse(ptr,clr+f);

    	if(!gdk_colormap_alloc_color(	gtk_widget_get_default_colormap(),
										clr+f,
										TRUE,
										TRUE ))
		{
			Log("Can't allocate color \"%s\" from %s",ptr,id);
			rc = EINVAL;
		}

        f++;
    }
    return rc;
 }

 void SaveTerminalColors(FILE *arq)
 {
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
		GdkColor *itn;
	} clrinfo[] =
	{
		{ TERMINAL_COLORS, terminal_cmap },
		{ FIELD_COLORS, field_cmap },
		{ CURSOR_COLORS, cursor_cmap },
		{ SELECTION_COLORS, selection_cmap },
		{ STATUS_COLORS, status_cmap }
	};
	
	int f,p;
	unsigned char tag;
	CLRCONFIG		rec;
	
	#pragma pack()
	
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
 }

 void LoadTerminalColors(FILE *cfg)
 { 
	 
    LoadColors(terminal_cmap, TERMINAL_COLORS, "Terminal");
    LoadColors(field_cmap,FIELD_COLORS,"Fields");
    LoadColors(cursor_cmap,CURSOR_COLORS,"Cursor");
    LoadColors(selection_cmap,SELECTION_COLORS,"Selection");
    LoadColors(status_cmap, STATUS_COLORS, "Status");
	 
 }

 #ifdef DEBUG
 
 #define NUM_COLOR_TABLES 5

 typedef struct _colorlist
 {
		 char *name;
		 char **title;
		 int max;
		 GdkColor *saved;
		 GdkColor *tbl;
 } COLORLIST;
 
 typedef struct _colordata
 {
	 GdkColor	*current;
	 COLORLIST	*clr;
	 
	 GdkColor	terminal_cmap[TERMINAL_COLORS];
	 GdkColor	field_cmap[FIELD_COLORS];
	 GdkColor	cursor_cmap[CURSOR_COLORS];
	 GdkColor	status_cmap[STATUS_COLORS];
	 GdkColor	selection_cmap[SELECTION_COLORS];
	 
 } COLORDATA;
 
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
		 gtk_color_selection_get_current_color(colorselection,last->current);
		 gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),last->current,TRUE,TRUE);
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
	 
	 printf("%s(%d): %s\n",__FILE__,__LINE__,str);
	 
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
			_( "STATUS_COLOR_SEPARATOR" ),
			_( "STATUS_COLOR_CURSOR_POSITION" ),
			_( "Lu Name" ),
			_( "Error message" ),
			_( "Clock" ),
			_( "Warning Message" ),
			_( "Message" ),
			_( "Toogle" ),
			_( "Secure connection indicator" ),
			_( "Connection mark" ),
			_( "STATUS_COLOR_KEYBOARD" ),
			_( "STATUS_COLOR_CONNECT_ICON" )
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
		 { _( "Base colors" ),	 	FieldColors, 	FIELD_COLORS,		last.field_cmap,		field_cmap		},
		 { _( "Graphics colors"),	GColors,		TERMINAL_COLORS,	last.terminal_cmap,		terminal_cmap	},
		 { _( "OIA" ),				SColors,		STATUS_COLORS,		last.status_cmap,		status_cmap		}, 
		 { _( "Cursors" ),			cursors,		CURSOR_COLORS,		last.cursor_cmap,		cursor_cmap		},
		 { _( "Selection box" ),	selec,			SELECTION_COLORS,	last.selection_cmap,	selection_cmap 	}
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
		 break;
	 
	 case GTK_RESPONSE_REJECT:
		 // Restore color using the values saved in "last"
	 	 for(f=0;f<NUM_COLOR_TABLES;f++)
		 {
			 memcpy(clr[f].tbl,clr[f].saved,sizeof(GdkColor)*clr[f].max);
			 for(p=0;p<clr[f].max;p++)
				 gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),clr[f].tbl+p,TRUE,TRUE);
		 }
		 RedrawTerminalContents();
		 break;
		 
	 default:
		 Log("Unexpected response from color selection dialog");
	 }
	 
     gtk_widget_destroy (widget);
	 
	 
     RedrawTerminalContents();
	 
 }
 
#endif

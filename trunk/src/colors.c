
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

 void LoadTerminalColors()
 { 
	 
    LoadColors(terminal_cmap, TERMINAL_COLORS, "Terminal");
    LoadColors(field_cmap,FIELD_COLORS,"Fields");
    LoadColors(cursor_cmap,CURSOR_COLORS,"Cursor");
    LoadColors(selection_cmap,SELECTION_COLORS,"Selection");
    LoadColors(status_cmap, STATUS_COLORS, "Status");
	 
 }

 #ifdef DEBUG
 
 typedef struct _colordata
 {
	 GdkColor *current;
	 GdkColor terminal_cmap[TERMINAL_COLORS];
	 GdkColor field_cmap[FIELD_COLORS];
	 GdkColor cursor_cmap[CURSOR_COLORS];
	 GdkColor status_cmap[STATUS_COLORS];
	 GdkColor selection_cmap[SELECTION_COLORS];
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
	 unsigned int idx;
	 
	 printf("%s(%d): %s\n",__FILE__,__LINE__,str);
	 
	 if(ptr)
	 {
		 *(ptr++) = 0;
		 idx = atoi(ptr);
		 
		 switch(atoi((char *) str))
		 {
		 case 0:	// Field colors
			 if(idx < FIELD_COLORS)
			 {
				 last->current = field_cmap+idx;
				 gtk_color_selection_set_previous_color(GTK_COLOR_SELECTION(color),last->field_cmap+idx);
				 gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(color),field_cmap+idx);
			 }
			 break;
		 
		 case 1:	// Graphics colors
			 if(idx < TERMINAL_COLORS)
			 {
				 last->current = terminal_cmap+idx;
				 gtk_color_selection_set_previous_color(GTK_COLOR_SELECTION(color),last->terminal_cmap+idx);
				 gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(color),terminal_cmap+idx);
			 }
			 break;
			 
		 case 2:	// OIA
			 if(idx < TERMINAL_COLORS)
			 {
				 last->current = status_cmap+idx;
				 gtk_color_selection_set_previous_color(GTK_COLOR_SELECTION(color),last->status_cmap+idx);
				 gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(color),status_cmap+idx);
			 }
			 break;
		 }
	 }
	 
	 g_free(str);
 }
 
 void action_set_colors(GtkWidget *w, gpointer data)
 {
	 char *FieldColors[] = {	_( "Normal, Unprotected" ),
								_( "Intensified, Unprotected" ),
								_( "Normal, Protected" ),
								_( "Intensified, Protected" )
						    };
							
	 char *GColors[]	  = {	_( "Background" ),
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
								_( "Gray" ),
							    _( "Brown" ) 
							 };
							 
							 
    char *SColors[]       = {	_( "Background" ),
								_( "STATUS_COLOR_SEPARATOR" ),
								_( "STATUS_COLOR_CURSOR_POSITION" ),
								_( "STATUS_COLOR_LUNAME" ),
								_( "STATUS_COLOR_ERROR" ),
								_( "STATUS_COLOR_TIME" ),
								_( "STATUS_COLOR_WARNING" ),
								_( "STATUS_COLOR_NORMAL" ),
								_( "STATUS_COLOR_TOOGLE" ),
								_( "STATUS_COLOR_SSL" ),
								_( "STATUS_COLOR_CONNECTED" ),
								_( "STATUS_COLOR_KEYBOARD" ),
								_( "STATUS_COLOR_CONNECT_ICON" )
							 };
							 
	 COLORDATA			last;
		 
	 GtkWidget			*widget;
	 GtkWidget			*hbox;
	 GtkWidget			*color;
	 GtkWidget			*view;
	 GtkCellRenderer	*renderer;
	 GtkTreeStore  		*store;
	 GtkTreeIter 		iter;
	 GtkWidget			*frame;
	 int 				f;
	 
	 memset(&last,0,sizeof(last));
	 memcpy(last.terminal_cmap,terminal_cmap,sizeof(GdkColor)*TERMINAL_COLORS);
	 memcpy(last.field_cmap,field_cmap,sizeof(GdkColor)*FIELD_COLORS);
	 memcpy(last.cursor_cmap,cursor_cmap,sizeof(GdkColor)*CURSOR_COLORS);
	 memcpy(last.selection_cmap,selection_cmap,sizeof(GdkColor)*SELECTION_COLORS);
							 
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
	 gtk_tree_store_append(store,&iter,NULL);
	 gtk_tree_store_set(store, &iter, 0, _( "Base colors" ), -1);
	 for(f=0;f<(sizeof(FieldColors)/sizeof(char *));f++)
		 InsertInTree(store,&iter,FieldColors[f]);
	 
	 gtk_tree_store_append(store,&iter,NULL);
	 gtk_tree_store_set(store, &iter, 0, _( "Graphics colors" ), -1);
	 for(f=0;f<(sizeof(GColors)/sizeof(char *));f++)
		 InsertInTree(store,&iter,GColors[f]);

 	 gtk_tree_store_append(store,&iter,NULL);
	 gtk_tree_store_set(store, &iter, 0, _( "OIA" ), -1);
	 for(f=0;f<(sizeof(SColors)/sizeof(char *));f++)
		 InsertInTree(store,&iter,SColors[f]);

	 gtk_tree_view_set_model(GTK_TREE_VIEW(view), GTK_TREE_MODEL(store));

	 g_signal_connect(G_OBJECT(view), "row-activated", G_CALLBACK(row_activated), (gpointer) color);
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
		 break;
		 
	 default:
		 Log("Unexpected response from color selection dialog");
	 }
	 
     gtk_widget_destroy (widget);
	 
	 
     RedrawTerminalContents();
	 
 }
 
#endif

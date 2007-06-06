
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

#ifdef __GTK_COLOR_BUTTON_H__ 
 
 static void ColorChanged(GtkColorButton *widget, GdkColor *clr)
 {
	CHKPoint();
    gtk_color_button_get_color(widget,clr);
    gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),clr,TRUE,TRUE);
    RedrawTerminalContents();
 }
  
 static void AddColors(int pos, GtkTable *table, const char *string, GdkColor *list, GdkColor *bkp, int count)
 {
    GtkTable  *colors; 
	GtkWidget *button;
	int      f;
	int      row		= 0;
	int      col		= 0;
	gboolean sensitive	= !access(SYSCONFIG,W_OK);
	 
    // FIXME: http://developer.gnome.org/doc/API/2.0/gtk/GtkTable.html#gtk-table-attach-defaults	 
    gtk_table_attach_defaults(table,gtk_label_new( string ), 0,1,pos,pos+1);
	 
    colors = GTK_TABLE(gtk_table_new( (count/8),9,1));	 
	 
	for(f=0;f<count;f++)
    {
		bkp[f] = list[f];
		button = gtk_color_button_new_with_color(list+f);
        g_signal_connect(G_OBJECT(button),"color-set",G_CALLBACK(ColorChanged),list+f);
		
		gtk_widget_set_sensitive(button,sensitive);
        gtk_table_attach_defaults(colors,button,col,col+1,row,row+1);
		if(col++ > 6)
		{
			row++;
			col = 0;
		}
    }		
	 
    gtk_table_attach_defaults(table,GTK_WIDGET(colors),1,2,pos,pos+1);
    gtk_table_set_row_spacing(table,pos,4);

 }
 
 static void RestoreColors(GdkColor *clr, GdkColor *bkp, int count)
 {
	int f;
	for(f=0;f<count;f++)
	{
		clr[f] = bkp[f];
        gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),clr+f,TRUE,TRUE);
	}
 }
 
 void action_set_colors(GtkWidget *w, gpointer data)
 {
     GdkColor  terminal_backup[TERMINAL_COLORS];
	 GdkColor  field_backup[FIELD_COLORS];
     GdkColor  cursor_backup[CURSOR_COLORS];
     GdkColor  selection_backup[SELECTION_COLORS];
     GdkColor  status_backup[STATUS_COLORS];
	 
	 GtkWidget *widget;
	 GtkTable  *table; 

     widget = gtk_dialog_new_with_buttons (	_( "Configuração de cores" ),
                                            GTK_WINDOW(top_window),
                                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
                                            GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
                                            NULL);
								

     table = GTK_TABLE(gtk_table_new(1,5,0));

	 gtk_table_set_col_spacing(table,0,10);
	 
     AddColors(0, table, _( "Terminal" ),  		terminal_cmap,	terminal_backup,	TERMINAL_COLORS );
	 AddColors(1, table, _( "Fields"),			field_cmap,		field_backup,		FIELD_COLORS);
     AddColors(2, table, _( "Cursor"),			cursor_cmap,	cursor_backup,		CURSOR_COLORS);
     AddColors(3, table, _( "Selection Box" ),	selection_cmap,	selection_backup,	SELECTION_COLORS);
     AddColors(4, table, _( "Status Bar" ),		status_cmap,	status_backup,		STATUS_COLORS);

	 gtk_container_add(GTK_CONTAINER(GTK_DIALOG(widget)->vbox),GTK_WIDGET(table));
	 gtk_widget_show_all(GTK_WIDGET(GTK_DIALOG(widget)->vbox));
   	 switch(gtk_dialog_run(GTK_DIALOG(widget)))
	 {
	 case GTK_RESPONSE_ACCEPT:
		 DBGMessage("Cores confirmadas, atualizar arquivo de configuracao");
	     break;
	 
	 case GTK_RESPONSE_REJECT:
		 DBGMessage("Alteracao cancelada, restaurar todas as cores");
     	 RestoreColors(terminal_cmap,	terminal_backup,	TERMINAL_COLORS);
	 	 RestoreColors(field_cmap,		field_backup,		FIELD_COLORS);
     	 RestoreColors(cursor_cmap,		cursor_backup,		CURSOR_COLORS);
     	 RestoreColors(selection_cmap,	selection_backup,	SELECTION_COLORS);
     	 RestoreColors(status_cmap,		status_backup,		STATUS_COLORS);
		 break;
	 
	 default:
	 	 Log("Invalid response from color selection dialog");
	 }
	 
     gtk_widget_destroy (widget);
	 
     RedrawTerminalContents();
	 
 }

 #endif // __GTK_COLOR_BUTTON_H__

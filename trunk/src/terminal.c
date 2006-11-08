
 #include "g3270.h"
 #include <gdk/gdkkeysyms.h>
 #include <string.h>

 #include "lib/hostc.h"
 #include "lib/kybdc.h"
 #include "lib/actionsc.h"
 #include "lib/3270ds.h"

/*---[ Defines ]--------------------------------------------------------------*/

 #define MIN_LINE_SPACING	2
 #define FIELD_COLORS	4

 #define DEFCOLOR_MAP(f) ((((f) & FA_PROTECT) >> 4) | (((f) & FA_INT_HIGH_SEL) >> 3))

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void InvalidateCursor(void);

/*---[ Structures ]-----------------------------------------------------------*/

 typedef struct _fontelement
 {
	GdkFont *fn;
	int	  	Width;
	int	  	Height;
 } FONTELEMENT;

/*---[ Constants ]------------------------------------------------------------*/

  static const int widget_states[] = { GTK_STATE_NORMAL, GTK_STATE_ACTIVE, GTK_STATE_PRELIGHT, GTK_STATE_SELECTED, GTK_STATE_INSENSITIVE };


/*---[ Terminal config ]------------------------------------------------------*/
  // TODO (perry#2#): Read from file(s).

  // /usr/X11R6/lib/X11/rgb.txt
  static const char *TerminalColors = "black,blue1,red,pink,green1,turquoise,yellow,white,black,DeepSkyBlue,orange,DeepSkyBlue,PaleGreen,PaleTurquoise,grey,white";
  static const char *FieldColors    = "green1,red,blue,white";


/*
        static int field_colors[4] = {
            COLOR_GREEN,         default
            COLOR_RED,           intensified
            COLOR_BLUE,          protected
            COLOR_WHITE          protected, intensified
#       define DEFCOLOR_MAP(f) \
                ((((f) & FA_PROTECT) >> 4) | (((f) & FA_INT_HIGH_SEL) >> 3))
*/


  static const char *FontDescr[] =
  {

		"-xos4-terminus-medium-*-normal-*-12-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-14-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-16-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-20-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-24-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-28-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-32-*-*-*-*-*-*-*",

/*
		"-xos4-terminus-bold-*-*-*-12-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-14-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-16-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-20-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-24-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-28-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-32-*-*-*-*-*-*-*"
*/

  };

  #define FONT_COUNT (sizeof(FontDescr)/sizeof(const char *))

/*---[ Globals ]--------------------------------------------------------------*/

 const char			*cl_hostname	= 0;

 static FONTELEMENT *fontlist				= 0;
 static FONTELEMENT *font					= 0;
 static int			top_margin				= 0;
 static int 		left_margin				= 0;
 static int			cursor_row				= 0;
 static int			cursor_col				= 0;
 static int			cursor_height			= 3;
 static GdkColor	*terminal_cmap			= 0;
 static int			terminal_color_count	= 0;
 static int			line_spacing			= MIN_LINE_SPACING;
 static GdkColor	field_cmap[FIELD_COLORS];

/*---[ Implement ]------------------------------------------------------------*/

 static void stsConnect(Boolean status)
 {
    g3270_log("lib3270", "%s", status ? "Connected" : "Disconnected");
    if(!status)
       SetWindowTitle(0);
 }

 static void stsHalfConnect(Boolean ignored)
 {
 	DBGPrintf("HalfConnect: %s", ignored ? "Yes" : "No");
 }

 static void stsExiting(Boolean ignored)
 {
 	DBGPrintf("Exiting: %s", ignored ? "Yes" : "No");
 }

 static void stsResolving(Boolean ignored)
 {
 	DBGPrintf("Resolving: %s", ignored ? "Yes" : "No");
 }

 static gboolean expose(GtkWidget *widget, GdkEventExpose *event, void *t)
 {
    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventExpose

 	const struct ea *trm;
 	int				rows;
 	int				cols;
 	int				row;
 	int				col;
 	int				cRow;
 	int				cCol;
 	int				hPos;
 	int				vPos;
 	char			chr[2];

    GdkGC 			*gc     	= widget->style->fg_gc[GTK_WIDGET_STATE(widget)];

    gboolean		rc			= FALSE;
    int				mode		= 0;

 	trm = Get3270DeviceBuffer(&rows, &cols);

    if(!trm)
       return rc;

    // TODO (perry#2#): Find a better way (Is it necessary to run all over the buffer?)
    vPos = top_margin+font->Height;
    for(row = 0; row < rows; row++)
    {
    	hPos = left_margin;
    	for(col = 0; col < cols; col++)
    	{
		   chr[0] = Ebc2ASC(trm->cc);

		   chr[1] = 0;

		   rc  = TRUE;

		   if(trm->fa)
		   {
		      chr[0] = ' ';

              if( (trm->fa & (FA_INTENSITY|FA_INT_NORM_SEL|FA_INT_HIGH_SEL)) == (FA_INTENSITY|FA_INT_NORM_SEL|FA_INT_HIGH_SEL) )
                 mode = (trm->fa & FA_PROTECT) ? 1 : 2;
			  else
			     mode = 0;

/*
			  DBGPrintf("Flag %04x at %d,%d %s %s %s %s %s %s %s %s %s",
								trm->fa,row,col,
								trm->fa & FA_PROTECT ? "FA_PROTECT" : "",
								trm->fa & FA_NUMERIC ? "FA_NUMERIC" : "",
								trm->fa & FA_INTENSITY ? "FA_INTENSITY" : "",
								trm->fa & FA_INT_NORM_NSEL ? "FA_INT_NORM_NSEL" : "",
								trm->fa & FA_INT_NORM_SEL ? "FA_INT_NORM_SEL" : "",
								trm->fa & FA_INT_HIGH_SEL ? "FA_INT_HIGH_SEL" : "",
								trm->fa & FA_INT_ZERO_NSEL ? "" : "",
								trm->fa & FA_RESERVED ? "FA_RESERVED" : "",
								trm->fa & FA_MODIFY ? "FA_MODIFY" : "" );
*/



			  if(trm->fg || trm->bg)
			  {
			     gdk_gc_set_foreground(gc,terminal_cmap + (trm->fg % terminal_color_count));
		         gdk_gc_set_background(gc,terminal_cmap + (trm->bg % terminal_color_count));
			  }
			  else
			  {
			     gdk_gc_set_foreground(gc,field_cmap+(DEFCOLOR_MAP(trm->fa)));
			  }


		   }
		   else if(trm->gr || trm->fg || trm->bg)
		   {

// TODO (perry#1#): Set GC attributes based on terminal flags (and blink?)
//              if(trm->gr & GR_BLINK)
//              if(trm->gr & GR_REVERSE)
//              if(trm->gr & GR_UNDERLINE)
//              if(trm->gr & GR_INTENSIFY)

			  if(trm->fg || trm->bg)
			  {
		         gdk_gc_set_foreground(gc,terminal_cmap + (trm->fg % terminal_color_count));
		         gdk_gc_set_background(gc,terminal_cmap + (trm->fg % terminal_color_count));
			  }

		   }

		   if(col == cursor_col)
		      cCol = hPos;

		   if(row == cursor_row)
		      cRow = vPos;

		   switch(mode)
		   {
		   case 0:	// Nothing special
		      gdk_draw_text(widget->window,font->fn,gc,hPos,vPos,chr,1);
		      break;

		   case 1:  // Hidden
		      gdk_draw_text(widget->window,font->fn,gc,hPos,vPos," ",1);
		      break;

		   case 2:	// Hidden/Editable
		      gdk_draw_text(widget->window,font->fn,gc,hPos,vPos,chr[0] == ' ' ? chr : "*", 1);
		      break;

		   }

		   hPos += font->Width;
	       trm++;
    	}
    	vPos += (font->Height + line_spacing);
    }

    /* Draw cursor */

    gdk_gc_set_foreground(gc,field_cmap+3);

	gdk_draw_rectangle(	widget->window,
                        widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
                        1,
                        cCol, cRow,
                        font->Width,
                        cursor_height );

/*
	gdk_draw_line(	widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
					col, 0, col, row );

	gdk_draw_line(	widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
					0, row, col, row );
*/

    return rc;
 }

 static gboolean key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
#ifdef DEBUG
     #define DECLARE_ACTION(key, action, cause, parm1, parm2) { key, #key, action, cause, parm1, parm2 }
#else
     #define DECLARE_ACTION(key, action, cause, parm1, parm2) { key, action, cause, parm1, parm2 }
#endif

    static const struct _actions
    {
       guint		 	keyval;
#ifdef DEBUG
       const char		*trace;
#endif
	   XtActionProc 	action;
	   enum iaction 	cause;
	   const char 		*parm1;
	   const char 		*parm2;
    } actions[] =
    {
		// /opt/gnome/include/gtk-2.0/gdk/gdkkeysyms.h
		// http://www.koders.com/c/fidA3A9523D24A70BAFCE05733E73D558365D103DB3.aspx

		DECLARE_ACTION( GDK_Home,		Home_action, 		IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Left,		Left_action,		IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Up,			Up_action,			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Right,		Right_action, 		IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Down,		Down_action,		IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_Clear,  	Clear_action, 		IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_3270_Reset,	Reset_action,		IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_Tab,	    Tab_action,			IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_Delete,		Delete_action,		IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_BackSpace,	BackSpace_action,	IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Return,		Enter_action,		IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_KP_Enter,	Enter_action,		IA_DEFAULT, CN, CN ),

//        DECLARE_ACTION( GDK_Escape,		Escape_action,		IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Linefeed,	Newline_action,		IA_DEFAULT, CN, CN )

    };

//        DECLARE_ACTION(        Redraw_action,	IA_DEFAULT, CN, CN ),

	char			ks[6];
    String			params[2];
    Cardinal 		one			= 1;
    int				f;

    for(f=0; f < (sizeof(actions)/sizeof(struct _actions));f++)
    {
    	if(actions[f].keyval == event->keyval)
    	{
#ifdef DEBUG
		   DBGPrintf("Action: %s",actions[f].trace);
#endif
           action_internal(	actions[f].action,
							actions[f].cause,
							actions[f].parm1,
							actions[f].parm2 );
           return TRUE;
    	}
    }

    /* Check for Function keys */
    if(event->keyval >= GDK_F1 && event->keyval <=  GDK_F12)
    {
    	snprintf(ks,5,"%d",(event->keyval - GDK_F1)+1);
    	DBGPrintf("Function-%s",ks);
		action_internal(PF_action, IA_DEFAULT, ks, CN);
    	return TRUE;
    }

    /* Check for regular key */

    if(event->keyval < 0x00FF && event->string[0] >= ' ')
    {
 	   // Standard char, use it.
	   params[0] = event->string;
	   params[1] = 0;
       Key_action(NULL, NULL, params, &one);
 	   return TRUE;
	}



	/* Unknown key, ignore-it */
    DBGTracex(event->keyval);
    return FALSE;

 }

 static gboolean button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
    // http://developer.gnome.org/doc/API/2.0/gtk/GtkWidget.html#GtkWidget-button-press-event
    CHKPoint();

    return 0;
 }

 static void SetFont(GtkWidget *widget, FONTELEMENT *fn, int width, int height)
 {
 	int rows = 0;
 	int cols = 0;

    Get3270DeviceBuffer(&rows, &cols);

 	font = fn;

    /* Calculate new left margin */

	left_margin = (width - (font->Width*cols)) >> 1;
 	if(left_margin < 0)
	   left_margin = 0;

	/* Calculate new line-spacing */
	line_spacing = (height - (font->Height*rows)) / rows;
	if(line_spacing < MIN_LINE_SPACING)
	   line_spacing = MIN_LINE_SPACING;

    top_margin = (height - ((font->Height+line_spacing)*rows)) >> 1;
    if(top_margin < 0)
       top_margin = 0;

	gtk_widget_queue_draw(widget);

 }

 static void resize(GtkWidget *widget, GtkAllocation *allocation, void *t)
 {
 	int			rows;
 	int			cols;
 	int			width;
 	FONTELEMENT *fn   	= font;
 	int         f;

    Get3270DeviceBuffer(&rows, &cols);

 	width = allocation->width / cols;

	DBGPrintf("Resize %d,%d em %d,%d (%d)", allocation->width, allocation->height,allocation->x,allocation->y,width);

    /* Search for a font with exact match */
    for(f = 0; f< FONT_COUNT;f++)
    {
    	if(fontlist[f].Width == width)
    	{
		   DBGPrintf("Font %d has %d pixels width",f,width);
		   SetFont(widget,fontlist+f,allocation->width,allocation->height);
		   return;
    	}
    }

    /* No match, search for the near one */
    for(f = 0; f < FONT_COUNT;f++)
    {
    	if(fontlist[f].Width <= width && fontlist[f].Width > fn->Width)
    	   fn = fontlist+f;
    }

    /* Reconfigure drawing box */
    SetFont(widget,fn,allocation->width,allocation->height);

 }

 GtkWidget *g3270_new(const char *hostname)
 {

 	int		  sz;
 	int		  f;
    gint 	  lbearing	= 0;
    gint 	  rbearing	= 0;
    gint 	  width		= 0;
    gint 	  ascent	= 0;
    gint 	  descent	= 0;
    int		  rows		= 0;
    int		  cols		= 0;

 	char      *ptr;
 	char      *tok;
 	char      buffer[4096];

 	GtkWidget *ret;

 	if(!hostname)
 	   cl_hostname = hostname;

    gsource_init();

    Initialize_3270();

    register_3270_schange(ST_CONNECT,		stsConnect);
    register_3270_schange(ST_EXITING,		stsExiting);
    register_3270_schange(ST_HALF_CONNECT,	stsHalfConnect);
    register_3270_schange(ST_RESOLVING,		stsResolving);

    /* Load font table */
    sz   	 = sizeof(FONTELEMENT) * FONT_COUNT;
    fontlist = g_malloc(sz);
    if(!fontlist)
    {
    	Log("Memory allocation error when creating font table");
    	return 0;
    }

    memset(fontlist,0,sz);

    for(f=0;f<FONT_COUNT;f++)
    {
    	fontlist[f].fn = gdk_font_load(FontDescr[f]);

    	if(fontlist[f].fn)
    	{
    	   gdk_text_extents(fontlist[f].fn,"A",1,&lbearing,&rbearing,&width,&ascent,&descent);
    	   fontlist[f].Width  = width;
           fontlist[f].Height = (ascent+descent)+line_spacing;
    	}
    	else
    	{
    		Log("Error loading font %s",FontDescr[f]);
    	}
    }

    /* Create drawing area */
    ret = gtk_drawing_area_new();
    GTK_WIDGET_SET_FLAGS(ret, GTK_CAN_DEFAULT);
    GTK_WIDGET_SET_FLAGS(ret, GTK_CAN_FOCUS);

    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Events.html#GdkEventMask
    gtk_widget_add_events(ret,GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK);

    // FIXME (perry#3#): Make it better! Get the smaller font, not the first one.
	SetFont(ret,fontlist,0,0);

    Get3270DeviceBuffer(&rows, &cols);
    DBGPrintf("Screen size: %dx%d",rows,cols);
    gtk_widget_set_size_request(ret, font->Width*(cols+1), (font->Height+MIN_LINE_SPACING)*(rows+1));

    g_signal_connect(G_OBJECT(ret), "expose_event",  		G_CALLBACK(expose),		  0);
    g_signal_connect(G_OBJECT(ret), "size-allocate",		G_CALLBACK(resize),	   	  0);
    g_signal_connect(G_OBJECT(ret), "key-release-event",	G_CALLBACK(key_release),  0);
    g_signal_connect(G_OBJECT(ret), "button-press-event",	G_CALLBACK(button_press), 0);

    // Set terminal colors
    DBGTracex(gtk_widget_get_default_colormap());

    strncpy(buffer,TerminalColors,4095);
    for(ptr=strtok_r(buffer,",",&tok);ptr;ptr = strtok_r(0,",",&tok))
       terminal_color_count++;

    sz   		  = sizeof(GdkColor) * terminal_color_count;
    terminal_cmap =  g_malloc(sz);

    if(!terminal_cmap)
    {
    	Log("Memory allocation error when creating %d colors",terminal_color_count);
    	return 0;
    }

    f = 0;
    strncpy(buffer,TerminalColors,4095);
    for(ptr=strtok_r(buffer,",",&tok);ptr;ptr = strtok_r(0,",",&tok))
    {
    	gdk_color_parse(ptr,terminal_cmap+f);

    	if(!gdk_colormap_alloc_color(	gtk_widget_get_default_colormap(),
										terminal_cmap+f,
										FALSE,
										TRUE ))
		{
			Log("Can't allocate terminal color \"%s\"",ptr);
		}
        f++;
    }

    for(f=0;f < (sizeof(widget_states)/sizeof(int));f++)
    {
	   // http://ometer.com/gtk-colors.html
       gtk_widget_modify_bg(ret,widget_states[f],terminal_cmap);
       gtk_widget_modify_fg(ret,widget_states[f],terminal_cmap+4);
    }

    /* Set field colors */
    memset(&field_cmap,0,sizeof(GdkColor)*FIELD_COLORS);

    f = 0;
    strncpy(buffer,FieldColors,4095);
    for(ptr=strtok_r(buffer,",",&tok);ptr && f < FIELD_COLORS;ptr = strtok_r(0,",",&tok))
    {
    	gdk_color_parse(ptr,field_cmap+f);

    	if(!gdk_colormap_alloc_color(	gtk_widget_get_default_colormap(),
										field_cmap+f,
										FALSE,
										TRUE ))
		{
			Log("Can't allocate field color \"%s\"",ptr);
		}

        f++;
    }

    /* Finish */

	// TODO (perry#3#): Start connection in background.
    if(cl_hostname)
    {
       g3270_log(TARGET, "Connecting to \"%s\"",cl_hostname);
       host_connect(cl_hostname);
    }

    return ret;
 }

 static void InvalidateCursor(void)
 {
    // TODO (perry#1#): Invalidate only the cursor position.
	gtk_widget_queue_draw(terminal);
 }

 void SetCursorPosition(int row, int col)
 {
    InvalidateCursor();
    cursor_row = row;
    cursor_col = col;
    InvalidateCursor();
// 	DBGPrintf("Cursor moved to %dx%d",row,col);

 }


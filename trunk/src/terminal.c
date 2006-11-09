
 #include "g3270.h"
// #include <gdk/gdkkeysyms.h>
 #include <string.h>

 #include "lib/hostc.h"
 #include "lib/kybdc.h"
// #include "lib/actionsc.h"
 #include "lib/3270ds.h"

/*---[ Defines ]--------------------------------------------------------------*/

 #define MIN_LINE_SPACING	2
 #define FIELD_COLORS		4
 #define CURSOR_COLORS		(CURSOR_TYPE_CROSSHAIR * 2)

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
  static const char *CursorColors	= "white,white,DarkSlateGray,DarkSlateGray";

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

 const char			*cl_hostname							= 0;

 static FONTELEMENT *fontlist								= 0;
 static FONTELEMENT *font									= 0;
 static int			top_margin								= 0;
 static int 		left_margin								= 0;
 static GdkColor	*terminal_cmap							= 0;
 static int			terminal_color_count					= 0;
 static int			line_spacing							= MIN_LINE_SPACING;
 static GdkColor	field_cmap[FIELD_COLORS];

 static int			cursor_row								= 0;
 static int			cursor_col								= 0;
 static int			cursor_height[CURSOR_TYPE_CROSSHAIR]	= { 3, 6 };
 static int			cursor_type								= CURSOR_TYPE_OVER;
 static gboolean	cross_hair								= TRUE;
 static GdkColor	cursor_cmap[CURSOR_COLORS];


/*---[ Gui-Actions ]----------------------------------------------------------*/

 void toogle_crosshair(void)
 {
 	cross_hair = !cross_hair;
    InvalidateCursor();
 }

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
    int				ps;

 	trm = Get3270DeviceBuffer(&rows, &cols);

    if(!trm)
       return rc;

	/* Adjust cursor color */
	ps = (cursor_row * rows) + cols;

    /* Calculate coordinates */
    vPos = (top_margin + font->Height);
    cRow = (cursor_row * (font->Height + line_spacing)) + vPos;
    cCol = (cursor_col * font->Width) + left_margin;

    if(cross_hair && (cursor_type != CURSOR_TYPE_NONE))
    {
	   /* Draw cross-hair cursor */
       gdk_gc_set_foreground(gc,cursor_cmap+CURSOR_TYPE_CROSSHAIR+cursor_type);

   	   gdk_draw_line(	widget->window,
						widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
						cCol, event->area.y, cCol, event->area.y + event->area.height );

	   gdk_draw_line(	widget->window,
						widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
						event->area.x, cRow, event->area.x + event->area.width, cRow );
    }

    // TODO (perry#2#): Find a better way (Is it necessary to run all over the buffer?)
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

    if(cursor_type != CURSOR_TYPE_NONE)
    {
       /* Draw cursor */

       gdk_gc_set_foreground(gc,cursor_cmap+cursor_type);

	   gdk_draw_rectangle(	widget->window,
							widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
							1,
							cCol, (cRow + 3) - cursor_height[cursor_type],
							font->Width,
							cursor_height[cursor_type] );
    }

    return rc;
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

    g_signal_connect(G_OBJECT(ret), "expose_event",  		G_CALLBACK(expose),		  	0);
    g_signal_connect(G_OBJECT(ret), "size-allocate",		G_CALLBACK(resize),	   	  	0);
    g_signal_connect(G_OBJECT(ret), "key-release-event",	G_CALLBACK(KeyboardAction),	0);
    g_signal_connect(G_OBJECT(ret), "button-press-event",	G_CALLBACK(button_press), 	0);

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

    /* Set cursor colors */
// static GdkColor	cursor_cmap[];

    memset(&cursor_cmap,0,sizeof(GdkColor)*CURSOR_COLORS);

    f = 0;
    strncpy(buffer,CursorColors,4095);
    for(ptr=strtok_r(buffer,",",&tok);ptr && f < CURSOR_COLORS;ptr = strtok_r(0,",",&tok))
    {
    	gdk_color_parse(ptr,cursor_cmap+f);

    	if(!gdk_colormap_alloc_color(	gtk_widget_get_default_colormap(),
										cursor_cmap+f,
										FALSE,
										TRUE ))
		{
			Log("Can't allocate cursor color \"%s\"",ptr);
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
	gtk_widget_queue_draw(terminal);
 }

 void SetCursorPosition(int row, int col)
 {
    InvalidateCursor();
    cursor_row = row;
    cursor_col = col;
    InvalidateCursor();
 }

 void SetCursorType(int type)
 {
 	cursor_type = type;
    InvalidateCursor();
 }


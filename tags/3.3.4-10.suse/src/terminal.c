
 #include "g3270.h"
 #include <string.h>
 #include <errno.h>
 #include <gdk/gdkkeysyms.h>

 #include "lib/hostc.h"
 #include "lib/kybdc.h"
 #include "lib/actionsc.h"
 #include "lib/ctlrc.h"
 #include "lib/tablesc.h"
 #include "lib/utilc.h"

/*---[ Defines ]--------------------------------------------------------------*/

 #define MIN_LINE_SPACING	2

 #define	STATUS_LINE_SPACE		4


/*---[ Prototipes ]-----------------------------------------------------------*/

/*---[ Constants ]------------------------------------------------------------*/

  static const int widget_states[] = { GTK_STATE_NORMAL, GTK_STATE_ACTIVE, GTK_STATE_PRELIGHT, GTK_STATE_SELECTED, GTK_STATE_INSENSITIVE };

/*---[ Globals ]--------------------------------------------------------------*/

#ifdef DATADIR
  static const char *fontfile = DATADIR "/fonts.conf";
#else
  static const char *fontfile = "./fonts.conf";
#endif

 const char			*cl_hostname							= 0;

 gboolean			WaitForScreen							= TRUE;

 static FONTELEMENT *fontlist								= 0;
 static int			fontCount								= 0;

 #define FONT_COUNT fontCount

 FONTELEMENT 		*font									= 0;
 int				top_margin								= 0;
 int 				left_margin								= 0;

 GdkColor			*terminal_cmap							= 0;
 int				terminal_color_count					= 0;
 int				line_spacing							= MIN_LINE_SPACING;

// static int			fromRow									= -1;
// static int			fromCol									= -1;
// static int			toRow									= -1;
// static int			toCol									= -1;

// static long		xFrom									= -1;
// static long		yFrom									= -1;
// static long		xTo										= -1;
// static long		yTo										= -1;
// static int			MouseMode								= MOUSE_MODE_NORMAL;

 static int			cursor_height[CURSOR_TYPE_CROSSHAIR]	= { 3, 6 };
 static gboolean    cursor_enabled							= TRUE;
 int				cursor_row								= 0;
 int				cursor_col								= 0;
 static gboolean	cross_hair								= FALSE;

 int				cursor_type								= CURSOR_TYPE_OVER;
 gboolean			reconnect								= TRUE;
 int				reconnect_retry							= 0;

// static int 		selection_row 							= -1;
// static int 		selection_col 							= -1;
// gboolean			mouse_click								= 0;


#ifdef USE_GTKIMCONTEXT
 /* Input context for dead key support */
 // http://developer.gnome.org/doc/API/2.0/gtk/GtkIMContext.html
 GtkIMContext		*im;
#endif

/*---[ Terminal colors ]------------------------------------------------------*/

 GdkColor	field_cmap[FIELD_COLORS];
 GdkColor	cursor_cmap[CURSOR_COLORS];
 GdkColor	status_cmap[STATUS_COLORS];
 GdkColor	selection_cmap[SELECTION_COLORS];

/*---[ Gui-Actions ]----------------------------------------------------------*/

/*---[ Implement ]------------------------------------------------------------*/

 static void Reconnect(void)
 {
    SetOIAStatus(STATUS_RECONNECTING);
    action_connect(0,0);
 }

 static void sts3270Mode(Boolean connected)
 {
 	DBGPrintf("3270Mode: %s", connected ? "Connected" : "Not connected");
	stop_3270_timer();
    RedrawStatusLine();
 }

 static void stsConnect(Boolean connected)
 {
 	char key[40];
 	char *host;

    g3270_log("lib3270", "%s", connected ? "Connected" : "Disconnected");

    if(connected)
    {
	   reconnect_retry = 0;
	   if (GetKeyboardStatus() & KL_AWAITING_FIRST)
	      SetOIAStatus(STATUS_AWAITING_FIRST);
       else
	      SetOIAStatus(STATUS_BLANK);
       ctlr_erase(True);
       EnableCursor(TRUE);
    }
    else
    {
       SetOIAStatus(STATUS_DISCONNECTED);
       EnableCursor(FALSE);
       ctlr_erase(True);

       if(reconnect)
       {
          snprintf(key,39,"HOST3270_%d",++reconnect_retry);
          DBGPrintf("Host Key: %s",key);
		  host = getenv(key);
		  if(host)
		  {
		  	 cl_hostname = host;
             AddTimeOut(1000,Reconnect);
		  }
		  else
		  {
		  	 Log("No more hosts to try, stopping");
		  }
       }
    }

    stop_3270_timer();
    UpdateWindowTitle();
    RedrawTerminalContents();

 }

 static void stsResolving(Boolean ignored)
 {
 	DBGPrintf("Resolving: %s", ignored ? "Yes" : "No");
    SetOIAStatus(STATUS_RESOLVING);
 }

 static void stsHalfConnect(Boolean ignored)
 {
 	DBGPrintf("HalfConnect: %s", ignored ? "Yes" : "No");
    SetOIAStatus(STATUS_CONNECTING);
 }

 static void stsExiting(Boolean ignored)
 {
 	DBGPrintf("Exiting: %s", ignored ? "Yes" : "No");
 }

/*
 gboolean Mouse2Terminal(long x, long y, long *cRow, long *cCol)
 {
 	int 	rows;
 	int 	cols;

    Get3270DeviceBuffer(&rows, &cols);

    // Convert mouse coordinates into cursor coordinates

    if((x < left_margin) || (y < top_margin) )
    {
	   *cRow = *cCol = 0;
       return FALSE;
    }

    *cCol = (x - left_margin) / font->Width;
    *cRow = (y - top_margin)  / (font->Height + line_spacing);

    if( (*cCol > cols) || (*cRow > rows))
    {
       if(*cCol > cols)
          *cCol = cols;

	   if(*cRow > rows)
	      *cRow = rows;

	   return FALSE;
    }

    return TRUE;
 }
 */

/*
 static int CheckForCopy(void)
 {
    if( !(MouseMode & MOUSE_MODE_CLIPBOARD))
       return 0;

	DBGTracex(MouseMode);
	DBGTracex(MouseMode & MOUSE_MODE_APPEND);

    if(MouseMode & MOUSE_MODE_APPEND)
       action_append(0,0);
	else
	   action_copy(0,0);

    return 1;
 }
 */

 static gboolean expose(GtkWidget *widget, GdkEventExpose *event, void *t)
 {
    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventExpose
    int		cRow;
    int		cCol;
    int		rows;
    int		cols;
    GdkGC	*gc		= widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
//    int		x[2];
//    int		y[2];
    int		vPos = (top_margin + font->Height);
    gint 	width;
    gint 	height;

    Get3270DeviceBuffer(&rows, &cols);

    cRow = (cursor_row * (font->Height + line_spacing)) + vPos;
    cCol = (cursor_col * font->Width) + left_margin;

    /* Get window size */
    gdk_drawable_get_size(widget->window,&width,&height);

    /* Draw background */
    gdk_gc_set_foreground(gc,terminal_cmap);
    gdk_draw_rectangle(widget->window,gc,1,0,0,width,height);

    DrawSelectionBox(widget->window,gc);

    /* Check for selection box
    if(fromRow >= 0 && toRow > 0)
    {
       x[0] = (min(fromCol,toCol) * font->Width) + left_margin;
       x[1] = (max(fromCol,toCol) * font->Width) + left_margin;

       y[0] = ((min(fromRow,toRow) * (font->Height + line_spacing)) + vPos)-font->Height;
       y[1] = ((max(fromRow,toRow) * (font->Height + line_spacing)) + vPos)-font->Height;

       gdk_gc_set_foreground(gc,selection_cmap);
       gdk_draw_rectangle(widget->window,gc,1,x[0],y[0],x[1]-x[0],y[1]-y[0]);

       gdk_gc_set_foreground(gc,selection_cmap+1);
       gdk_draw_rectangle(widget->window,gc,0,x[0],y[0],x[1]-x[0],y[1]-y[0]);

    }
    */

    // TODO (perry#2#): Draw to a pixmap and paint here the contents.
    if(!cursor_enabled)
       *oia_cursor = 0;

    DrawTerminal(	widget->window,
					gc,
					font,
					left_margin,
					top_margin,
					line_spacing);

    /* Draw cursor */
    if(cursor_enabled)
    {
       gdk_gc_set_foreground(gc,cursor_cmap+cursor_type);

	   gdk_draw_rectangle(	widget->window,
							gc,
							1,
							cCol, (cRow + 3) - cursor_height[cursor_type],
							font->Width,
							cursor_height[cursor_type] );

       if(cross_hair)
       {
          gdk_gc_set_foreground(gc,cursor_cmap+CURSOR_TYPE_CROSSHAIR+cursor_type);

   	      gdk_draw_line(	widget->window,
							gc,
							cCol,
							top_margin,
							cCol,
							top_margin + ( rows * (font->Height + line_spacing) ));

			gdk_draw_line(	widget->window,
							gc,
							left_margin, cRow,
							left_margin + (cols * font->Width), cRow );
       }
    }

    return 0;
 }

/*
 static gboolean single_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
 	long	cRow;
 	long	cCol;

    DBGPrintf("Button %d press at %ld,%ld", event->button,(unsigned long) event->x, (unsigned long) event->y);

    switch(event->button)
    {
    case 1:

       if(MouseMode != MOUSE_MODE_NORMAL)
       {
          MouseMode = MOUSE_MODE_NORMAL;
          action_remove_selection(0,0);
       }

       xFrom = (unsigned long) event->x;
       yFrom = (unsigned long) event->y;

       Mouse2Terminal((long) event->x, (long) event->y, &cRow, &cCol);

       fromRow = cRow;
       fromCol = cCol;
       break;

	case 3:
	   DBGMessage("Copy!");
	   MouseMode |= MOUSE_MODE_COPY;
	   break;
    }

 	return 0;
 }
*/

/*
 static void Check4Function(const char *field)
 {
 	const char *ptr;
 	char ks[6];

 	for(ptr = field;*ptr;ptr++)
 	{
 		if(!isdigit(*ptr))
 		   return;
 	}

    snprintf(ks,5,"%d",atoi(field));
    DBGPrintf("Function: %s",ks);
    action_internal(PF_action, IA_DEFAULT, ks, CN);

 }
*/

/*
 static gboolean double_click(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
    long 			cRow;
    long 			cCol;
    int  			rows;
    int  			cols;
    int  			baddr;
    const struct ea *trm;
    char			buffer[10];
    int				f;

    DBGPrintf("Button %d double-click at %ld,%ld", event->button,(unsigned long) event->x, (unsigned long) event->y);

    switch(event->button)
    {
    case 1:
       mouse_click = 0;
       Mouse2Terminal((long) event->x, (long) event->y, &cRow, &cCol);
       trm = Get3270DeviceBuffer(&rows, &cols);

       if(trm)
       {
          baddr = find_field_attribute((cRow * cols) + cCol);
          if(baddr > 0)
          {
             DBGPrintf("Field por position %ldx%ld: %d",cRow,cCol,baddr);
			 baddr++;
             for(f=0;f<10 && !trm[baddr].fa;f++)
             	buffer[f] =  ebc2asc[trm[baddr++].cc];

			 DBGTrace(f);
             if(f < 10)
             {
             	buffer[f] = 0;
             	if(*buffer == 'F')
             	   Check4Function(buffer+1);
             }
          }
       }
       break;

	case 3:
	   DBGMessage("Append!");
	   MouseMode = MOUSE_MODE_APPEND;
	   break;
    }


 	return 0;
 }
*/

/*
 static gboolean button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
    // http://developer.gnome.org/doc/API/2.0/gtk/GtkWidget.html#GtkWidget-button-press-event
 	int 	rows;
 	int 	cols;
 	long	cRow;
 	long	cCol;

    if(!mouse_click)
       return 0;

    Get3270DeviceBuffer(&rows, &cols);

    DBGPrintf("Button %d release at %ld,%ld", event->button,(unsigned long) event->x, (unsigned long) event->y);
    xFrom = yFrom = -1;


    switch(event->button)
    {
    case 1:
       if(MouseMode == MOUSE_MODE_NORMAL && Mouse2Terminal((long) event->x, (long) event->y, &cRow, &cCol))
       {
          action_remove_selection(0,0);
          move3270Cursor((cRow * cols) + cCol);
       }
       CheckForCopy();
       MouseMode = MOUSE_MODE_NORMAL;
       break;

	case 3:
	   break;
    }

    return 0;
 }
*/

/*
 static gboolean motion_notify(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
 {
 	long	cRow;
 	long	cCol;

 	if(event->state & GDK_BUTTON1_MASK)
 	{

       MouseMode = MOUSE_MODE_SELECTING;

       xTo = ((long) event->x);
       yTo = ((long) event->y);

       Mouse2Terminal(xTo, yTo, &cRow, &cCol);

       toRow = cRow;
       toCol = cCol;

       DBGPrintf("Box from %d,%d to %d,%d",min(toRow,fromRow),min(toCol,fromCol),max(toRow,fromRow),max(toCol,fromCol));
       gtk_widget_queue_draw(terminal);

 	}

 	return 0;
 }
*/

/*
#if GTK == 2

 static gboolean scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
 {
    // FIXME (perry#1#): Read associoation from scroll to function key from configuration file.
 	static const char *scroll[] = { "8", "7" };
 	DBGTracex(event->direction);

 	if(event->direction > 1 || WaitForScreen)
 	   return 0;

	// It's whell mouse action, execute function codes.
    WaitForScreen = TRUE;
    action_internal(PF_action, IA_DEFAULT, scroll[event->direction], CN);

 	return 0;
 }

#endif
*/

 static void SetFont(GtkWidget *widget, FONTELEMENT *fn, int width, int height)
 {
 	int rows = 0;
 	int cols = 0;

    Get3270DeviceBuffer(&rows, &cols);
    rows++;

 	font = fn;

    /* Calculate new left margin */

	left_margin = (width - (font->Width*cols)) >> 1;
 	if(left_margin < 0)
	   left_margin = 0;

	/* Calculate new line-spacing */
	line_spacing = (height - (font->Height*rows)) / rows;
	if(line_spacing < MIN_LINE_SPACING)
	   line_spacing = MIN_LINE_SPACING;

    top_margin = (height - (STATUS_LINE_SPACE + ((font->Height+line_spacing)*rows))) >> 1;
    if(top_margin < 0)
       top_margin = 0;

 }

 static void configure_terminal(GtkWidget *widget, int hWidth, int hHeight)
 {
 	int			rows;
 	int			cols;
 	int			width;
 	int			height;
 	FONTELEMENT *fn   	= 0;
 	int         f;

    Get3270DeviceBuffer(&rows, &cols);

 	width  = hWidth / cols;
 	height = (hHeight / (rows+2)) - MIN_LINE_SPACING;


    /* Search for a font with exact match */
    for(f = 0; f < FONT_COUNT && !fn;f++)
    {
       if( fontlist[f].fn && (fontlist[f].Width == width) && fontlist[f].Height <= height )
          fn = fontlist+f;
    }

    if(!fn)
    {
       /* No match, search for the near one */
       fn = fontlist;
       for(f = 0; f < FONT_COUNT;f++)
       {
          if( fontlist[f].fn  && fontlist[f].Width <= width
                              && fontlist[f].Width > fn->Width
                              && fontlist[f].Height <= height )
              {
                 fn = fontlist+f;
              }
       }
    }

    /* Reconfigure drawing box */
    SetFont(widget,fn,hWidth,hHeight);

 }

 static void configure_event(GtkWidget *widget, GdkEventConfigure *event, void *t)
 {
    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventConfigure
    configure_terminal(widget,event->width,event->height);
    ConfigureSelectionBox();
 }

 static void realize(GtkWidget *widget, void *t)
 {
#ifdef USE_GTKIMCONTEXT
    gtk_im_context_set_client_window(im,widget->window);
#endif
    LoadImages(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE(widget)]);
    LoadMousePointers(widget->window,widget->style->fg_gc[GTK_WIDGET_STATE(widget)]);
 }

 static gboolean focus_in_event(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
 	CHKPoint();
#ifdef USE_GTKIMCONTEXT
    gtk_im_context_focus_in(im);
#endif
 	return 0;
 }

 static gboolean focus_out_event(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
 	CHKPoint();
#ifdef USE_GTKIMCONTEXT
    gtk_im_context_focus_out(im);
#endif
 	return 0;
 }

 static void UpdateControlKeys(GdkEventKey *event)
 {
   static const struct _ctrltable
   {
           guint keyval;
           guint flag;
   } ctrltable[] =
   {
           { GDK_Shift_L,          GDK_SHIFT_MASK      },
           { GDK_Shift_R,          GDK_SHIFT_MASK      },
           { GDK_Alt_L,            GDK_ALT_MASK        },
           { GDK_Alt_R,            GDK_ALT_MASK        },
           { GDK_Control_L,        GDK_CONTROL_MASK    },
           { GDK_Control_R,        GDK_CONTROL_MASK    },
    };

	int   f;
    guint state = event->state & (GDK_SHIFT_MASK|GDK_ALT_MASK|GDK_CONTROL_MASK);

    for(f=0;f<(sizeof(ctrltable)/sizeof(struct _ctrltable));f++)
    {
    	if(event->keyval == ctrltable[f].keyval)
    	{
           if(event->type == GDK_KEY_PRESS)
              state |= ctrltable[f].flag;
		   else
              state &= ~ctrltable[f].flag;
    	}
    }

    if(state != oia_KeyboardState)
    {
    	oia_KeyboardState = state;
    	DBGTracex(oia_KeyboardState);
    	RedrawStatusLine();
    }
 }

 static guint last_keyval = 0;

 static gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	UpdateControlKeys(event);

 	if(KeyboardAction(widget,event,user_data))
 	{
#ifdef USE_GTKIMCONTEXT
       gtk_im_context_reset(im);
#endif
 	   return TRUE;
 	}

#ifdef USE_GTKIMCONTEXT
    if(gtk_im_context_filter_keypress(im,event))
       return TRUE;
#endif

    // Guarda tecla pressionada
    last_keyval = event->keyval;

    return FALSE;
 }

 static gboolean key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	UpdateControlKeys(event);

#ifdef USE_GTKIMCONTEXT
    if(gtk_im_context_filter_keypress(im,event))
       return TRUE;
#endif

    if(last_keyval == event->keyval && last_keyval == GDK_Control_L)
    {
    	DBGMessage("Left control");
    	action_Reset(0,0);
    }

 	return 0;
 }

#ifdef USE_GTKIMCONTEXT
 static void im_commit(GtkIMContext *imcontext, gchar *arg1, gpointer user_data)
 {
 	DBGPrintf("Commit: %s (%02x)", arg1,(unsigned int) *arg1);
	ParseInput(arg1);
 }
#endif

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
										FALSE,
										TRUE ))
		{
			Log("Can't allocate color \"%s\" from %s",ptr,id);
			rc = EINVAL;
		}

        f++;
    }
    return rc;
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

    int		  qtd;

 	char      *ptr;
 	char      *tok;
 	char      buffer[4096];

 	FILE	  *arq;

 	GtkWidget *ret;

 	if(!hostname)
 	   cl_hostname = hostname;

    gsource_init();

#ifdef USE_GTKIMCONTEXT
    im = gtk_im_context_simple_new();
#endif

    Initialize_3270();

    register_3270_schange(ST_CONNECT,		stsConnect);
    register_3270_schange(ST_EXITING,		stsExiting);
    register_3270_schange(ST_HALF_CONNECT,	stsHalfConnect);
    register_3270_schange(ST_RESOLVING,		stsResolving);
	register_3270_schange(ST_3270_MODE, 	sts3270Mode);

    /* Load font table */
    qtd = 0;

    arq = fopen(fontfile,"r");

    if(!arq)
    {
    	ErrorPopup("Unable to open \"%s\" for reading",fontfile);
    	return 0;
    }

    while(fgets((char *) buffer,1023,arq))
    {
	   for(ptr = buffer;*ptr && *ptr != '\n';ptr++);
	   *ptr = 0;

       for(ptr = buffer;*ptr && isspace(*ptr);ptr++);

       if(*ptr && *ptr != '#')
       {
          DBGMessage(buffer);
          if(!fontlist || (fontCount >= qtd) )
          {
		     qtd += 10;
		     if(fontlist)
		        fontlist = realloc(fontlist,sizeof(FONTELEMENT) * qtd);
			 else
			    fontlist = malloc(sizeof(FONTELEMENT) * qtd);

			 if(!fontlist)
			 {
			 	ErrorPopup("Can't allocate memory for font list");
			 	return 0;
			 }
          }

 		  fontlist[fontCount].fn = gdk_font_load(buffer);
		  if(fontlist[fontCount].fn)
		  {
		     gdk_text_extents(fontlist[fontCount].fn,"A",1,&lbearing,&rbearing,&width,&ascent,&descent);
		 	 fontlist[fontCount].Width  = width;
		  	 fontlist[fontCount].Height = (ascent+descent)+line_spacing;
             fontCount++;
		  }
	 	  else
		  {
			 Log("Error loading font %s",buffer);
		  }
       }
    }

    DBGTrace(fontCount);

    if(fontCount)
    {
	   fontlist = realloc(fontlist,sizeof(FONTELEMENT) * fontCount);
    }
    else
    {
    	ErrorPopup("Can't find fonts!");
    	return 0;
    }

    fclose(arq);

    /* Create drawing area */
    ret = gtk_drawing_area_new();
    GTK_WIDGET_SET_FLAGS(ret, GTK_CAN_DEFAULT);
    GTK_WIDGET_SET_FLAGS(ret, GTK_CAN_FOCUS);

    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Events.html#GdkEventMask
    gtk_widget_add_events(ret,GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK|GDK_BUTTON_RELEASE_MASK);


    // FIXME (perry#3#): Make it better! Get the smaller font, not the first one.
	SetFont(ret,fontlist,0,0);

    Get3270DeviceBuffer(&rows, &cols);
    DBGPrintf("Screen size: %dx%d",rows,cols);

    gtk_widget_set_size_request(ret, font->Width*(cols+1), (font->Height+MIN_LINE_SPACING)*(rows+1));

    g_signal_connect(G_OBJECT(ret), "expose_event",  		G_CALLBACK(expose),		  		0);
    g_signal_connect(G_OBJECT(ret), "key-press-event",		G_CALLBACK(key_press_event),	0);
    g_signal_connect(G_OBJECT(ret), "key-release-event",	G_CALLBACK(key_release_event),	0);
    g_signal_connect(G_OBJECT(ret), "configure-event",		G_CALLBACK(configure_event), 	0);
    g_signal_connect(G_OBJECT(ret), "realize",				G_CALLBACK(realize), 			0);
    g_signal_connect(G_OBJECT(ret), "focus-in-event",		G_CALLBACK(focus_in_event), 	0);
    g_signal_connect(G_OBJECT(ret), "focus-out-event",		G_CALLBACK(focus_out_event), 	0);

#ifdef USE_GTKIMCONTEXT
    g_signal_connect(G_OBJECT(im), "commit",				G_CALLBACK(im_commit), 			0);
#endif

    // Connect mouse events
    g_signal_connect(G_OBJECT(ret), "button-press-event",	G_CALLBACK(mouse_button_press),		0);
    g_signal_connect(G_OBJECT(ret), "button-release-event",	G_CALLBACK(mouse_button_release),	0);
    g_signal_connect(G_OBJECT(ret), "motion-notify-event",	G_CALLBACK(mouse_motion),			0);

#if GTK == 2
    g_signal_connect(G_OBJECT(ret), "scroll-event",			G_CALLBACK(mouse_scroll),		0);
#endif

    /* Load terminal colors */
    GetColorDescription("Terminal",buffer);
    for(ptr=strtok_r(buffer,",",&tok);ptr;ptr = strtok_r(0,",",&tok))
       terminal_color_count++;

    sz   		  = sizeof(GdkColor) * terminal_color_count;
    terminal_cmap =  g_malloc(sz);

    if(!terminal_cmap)
    {
    	ErrorPopup("Memory allocation error when creating %d colors",terminal_color_count);
    	return 0;
    }

    LoadColors(terminal_cmap, terminal_color_count, "Terminal");

    /* Load colors */
    LoadColors(field_cmap,FIELD_COLORS,"Fields");
    LoadColors(cursor_cmap,CURSOR_COLORS,"Cursor");
    LoadColors(selection_cmap,SELECTION_COLORS,"Selection");
    LoadColors(status_cmap, STATUS_COLORS, "Status");

    for(f=0;f < (sizeof(widget_states)/sizeof(int));f++)
    {
	   // http://ometer.com/gtk-colors.html
       gtk_widget_modify_bg(ret,widget_states[f],terminal_cmap);
       gtk_widget_modify_fg(ret,widget_states[f],terminal_cmap+4);
    }

    action_connect(ret,0);

    InitClipboard(ret);

    return ret;
 }

 void InvalidateCursor(void)
 {
    LockThreads();

#ifdef USE_GTKIMCONTEXT
    gtk_im_context_reset(im);
#endif

    if(terminal)
       gtk_widget_queue_draw(terminal);

    UnlockThreads();
 }

 void action_crosshair( GtkWidget *w, gpointer   data )
 {
 	cross_hair = !cross_hair;
 	DBGPrintf("Cross_hair: %s Cursor: %s",cross_hair ? "Yes" : "No",cursor_enabled ? "Yes" : "No");
    InvalidateCursor();
 }

 void EnableCursor(gboolean mode)
 {
 	cursor_enabled = mode;
 	DBGPrintf("Cross_hair: %s Cursor: %s",cross_hair ? "Yes" : "No",cursor_enabled ? "Yes" : "No");
    InvalidateCursor();
 }

 void SetCursorType(int type)
 {
    cursor_type = (type % CURSOR_TYPE_CROSSHAIR);
    InvalidateCursor();
 }

 void SetCursorPosition(int row, int col)
 {
#ifdef USE_GTKIMCONTEXT
    GdkRectangle area;
#endif

    InvalidateCursor();
    cursor_row = row;
    cursor_col = col;
    InvalidateCursor();

#ifdef USE_GTKIMCONTEXT
    memset(&area,0,sizeof(area));

    area.width	= font->Width;
    area.height	= (font->Height + line_spacing);
    area.x		= (cursor_col * area.width)  + left_margin;
    area.y		= (cursor_row * area.height) + (top_margin + font->Height);

    gtk_im_context_set_cursor_location(im,&area);
#endif
 }

 void RedrawTerminalContents(void)
 {
    if(terminal)
    {
       LockThreads();
       gtk_widget_queue_draw(terminal);
       UnlockThreads();
    }
 }


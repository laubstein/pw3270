
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
 #include "lib/togglesc.h"

/*---[ Defines ]--------------------------------------------------------------*/

 #define MIN_LINE_SPACING	2
 #define STATUS_LINE_SPACE	4

/*---[ Globals ]--------------------------------------------------------------*/

#ifdef DATADIR
  static const char *fontfile = DATADIR "/fonts.conf";
#else
  static const char *fontfile = "./fonts.conf";
#endif

 const char		*cl_hostname							= 0;

 gboolean			WaitForScreen							= TRUE;

 static FONTELEMENT *fontlist								= 0;
 static int			fontCount							= 0;

 #define FONT_COUNT fontCount

 FONTELEMENT 		*font									= 0;
 int				top_margin								= 0;
 int 				left_margin								= 0;

 int				line_spacing							= MIN_LINE_SPACING;

 static int		cursor_height[CURSOR_TYPE_CROSSHAIR]	= { 3, 6 };
 static gboolean   cursor_enabled							= TRUE;
 int				cursor_row								= 0;
 int				cursor_col								= 0;

 int				cursor_type								= CURSOR_TYPE_OVER;
 int				reconnect_retry							= 0;

#ifdef USE_GTKIMCONTEXT
 GtkIMContext		*im;
#endif

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
       reconnect_retry = toggled(RECONNECT) ? 0 : -1;

	   if (GetKeyboardStatus() & KL_AWAITING_FIRST)
	      SetOIAStatus(STATUS_AWAITING_FIRST);
       else
	      SetOIAStatus(STATUS_CONNECTED);
       ctlr_erase(True);
       EnableCursor(TRUE);
    }
    else
    {
       SetOIAStatus(STATUS_DISCONNECTED);
       EnableCursor(FALSE);
       ctlr_erase(True);

	   DBGTrace(reconnect_retry);

	   if(reconnect_retry >= 0)
	   {
		  snprintf(key,39,"HOST3270_%d",reconnect_retry++);
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

 static gboolean expose(GtkWidget *widget, GdkEventExpose *event, void *t)
 {
    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventExpose
    int		cRow;
    int		cCol;
    int		rows;
    int		cols;
    GdkGC	*gc		= widget->style->fg_gc[GTK_WIDGET_STATE(widget)];
    int		vPos	= (top_margin + font->Height);
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

       if(toggled(CROSSHAIR))
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

 static int ReadFont(const char *buffer, int fontCount, int *qtd)
 {
    gint 	  lbearing	= 0;
    gint 	  rbearing	= 0;
    gint 	  width		= 0;
    gint 	  ascent	= 0;
    gint 	  descent	= 0;

    if(!fontlist || (fontCount >= *qtd) )
    {
       (*qtd) += 10;
       if(fontlist)
	      fontlist = realloc(fontlist,sizeof(FONTELEMENT) * (*qtd));
	   else
		  fontlist = malloc(sizeof(FONTELEMENT) * (*qtd));

	   if(!fontlist)
	   {
	      ErrorPopup( _( "Can't allocate memory for font list" ) );
	      *qtd = -1;
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
	return fontCount;
 }

 GtkWidget *g3270_new(const char *hostname)
 {
    static const int widget_states[] = { GTK_STATE_NORMAL, GTK_STATE_ACTIVE, GTK_STATE_PRELIGHT, GTK_STATE_SELECTED, GTK_STATE_INSENSITIVE };

 	int		  f;
    int		  rows		= 0;
    int		  cols		= 0;

    int		  qtd		= 0;

 	char      *ptr;
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
    arq = fopen(fontfile,"r");

    if(!arq)
    {
    	ErrorPopup( _( "Unable to open \"%s\" for reading" ),fontfile);
    	return 0;
    }

    while(fgets((char *) buffer,1023,arq))
    {
	   for(ptr = buffer;*ptr && *ptr != '\n';ptr++);
	   *ptr = 0;

       for(ptr = buffer;*ptr && isspace(*ptr);ptr++);

       if(*ptr && *ptr != '#')
       {
	      fontCount = ReadFont(buffer,fontCount,&qtd);
	      if(qtd < 0)
	         return 0;
       }
    }

    DBGTrace(fontCount);

    if(!fontCount)
    {
    	int t;
    	static const char *DefaultFont[] = { "terminus", "fixed", "courier" };

		for(t=0;!fontCount && t < (sizeof(DefaultFont)/sizeof(const char *)); t++)
		{
			Log("Loading default font \"%s\"",DefaultFont[t]);

    	    for(f=6;f<40;f++)
    	    {
		       char buffer[80];
    		   snprintf(buffer,79,"-*-%s-*-*-*-*-%d-*-*-*-*-*-*-*",DefaultFont[t],f);
			   fontCount = ReadFont(buffer,fontCount,&qtd);
	           if(qtd < 0)
	              return 0;
    	    }
    	}

	    DBGTrace(fontCount);

    }

    if(fontCount)
    {
	   fontlist = realloc(fontlist,sizeof(FONTELEMENT) * fontCount);
    }
    else
    {
    	ErrorPopup( _( "Can't find fonts!" ) );
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
    g_signal_connect(G_OBJECT(ret), "scroll-event",			G_CALLBACK(mouse_scroll),		0);

    /* Set default terminal colors */
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
	CHKPoint();
    LockThreads();

#ifdef USE_GTKIMCONTEXT
    gtk_im_context_reset(im);
#endif

    if(terminal)
       gtk_widget_queue_draw(terminal);

	CHKPoint();
    UnlockThreads();
 }

 void EnableCursor(gboolean mode)
 {
 	cursor_enabled = mode;
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

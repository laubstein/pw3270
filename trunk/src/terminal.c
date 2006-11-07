
 #include "g3270.h"
 #include <gdk/gdkkeysyms.h>

 #include "lib/hostc.h"
 #include "lib/kybdc.h"
 #include "lib/actionsc.h"

/*---[ Defines ]--------------------------------------------------------------*/

 #define TERMINAL_HPAD 2
 #define TERMINAL_VPAD 2


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

  // TODO (perry#2#): Read from file(s).
  static const char *FontDescr[] =
  {

		"-xos4-terminus-medium-*-normal-*-12-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-14-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-16-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-20-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-24-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-28-*-*-*-*-*-*-*",
	 	"-xos4-terminus-medium-*-normal-*-32-*-*-*-*-*-*-*",

		"-xos4-terminus-bold-*-*-*-12-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-14-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-16-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-20-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-24-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-28-*-*-*-*-*-*-*",
	 	"-xos4-terminus-bold-*-*-*-32-*-*-*-*-*-*-*"

  };

  #define FONT_COUNT (sizeof(FontDescr)/sizeof(const char *))

/*---[ Globals ]--------------------------------------------------------------*/

 const char			*cl_hostname	= 0;

 static FONTELEMENT *fontlist		= 0;
 static FONTELEMENT *font			= 0;
 static int			top_margin		= 0;
 static int 		left_margin		= 0;
 static int			cursor_row		= 0;
 static int			cursor_col		= 0;

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
 	int				hPos;
 	int				vPos;
 	char			chr[2];

    GdkGC 			*gc;

    gboolean		rc		= FALSE;

 	int				left	= (event->area.x - font->Width);
 	int				right	= left + event->area.width + (font->Width << 1);

 	int				top		= (event->area.y - font->Height);
 	int				bottom	= top + event->area.height + (font->Height << 1);

//    DBGPrintf("H %d<->%d V %d<->%d",left,right,top,bottom);

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
    		if(  hPos >= left && hPos <= right && vPos >= top && vPos <= bottom)
    		{
    			/* It's inside the drawing area, redraw */
    			gc     = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];

    			chr[0] = Ebc2ASC(trm->cc);
    			chr[1] = 0;

    			/* Detect the right color */



    		    rc  = TRUE;
			    gdk_draw_text(	widget->window,font->fn,gc,hPos,vPos,chr,1);
    		}

			hPos += font->Width;
	        trm++;
    	}
    	vPos += font->Height;
    }

    return rc;
 }

 static gboolean key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
    /*
     * http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventKey
     *
     * typedef struct {
     *    GdkEventType type;
     *    GdkWindow *window;
     *    gint8 send_event;
     *    guint32 time;
     *    guint state;
     *    guint keyval;
     *    gint length;
     *    gchar *string;
     *    guint16 hardware_keycode;
     *    guint8 group;
     *    guint is_modifier : 1;
     *  } GdkEventKey;
     *
     */

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

    DBGTracex(event->keyval);

//    const struct ea	*ea			= QueryDeviceChar(-1);
//	DBGPrintf("Key: %s %c (%d)", event->string, Ebc2ASC(ea->cc), event->keyval);

    // /opt/gnome/include/gtk-2.0/gdk/gdkkeysyms.h

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

/*
    if(event->keyval < 0xFFFF)
    {
	   snprintf(ks,5,"0x%x",event->keyval);
	   params[0] = ks;
	   params[1] = 0;
       Key_action(NULL, NULL, params, &one);
 	   return TRUE;
	}
*/

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

 	if(width)
       left_margin = (width - (font->Width*cols)) >> 1;
	else
	   left_margin = 0;

	if(height)
       top_margin = (height - (font->Height*rows)) >> 1;
	else
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

 	width = (allocation->width-(TERMINAL_HPAD<<1)) / cols;

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
           fontlist[f].Height = (ascent+descent)+2;
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
    gtk_widget_set_size_request(ret, (font->Width*cols)+(TERMINAL_HPAD<<1), (font->Height*rows)+(TERMINAL_VPAD<<1));

    g_signal_connect(G_OBJECT(ret), "expose_event",  		G_CALLBACK(expose),		  0);
    g_signal_connect(G_OBJECT(ret), "size-allocate",		G_CALLBACK(resize),	   	  0);
    g_signal_connect(G_OBJECT(ret), "key-release-event",	G_CALLBACK(key_release),  0);
    g_signal_connect(G_OBJECT(ret), "button-press-event",	G_CALLBACK(button_press), 0);

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
 }


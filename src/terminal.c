
 #include "g3270.h"
 #include "lib/hostc.h"

/*---[ Defines ]--------------------------------------------------------------*/

 #define TERMINAL_HPAD 2
 #define TERMINAL_VPAD 2

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
 	const struct ea *trm;
 	int				rows;
 	int				cols;
 	int				row;
 	int				col;
 	int				hPos;
 	int				vPos;
 	char			chr;

 	trm = Get3270DeviceBuffer(&rows, &cols);

    // FIXME (perry#2#): Draw only the needed area
    DBGPrintf("Redraw %dx%d (%p)",rows,cols,trm);

    vPos = top_margin;
    for(row = 0; row < rows; row++)
    {
    	hPos = left_margin;
    	for(col = 0; col < cols; col++)
    	{
    		chr = GetASCIICharacter(trm);
			gdk_draw_text(	widget->window,
							font->fn,
							widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
							hPos,vPos,
							&chr,
							1);

			hPos += font->Width;
	        trm++;
    	}
    	vPos += font->Height;
    }

    return TRUE;
 }

 static void SetFont(GtkWidget *widget, FONTELEMENT *fn)
 {
 	font = fn;
    DBGPrintf("Reconfiguring for font %p with width %d",fn,fn->Width);
 }

 static void SetMargins(GtkWidget *widget, FONTELEMENT *fn, int width, int height)
 {
 	int rows = 0;
 	int cols = 0;

    Get3270DeviceBuffer(&rows, &cols);

    SetFont(widget,fn);
    left_margin = (width - (font->Width*cols)) >> 1;
    top_margin  = (height - (font->Height*rows)) >> 1;
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
		   SetMargins(widget,fontlist+f,allocation->width,allocation->height);
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
    SetMargins(widget,fn,allocation->width,allocation->height);

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

    // FIXME (perry#3#): Make it better! Get the smaller font, not the first one.
	SetFont(ret,fontlist);

    Get3270DeviceBuffer(&rows, &cols);
    DBGPrintf("Screen size: %dx%d",rows,cols);
    gtk_widget_set_size_request(ret, (font->Width*cols)+(TERMINAL_HPAD<<1), (font->Height*rows)+(TERMINAL_VPAD<<1));

    g_signal_connect(G_OBJECT(ret), "expose_event",  G_CALLBACK(expose), 0);
    g_signal_connect(G_OBJECT(ret), "size-allocate", G_CALLBACK(resize), 0);

    /* Finish */

	// TODO (perry#3#): Start connection in background.
    if(cl_hostname)
    {
       g3270_log(TARGET, "Connecting to \"%s\"",cl_hostname);
       host_connect(cl_hostname);
    }

    return ret;
 }


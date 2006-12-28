
 #include "g3270.h"
 #include <string.h>

 #include "lib/kybdc.h"
 #include "lib/3270ds.h"
 #include "lib/tablesc.h"
 #include "lib/hostc.h"
 #include "lib/telnetc.h"

/*---[ Defines ]--------------------------------------------------------------*/

 #define DEFCOLOR_MAP(f) ((((f) & FA_PROTECT) >> 4) | (((f) & FA_INT_HIGH_SEL) >> 3))

/*---[ Status codes ]---------------------------------------------------------*/

 #ifdef DEBUG
     #define DECLARE_STATUS_MESSAGE(code, color, msg) { code, #code, color, msg }
     #define NO_STATUS_MESSAGE ""
 #else
     #define DECLARE_STATUS_MESSAGE(code, color, msg) { code, color, msg }
     #define NO_STATUS_MESSAGE 0
 #endif

 static struct _status
 {
 	unsigned short	code;
#ifdef DEBUG
    const char		*dbg;
#endif
	unsigned short	color;
 	const char		*msg;
 } status[] =
 {
	DECLARE_STATUS_MESSAGE( STATUS_DISCONNECTED,	STATUS_COLOR_WARNING,	"X Desconectado"	),
	DECLARE_STATUS_MESSAGE( STATUS_RESOLVING,		STATUS_COLOR_ERROR,		"X Resolvendo"		),
	DECLARE_STATUS_MESSAGE( STATUS_CONNECTING,		STATUS_COLOR_ERROR,		"X Conectando"		),
	DECLARE_STATUS_MESSAGE( STATUS_NONSPECIFIC,		STATUS_COLOR_WARNING,	NO_STATUS_MESSAGE	),
	DECLARE_STATUS_MESSAGE( STATUS_INHIBIT,			STATUS_COLOR_ERROR,		"X Inhibit"			),
	DECLARE_STATUS_MESSAGE( STATUS_BLANK,			STATUS_COLOR_TIME,		NO_STATUS_MESSAGE	),

	DECLARE_STATUS_MESSAGE( STATUS_TWAIT,			STATUS_COLOR_TIME,		"X Aguarde"			),
	DECLARE_STATUS_MESSAGE( STATUS_AWAITING_FIRST,	STATUS_COLOR_CONNECTED,	"X Conectando"		),

	DECLARE_STATUS_MESSAGE( STATUS_SYSWAIT,			STATUS_COLOR_TIME,		"X System"			),
	DECLARE_STATUS_MESSAGE( STATUS_PROTECTED,		STATUS_COLOR_ERROR,		"X Protegido"		),
	DECLARE_STATUS_MESSAGE( STATUS_NUMERIC,			STATUS_COLOR_ERROR,		"X Numerico"		),
	DECLARE_STATUS_MESSAGE( STATUS_OVERFLOW,		STATUS_COLOR_ERROR,		"X Overflow"		),
	DECLARE_STATUS_MESSAGE( STATUS_DBCS,			STATUS_COLOR_ERROR,		"X DBCS"			),
	DECLARE_STATUS_MESSAGE( STATUS_SCROLLED,		STATUS_COLOR_ERROR,		"X Scrolled"		),
	DECLARE_STATUS_MESSAGE( STATUS_MINUS,			STATUS_COLOR_ERROR,		"X -f"				),
	DECLARE_STATUS_MESSAGE( STATUS_RECONNECTING,	STATUS_COLOR_WARNING,	"X Reconnecting"				),

 };

 static struct _status *current_status = 0;

/*---[ Globals ]--------------------------------------------------------------*/

 guint oia_KeyboardState = 0;

/*---[ Status icons ]---------------------------------------------------------*/

 #include "locked.bm"

 static const struct _imagedata
 {
	const unsigned char	*data;
    gint				width;
    gint				height;
    short				color;
 } imagedata[] =
 {
 	{ locked_bits, locked_width, locked_height, STATUS_COLOR_SSL }
 };

 #define IMAGE_COUNT (sizeof(imagedata)/sizeof(struct _imagedata))

 static GdkPixbuf *pix[IMAGE_COUNT];

 void LoadImages(GdkDrawable *drawable, GdkGC *gc)
 {
 	int			f;
 	GdkPixmap	*temp;

 	for(f=0;f<IMAGE_COUNT;f++)
 	{
 		// Load bitmap setting the right colors
 		temp = gdk_pixmap_create_from_data(	drawable,
											(const gchar *) imagedata[f].data,
                                       		imagedata[f].width,
                                       		imagedata[f].height,
											gdk_drawable_get_depth(drawable),
											status_cmap+imagedata[f].color,
											terminal_cmap );

		pix[f] = gdk_pixbuf_get_from_drawable(	0,
												temp,
												gdk_drawable_get_colormap(drawable),
												0,0,
												0,0,
												imagedata[f].width,
												imagedata[f].height );

        gdk_pixmap_unref(temp);
 	}
 }

/*---[ Implement ]------------------------------------------------------------*/

 static void SetColorAttribute(GdkGC *gc, const struct ea *trm, GdkColor **fgColor, GdkColor **bgColor)
 {
 	int 		fg	= (trm->fg % terminal_color_count);
 	int 		bg	= (trm->bg % terminal_color_count);
 	GdkColor	*clr;

    *fgColor = (terminal_cmap + fg);
 	*bgColor = (terminal_cmap + bg);

    if(*bgColor && (trm->gr & GR_REVERSE))
    {
	   // Reverse colors
	   clr 		= *fgColor;

	   if(*bgColor)
	      *fgColor = *bgColor;
	   else
	      *fgColor = (terminal_cmap);

	   *bgColor = clr;
       return;
    }

    if(*bgColor == terminal_cmap)
       *bgColor = 0;

 }

#ifdef DEBUG
 static void DrawImage(GdkDrawable *drawable, GdkGC *gc, int id, int x, int y, int Height)
 {
    gdk_pixbuf_render_to_drawable(pix[id],drawable,gc,0,0,x,(y-Height)+1,-1,-1,GDK_RGB_DITHER_NORMAL,0,0);
 }
#endif

 void DrawTerminal(GdkDrawable *drawable, GdkGC *gc, const FONTELEMENT *font, int left, int top, int line_spacing)
 {
 	// Draw terminal data
 	const struct ea *trm;
 	int				rows;
 	int				cols;
 	int				row;
 	int				col;
 	int				hPos;
 	int				vPos;
 	unsigned char	chr[2];

    gboolean		rc			= FALSE;
    int				mode		= 0;
    GdkColor		*fg			= terminal_cmap+1;
    GdkColor		*bg			= 0;

    if(!font->fn)
    {
       Log("Invalid FONT definition");
       return;
    }

    trm = Get3270DeviceBuffer(&rows, &cols);

    /* Get top of the screen */
    vPos = (top + font->Height);

    if(trm)
    {
       for(row = 0; row < rows; row++)
       {
          hPos = left;
          for(col = 0; col < cols; col++)
          {
             chr[0] = ebc2asc[trm->cc];
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
                   chr[0] = 0;
                   SetColorAttribute(gc, trm, &fg, &bg);
                }
                else
                {
                   fg = field_cmap+(DEFCOLOR_MAP(trm->fa));
                   bg = 0;
                }
             }
             else if(trm->gr || trm->fg || trm->bg)
             {
                chr[0] = ' ';
                SetColorAttribute(gc, trm, &fg, &bg);
             }

             if(chr[0])
             {
                if(bg)
                {
                   gdk_gc_set_foreground(gc,bg);
                   gdk_draw_rectangle(drawable,gc,1,hPos,vPos-font->Height,font->Width,font->Height+line_spacing);
                }

                if(fg)
                   gdk_gc_set_foreground(gc,fg);

                switch(mode)
                {
                case 0:	// Nothing special
                   gdk_draw_text(drawable,font->fn,gc,hPos,vPos,(gchar *) chr,1);
                   break;

                case 1:  // Hidden
                   gdk_draw_text(drawable,font->fn,gc,hPos,vPos," ",1);
                   break;

                case 2:	// Hidden/Editable
                   break;

                }
             }

             hPos += font->Width;
             trm++;
           }
          vPos += (font->Height + line_spacing);
       }
    }

    /*
     * The status line is laid out thusly (M is maxCOLS):
     *
     *   0          "4" in a square
     *   1          "A" underlined
     *   2          solid box if connected, "?" in a box if not
     *   3..7       empty
     *   8...       message area
     *   M-41       Meta indication ("M" or blank)
     *   M-40       Alt indication ("A" or blank)
     *   M-39       Shift indication (Special symbol/"^" or blank)
     *   M-38..M-37 empty
     *   M-36       Compose indication ("C" or blank)
     *   M-35       Compose first character
     *   M-34       empty
     *   M-33       Typeahead indication ("T" or blank)
     *   M-32       SSL Status
     *   M-31       Alternate keymap indication ("K" or blank)
     *   M-30       Reverse input mode indication ("R" or blank)
     *   M-29       Insert mode indication (Special symbol/"I" or blank)
     *   M-28       Printer indication ("P" or blank)
     *   M-27       Script indication ("S" or blank)
     *   M-26..M-16 empty
     *   M-15..M-9  command timing (Clock symbol and m:ss, or blank)
     *   M-7..M     cursor position (rrr/ccc or blank)
     *
     */

#define DrawStatusRight(x,color,string) gdk_gc_set_foreground(gc,status_cmap+color); \
										gdk_draw_text(drawable,font->fn,gc,left+(((cols-x) * font->Width)),vPos,string,strlen(string))

#define DrawStatusLeft(x,color,string)  gdk_gc_set_foreground(gc,status_cmap+color); \
										gdk_draw_text(drawable,font->fn,gc,left+(x * font->Width),vPos,string,strlen(string))

    vPos -= font->Height;
    vPos++;

    gdk_gc_set_foreground(gc,status_cmap+STATUS_COLOR_SEPARATOR);
    gdk_draw_line( drawable, gc, left, vPos, hPos, vPos);

    vPos += (font->Height+1);

    /* Cursor position */
    DrawStatusRight(7,STATUS_COLOR_CURSOR_POSITION,oia_cursor);

    /* Insert mode */
    if(cursor_type == CURSOR_TYPE_INSERT)
    {
       DrawStatusRight(29,STATUS_COLOR_TOOGLE,"I");
    }

    /* LU Name */
    DrawStatusRight(25,STATUS_COLOR_LUNAME,oia_LUName);

    /* Timer */
    DrawStatusRight(15,STATUS_COLOR_TIME,oia_Timer);

    /* Keyboard status */

    DrawStatusRight(40,STATUS_COLOR_KEYBOARD,(oia_KeyboardState & GDK_ALT_MASK) ? "A" : " ");
    DrawStatusRight(39,STATUS_COLOR_KEYBOARD,(oia_KeyboardState & GDK_SHIFT_MASK) ? "^" : " ");

    /* SSL Status */
    if(query_secure_connection())
    {
       DrawStatusRight(32,STATUS_COLOR_SSL,"s");
    }

    /* Terminal Status */
    if(current_status && current_status->msg)
    {
       DrawStatusLeft(8,current_status->color,current_status->msg);

#ifdef DEBUG
	   if(!*current_status->msg)
	   {
          DrawStatusLeft(8,current_status->color,current_status->dbg);
	   }
#endif

    }

#ifdef DEBUG
	DrawImage(drawable, gc, 0, left+(((cols-32) * font->Width)), vPos, font->Height);
#endif

 }

 void SetOIAStatus(int code)
 {
    int 			f;
    struct _status	*sts	= 0;

	if(code < 0)
	{
		current_status = 0;
  	    DBGMessage("Status:\tNONE");
	}
	else
	{
	   for(f=0;!sts && f<(sizeof(status)/sizeof(struct _status));f++)
	   {
		   if(status[f].code == code)
		   {
		   	   current_status = sts = (status+f);
			   DBGPrintf("Status:\t%s",current_status->dbg);
		   }
	   }
	   if(!sts)
	      Log("Unexpected status code %d from 3270 library",code);
	}

    RedrawStatusLine();

 }


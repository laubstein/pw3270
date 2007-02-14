
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
     #define DECLARE_STATUS_MESSAGE(code, cursor, color, msg) { code, 0, cursor, #code, color, msg }
     #define NO_STATUS_MESSAGE ""
 #else
     #define DECLARE_STATUS_MESSAGE(code, cursor, color, msg) { code, 0, cursor, color, msg }
     #define NO_STATUS_MESSAGE 0
 #endif

 #define STATUS_CURSOR_NONE	-1

 static struct _status
 {
 	unsigned short	code;
 	GdkCursor		*cursor;
 	int				cursor_type;

#ifdef DEBUG
    const char		*dbg;
#endif
	unsigned short	color;
 	const char		*msg;
 } status[] =
 {
	DECLARE_STATUS_MESSAGE(
			STATUS_DISCONNECTED,
			GDK_ARROW,
			STATUS_COLOR_WARNING,
			"XDesconectado"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_RESOLVING,
			GDK_WATCH,
			STATUS_COLOR_ERROR,
			"XResolvendo"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_CONNECTING,
			GDK_WATCH,
			STATUS_COLOR_ERROR,
			"XConectando"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_NONSPECIFIC,
			GDK_XTERM,
			STATUS_COLOR_WARNING,
			NO_STATUS_MESSAGE
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_INHIBIT,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_ERROR,
			"XInhibit"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_BLANK,
			GDK_XTERM,
			STATUS_COLOR_TIME,
			NO_STATUS_MESSAGE
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_TWAIT,
			GDK_WATCH,
			STATUS_COLOR_TIME,
			"XAguarde"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_AWAITING_FIRST,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_CONNECTED,
			"XConectando"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_SYSWAIT,
			GDK_WATCH,
			STATUS_COLOR_TIME,
			"XSystem"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_PROTECTED,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_ERROR,
			"XProtegido"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_NUMERIC,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_ERROR,
			"XNumerico"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_OVERFLOW,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_ERROR,
			"XOverflow"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_DBCS,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_ERROR,
			"X DBCS"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_SCROLLED,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_ERROR,
			"X Scrolled"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_MINUS,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_ERROR,
			"X -f"
	),

	DECLARE_STATUS_MESSAGE(
			STATUS_RECONNECTING,
			STATUS_CURSOR_NONE,
			STATUS_COLOR_WARNING,
			"X Reconnecting"
	),

 };

 static struct _status *current_status = 0;

/*---[ Globals ]--------------------------------------------------------------*/

 guint oia_KeyboardState = 0;

/*---[ Status icons ]---------------------------------------------------------*/

 #include "locked.bm"
 #include "unlocked.bm"
 #include "shift.bm"
 #include "a.bm"
 #include "b.bm"
 #include "four.bm"

 static const struct _imagedata
 {
	const unsigned char	*data;
    gint				width;
    gint				height;
    short				color;
 } imagedata[] =
 {
 	{ locked_bits,   locked_width,   locked_height,   STATUS_COLOR_SSL 		},
 	{ unlocked_bits, unlocked_width, unlocked_height, STATUS_COLOR_SSL 		},
 	{ shift_bits,    shift_width,    shift_height,    STATUS_COLOR_KEYBOARD },
 	{ a_bits,        a_width,        a_height,    	  STATUS_COLOR_CNCT     },
 	{ b_bits,        b_width,        b_height,    	  STATUS_COLOR_CNCT     },
 	{ four_bits,     four_width,     four_height,  	  STATUS_COLOR_CNCT     },
 };

 #define IMAGE_COUNT (sizeof(imagedata)/sizeof(struct _imagedata))

 static struct _pix
 {
 	GdkPixbuf *base;
 	GdkPixbuf *pix;
 	int		   Width;
 	int		   Height;
 } pix[IMAGE_COUNT];

/*---[ Implement ]------------------------------------------------------------*/

 static void SetColorAttribute(GdkGC *gc, const struct ea *trm, GdkColor **fgColor, GdkColor **bgColor)
 {
 	int 		fg	= (trm->fg % TERMINAL_COLORS);
 	int 		bg	= (trm->bg % TERMINAL_COLORS);
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

 static void DrawImage(GdkDrawable *drawable, GdkGC *gc, int id, int x, int y, int Height, int Width)
 {
    double ratio;
    int    temp;

 	if( ((Height != pix[id].Height) || (Width != pix[id].Width)) && pix[id].pix )
 	{
 		gdk_pixbuf_unref(pix[id].pix);
		pix[id].pix = 0;
 	}

 	if(!pix[id].pix)
 	{
 		/* Resize by Height */
        ratio = ((double) gdk_pixbuf_get_width(pix[id].base)) / ((double) gdk_pixbuf_get_height(pix[id].base));
		temp  = (int) ((double) ratio * ((double) Height));

        DBGPrintf("Ratio: %ld %dx%d <-> %dx%d (%dx%d)",
									(long) (ratio * 100),
									gdk_pixbuf_get_width(pix[id].base),gdk_pixbuf_get_height(pix[id].base),
									temp,Height,
									Width,Height);

	    pix[id].pix = gdk_pixbuf_scale_simple(pix[id].base,temp,Height,GDK_INTERP_HYPER);
 	}

    if(pix[id].pix)
    {
   	   pix[id].Height = Height;
	   pix[id].Width  = Width;
	   gdk_pixbuf_render_to_drawable(pix[id].pix,drawable,gc,0,0,x,(y-Height)+2,-1,-1,GDK_RGB_DITHER_NORMAL,0,0);
    }
 }

 void DrawTerminal(GdkDrawable *drawable, GdkGC *gc, const FONTELEMENT *font, int left, int top, int line_spacing)
 {
    int StatusLine;

    if(!font->fn)
    {
       Log("Invalid FONT definition");
       return;
    }

    StatusLine = PaintBuffer(drawable, gc, font, left, top, line_spacing);
    PaintStatus(StatusLine,drawable, gc, font, left, top, line_spacing);
 }

 int PaintBuffer(GdkDrawable *drawable, GdkGC *gc, const FONTELEMENT *font, int left, int top, int line_spacing)
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

    vPos -= font->Height;
    vPos++;

    gdk_gc_set_foreground(gc,status_cmap+STATUS_COLOR_SEPARATOR);
    gdk_draw_line( drawable, gc, left, vPos, hPos, vPos);

    vPos += (font->Height+1);

    return vPos;
 }

 void PaintStatus(int vPos, GdkDrawable *drawable, GdkGC *gc, const FONTELEMENT *font, int left, int top, int line_spacing)
 {
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

#define DrawIconRight(x,i,color,string) DrawImage(drawable, gc, i,left+(((cols-x) * font->Width)), vPos, font->Height, font->Width);

#define DrawStatusRight(x,color,string) gdk_gc_set_foreground(gc,status_cmap+color); \
										gdk_draw_text(drawable,font->fn,gc,left+(((cols-x) * font->Width)),vPos,string,strlen(string))

#define DrawStatusLeft(x,color,string)  gdk_gc_set_foreground(gc,status_cmap+color); \
										gdk_draw_text(drawable,font->fn,gc,left+(x * font->Width),vPos,string,strlen(string))

 	int				rows;
 	int				cols;

    Get3270DeviceBuffer(&rows, &cols);


    if(oia_undera)
    {
        DrawImage(drawable,
						gc,
						(IN_E ? 4 : 3),
						(left+font->Width)+1,
						vPos,
						font->Height,
						font->Width
				  );
    }

    DrawImage(drawable,gc,5,left,vPos,font->Height,font->Width);

    gdk_gc_set_foreground(gc,status_cmap+STATUS_COLOR_CONNECT_ICON);
	gdk_draw_rectangle(	drawable,gc,
						CONNECTED ? 1 : 0,
						(left+(2 * font->Width)+4),vPos-(font->Height-1),font->Width+1,font->Height);
	gdk_draw_text(drawable,font->fn,gc,left+(2 * font->Width)+5,vPos,"?",1);

    /* Cursor position & Insert mode */
    if(*oia_cursor) // Is cursor enabled?
    {
       /* Cursor position */
       DrawStatusRight(7,STATUS_COLOR_CURSOR_POSITION,oia_cursor);

       /* Insert mode */
       if(cursor_type == CURSOR_TYPE_INSERT)
       {
          DrawStatusRight(29,STATUS_COLOR_TOOGLE,"I");
       }
    }

    /* Typeahead indicator */
    DrawStatusRight(33,STATUS_COLOR_TYPEAHEAD,oia_Typeahead);

    /* LU Name */
    DrawStatusRight(25,STATUS_COLOR_LUNAME,oia_LUName);

    /* Timer */
    DrawStatusRight(15,STATUS_COLOR_TIME,oia_Timer);

    /* Keyboard status */
    DrawStatusRight(40,STATUS_COLOR_KEYBOARD,(oia_KeyboardState & GDK_ALT_MASK) ? "A" : " ");

    if(oia_KeyboardState & GDK_SHIFT_MASK)
    {
       DrawIconRight(39,2,STATUS_COLOR_SSL,"^");
    }

    /* SSL Status */
    if(query_secure_connection())
    {
       DrawIconRight(32,0,STATUS_COLOR_SSL,"s");
    }
	else
	{
       DrawIconRight(32,1,STATUS_COLOR_SSL," ");
	}

    /* Terminal Status */
    if(current_status && current_status->msg)
    {

	   int col = left+(8*font->Width);

       gdk_gc_set_foreground(gc,status_cmap+current_status->color);

       if(*current_status->msg == 'X')
       {
       	  int        f;
       	  int        xCols = 3;
       	  const char *ptr  = current_status->msg+1;

       	  for(f=0;f < xCols;f++)
       	  {
		     gdk_draw_line(drawable,gc,col+f,vPos-(font->Height-1), col+f+(font->Width-xCols),vPos);
		     gdk_draw_line(drawable,gc,col+f+(font->Width-xCols),vPos-(font->Height-1),col+f,vPos);
       	  }

		  gdk_draw_text(drawable,font->fn,gc,col+font->Width+(font->Width >> 1),vPos,ptr,strlen(ptr));
       }
       else if(*current_status->msg)
       {
		  gdk_draw_text(drawable,font->fn,gc,col,vPos,current_status->msg,strlen(current_status->msg));
       }

#ifdef DEBUG
	   if(!*current_status->msg)
	   {
          DrawStatusLeft(8,current_status->color,current_status->dbg);
	   }
#endif

    }

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
			   SetMousePointer(sts->cursor);
               RedrawStatusLine();
			   return;
		   }
	   }
       Log("Unexpected status code %d from 3270 library",code);
	}

 }

 void LoadImages(GdkDrawable *drawable, GdkGC *gc)
 {
 	int			f;
 	GdkPixmap	*temp;

    memset(pix,0,sizeof(struct _pix) * IMAGE_COUNT);

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

		pix[f].base = gdk_pixbuf_get_from_drawable(	0,
													temp,
													gdk_drawable_get_colormap(drawable),
													0,0,
													0,0,
													imagedata[f].width,
													imagedata[f].height );

        gdk_pixmap_unref(temp);
 	}

    for(f=0;f<(sizeof(status)/sizeof(struct _status));f++)
	{
	   if(status[f].cursor_type != STATUS_CURSOR_NONE)
	      status[f].cursor = gdk_cursor_new(status[f].cursor_type);
	}

 }

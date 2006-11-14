
 #include "g3270.h"
 #include <string.h>

 #include "lib/kybdc.h"
 #include "lib/3270ds.h"


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
	DECLARE_STATUS_MESSAGE( STATUS_DISCONNECTED,	STATUS_COLOR_WARNING,	"X Not Connected"	),
	DECLARE_STATUS_MESSAGE( STATUS_RESOLVING,		STATUS_COLOR_ERROR,		"X Resolving"		),
	DECLARE_STATUS_MESSAGE( STATUS_CONNECTING,		STATUS_COLOR_ERROR,		"X Connecting"		),
	DECLARE_STATUS_MESSAGE( STATUS_NONSPECIFIC,		STATUS_COLOR_WARNING,	NO_STATUS_MESSAGE	),
	DECLARE_STATUS_MESSAGE( STATUS_INHIBIT,			STATUS_COLOR_ERROR,		"X Inhibit"			),
	DECLARE_STATUS_MESSAGE( STATUS_BLANK,			STATUS_COLOR_TIME,		NO_STATUS_MESSAGE	),
	DECLARE_STATUS_MESSAGE( STATUS_TWAIT,			STATUS_COLOR_TIME,		"X Wait"			),
	DECLARE_STATUS_MESSAGE( STATUS_SYSWAIT,			STATUS_COLOR_TIME,		"X System"			),
	DECLARE_STATUS_MESSAGE( STATUS_PROTECTED,		STATUS_COLOR_ERROR,		"X Protected"		),
	DECLARE_STATUS_MESSAGE( STATUS_NUMERIC,			STATUS_COLOR_ERROR,		"X Numeric"			),
	DECLARE_STATUS_MESSAGE( STATUS_OVERFLOW,		STATUS_COLOR_ERROR,		"X Overflow"		),
	DECLARE_STATUS_MESSAGE( STATUS_DBCS,			STATUS_COLOR_ERROR,		"X DBCS"			),
	DECLARE_STATUS_MESSAGE( STATUS_SCROLLED,		STATUS_COLOR_ERROR,		"X Scrolled"		),
	DECLARE_STATUS_MESSAGE( STATUS_MINUS,			STATUS_COLOR_ERROR,		"X -f"				),
 };

 static struct _status *current_status = 0;

/*---[ Globals ]--------------------------------------------------------------*/

 guint KeyboardState = 0;

/*---[ Statics ]--------------------------------------------------------------*/

/*---[ Implement ]------------------------------------------------------------*/

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
 	char			chr[2];

    gboolean		rc			= FALSE;
    int				mode		= 0;

 	trm = Get3270DeviceBuffer(&rows, &cols);

    /* Get top of the screen */
    vPos = (top + font->Height);

    for(row = 0; row < rows; row++)
    {
    	hPos = left;
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
		         gdk_gc_set_background(gc,terminal_cmap + (trm->bg % terminal_color_count));
			  }

		   }

		   switch(mode)
		   {
		   case 0:	// Nothing special
		      gdk_draw_text(drawable,font->fn,gc,hPos,vPos,chr,1);
		      break;

		   case 1:  // Hidden
		      gdk_draw_text(drawable,font->fn,gc,hPos,vPos," ",1);
		      break;

		   case 2:	// Hidden/Editable
		      gdk_draw_text(drawable,font->fn,gc,hPos,vPos,chr[0] == ' ' ? chr : "*", 1);
		      break;

		   }

		   hPos += font->Width;
	       trm++;
    	}
    	vPos += (font->Height + line_spacing);
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
     *   M-32       empty
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
    DrawStatusRight(55,STATUS_COLOR_TIME,oia_Timer);

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
#ifdef DEBUG
			   Log("Status:\t%s",current_status->dbg);
#endif
		   }
	   }
	   if(!sts)
	      Log("Unexpected status code %d from 3270 library",code);
	}

    RedrawTerminalContents();

 }


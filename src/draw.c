
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

/*---[ Translation codes ]----------------------------------------------------*/

const gchar ebcdic2asc[] =
	{	/*00*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \
		/*08*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \
		/*10*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \
		/*18*/	0x20, 0x20, 0x20, 0x20, 0x2a, 0x20, 0x3b, 0x20, \
		/*20*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \
		/*28*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \
		/*30*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \
		/*38*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, \
		/*40*/	0x20, 0x20, 0xe2, 0xe4, 0xe0, 0xe1, 0xe3, 0xe5, \
		/*48*/	0xe7, 0xf1, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c, \
		/*50*/	0x26, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef, \
		/*58*/	0xec, 0xdf, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0xac, \
		/*60*/	0x2d, 0x2f, 0xc2, 0xc4, 0xc0, 0xc1, 0xc3, 0xc5, \
		/*68*/	0xc7, 0xd1, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f, \
		/*70*/	0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd, 0xce, 0xcf, \
		/*78*/	0xcc, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22, \
		/*80*/	0xd8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, \
		/*88*/	0x68, 0x69, 0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1, \
		/*90*/	0xb0, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, \
		/*98*/	0x71, 0x72, 0xaa, 0xba, 0xe6, 0xb8, 0xc6, 0xa4, \
		/*a0*/	0xb5, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, \
		/*a8*/	0x79, 0x7a, 0xa1, 0xbf, 0xd0, 0xdd, 0xde, 0xae, \
		/*b0*/	0x5e, 0xa3, 0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc, \
		/*b8*/	0xbd, 0xbe, 0x5b, 0x5d, 0xaf, 0xa8, 0xb4, 0xd7, \
		/*c0*/	0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, \
		/*c8*/	0x48, 0x49, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5, \
		/*d0*/	0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, \
		/*d8*/	0x51, 0x52, 0xb9, 0xfb, 0xfc, 0xf9, 0xfa, 0xff, \
		/*e0*/	0x5c, 0xf7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, \
		/*e8*/	0x59, 0x5a, 0xb2, 0xd4, 0xd6, 0xd2, 0xd3, 0xd5, \
		/*f0*/	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, \
		/*f8*/	0x38, 0x39, 0xb3, 0xdb, 0xdc, 0xd9, 0xda, 0x20
	};

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
//    gchar   		*string;

 	trm = Get3270DeviceBuffer(&rows, &cols);

 	if(!font->fn)
 	   return;

    /* Get top of the screen */
    vPos = (top + font->Height);

    for(row = 0; row < rows; row++)
    {
    	hPos = left;
    	for(col = 0; col < cols; col++)
    	{
		   chr[0] = ebcdic2asc[trm->cc];
		   chr[1] = 0;

/*
           string = g_convert(chr, 1, "ISO-8859-1", "UTF-8", NULL, NULL, NULL);
		   if(string)
		      chr[0] = *string;
			else
			  chr[0] = ' ';

		   g_free(string);
*/

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
    DrawStatusRight(15,STATUS_COLOR_TIME,oia_Timer);

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


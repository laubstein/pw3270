/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como screen.c e possui 1301 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */


#include "gui.h"
#include <lib3270/config.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <lib3270/localdefs.h>
#include <lib3270/toggle.h>

#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <stdlib.h>
#endif

#include "locked.bm"
#include "unlocked.bm"
#include "shift.bm"
#include "four.bm"

/*---[ Structures ]----------------------------------------------------------------------------------------*/

 #ifdef DEBUG
 	#define STATUS_CODE_DESCRIPTION(x,c,y) { #x, c, y }
 #else
 	#define STATUS_CODE_DESCRIPTION(x,c,y) { c, y }
 #endif

 struct status_code
 {
#ifdef DEBUG
	const char *dbg;
#endif
	int			clr;
	const char	*string;
 };


/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

 static void	setsize(int rows, int cols);
 static int  	addch(int row, int col, int c, unsigned short attr);
 static void	set_charset(char *dcs);
 static void	erase(void);
 static int		SetSuspended(int state);
 static void	SetScript(SCRIPT_STATE state);
 static void	set_cursor(CURSOR_MODE mode);
 static void	set_oia(OIA_FLAG id, int on);
 static void	set_compose(int on, unsigned char c, int keytype);
 static void	set_lu(const char *lu);
 static void	changed(int bstart, int bend);
 static void	error(const char *fmt, va_list arg);
 static void 	warning(const char *fmt, va_list arg);
 static void	syserror(const char *title, const char *message, const char *system);
 static int		init(void);
 static void 	update_toggle(int ix, int value, int reason, const char *name);
 static void	show_timer(long seconds);
 static void	DrawImage(GdkDrawable *drawable, GdkGC *gc, int id, int x, int y, int Width, int Height);
 static void 	DrawImageByWidth(GdkDrawable *drawable, GdkGC *gc, int id, int x, int y, int Width, int Height);
 static gchar	*convert_monocase(int c, gsize *sz);
 static gchar	*convert_regular(int c, gsize *sz);

/*---[ Globals ]-------------------------------------------------------------------------------------------*/

 const struct lib3270_screen_callbacks program_screen_callbacks =
 {
	sizeof(struct lib3270_screen_callbacks),

	init,				// int (*init)(void);
	error,				// void (*Error)(const char *fmt, va_list arg);
	warning,			// void (*Warning)(const char *fmt, va_list arg);
	syserror,			// void	(*SysError)(const char *title, const char *message, const char *system);
	setsize,			// void (*setsize)(int rows, int cols);
	addch,				// void (*addch)(int row, int col, int c, int attr);
	set_charset,		// void (*charset)(char *dcs);
	settitle,			// void (*title)(char *text);
	changed,			// void (*changed)(int bstart, int bend);
	gdk_beep,			// void (*ring_bell)(void);
	action_Redraw,		// void (*redraw)(void);
	MoveCursor,			// void (*move_cursor)(int row, int col);
	SetSuspended,		// int	(*set_suspended)(int state);
	SetScript,			// void	(*set_script)(SCRIPT_STATE state);
	NULL,				// void (*reset)(int lock);
	SetStatusCode,		// void (*status)(STATUS_CODE id);
	set_compose,		// void (*compose)(int on, unsigned char c, int keytype);
	set_cursor,			// void (*cursor)(CURSOR_MODE mode);
	set_lu,				// void (*lu)(const char *lu);
	set_oia,			// void (*set)(OIA_FLAG id, int on);
	erase,				// void (*erase)(void);
	update_toggle,		// void (*toggle_changed)(int ix, int value, int reason, const char *name);
	show_timer,			// void	(*show_timer)(long seconds);

	gui_console_window_new,						// HCONSOLE	(*console_new)(const char *title, const char *label);
	gui_console_window_delete,					// void		(*console_delete)(HCONSOLE hwnd);
	gui_console_window_append,					// int		(*console_append)(HCONSOLE hwnd, const char *fmt, ...);
	gui_console_window_wait_for_user_entry,		// char *	(*console_entry)(HCONSOLE hwnd);

 };

 static const struct _imagedata
 {
	const unsigned char	*data;
    gint					width;
    gint					height;
    short					color;
 } imagedata[] =
 {
 	{ locked_bits,		locked_width,	locked_height,   	TERMINAL_COLOR_SSL 		},
 	{ unlocked_bits,	unlocked_width,	unlocked_height, 	TERMINAL_COLOR_SSL 		},
 	{ shift_bits,		shift_width, 	shift_height,		TERMINAL_COLOR_OIA 		},
 	{ four_bits,		four_width, 	four_height,		TERMINAL_COLOR_OIA 		},
 };

 #define IMAGE_COUNT (sizeof(imagedata)/sizeof(struct _imagedata))

 static struct _pix
 {
 	GdkPixbuf *base;
 	GdkPixbuf *pix;
 	int		   Width;
 	int		   Height;
 } pix[IMAGE_COUNT];


 int 								terminal_rows	= 0;
 int 								terminal_cols	= 0;
 int								left_margin		= 0;
 int								top_margin		= 0;

 ELEMENT							*screen			= NULL;
 char								*charset		= NULL;
 char								*window_title	= PROGRAM_NAME;
 int								screen_suspended = 0;

 static int							szScreen		= 0;
 static const struct status_code	*sts_data		= NULL;
 static unsigned char				compose			= 0;
 static gchar						*luname			= 0;
 static const gchar					*status_msg		= NULL;
 static guint						kbrd_state		= 0;
 static char 						timer[9]		= "";

 static gchar 						*(*convert_charset)(int c, gsize *sz) = convert_regular;
 static gboolean					oia_flag[OIA_FLAG_USER];
 static SCRIPT_STATE 				script_state = SCRIPT_STATE_NONE;

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 static void changed(int bstart, int bend)
 {
 }

 static void set_compose(int on, unsigned char c, int keytype)
 {
 	if(on)
 		compose = c;
	else
		compose = 0;
 }

 GdkGC * getCachedGC(GdkDrawable *draw)
 {
 	GdkGC *gc;

#ifdef DEBUG
	if(!draw)
	{
		Trace("Error: Called %s with null object",__FUNCTION__);
	}
#endif

 	gc = ((GdkGC *) g_object_get_data(G_OBJECT(draw),"CachedGC"));
 	if(gc)
		return gc;

	gc = gdk_gc_new(draw);
	g_object_set_data_full(G_OBJECT(draw),"CachedGC",gc,g_object_unref);
	return gc;
 }

 static int SetSuspended(int state)
 {
 	int rc = screen_suspended;
 	screen_suspended = state;
	Trace("%s state: %d",__FUNCTION__,state);

	if(!screen_suspended && terminal && pixmap)
 	{
		DrawScreen(color,pixmap);
		DrawOIA(pixmap,color);
		RedrawCursor();
		if(Toggled(CURSOR_POS))
			DrawCursorPosition();
		gtk_widget_queue_draw(terminal);
 	}

 	return rc;
 }


 void settitle(char *text)
 {
 	gchar buffer[80];

 	if(text && *text)
 		g_snprintf(buffer,79,"%s - %s",window_title,text);
	else
 		g_snprintf(buffer,79,"%s",window_title);

 	if(topwindow)
		gtk_window_set_title(GTK_WINDOW(topwindow),buffer);

 }

 void ParseInput(const gchar *string)
 {
    gchar *input = g_convert(string, -1, CHARSET, "UTF-8", NULL, NULL, NULL);

    if(!input)
    {
        Log("Error converting string \"%s\" to %s",string,CHARSET);
        return;
    }

    // NOTE (perry#1#): Is it the best way?
    Input_String((const unsigned char *) input);

    g_free(input);
 }

 static gchar * convert_monocase(int c, gsize *sz)
 {
	gchar in[2]	= { (char) c, 0 };
	gchar *ptr	= g_convert(in, -1, "UTF-8", CHARSET, NULL, sz, NULL);
	gchar *ret;

	if(!ptr)
		return ptr;

	ret = g_utf8_strup(ptr,*sz);

	g_free(ptr);

	return ret;
 }

 static gchar * convert_regular(int c, gsize *sz)
 {
	gchar in[2] = { (char) c, 0 };
 	return g_convert(in, -1, "UTF-8", CHARSET, NULL, sz, NULL);
 }

 void set_monocase(int value, enum toggle_type reason)
 {
	convert_charset = value ? convert_monocase : convert_regular;
	screen_disp();
 }

 static int addch(int row, int col, int c, unsigned short attr)
 {
 	ELEMENT in;
 	ELEMENT *el;
 	int		pos = (row*terminal_cols)+col;

 	if(!screen || col >= terminal_cols || row >= terminal_rows)
		return EINVAL;

	if(pos > szScreen)
		return EFAULT;

	memset(&in,0,sizeof(in));

	if(c)
	{
		if(attr & CHAR_ATTR_CG)
		{
			in.cg	= (unsigned short) c;
			*in.ch			= ' ';
		}
		else
		{
			gsize sz = 1;
			gchar *str = convert_charset(c,&sz);

			if(sz < MAX_CHR_LENGTH)
				memcpy(in.ch,str,sz);
			else
				Log("Invalid size when converting \"%c\" to \"%s\"",c,str);
			g_free(str);
		}
	}

	in.bg = (attr & 0xF0) >> 4;

	if(attr & COLOR_ATTR_FIELD)
		in.fg = (attr & 0x03)+TERMINAL_COLOR_FIELD;
	else
		in.fg = (attr & 0x0F);

	if(attr & COLOR_ATTR_UNDERLINE)
		in.fg |= COLOR_ATTR_UNDERLINE;

	// Get element entry in the buffer, update ONLY if changed
 	el = screen + pos;

	in.status = el->status;

	if(attr & CHAR_ATTR_MARKER)
		in.status |= ELEMENT_STATUS_FIELD_MARKER;
	else
		in.status &= ~ELEMENT_STATUS_FIELD_MARKER;

	if(!memcmp(&in,el,sizeof(ELEMENT)))
		return 0;

	memcpy(el,&in,sizeof(ELEMENT));

	if(!screen_suspended && terminal && pixmap)
	{
		// Update pixmap, queue screen redraw.
		gint 		x, y;
	 	GdkGC		*gc		= gdk_gc_new(pixmap);

		x = left_margin + (col * fontWidth);
		y = top_margin + (row * fontHeight);

		DrawElement(pixmap,color,gc,x,y,el);

		gdk_gc_destroy(gc);

		gtk_widget_queue_draw_area(terminal,x,y,fontWidth,fontHeight);

		if(row == cRow && col == cCol)
			RedrawCursor();

	}

	return 0;
 }

 static void setsize(int rows, int cols)
 {
	g_free(screen);
	screen = NULL;

	if(rows && cols)
	{
		szScreen = rows*cols;
		screen = g_new0(ELEMENT,szScreen);
		terminal_rows = rows;
		terminal_cols = cols;
	}


 	Trace("Terminal set to %d rows with %d cols, screen set to %p",rows,cols,screen);

 }

 void action_Redraw(void)
 {
#ifdef DEBUG
 	screen_disp();
#endif
 	DrawScreen(color,pixmap);
	DrawOIA(pixmap,color);
	gtk_widget_queue_draw(terminal);
 }

 /**
  * Erase screen.
  *
  */
 static void erase(void)
 {
	GdkGC		*gc;
	int			width;
	int			height;
	int			f;

	Trace("Erasing screen! (pixmap: %p screen: %p terminal: %p)",pixmap,screen,terminal);

	if(screen)
	{
		unsigned char status;

		for(f=0;f<szScreen;f++)
		{
			status				= screen[f].status & ~ELEMENT_STATUS_FIELD_MARKER;
			memset(screen+f,0,sizeof(ELEMENT));
			screen[f].ch[0]		= ' ';
			screen[f].status	= status;
		}
	}

	if(pixmap && terminal)
	{
		gc = getCachedGC(pixmap);
		gdk_drawable_get_size(pixmap,&width,&height);
		gdk_gc_set_foreground(gc,color);
		gdk_draw_rectangle(pixmap,gc,1,0,0,width,height);
		DrawOIA(pixmap,color);
		gtk_widget_queue_draw(terminal);
	}
 }

 static void set_lu(const char *lu)
 {
 	if(luname)
 	{
		g_free(luname);
		luname = NULL;
 	}

 	if(lu)
		luname = g_convert(lu, -1, "UTF-8", CHARSET, NULL, NULL, NULL);

	CallPlugins("pw3270_plugin_update_luname",luname);

	DrawOIA(pixmap,color);

	if(terminal)
		gtk_widget_queue_draw_area(terminal,left_margin,OIAROW,fontWidth*terminal_cols,fontHeight+1);

 }

 static void DrawStatus(GdkDrawable *draw, GdkColor *clr)
 {
 	PangoLayout *layout = getPangoLayout(TEXT_LAYOUT_OIA);
	GdkGC 		*gc = getCachedGC(draw);

 	int col = left_margin+(fontWidth << 3);

	gdk_gc_set_foreground(gc,clr+TERMINAL_COLOR_OIA_BACKGROUND);
	gdk_draw_rectangle(draw,gc,1,col,OIAROW+1,fontWidth << 4,fontHeight+1);

	if(sts_data && status_msg && *status_msg)
	{
		if(*status_msg == 'X')
		{
			int f;
			int cols = (fontWidth/3)+1;

			gdk_gc_set_foreground(gc,clr+sts_data->clr);

			for(f=0;f<cols;f++)
			{
				gdk_draw_line(draw,gc,col,OIAROW+f+3, col+(fontWidth-1),(OIAROW+(fontHeight-2)+f)-cols);
				gdk_draw_line(draw,gc,col+(fontWidth-1),OIAROW+f+3, col,(OIAROW+(fontHeight-2)+f)-cols);
			}

			col += fontWidth;
			pango_layout_set_text(layout,status_msg+1,-1);
		}
		else
		{
			pango_layout_set_text(layout,status_msg,-1);
		}
		gdk_draw_layout_with_colors(draw,gc,col,OIAROW+1,layout,clr+sts_data->clr,clr+TERMINAL_COLOR_OIA_BACKGROUND);
	}
#ifdef DEBUG
	else if(sts_data)
	{
		pango_layout_set_text(layout,sts_data->dbg,-1);
		gdk_draw_layout_with_colors(draw,gc,col,OIAROW+1,layout,clr+sts_data->clr,clr+TERMINAL_COLOR_OIA_BACKGROUND);
	}
	else
	{
		pango_layout_set_text(layout,"STATUS_NONE",-1);
		gdk_draw_layout_with_colors(draw,gc,col,OIAROW+1,layout,clr+TERMINAL_COLOR_OIA,clr+TERMINAL_COLOR_OIA_BACKGROUND);
	}
#endif
 }

 void DrawOIA(GdkDrawable *draw, GdkColor *clr)
 {
    /*
     * The status line is laid out thusly (M is maxCOLS):
     *
     *	0			"4" in a square
     *	1			"A" underlined
     *	2			solid box if connected, "?" in a box if not
     *	3..7		empty
     *	8...		message area
     *	M-43...42	SSL Status
     *	M-41		Meta indication ("M" or blank)
     *	M-40		Alt indication ("A" or blank)
     *	M-39...38	Shift Status
     *	M-37		empty
     *	M-36		Compose indication ("C" or blank)
     *	M-35		Compose first character
     *	M-34		Caps indications ("A" or blank)
     *	M-33		Typeahead indication ("T" or blank)
     *	M-31		Alternate keymap indication ("K" or blank)
     *	M-30		Reverse input mode indication ("R" or blank)
     *	M-29		Insert mode indication (Special symbol/"I" or blank)
     *	M-28		Printer indication ("P" or blank)
     *	M-27		Script indication ("S" or blank)
     *	M-26		empty
     *	M-25..M-14	LU Name
     *	M-15..M-9	command timing (Clock symbol and m:ss, or blank)
     *	M-7..M		cursor position (rrr/ccc or blank)
     *
     */
	GdkGC 			*gc;
	PangoLayout 	*layout;
	int   			row		= OIAROW;
	int				width	= (fontWidth*terminal_cols);
	GdkColor		*bg		= clr+TERMINAL_COLOR_OIA_BACKGROUND;
	GdkColor		*fg		= clr+TERMINAL_COLOR_OIA;
	int				col		= left_margin;
	char			str[12];

	if(!draw)
		return;

	gc = getCachedGC(draw);

	gdk_gc_set_foreground(gc,bg);
	gdk_draw_rectangle(draw,gc,1,left_margin,row,width,fontHeight+1);

	gdk_gc_set_foreground(gc,clr+TERMINAL_COLOR_OIA_SEPARATOR);
	gdk_draw_line(draw,gc,left_margin,row,left_margin+width,row);
	row++;

	layout = getPangoLayout(TEXT_LAYOUT_OIA);

	gdk_gc_set_foreground(gc,fg);

	//  0          "4" in a square
//	pango_layout_set_text(layout,"4",-1);
//	gdk_draw_layout_with_colors(draw,gc,col,row,layout,bg,fg);
	DrawImageByWidth(draw,gc,3,col,row,fontWidth,fontHeight);

	col += fontWidth;

	//  1          "A" underlined
	if(oia_flag[OIA_FLAG_UNDERA])
	{
		pango_layout_set_text(layout,(IN_E) ? "B" : "A",-1);
		gdk_draw_layout_with_colors(draw,gc,col,row,layout,fg,bg);
		gdk_gc_set_foreground(gc,fg);
	}

	col += fontWidth;

	// 2          solid box if connected, "?" in a box if not
	if(IN_ANSI)
	{
		pango_layout_set_text(layout,"N",-1);
		gdk_draw_layout_with_colors(draw,gc,col,row,layout,bg,fg);
	}
	else if(oia_flag[OIA_FLAG_BOXSOLID])
	{
		gdk_gc_set_foreground(gc,fg);
		gdk_draw_rectangle(draw,gc,1,col,row,fontWidth-1,fontHeight+1);
	}
	else if(IN_SSCP)
	{
		pango_layout_set_text(layout,"S",-1);
		gdk_draw_layout_with_colors(draw,gc,col,row,layout,bg,fg);
	}
	else
	{
		gdk_gc_set_foreground(gc,fg);
		pango_layout_set_text(layout,"?",-1);
		gdk_draw_layout_with_colors(draw,gc,col,row,layout,fg,bg);
		gdk_draw_rectangle(draw,gc,0,col,row,fontWidth-1,fontHeight+1);
	}

	// 8...       message area
	DrawStatus(draw, clr);

	memset(str,' ',10);

    // M-36...35	Compose indication ("C" or blank)
	col = left_margin+(fontWidth*(terminal_cols-36));

	if(compose)
	{
		str[0] = 'C';
		str[1] = compose;
	}

	//	M-34		Caps indications ("A" or blank)
	if(kbrd_state & GDK_LOCK_MASK)
		str[2] = 'A';

    //   M-33       Typeahead indication ("T" or blank)
	str[3] = oia_flag[OIA_FLAG_TYPEAHEAD]	? 'T' : ' ';

    //   M-31       Alternate keymap indication ("K" or blank)

    //   M-30       Reverse input mode indication ("R" or blank)
	str[6] = oia_flag[OIA_FLAG_REVERSE] ? 'R' : ' ';

    //   M-29       Insert mode indication (Special symbol/"I" or blank)
	str[7] = Toggled(INSERT) ? 'I' : ' ';

    //   M-28       Printer indication ("P" or blank)
	str[8] = oia_flag[OIA_FLAG_PRINTER]	? 'P' : ' ';

    //   M-27		Script indication ("S" or blank)
	str[9] = (script_state != SCRIPT_STATE_NONE) ? 'S' : ' ';

	str[10] = 0;

	pango_layout_set_text(layout,str,-1);
	gdk_draw_layout_with_colors(draw,gc,col,row,layout,fg,bg);

	//	M-39       Shift indication (Special symbol/"^" or blank)
	if(kbrd_state & GDK_SHIFT_MASK)
		DrawImage(draw,gc,2,left_margin+(fontWidth*(terminal_cols-39)),row,fontWidth<<1,fontHeight);

	// Draw SSL indicator (M-43)
	DrawImage(draw,gc,oia_flag[OIA_FLAG_SECURE] ? 0 : 1 ,left_margin+(fontWidth*(terminal_cols-43)),row+1,fontWidth<<1,fontHeight);

	// M-25 LU Name
	if(luname)
	{
		pango_layout_set_text(layout,luname,-1);
		gdk_draw_layout_with_colors(draw,gc,left_margin+(fontWidth*(terminal_cols-25)),row,layout,clr+TERMINAL_COLOR_OIA_LU,bg);
	}

	//	M-15..M-9	command timing (Clock symbol and m:ss, or blank)
	if(*timer)
	{
		pango_layout_set_text(layout,timer,-1);
		gdk_draw_layout_with_colors(draw,gc,left_margin+(fontWidth*(terminal_cols-15)),row,layout,clr+TERMINAL_COLOR_OIA_TIMER,bg);
	}

	//  M-7..M     cursor position (rrr/ccc or blank)
	if(Toggled(CURSOR_POS))
	{
		g_snprintf(str,8,"%03d/%03d",cRow+1,cCol+1);
		pango_layout_set_text(layout,str,-1);
		gdk_draw_layout_with_colors(draw,gc,left_margin+(fontWidth*(terminal_cols-7)),row,layout,clr+TERMINAL_COLOR_OIA_CURSOR,bg);
	}

 }

 static void set_oia(OIA_FLAG id, int on)
 {
 	if(id > OIA_FLAG_USER)
		return;

 	oia_flag[id] = on;

	DrawOIA(pixmap,color);

	if(terminal)
		gtk_widget_queue_draw_area(terminal,left_margin,OIAROW,fontWidth*terminal_cols,fontHeight+1);
 }

 static void set_charset(char *dcs)
 {
 	Trace("Screen charset: %s",dcs);
	g_free(charset);
	charset = g_strdup(dcs);
 }

 static STATUS_CODE last_id = (STATUS_CODE) -1;

 void SetStatusCode(STATUS_CODE id)
 {
 	static const struct status_code tbl[STATUS_CODE_USER] =
 	{
		STATUS_CODE_DESCRIPTION(	STATUS_CODE_BLANK,
									TERMINAL_COLOR_OIA_STATUS_OK,
									"" ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_SYSWAIT,
									TERMINAL_COLOR_OIA_STATUS_OK,
									N_( "X System" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_TWAIT,
									TERMINAL_COLOR_OIA_STATUS_OK,
									N_( "X Wait" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_CONNECTED,
									TERMINAL_COLOR_OIA_STATUS_OK,
									NULL ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_DISCONNECTED,
									TERMINAL_COLOR_OIA_STATUS_INVALID,
									N_( "X Not Connected" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_AWAITING_FIRST,
									TERMINAL_COLOR_OIA_STATUS_OK,
									N_( "X" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_MINUS,
									TERMINAL_COLOR_OIA_STATUS_OK,
									N_( "X -f" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_PROTECTED,
									TERMINAL_COLOR_OIA_STATUS_INVALID,
									N_( "X Protected" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_NUMERIC,
									TERMINAL_COLOR_OIA_STATUS_INVALID,
									N_( "X Numeric" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_OVERFLOW,
									TERMINAL_COLOR_OIA_STATUS_INVALID,
									N_( "X Overflow" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_INHIBIT,
									TERMINAL_COLOR_OIA_STATUS_INVALID,
									N_( "X Inhibit" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_KYBDLOCK,
									TERMINAL_COLOR_OIA_STATUS_INVALID,
									NULL ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_X,
									TERMINAL_COLOR_OIA_STATUS_INVALID,
									N_( "X" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_RESOLVING,
									TERMINAL_COLOR_OIA_STATUS_WARNING,
									N_( "X Resolving" ) ),

		STATUS_CODE_DESCRIPTION(	STATUS_CODE_CONNECTING,
									TERMINAL_COLOR_OIA_STATUS_WARNING,
									N_( "X Connecting" ) ),


	};

	/* Check if status has changed to avoid unnecessary redraws */
	if(id == last_id && !sts_data)
		return;

	if(id >= STATUS_CODE_USER)
	{
		Log("Unexpected status code %d",(int) id);
		return;
	}

	last_id 	= id;
	sts_data 	= tbl+id;

	Trace("Status changed to %s (%s)",sts_data->dbg,sts_data->string ? sts_data->string : "NULL");

	if(sts_data->string)
	{
		if(*sts_data->string)
			status_msg = gettext(sts_data->string);
		else
			status_msg = "";

		// FIXME (perry#2#): Find why the library is keeping the cursor as "locked" in some cases. When corrected this "workaround" can be removed.
		if(id == STATUS_CODE_BLANK)
			set_cursor(CURSOR_MODE_NORMAL);

		if(pixmap)
			DrawStatus(pixmap, color);

		if(terminal)
			gtk_widget_queue_draw_area(terminal,left_margin+(fontWidth << 3),OIAROW,fontWidth << 4,fontHeight+1);

		Trace("%s","Status updated");
	}

 }

 CURSOR_MODE cursor_mode = -1;

 static void set_cursor(CURSOR_MODE mode)
 {
 	if(mode == cursor_mode || mode > CURSOR_MODE_USER || !terminal || !terminal->window)
		return;

	cursor_mode = mode;

#ifdef MOUSE_POINTER_CHANGE
	if(drag_type == DRAG_TYPE_NONE)
		gdk_window_set_cursor(terminal->window,wCursor[cursor_mode]);
#endif

 }

 static int Loaded = 0;

 void LoadImages(GdkDrawable *drawable, GdkGC *gc)
 {
 	int			f;
 	GdkPixmap	*temp;

	if(!Loaded)
	{
    	memset(pix,0,sizeof(struct _pix) * IMAGE_COUNT);
    	Loaded = 1;
	}

	Trace("Recarregando %d imagens",IMAGE_COUNT);

 	for(f=0;f<IMAGE_COUNT;f++)
 	{
 		// Load bitmap setting the right colors
 		temp = gdk_pixmap_create_from_data(	drawable,
											(const gchar *) imagedata[f].data,
                                       		imagedata[f].width,
                                       		imagedata[f].height,
											gdk_drawable_get_depth(drawable),
											color+imagedata[f].color,
											color+TERMINAL_COLOR_OIA_BACKGROUND );

		if(pix[f].base)
			gdk_pixbuf_unref(pix[f].base);

		pix[f].base = gdk_pixbuf_get_from_drawable(	0,
													temp,
													gdk_drawable_get_colormap(drawable),
													0,0,
													0,0,
													imagedata[f].width,
													imagedata[f].height );

        gdk_pixmap_unref(temp);

		// Destroy cached pixmap
 		pix[f].Height	= 0;
 		pix[f].Width	= 0;
		if(pix[f].pix)
		{
			gdk_pixbuf_unref(pix[f].pix);
			pix[f].pix = 0;
		}
 	}

 }

 void ReloadPixmaps(void)
 {
	LoadImages(terminal->window, terminal->style->fg_gc[GTK_WIDGET_STATE(terminal)]);
 }

 static void DrawImageByWidth(GdkDrawable *drawable, GdkGC *gc, int id, int x, int y, int Width, int Height)
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
 		/* Resize by Width */
        ratio = ((double) gdk_pixbuf_get_height(pix[id].base)) / ((double) gdk_pixbuf_get_width(pix[id].base));
		temp  = (int) ((double) ratio * ((double) Height));
	    pix[id].pix = gdk_pixbuf_scale_simple(pix[id].base,Width,temp,GDK_INTERP_HYPER);
	    pix[id].Height = Height;
	    pix[id].Width = Width;
	    Trace("Rescaling pixmap %d (%p)",id,pix[id].pix);
 	}

	DrawImage(drawable, gc, id, x, y, Width, Height);

 }

 static void DrawImage(GdkDrawable *drawable, GdkGC *gc, int id, int x, int y, int Width, int Height)
 {

 	if( ((Height != pix[id].Height) || (Width != pix[id].Width)) && pix[id].pix )
 	{
 		gdk_pixbuf_unref(pix[id].pix);
		pix[id].pix = 0;
 	}

 	if(!pix[id].pix)
 	{
		pix[id].pix = gdk_pixbuf_scale_simple(pix[id].base,Width,Height,GDK_INTERP_HYPER);
		pix[id].Height = Height;
		pix[id].Width  = Width;
 	}

    if(pix[id].pix)
		gdk_pixbuf_render_to_drawable(pix[id].pix,drawable,gc,0,0,x,y,-1,-1,GDK_RGB_DITHER_NORMAL,0,0);

 }

 void UpdateKeyboardState(guint state)
 {

 }

 gchar * GetScreenContents(gboolean all)
 {
 	gsize	max = terminal_rows*terminal_cols*MAX_CHR_LENGTH;
 	GString	*str = g_string_sized_new(max);
 	int		row,col;
 	int		pos	= 0;

	for(row = 0; row < terminal_rows;row++)
	{
		gchar		line[max];

		*line = 0;
		for(col = 0; col < terminal_cols;col++)
		{
			if(all || (screen[pos].status & ELEMENT_STATUS_SELECTED))
			{
				if(!*line)
				{
					if(*str->str)
						g_string_append_c(str,'\n');
				}
				g_strlcat(line,*screen[pos].ch ? screen[pos].ch : " ",max);
			}
			pos++;
		}

		if(*line)
			g_string_append(str,g_strchomp(line));

	}

	Trace("Screen-read:\n%s<---\n",str->str);

	return g_string_free(str,FALSE);
 }

 static void syserror(const char *title, const char *message, const char *system)
 {
 	GtkWidget 	*dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CLOSE,
													"%s",gettext( message ) );

 	g_warning("%s",message);

	if(title)
		gtk_window_set_title(GTK_WINDOW(dialog), gettext(title));
	else
		gtk_window_set_title(GTK_WINDOW(dialog), _( "System Error" ) );

	if(system)
	{
		// Reencode and show secondary message
#ifdef WIN32
		gsize	bytes_written;
		gchar	*ptr = g_locale_to_utf8(system, -1, NULL, &bytes_written, NULL);

		if(ptr)
		{
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), ptr);
			g_free(ptr);
		}
		else
		{
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), system);
		}
#else
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), system);
#endif
	}

	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);

 }

 static void warning(const char *fmt, va_list arg)
 {
 	gchar		*msg	= g_strdup_vprintf(gettext(fmt),arg);
 	GtkWidget 	*dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_WARNING,
													GTK_BUTTONS_CLOSE,
													"%s",msg );

 	g_warning("%s",msg);

	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);

	g_free(msg);
 }

 static void error(const char *fmt, va_list arg)
 {
 	gchar		*msg	= g_strdup_vprintf(gettext(fmt),arg);
 	GtkWidget 	*dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CLOSE,
													"%s",msg );

 	g_warning("%s",msg);

	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);

	g_free(msg);
 }

 static int init(void)
 {
	return 0;
 }

 void update_toggle(int ix, int value, int reason, const char *name)
 {
 	GtkAction	*action;
 	gchar		*ptr;

	ptr = g_strconcat("Toggle",name,NULL);
	action = get_action_by_name(ptr);
	g_free(ptr);

	// Update toggle buttons
	if(action)
		gtk_toggle_action_set_active((GtkToggleAction *) action,value);

	// Update toolbar items
	ptr = g_strconcat("ToggleReset",name,NULL);
	action = get_action_by_name(ptr);
	g_free(ptr);

	if(action)
		gtk_action_set_visible(action,value ? TRUE : FALSE);

	ptr = g_strconcat("ToggleSet",name,NULL);
	action = get_action_by_name(ptr);
	g_free(ptr);
	if(action)
		gtk_action_set_visible(action,value ? FALSE : TRUE);

 }

 static void show_timer(long seconds)
 {
 	if(seconds > 0)
 	{
		Trace("Timer: %d seconds",(int) seconds);
		g_snprintf(timer,6,"%02d:%02d",(int) (seconds/60),(int) (seconds % 60));
		DrawOIA(pixmap,color);
 	}
 	else if(*timer)
 	{
 		*timer = 0;
		DrawOIA(pixmap,color);
 	}

	gtk_widget_queue_draw_area(terminal,left_margin,OIAROW,fontWidth*terminal_cols,fontHeight+1);

 }

 static gboolean script_timer(gpointer dunno)
 {
	// TODO (perry#9#): Blink script indicator according current state
 	return script_state != SCRIPT_STATE_NONE;
 }

 static void SetScript(SCRIPT_STATE state)
 {
	if(script_state == SCRIPT_STATE_NONE)
	{
		// No script. Start timer
		script_state = state;
		g_timeout_add((guint) 10, (GSourceFunc) script_timer, 0);
		return;
	}

	// Update OIA
	script_state = state;
	DrawOIA(pixmap,color);
	if(terminal)
		gtk_widget_queue_draw_area(terminal,left_margin,OIAROW,fontWidth*terminal_cols,fontHeight+1);

 }

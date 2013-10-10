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
#include "oia.h"
#include "fonts.h"
#include "actions.h"

#include <lib3270/config.h>

#ifdef HAVE_MALLOC_H
	#include <malloc.h>
#endif

#include <string.h>
#include <errno.h>
#include <lib3270/toggle.h>

#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
	#include <sys/types.h>
	#include <signal.h>
#endif

#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
	#include <stdlib.h>
#endif

/*---[ Structures ]----------------------------------------------------------------------------------------*/

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

 static int  	  addch(int row, int col, unsigned char c, unsigned short attr);
 static void	  set_charset(char *dcs);

 static void	  erase(H3270 *session);
 static void	  display(H3270 *session);

 static int	  SetSuspended(int state);
 static void	  SetScript(SCRIPT_STATE state);
 static void	  set_cursor(H3270 *session, LIB3270_CURSOR mode);
 static void	  set_oia(H3270 *session, OIA_FLAG id, unsigned char on);
 static void	  set_lu(H3270 *session, const char *lu);
// static void	  changed(int bstart, int bend);
 static void	  error(const char *fmt, va_list arg);
 static void 	  warning(const char *fmt, va_list arg);
 static void	  syserror(const char *title, const char *message, const char *system);
 static int	  init(void);
 static void 	  update_toggle(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason, const char *name);
 static void 	  redraw(void);
 static gchar	* convert_monocase(int c, gsize *sz);
 static gchar	* convert_regular(int c, gsize *sz);
 static int	  popup_dialog(H3270 *session, PW3270_DIALOG type, const char *title, const char *msg, const char *fmt, va_list arg);
 static void	  model_changed(H3270 *session, const char *name, int model, int rows, int cols);

#ifdef HAVE_ALTSCREEN
 static void view_changed(H3270 *session, unsigned short rows, unsigned short cols);
#endif

/*---[ Globals ]-------------------------------------------------------------------------------------------*/

 static void ring_bell(void)
 {
	if(TOGGLED_BEEP)
	{
		gdk_beep();
	}
 }

 const struct lib3270_screen_callbacks program_screen_callbacks =
 {
	sizeof(struct lib3270_screen_callbacks),

	init,					// int (*init)(void);

	popup_dialog,			// int	(*popup_dialog)(H3270 *session, int type, const char *title, const char *msg, const char *fmt, va_list arg);
	error,					// void (*Error)(const char *fmt, va_list arg);
	warning,				// void (*Warning)(const char *fmt, va_list arg);
	syserror,				// void	(*SysError)(const char *title, const char *message, const char *system);

	model_changed,			// void	(*model_changed)(H3270 *session, const char *name, int model, int rows, int cols);

	addch,					// void (*addch)(int row, int col, unsigned char c, int attr);
	set_charset,			// void (*charset)(char *dcs);
	settitle,				// void (*title)(char *text);
	ring_bell,				// void (*ring_bell)(void);
	redraw,					// void (*redraw)(void);
	update_cursor_position,	// void (*move_cursor)(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
	SetSuspended,			// int	(*set_suspended)(int state);
	SetScript,				// void	(*set_script)(SCRIPT_STATE state);
	NULL,					// void (*reset)(int lock);
	SetStatusCode,			// void (*status)(STATUS_CODE id);
	set_cursor,				// void (*cursor)(CURSOR_MODE mode);
	set_lu,					// void (*lu)(const char *lu);
	set_oia,				// void (*set)(OIA_FLAG id, unsigned char on);

	erase,					// void (*erase)(H3270 *session);
	display,				// void	(*display)(H3270 *session);
#ifdef HAVE_ALTSCREEN
	view_changed,			// 			void 	(*set_viewsize)(H3270 *session, unsigned short rows, unsigned short cols);
#else
	NULL,					// void (*set_viewsize)(unsigned short rows, unsigned short cols);
#endif

	update_toggle,			// void (*toggle_changed)(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason, const char *name);
	oia_set_timer,			// void	(*show_timer)(long seconds);

	gui_console_window_new,						// HCONSOLE	(*console_new)(const char *title, const char *label);
	gui_console_window_delete,					// void		(*console_delete)(HCONSOLE hwnd);
	gui_console_window_append,					// int		(*console_append)(HCONSOLE hwnd, const char *fmt, ...);
	gui_console_window_wait_for_user_entry,		// char *	(*console_entry)(HCONSOLE hwnd);

 };

 struct _screen		* screen											= NULL;
 struct _view			  view												= { 0 };
 char					* charset											= NULL;
 char					* window_title										= PROGRAM_NAME;
 LIB3270_CURSOR 	  cursor_mode = -1;

 gboolean				  screen_updates_enabled							= FALSE;

 static const gchar	* screen_size_text[]								= { "80x24", "80x32", "80x43", "132x27" };
 static GtkWidget		* screen_size_menu[G_N_ELEMENTS(screen_size_text)]	= { 0 };


 static gchar 	* (*convert_charset)(int c, gsize *sz) = convert_regular;

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 static void redraw(void)
 {
	action_redraw(0);
 }

 static int SetSuspended(int state)
 {
 	gboolean enabled = (state == 0);

	if(enabled == screen_updates_enabled)
		return 0;

	screen_updates_enabled = enabled;

// 	Trace("%s(%d): screen updates are %s",__FUNCTION__,state,screen_updates_enabled ? "enabled" : "disabled");

	if(screen_updates_enabled)
 	{
 		if(valid_terminal_window())
 		{
			update_cursor_info();
			update_terminal_contents();
 		}
 		gtk_widget_queue_draw(terminal);
 	}

 	return TRUE;
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
	// lib3270_set_string(hSession, (const unsigned char *) input);

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

 void set_monocase(H3270 *session, int value, LIB3270_TOGGLE_TYPE reason)
 {
	convert_charset = value ? convert_monocase : convert_regular;
	screen_disp(hSession);
 }

 static void convert_cg(ELEMENT *el, int c)
 {
 	static const struct _xlat
 	{
 		int c;
 		const gchar *str;
 	} xlat[] =
 	{
		{ 0x8c,	"≤" },	// CG 0xf7, less or equal
		{ 0xae, "≥" },	// CG 0xd9, greater or equal
		{ 0xbe, "≠" },	// CG 0x3e, not equal
//		{ 0xa3, " " },	// CG 0x93, bullet
		{ 0xad, "["	},
		{ 0xbd, "]" }
 	};

 	int f;

 	for(f=0;f < G_N_ELEMENTS(xlat); f++)
 	{
		if(xlat[f].c == c)
		{
//			Trace("%s \"%s\" %d",__FUNCTION__,xlat[f].str,strlen(xlat[f].str));
			strcpy(el->ch,xlat[f].str);
			return;
		}
 	}

	el->cg	= (unsigned short) c;
	*el->ch	= ' ';

 }

 static int addch(int row, int col, unsigned char c, unsigned short attr)
 {

	int		baddr = (row*screen->cols)+col;
 	ELEMENT in;
 	ELEMENT *el;

 	if(!screen || baddr > screen->length)
		return EFAULT;

	memset(&in,0,sizeof(in));

	if(c)
	{
		if(attr & CHAR_ATTR_CG)
		{
			convert_cg(&in,c);
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
 	el = screen->content + baddr;

	in.status = el->status;

	if(attr & CHAR_ATTR_MARKER)
		in.status |= ELEMENT_STATUS_FIELD_MARKER;
	else
		in.status &= ~ELEMENT_STATUS_FIELD_MARKER;

	if(!memcmp(&in,el,sizeof(ELEMENT)))
		return 0;

	memcpy(el,&in,sizeof(ELEMENT));
	el->changed = TRUE;

	return 0;
 }

 /**
  * Erase screen.
  *
  */
 static void erase(H3270 *session)
 {
 	int f;

	Trace("Erasing screen! (pixmap: %p screen: %p terminal: %p window: %p valid: %s)",get_terminal_pixmap(),screen,terminal,terminal->window,valid_terminal_window() ? "Yes" : "No");

	if(screen)
	{
		unsigned char status;

		for(f=0;f<screen->length;f++)
		{
			status = screen->content[f].status & ~ELEMENT_STATUS_FIELD_MARKER;
			memset(screen->content+f,0,sizeof(ELEMENT));
			screen->content[f].ch[0]	= ' ';
			screen->content[f].status	= status;
		}
	}

	if(valid_terminal_window())
	{

		if(screen_updates_enabled)
		{
			int 	width  = view.cols * fontWidth;
			int 	height = view.rows * terminal_font_info.spacing;
			cairo_t *cr	= get_terminal_cairo_context();

			gdk_cairo_set_source_color(cr,color+TERMINAL_COLOR_BACKGROUND);
			cairo_rectangle(cr, view.left, view.top, width, height);
			cairo_fill(cr);

			cairo_destroy(cr);
			gtk_widget_queue_draw_area(terminal,view.left, view.top,width,height);
		}
	}

	if(get_cursor_pixmap())
	{
		gdk_pixmap_unref(get_cursor_pixmap());
		pixmap_cursor = NULL;
	}
 }

 static GPid on_lu_pid = 0;

 static void set_lu(H3270 *session, const char *lu)
 {
 	gchar *luname = NULL;

	update_oia_element(OIA_ELEMENT_LUNAME);

 	if(lu)
 	{
		luname = g_convert(lu, -1, "UTF-8", CHARSET, NULL, NULL, NULL);

		if(on_lu_command && !on_lu_pid)
		{
			if(run_script_command_line(on_lu_command, &on_lu_pid))
			{
				on_lu_pid = 0;
			}
		}
 	}
 	else if(on_lu_pid)
 	{
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE

		Trace("Sending SIGTERM do pid %d",(int) on_lu_pid);

		if(kill( (pid_t) on_lu_pid, SIGTERM) < 0)
		{
			GtkWidget *dialog;

			// Can't kill process, notify user
			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_WARNING,
												GTK_BUTTONS_OK,
												_(  "Can't stop LU association script" ));

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't stop script" ) );
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog) ,"%s", strerror(errno));

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

		}
#elif defined( WIN32 )

		if(!TerminateProcess((HANDLE) on_lu_pid, -1))
		{
			GtkWidget *dialog;

			// Can't kill process, notify user
			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_WARNING,
												GTK_BUTTONS_OK,
												_(  "Can't stop LU association script" ));

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't stop script" ) );

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);
		}

#endif

 	}

	CallPlugins("pw3270_plugin_update_luname",luname);

	if(luname)
		g_free(luname);
 }

 static void set_oia(H3270 *session, OIA_FLAG id, unsigned char on)
 {
 	if(id > OIA_FLAG_USER)
		return;

	oia_flag[id] = on;

	switch(id)
	{
	case OIA_FLAG_BOXSOLID:
		update_oia_element(OIA_ELEMENT_CONNECTION_STATUS);
		break;

	case OIA_FLAG_UNDERA:
		update_oia_element(OIA_ELEMENT_UNDERA);
		break;

	case OIA_FLAG_SECURE:
		update_oia_element(OIA_ELEMENT_SSL_STATE);
		break;

	case OIA_FLAG_TYPEAHEAD:
		update_oia_element(OIA_ELEMENT_TYPEAHEAD_INDICATOR);
		break;

#ifdef OIA_ELEMENT_PRINTER_INDICATOR
	case OIA_FLAG_PRINTER:
		update_oia_element(OIA_ELEMENT_PRINTER_INDICATOR);
		break;
#endif //  OIA_ELEMENT_PRINTER_INDICATOR

#ifdef OIA_ELEMENT_REVERSE_INPUT_INDICATOR
	case OIA_FLAG_REVERSE:
		update_oia_element(OIA_ELEMENT_REVERSE_INPUT_INDICATOR);
		break;
#endif // OIA_ELEMENT_REVERSE_INPUT_INDICATOR

	default:

		// Unexpected flag, update all OIA.
		update_oia();
	}
 }

 static void set_charset(char *dcs)
 {
 	Trace("Screen charset: %s",dcs);
	g_free(charset);
	charset = g_strdup(dcs);
 }

 void SetStatusCode(H3270 *session, LIB3270_STATUS id)
 {
 	if(id == terminal_message_id)
		return;

	terminal_message_id = id;

/*
	if(id == LIB3270_STATUS_BLANK)
	{
		set_cursor(session,CURSOR_MODE_NORMAL);
		update_cursor_pixmap();
	}
	else if(id >= LIB3270_STATUS_USER)
	{
		Log("Unexpected status code %d",(int) id);
		return;
	}
*/

	update_oia_element(OIA_ELEMENT_MESSAGE_AREA);

 }

 static void set_cursor(H3270 *session, LIB3270_CURSOR mode)
 {

 	if(mode == cursor_mode || mode > LIB3270_CURSOR_USER || !terminal || !terminal->window)
		return;

	cursor_mode = mode;

#ifdef MOUSE_POINTER_CHANGE
	if(drag_type == DRAG_TYPE_NONE)
		gdk_window_set_cursor(terminal->window,wCursor[cursor_mode]);
#endif

 }

 gchar * GetScreenContents(gboolean all)
 {
 	gsize	max;
 	GString	*str;
 	int		row,col;
 	int		pos	= 0;

 	if(!screen)
		return NULL;

	max = screen->rows*screen->cols*MAX_CHR_LENGTH;

 	str = g_string_sized_new(max);

	for(row = 0; row < screen->rows;row++)
	{
		gchar		line[max];

		*line = 0;
		for(col = 0; col < screen->cols;col++)
		{
			if(all || (screen->content[pos].status & ELEMENT_STATUS_SELECTED))
			{
				if(!*line)
				{
					if(*str->str)
						g_string_append_c(str,'\n');
				}
				g_strlcat(line,*screen->content[pos].ch ? screen->content[pos].ch : " ",max);
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
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", system);
		}
#else
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", system);
#endif
	}

	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);

 }

 static void warning(const char *fmt, va_list arg)
 {
 	gchar		*msg	= g_strdup_vprintf(gettext(fmt),arg);
 	GtkWidget 	*dialog;

 	g_warning("%s",msg);

	if(!topwindow)
	{
		g_free(msg);
		return;
	}

 	dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
										GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_WARNING,
										GTK_BUTTONS_CLOSE,
										"%s",msg );

	Trace("msg: %s dialog: %p topwindow: %p",msg,dialog,topwindow);

	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);

	g_free(msg);
 }

 static int popup_dialog(H3270 *session, PW3270_DIALOG type, const char *title, const char *msg, const char *fmt, va_list arg)
 {
 	static const struct _tbl
 	{
 		const gchar	* title;
 		GtkMessageType	  dialog_type;
 		GtkButtonsType 	  dialog_buttons;
 	} tbl[] =
 	{
 		{ N_( "Information" ),	GTK_MESSAGE_INFO,	GTK_BUTTONS_CLOSE 		},
 		{ N_( "Error" ),		GTK_MESSAGE_ERROR, 	GTK_BUTTONS_OK			},
 	};

 	GtkWidget		*dialog;
 	gchar			*secondary		= NULL;
 	GtkResponseType	 response;

 	if(arg && fmt)
		secondary = g_strdup_vprintf(gettext(fmt),arg);

	if(type > G_N_ELEMENTS(tbl))
		type = 0;

	dialog = gtk_message_dialog_new_with_markup(	GTK_WINDOW(topwindow),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													tbl[type].dialog_type,
													tbl[type].dialog_buttons,
													"%s",msg );

	gtk_window_set_title(GTK_WINDOW(dialog), title ? title : tbl[type].title);

	if(secondary)
	{
		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"%s",secondary);
		g_free(secondary);
	}

	if(type == PW3270_DIALOG_CRITICAL)
	{
		// Insert button to close program (Usefull in openSSL message errors
		gtk_dialog_add_button(GTK_DIALOG(dialog),_( "Abort" ), 10);
	}

	response = gtk_dialog_run(GTK_DIALOG (dialog));

	gtk_widget_destroy(dialog);

	if(response == 10)
	{
		Log("Aborting by user action in error dialog \"%s\"",msg);
		gtk_main_quit();
	}

	return (int) response;
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

 void update_toggle(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason, const char *name)
 {
	update_3270_toggle_action(ix, value);
 }

 static gboolean script_timer(gpointer dunno)
 {
	oia_script_blink = !oia_script_blink;
 	update_oia_element(OIA_ELEMENT_SCRIPT_INDICATOR);
 	return oia_script_state != SCRIPT_STATE_NONE;
 }

 static void SetScript(SCRIPT_STATE state)
 {
 	if(oia_script_state == state)
		return;

	if(oia_script_state == SCRIPT_STATE_NONE)
		g_timeout_add((guint) 500, (GSourceFunc) script_timer, 0);

	oia_script_state = state;
	oia_script_blink = TRUE;
 	update_oia_element(OIA_ELEMENT_SCRIPT_INDICATOR);

 }

 static void display(H3270 *session)
 {
	if(valid_terminal_window() && screen_updates_enabled)
	{
		int		row;
		int		col;
		cairo_t *cr	= get_terminal_cairo_context();

		for(row = 0; row < view.rows; row++)
		{
			int		cstart	= -1;
			int 	bstart	= -1;
			int 	bend	= -1;
			int		baddr   =  row * screen->cols;

			for(col = 0;col < view.cols;col++)
			{
				if(screen->content[baddr].changed)
				{
					if(bstart < 0)
					{
						bstart = baddr;
						cstart = col;
					}
					bend = baddr;
					screen->content[baddr].changed = FALSE;

					if(baddr == cursor_position)
						update_cursor_pixmap();

				}
				else if(bstart >= 0)
				{
					GdkRectangle r;
					draw_region(cr,bstart,bend,color,&r);
					gdk_window_invalidate_rect(terminal->window,&r,FALSE);
					bstart = bend = -1;
				}
				baddr++;
			}

			if(bstart >= 0)
			{
				GdkRectangle r;
				draw_region(cr,bstart,bend,color,&r);
				gdk_window_invalidate_rect(terminal->window,&r,FALSE);
			}
		}

		cairo_destroy(cr);

		gdk_window_process_updates(terminal->window,FALSE);

	}
 }

 static void set_screen_size(GtkCheckMenuItem *item, int id)
 {
	if(gtk_check_menu_item_get_active(item))
	{
		Trace("Screen size set to %d",id);
		SetInt("Terminal","Model",id);
		set_3270_model(hSession,id);
	}
 }

/**
 * Load screen size menu.
 *
 * @param topmenu	Parent menu for screen size options.
 *
 */
 void load_screen_size_menu(GtkWidget *topmenu)
 {
 	int i;
 	GSList 		*group	= NULL;
 	GtkWidget	*menu;
 	int	model = get_3270_model(hSession)-2;

 	if(!topmenu)
 	{
 		memset(screen_size_menu,0,sizeof(screen_size_menu));
 		return;
 	}

 	menu = gtk_menu_new();

	for(i=0; i< G_N_ELEMENTS(screen_size_text); i++)
    {
    	gchar *name = g_strdup_printf( _( "Model %d (%s)"),i+2,screen_size_text[i]);

		screen_size_menu[i] = gtk_radio_menu_item_new_with_label(group,name);

		group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(screen_size_menu[i]));

		g_free(name);

		gtk_widget_show(screen_size_menu[i]);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),screen_size_menu[i]);

		if(i == model)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(screen_size_menu[i]),TRUE);

		g_signal_connect(G_OBJECT(screen_size_menu[i]),"toggled",G_CALLBACK(set_screen_size),(gpointer) i+2);

    }

	gtk_widget_show_all(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(topmenu),menu);

 }

 static void model_changed(H3270 *session, const char *name, int model, int rows, int cols)
 {
	int length = rows*cols;

 	// Check for screen buffer change
 	if(screen)
		g_free(screen);

	screen = g_malloc0(sizeof(struct _screen) + (sizeof(ELEMENT)*length));
	screen->length	= length;
	screen->rows	= rows;
	screen->cols	= cols;

	memset(&view,0,sizeof(view));

	view.rows = screen->rows = rows;
	view.cols = screen->cols = cols;

	// Update menu toggle (if available)
	model -= 2;

	if(model >= 0 && model <= G_N_ELEMENTS(screen_size_text))
	{
		if(screen_size_menu[model] && !gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(screen_size_menu[model])))
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(screen_size_menu[model]),TRUE);
	}

	// Redraw
	if(terminal && terminal->window)
		action_redraw(0);

 }

#ifdef HAVE_ALTSCREEN
 static void view_changed(H3270 *session, unsigned short rows, unsigned short cols)
 {
	if(!screen || rows > screen->rows || cols > screen->cols )
		return;

	Trace("View changed from %d x %d to %d x %d",view.rows,view.cols,rows,cols);

	view.rows = rows;
	view.cols = cols;

	// Redraw
	if(terminal && terminal->window)
		action_redraw(0);
 }
#endif

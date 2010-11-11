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
 * Este programa está nomeado como topwindow.c e possui - linhas de código.
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
#include "actions.h"
#include "ui_parse.h"

/*---[ Globals ]------------------------------------------------------------------------------------------*/

 GtkWidget	*topwindow 		= NULL;
 GList 		*main_icon		= NULL;
 gchar		*program_logo	= NULL;

#ifdef MOUSE_POINTER_CHANGE
 GdkCursor	*wCursor[CURSOR_MODE_3270];
#endif

/*---[ Implement ]----------------------------------------------------------------------------------------*/

 int CreateTopWindow(void)
 {
#ifdef MOUSE_POINTER_CHANGE

	#ifdef WIN32

		static const gchar *cr[CURSOR_MODE_3270] = {	"arrow",
														"wait",
														"arrow",

														"sizenwse",	// Top-left
														"sizenesw",	// Top-right
														"sizens",	// Top
														"sizenesw",	// Bottom-left
														"sizenwse",	// Bottom-right
														"sizens",	// Bottom
														"sizewe",	// Left
														"sizewe",	// Right
														"sizeall"	// Inside

													};

	#else

		static int 		cr[CURSOR_MODE_3270] = { 	GDK_XTERM,
														GDK_WATCH,
														GDK_X_CURSOR,

														GDK_TOP_LEFT_CORNER, 		// Top-left
														GDK_TOP_RIGHT_CORNER,		// Top-right
														GDK_TOP_SIDE,				// Top
														GDK_BOTTOM_LEFT_CORNER,		// Bottom-left
														GDK_BOTTOM_RIGHT_CORNER,	// Bottom-right
														GDK_BOTTOM_SIDE,			// Bottom
														GDK_LEFT_SIDE,				// Left
														GDK_RIGHT_SIDE,				// Right
														GDK_FLEUR					// Inside

													};

	#endif // WIN32

	int f;

#endif // MOUSE_POINTER_CHANGE

	GdkPixbuf 	*pix = NULL;
	gchar		*ptr;

	init_actions();

#ifdef MOUSE_POINTER_CHANGE

	// Load mouse pointers
	#ifdef WIN32
		for(f=0;f<CURSOR_MODE_3270;f++)
			wCursor[f] = gdk_cursor_new_from_name(gdk_display_get_default(), cr[f]);
	#else
		for(f=0;f<CURSOR_MODE_3270;f++)
			wCursor[f] = gdk_cursor_new(cr[f]);
	#endif

#endif // MOUSE_POINTER_CHANGE

	if(!CreateTerminalWindow())
		return -1;

	// Load UI - Create toplevel window
	ptr = g_build_filename(program_data,"ui",NULL);
	topwindow = create_window_from_ui_files(ptr,terminal);
	g_free(ptr);

	// Load program logo
	if(program_logo && g_file_test(program_logo,G_FILE_TEST_IS_REGULAR))
	{
		pix = gdk_pixbuf_new_from_file(program_logo,NULL);
	}
	else
	{
		gchar *filename = g_strdup_printf("%s%c%s.%s", program_data, G_DIR_SEPARATOR, PROGRAM_NAME, LOGOEXT);

		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			pix = gdk_pixbuf_new_from_file(filename, NULL);

		Trace("pixbuf(%s): %p",filename,pix);
		g_free(filename);
	}

	if(pix)
	{
		main_icon = g_list_append(main_icon, pix);
		g_object_set_data_full(G_OBJECT(topwindow),"logo",pix,g_object_unref);

#if defined( HAVE_IGEMAC )
		gtk_osxapplication_set_dock_icon_pixbuf(osxapp,pix);
#endif

	}

	gtk_window_set_icon_list(GTK_WINDOW(topwindow),main_icon);
	return 0;
 }

/*
 * Modifications and original code Copyright 1993, 1994, 1995, 1996,
 *    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *   All Rights Reserved.  GTRC hereby grants public use of this software.
 *   Derivative works based on this software must incorporate this copyright
 *   notice.
 *
 * c3270 and wc3270 are distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	g3270.c
 *		A GTK based 3270 Terminal Emulator
 *		Main proceudre.
 */

#include "config.h"
#include "globals.h"
#include <glib.h>
#include <glib/gstdio.h>

#if !defined(_WIN32) /*[*/
#include <sys/wait.h>
#include <signal.h>
#endif /*]*/

#include <errno.h>
#include <lib3270/3270ds.h>
#include "ftc.h"
#include "macrosc.h"
#include "printerc.h"
#include "togglesc.h"
#include "utilc.h"
#include "xioc.h"

#if defined(_WIN32) /*[*/
#include <windows.h>
#include "winversc.h"
#include "windirsc.h"
#endif /*]*/

#include "g3270.h"
#include <lib3270/hostc.h>

#if !defined(_WIN32) /*[*/
/* Base keymap. */
static char *base_keymap1 =
"Ctrl<Key>]: Escape\n"
"Ctrl<Key>a Ctrl<Key>a: Key(0x01)\n"
"Ctrl<Key>a Ctrl<Key>]: Key(0x1d)\n"
"Ctrl<Key>a <Key>c: Clear\n"
"Ctrl<Key>a <Key>e: Escape\n"
"Ctrl<Key>a <Key>i: Insert\n"
"Ctrl<Key>a <Key>r: Reset\n"
"Ctrl<Key>a <Key>l: Redraw\n"
"Ctrl<Key>a <Key>m: Compose\n"
"Ctrl<Key>a <Key>^: Key(notsign)\n"
"<Key>DC: Delete\n"
"<Key>UP: Up\n"
"<Key>DOWN: Down\n"
"<Key>LEFT: Left\n"
"<Key>RIGHT: Right\n"
"<Key>HOME: Home\n"
"Ctrl<Key>a <Key>1: PA(1)\n"
"Ctrl<Key>a <Key>2: PA(2)\n"
"Ctrl<Key>a <Key>3: PA(3)\n";
static char *base_keymap2 =
"<Key>F1: PF(1)\n"
"Ctrl<Key>a <Key>F1: PF(13)\n"
"<Key>F2: PF(2)\n"
"Ctrl<Key>a <Key>F2: PF(14)\n"
"<Key>F3: PF(3)\n"
"Ctrl<Key>a <Key>F3: PF(15)\n"
"<Key>F4: PF(4)\n"
"Ctrl<Key>a <Key>F4: PF(16)\n"
"<Key>F5: PF(5)\n"
"Ctrl<Key>a <Key>F5: PF(17)\n"
"<Key>F6: PF(6)\n"
"Ctrl<Key>a <Key>F6: PF(18)\n";
static char *base_keymap3 =
"<Key>F7: PF(7)\n"
"Ctrl<Key>a <Key>F7: PF(19)\n"
"<Key>F8: PF(8)\n"
"Ctrl<Key>a <Key>F8: PF(20)\n"
"<Key>F9: PF(9)\n"
"Ctrl<Key>a <Key>F9: PF(21)\n"
"<Key>F10: PF(10)\n"
"Ctrl<Key>a <Key>F10: PF(22)\n"
"<Key>F11: PF(11)\n"
"Ctrl<Key>a <Key>F11: PF(23)\n"
"<Key>F12: PF(12)\n"
"Ctrl<Key>a <Key>F12: PF(24)\n";

/* Base keymap, 3270 mode. */
static char *base_3270_keymap =
"Ctrl<Key>a <Key>a: Attn\n"
"Ctrl<Key>c: Clear\n"
"Ctrl<Key>d: Dup\n"
"Ctrl<Key>f: FieldMark\n"
"Ctrl<Key>h: Erase\n"
"Ctrl<Key>i: Tab\n"
"Ctrl<Key>j: Newline\n"
"Ctrl<Key>l: Redraw\n"
"Ctrl<Key>m: Enter\n"
"Ctrl<Key>r: Reset\n"
"Ctrl<Key>u: DeleteField\n"
"<Key>IC: ToggleInsert\n"
"<Key>DC: Delete\n"
"<Key>BACKSPACE: Erase\n"
"<Key>HOME: Home\n"
"<Key>END: FieldEnd\n";

#else /*][*/

/* Base keymap. */
static char *base_keymap =
       "Alt <Key>1:      PA(1)\n"
       "Alt <Key>2:      PA(2)\n"
       "Alt <Key>3:      PA(3)\n"
  "Alt Ctrl <Key>]:      Key(0x1d)\n"
      "Ctrl <Key>]:      Escape\n"
       "Alt <Key>^:      Key(notsign)\n"
       "Alt <Key>c:      Clear\n"
       "Alt <Key>l:      Redraw\n"
       "Alt <Key>m:      Compose\n"
     "Shift <Key>F1:     PF(13)\n"
     "Shift <Key>F2:     PF(14)\n"
     "Shift <Key>F3:     PF(15)\n"
     "Shift <Key>F4:     PF(16)\n"
     "Shift <Key>F5:     PF(17)\n"
     "Shift <Key>F6:     PF(18)\n"
     "Shift <Key>F7:     PF(19)\n"
     "Shift <Key>F8:     PF(20)\n"
     "Shift <Key>F9:     PF(21)\n"
     "Shift <Key>F10:    PF(22)\n"
     "Shift <Key>F11:    PF(23)\n"
     "Shift <Key>F12:    PF(24)\n";

/* Base keymap, 3270 mode. */
static char *base_3270_keymap =
      "Ctrl <Key>a:      Attn\n"
       "Alt <Key>a:      Attn\n"
      "Ctrl <Key>c:      Clear\n"
      "Ctrl <Key>d:      Dup\n"
       "Alt <Key>d:      Dup\n"
      "Ctrl <Key>f:      FieldMark\n"
       "Alt <Key>f:      FieldMark\n"
      "Ctrl <Key>h:      Erase\n"
       "Alt <Key>i:      Insert\n"
"Shift Ctrl <Key>i:      BackTab\n"
      "Ctrl <Key>i:      Tab\n"
      "Ctrl <Key>j:      Newline\n"
      "Ctrl <Key>l:      Redraw\n"
      "Ctrl <Key>m:      Enter\n"
      "Ctrl <Key>r:      Reset\n"
       "Alt <Key>r:      Reset\n"
      "Ctrl <Key>u:      DeleteField\n"
      "Ctrl <Key>v:      Paste\n"
           "<Key>INSERT: ToggleInsert\n"
     "Shift <Key>TAB:    BackTab\n"
           "<Key>BACK:   Erase\n"
     "Shift <Key>END:    EraseEOF\n"
           "<Key>END:    FieldEnd\n"
     "Shift <Key>LEFT:   PreviousWord\n"
     "Shift <Key>RIGHT:  NextWord\n";
#endif /*]*/

/* Callback for connection state changes. */
static void main_connect(Boolean status)
{
	Trace("%s: status: %d Connected: %d",__FUNCTION__,status,(int) CONNECTED);

	if(status)
	{
		cMode |= CURSOR_MODE_ENABLED;
	}
	else
	{
		SetStatusCode(STATUS_CODE_DISCONNECTED);
		cMode &= ~CURSOR_MODE_ENABLED;
		ctlr_erase(True);
	}

	if(terminal)
	{
		gtk_widget_set_sensitive(terminal,status ? TRUE : FALSE);
		DrawOIA(terminal,color,pixmap);
		gtk_widget_queue_draw(terminal);
		gtk_widget_grab_focus(terminal);
	}

}

static void log_callback(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer id)
{
	WriteLog(id, "%s", message);
}

static int g3270_init(int *argc, char ***argv)
{
#if defined(_WIN32) /*[*/
	gchar *ptr = g_strdup(*argv[0]);
	g_chdir(g_path_get_dirname(ptr));
	g_free(ptr);
	Trace("Current dir: %s",g_get_current_dir());
#endif

	/* Init GTK stuff */
	g_thread_init(0);
	gtk_init(argc, argv);

	g_log_set_default_handler(log_callback,"GLog");

	OpenConfigFile();

	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	if(Register3270IOCallbacks(&g3270_io_callbacks))
	{
		g_error( _( "Can't register into lib3270 I/O callback table." ) );
		return -1;
	}

	if(Register3270ScreenCallbacks(&g3270_screen_callbacks))
	{
		g_error( _( "Can't register into lib3270 screen callback table." ) );
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	const char	*cl_hostname = CN;

//	printf("%s\n\nCopyright 1989-2008 by Paul Mattes, GTRC and others.\n",build);

	if(g3270_init(&argc,&argv))
		return -1;

	/* Handle initial settings. */
	initialize_toggles();

	if(CreateTopWindow())
		return -1;

//#ifdef DEBUG
//	gdk_window_set_debug_updates(TRUE);
//#endif

	add_resource("keymap.base",
#if defined(_WIN32) /*[*/
	    base_keymap
#else /*][*/
	    xs_buffer("%s%s%s", base_keymap1, base_keymap2, base_keymap3)
#endif /*]*/
	    );
	add_resource("keymap.base.3270", NewString(base_3270_keymap));

	cl_hostname = lib3270_init(&argc, (const char **)argv);


	register_schange(ST_CONNECT, main_connect);
//	register_schange(ST_3270_MODE, main_connect);
//	register_schange(ST_EXITING, main_exiting);
#if defined(X3270_FT) /*[*/
	ft_init();
#endif /*]*/
#if defined(X3270_PRINTER) /*[*/
	printer_init();
#endif /*]*/

	Trace("Topwindow: %p (%d) terminal: %p (%d)",topwindow,GTK_IS_WIDGET(topwindow),terminal,GTK_IS_WIDGET(terminal));
	gtk_widget_show_all(topwindow);
	gtk_widget_grab_focus(terminal);
	gtk_widget_grab_default(terminal);

	/* Update screen */
	while(gtk_events_pending())
		gtk_main_iteration();

	/* Connect to the host. */
	screen_suspend();
	if (cl_hostname != CN) {

		if(host_connect(cl_hostname) < 0)
		{
			while(gtk_events_pending())
				gtk_main_iteration();

			x3270_exit(1);
		}

		/* Wait for negotiations to complete or fail. */
		Trace("Waiting for negotiations with %s to complete or fail",cl_hostname);

		while(!IN_ANSI && !IN_3270)
		{

			while(gtk_events_pending())
				gtk_main_iteration();

			if(!PCONNECTED)
			{
				Trace("Negotiation with %s failed!",cl_hostname);
				x3270_exit(1);
			}
		}
	}

	screen_resume();
	screen_disp();
	peer_script_init();

	/* Process events forever. */
	Trace("Entering %s main loop","GTK");
	gtk_main();
	Trace("%s main loop has finished","GTK");

	CloseConfigFile();
	return 0;
}

gchar * FindSystemConfigFile(const gchar *name)
{
	const gchar * const	*list =  g_get_system_config_dirs();
 	gchar					*filename;
 	int						f;

	// Search for the file in gtk's system config path
 	for(f=0;list[f];f++)
 	{
		filename = g_build_filename(list[f],PACKAGE_NAME,name,NULL);
		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			return filename;
		g_free(filename);
 	}

	// Check if the file is available in current directory
	if(g_file_test(name,G_FILE_TEST_IS_REGULAR))
		return g_strdup(name);

#ifdef DEBUG
	filename = g_build_filename("..","..","src",PACKAGE_NAME,name,NULL);
	if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		return filename;
	g_free(filename);
#endif

	return 0;
}


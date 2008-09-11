/*
 * Copyright 1995, 1999, 2000, 2001, 2002, 2003, 2005 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */

#ifndef PLUGINS_H_INCLUDED

	#define PLUGINS_H_INCLUDED 1

	#include <gtk/gtk.h>

	void SetHostname(const gchar *hostname);
	void SetLUname(const gchar *luname);
	void AddPluginUI(GtkUIManager *ui);
	void LoadCustomActions(GtkUIManager *ui, GtkActionGroup **groups, guint n_actions);

#endif // PLUGINS_H_INCLUDED


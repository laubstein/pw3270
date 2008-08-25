/*
* Copyright 2008, Banco do Brasil S.A.
*
* This file is part of g3270
*
* This program file is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program in a file named COPYING; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
*
* Authors:
*
* Perry Werneck<perry.werneck@gmail.com>
*
*/

#include <errno.h>
#include <glib.h>
#include <lib3270/plugins.h>
#include "g3270.h"

/*---[ Structs ]------------------------------------------------------------------------------------------------*/

 typedef struct _call_parameter
 {
 	const gchar *name;
 	const gchar *arg;
 } CALL_PARAMETER;

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 static GSList *plugins = NULL;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 int LoadPlugins(void)
 {
 	GDir			*dir;
 	const gchar	*name;
 	gchar			*path;
 	GModule			*handle;
 	gchar			*filename;

	if(!g_module_supported())
		return EINVAL;

	path = g_build_filename(DATAPATH,"plugins",NULL);

	Trace("Loading plugins in \"%s\"",path);

    dir = g_dir_open(path,0,NULL);
    if(!dir)
		return ENOENT;

	name = g_dir_read_name(dir);
	while(name)
	{
//		G_MODULE_SUFFIX
		filename = g_build_filename(path,name,NULL);
		handle = g_module_open(filename,G_MODULE_BIND_LOCAL);
		if(handle)
			plugins = g_slist_append(plugins,handle);
		else
			Log("Can't load %s",filename);
		g_free(filename);
		name = g_dir_read_name(dir);
	}

	g_dir_close(dir);
	g_free(path);

    return 0;
 }

 void unload(GModule *handle,gpointer arg)
 {
 	g_module_close(handle);
 }

 int UnloadPlugins(void)
 {
 	g_slist_foreach(plugins,(GFunc) unload,NULL);
 	g_slist_free(plugins);
 	plugins = NULL;
	return 0;
 }

 void call(GModule *handle, CALL_PARAMETER *arg)
 {
	void (*ptr)(const gchar *arg) = NULL;
	if(g_module_symbol(handle, arg->name, (gpointer) ptr))
		ptr(arg->arg);
 }

 void CallPlugins(const gchar *name, const gchar *arg)
 {
 	CALL_PARAMETER p = { name, arg };

 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) call,&p);

 }


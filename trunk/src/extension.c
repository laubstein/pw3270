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

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <dlfcn.h>

#include "g3270.h"
#include "trace.h"

/*---[ Publics ]--------------------------------------------------------------*/

 static EXTENSION *first = 0;
 static EXTENSION *last  = 0;

/*---[ Implement ]------------------------------------------------------------*/

 int LoadExtensions(const char *dir)
 {
    DIR 			*hDir = opendir(dir);
	struct dirent	*fl;
	char			buffer[4096];

	if(!hDir)
	{
		WriteLog("Can't search extensions in \"%s\": %s",dir,strerror(errno));
		return errno;
	}

    for(fl = readdir(hDir);fl; fl = readdir(hDir))
	{
	   if((fnmatch("*.so", fl->d_name, FNM_PATHNAME)) == 0)
	   {
		   snprintf(buffer,4095,"%s/%s",dir,fl->d_name);
		   OpenExtension(buffer);
	   }
	}

	closedir(hDir);
    return 0;
 }

 void UnloadExtensions(void)
 {
	 while(first)
	    CloseExtension(first);

	 CHKPoint();
 }

 EXTENSION *OpenExtension(const char *path)
 {
	 EXTENSION		*rc;
	 void			*handle;
	 int			i;

     handle = dlopen(path, RTLD_LAZY);
	 if(!handle)
	 {
		 WriteLog("Error loading \"%s\": %s",path,dlerror());
		 return 0;
	 }

	 rc = malloc(sizeof(EXTENSION));

	 if(!rc)
		 return rc;

     memset(rc,0,sizeof(EXTENSION));
	 rc->handle = handle;

	 if(!first)
	 {
		 first = last = rc;
	 }
	 else
	 {
		 rc->up = last;
		 last   = rc;
	 }

     i = CallExtension(rc,"g3270OpenExtension",0);
	 if(i)
		 WriteLog("Opening \"%s\": rc=%d",path,i);
	 else
		 WriteLog("Extension \"%s\" loaded.",path);

	 return rc;
 }

 int CloseExtension(EXTENSION *ext)
 {
	 if(ext->up)
		 ext->up->down = ext->down;
	 else
		 first = ext->down;

	 if(ext->down)
		 ext->down->up = ext->up;
	 else
		 last = ext->down;

	 DBGPrintf("Removendo %p (first: %p last: %p)",ext,first,last);

	 if(ext->handle)
	 {
         CallExtension(ext,"g3270CloseExtension",0);
		 CHKPoint();
		 dlclose(ext->handle);
		 CHKPoint();
	 }

	 CHKPoint();

	 free(ext);

	 CHKPoint();

	 return 0;
 }

 int CallExtension(EXTENSION *ext, const char *function, GtkWidget *widget)
 {
	 int (*call)(GtkWidget *) = 0;

	 if(!widget)
		 widget = top_window;

	 if(ext->handle)
	 {
	    call = dlsym(ext->handle,function);
	    if(call)
		   return call(widget);
	 }
	 return 0;
 }

 void SetExtensionsChar(const char *function, const char *parameter)
 {
    void 		(*call)(GtkWidget *, const char *) = 0;
    EXTENSION	*ext = first;

    while(ext)
    {
       call = dlsym(ext->handle,function);
	   if(call)
		   call(top_window,parameter);
       ext = ext->down;
    }
 }

 GtkItemFactoryCallback QueryActionCallback(const char *name)
 {
	GtkItemFactoryCallback	call;
    char 					function[512];

	snprintf(function,511,"g3270_action_%s",name);

    EXTENSION	*ext = first;

    while(ext)
    {
       call = dlsym(ext->handle,function);
	   if(call)
		   return call;
       ext = ext->down;
    }

	return 0;
 }

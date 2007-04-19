
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <fnmatch.h>
#include <dlfcn.h>

#include "g3270.h"

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
	   return errno;

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
 }
 
 EXTENSION *OpenExtension(const char *path)
 {
	 EXTENSION *rc;
	 void *handle;

     handle = dlopen(path, RTLD_NOW);
	 if(!handle)
		 return 0;

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

     CallExtension(rc,"g3270OpenExtension",0);
	 
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
	 
	 if(ext->handle)
	 {
         CallExtension(ext,"g3270CloseExtension",0);
		 dlclose(ext->handle);
	 }
	 
	 free(ext);
	 
	 return 0;
 }
 
 void CallExtension(EXTENSION *ext, const char *function, GtkWidget *widget)
 {
	 void (*call)(GtkWidget *) = 0;
	 
	 if(!widget)
		 widget = top_window;
	 
	 if(ext->handle)
	 {
	    call = dlsym(ext->handle,function);
	    if(call)
		   call(widget);
	 }
	 
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

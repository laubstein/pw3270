
 #include "g3270.h"
 #include <dlfcn.h>

/*---[ Macros ]---------------------------------------------------------------*/


/*---[ Static ]---------------------------------------------------------------*/

 static const struct _callbacks
 {
    const char				*name;
    GtkItemFactoryCallback	callback;
 }
 callbacks[] =
 {
	{ "print",				action_print			},
	{ "print_selection",	action_print_selection	},
	{ "print_copy",			action_print_copy		},
	{ "exit",				action_exit				},
	{ "copy",				action_copy				},
	{ "append",				action_append			},
	{ "paste",				action_paste			},
	{ "clear",				action_clear			},
	{ "select_all",			action_select_all		},
	{ "remove_selection",	action_remove_selection	},
	{ "crosshair",			action_crosshair		},
	{ "connect",			action_connect			},
	{ "F8",					action_F7				},
	{ "F7",					action_F8				},
	{ "disconnect",			action_disconnect		}

 };

/*---[ Implement ]------------------------------------------------------------*/

 static void Accelerator(GtkItemFactoryEntry *itn, const char *accelerator)
 {
    itn->accelerator = g_strdup(accelerator);
 }

 static void Action(GtkItemFactoryEntry *itn, const char *name)
 {
    int f;

    for(f=0;f< (sizeof(callbacks)/sizeof(struct _callbacks));f++)
    {
    	if(!strcmp(callbacks[f].name,name))
    	{
    		itn->callback = callbacks[f].callback;
    		DBGPrintf("%s=%p",itn->path,itn->callback);
    		return;
    	}
    }

    Log("Invalid action code: \"%s\"",name);
 }

 static void Type(GtkItemFactoryEntry *itn, const char *type)
 {
 	char key[1024];
 	snprintf(key,1023,"<%s>",type);
    itn->item_type = g_strdup(key);
 }

 int LoadMenu(const char *filename, GtkItemFactory *factory)
 {
 	static const struct _cmd
 	{
 		const char *key;
 		void (*exec)(GtkItemFactoryEntry *, const char *);
 	} cmd[] =
 	{
 		{ "Accelerator",	Accelerator },
 		{ "Action",			Action		},
 		{ "Type",			Type		}
 	};

 	unsigned char		buffer[1024];

 	FILE 				*arq	= fopen(filename,"r");
    int  				qtd		= 0;
    unsigned char		*ptr;
    unsigned char		*ln;
    int					current = -1;
    int					f;

    GtkItemFactoryEntry	*itens;

    if(!arq)
    {
    	Error("Can't load \"%s\"",filename);
    	return -1;
    }

    while(fgets((char *) buffer,1023,arq))
    {
    	if(*buffer == '/')
    	   qtd++;
    }

    DBGPrintf("%d menu options",qtd);

    itens = g_malloc(qtd * sizeof(GtkItemFactoryEntry));
    if(!itens)
       return -1;

    rewind(arq);

    while(fgets((char *) buffer,1023,arq) && current < qtd)
    {
   	    for(ln=buffer;*ln && *ln <= ' ';ln++);

    	for(ptr=ln;*ptr && *ptr >= ' ';ptr++);
    	*ptr = 0;

    	if(*buffer == '/')
    	{
    		current++;
    		DBGPrintf("%d = %s",current,buffer);
    		memset(itens+current,0,sizeof(GtkItemFactoryEntry));

    		(itens+current)->path = g_strdup((char *) buffer);

    	}
    	else if(current >= 0 && *ln)
    	{
		   ptr = (unsigned char *) strchr((char *) ln,'=');
		   if(ptr)
		   {
		      *ptr++ = 0;
//		      DBGPrintf("%s=\"%s\"",ln,ptr);

		      for(f=0;f< (sizeof(cmd)/sizeof(struct _cmd));f++)
		      {
			     if(!strcmp(cmd[f].key,(char *) ln))
			        cmd[f].exec(itens+current,(char *) ptr);
		      }
		   }
    	}
    }

    fclose(arq);

	if(current > 0)
	   gtk_item_factory_create_items(factory, qtd, itens, NULL);

    return qtd;
 }

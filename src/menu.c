
 #include "g3270.h"
 #include <dlfcn.h>

/*---[ Parameter structure ]--------------------------------------------------*/

 typedef struct _parmdata
 {
 	GtkItemFactoryEntry entry;
 } PARMDATA;

/*---[ Implement ]------------------------------------------------------------*/

 static void Accelerator(PARMDATA *data, const char *accelerator)
 {
    data->entry.accelerator = g_strdup(accelerator);
 }

 static void Action(PARMDATA *data, const char *name)
 {
    int f;

    for(f=0;f< action_callback_counter;f++)
    {
    	if(!strcmp(action_callbacks[f].name,name))
    	{
    		data->entry.callback = action_callbacks[f].callback;
    		return;
    	}
    }

    Log("Invalid action code: \"%s\"",name);
 }

 static void Type(PARMDATA *data, const char *type)
 {
 	char key[1024];
 	snprintf(key,1023,"<%s>",type);
    data->entry.item_type = g_strdup(key);
 }

 static void InsertItem(GtkItemFactory *factory, PARMDATA *item, int *qtd)
 {
    if(item->entry.path)
	{
	   // entry.callback_action e o valor passado no segundo parametro da action.
	   DBGPrintf("%d %s",item->entry.callback_action,item->entry.path);

       gtk_item_factory_create_item(	factory,
										(GtkItemFactoryEntry *) &item->entry,
										0,
										1 );
       (*qtd)++;

       g_free(item->entry.path);
	}
    memset(item,0,sizeof(PARMDATA));
 }

 int LoadMenu(const char *filename, GtkItemFactory *factory)
 {
 	static const struct _cmd
 	{
 		const char *key;
 		void (*exec)(PARMDATA *, const char *);
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
    int					f;

    PARMDATA			item;

    memset(&item,0,sizeof(item));

    while(fgets((char *) buffer,1023,arq))
    {
   	    for(ln=buffer;*ln && *ln <= ' ';ln++);

    	for(ptr=ln;*ptr && *ptr >= ' ';ptr++);
    	*ptr = 0;

    	if(*buffer == '/')
    	{
           InsertItem(factory, &item, &qtd);
		   item.entry.path = g_strdup((char *) buffer);
    	}
    	else if(item.entry.path && *ln && *ln != '#')
    	{
		   ptr = (unsigned char *) strchr((char *) ln,'=');
		   if(ptr)
		   {
		      *ptr++ = 0;
		      for(f=0;f< (sizeof(cmd)/sizeof(struct _cmd));f++)
		      {
			     if(!strcmp(cmd[f].key,(char *) ln))
			        cmd[f].exec(&item,(char *) ptr);
		      }
		   }
    	}
    }

	if(item.entry.path)
       InsertItem(factory, &item, &qtd);

    fclose(arq);

    return qtd;
 }

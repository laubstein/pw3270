
 #include "g3270.h"

/*---[ Parameter buffer ]-----------------------------------------------------*/

 enum item_types
 {
	 ITEM_TYPE_BUTTON,
	 ITEM_TYPE_SEPARATOR
 };

/*---[ Parameter buffer ]-----------------------------------------------------*/

 typedef struct _parameter
 {
	  int	pos;
	  int  type;
	  char	*item;
	  void (*action)(GtkWidget *w, gpointer data);
 } PARAMETER;
 
/*---[ Implement ]------------------------------------------------------------*/

 static void ProcessDefinition(PARAMETER *prm, void (*proc)(PARAMETER *p, void *data), void *data)
 {
	 if(!prm->item)
		 return;

	 DBGPrintf("%s=%p",prm->item,prm->action);
	 
	 proc(prm,data);
	 
	 free(prm->item);
	 memset(prm,0,sizeof(PARAMETER));
 }
 
 static void Action(PARAMETER *prm, const char *name)
 {
    int f;

    for(f=0;f< action_callback_counter;f++)
    {
    	if(!strcmp(action_callbacks[f].name,name))
    	{
    		prm->action = action_callbacks[f].callback;
    		return;
    	}
    }
    prm->action = 0;
	
    Log("Invalid action code: \"%s\"",name);
 }

 static void Type(PARAMETER *prm, const char *name)
 {
	 static const struct _type
	 {
	    const char			*key;
		unsigned short	id;
	 } type[] =
	 {
		 {	"button",		ITEM_TYPE_BUTTON	},
		 {	"separator",	ITEM_TYPE_SEPARATOR	}
	 };
	 int f;
	 
	 for(f = 0; f < (sizeof(type)/sizeof(struct _type));f++)
	 {
		 if(!strcmp(name,type[f].key))
		 {
			 prm->type = type[f].id;
			 return;
		 }
	 }
	 
	 Log("Invalid type code: \"%s\"",name);
	 
 }
 
 static int LoadUIDefinition(FILE *in, const char *id, void (*proc)(PARAMETER *p, void *data), void *data)
 {
	 static const struct _cmd
 	 {
 		const char *key;
 		void (*exec)(PARAMETER *, const char *);
 	 } cmd[] =
  	 {
 		{ "action",			Action		},
		{ "type",			Type		}
 	 };
	 
	 char			buffer[1024];
     char 			*ptr;
     char 			*ln;
	 int			enabled = 0;
	 int 			pos		= 0;
	 int 			szId	= strlen(id);
	 PARAMETER		parm;
	 int 			f;
	 
	 memset(&parm,0,sizeof(PARAMETER));
	 
	 rewind(in);
	 
     while(fgets(buffer,1023,in))
     {
   	    for(ln=buffer;*ln && *ln <= ' ';ln++);

    	for(ptr=ln;*ptr && *ptr >= ' ';ptr++);
    	*ptr = 0;
		
		if(*buffer == '/')
		{
			ProcessDefinition(&parm,proc,data);
			
			enabled = !strncmp(id,buffer+1,szId);
			if(enabled)
			{
			   parm.pos  = pos++;
			   parm.item = strdup(buffer+szId+1);
			}
		}
		else if(enabled)
		{
		   ptr = strchr(ln,'=');
		   if(ptr)
		   {
		      DBGMessage(ln);
		      *ptr++ = 0;
		      for(f=0;f< (sizeof(cmd)/sizeof(struct _cmd));f++)
		      {
			     if(!strcmp(cmd[f].key,ln))
				 {
		            DBGPrintf("%s set to %s in (%s)",ln,ptr,parm.item);
			        cmd[f].exec(&parm,ptr);
				 }
		      }
		   }
		}
	}
	 
	ProcessDefinition(&parm,proc,data);
	
	return 0;
 }
 
 static void AddToolbar(PARAMETER *p, GtkWidget *toolbar)
 {
	 GtkToolItem *item;


	 switch(p->type)
	 {
	 case ITEM_TYPE_BUTTON:
		 item = gtk_tool_button_new_from_stock(p->item+1);
	     if(p->action)
 	        g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(p->action), 0);
	     break;
	 
	 case ITEM_TYPE_SEPARATOR:
		 item = gtk_separator_tool_item_new();
		 break;
 	 }
	 
     gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(item),-1);
 }
 
 GtkWidget	*LoadToolbar(GtkWidget *window)
 {
#ifdef DATADIR
    static const char	*filename = DATADIR "/toolbar.conf";
#else
    static const char	*filename = "./toolbar.conf";
#endif	 
	 
	GtkWidget	*toolbar = 0;
	FILE 		*in		 = fopen(filename,"r");
	 
	if(!in)
		return 0;

    toolbar = gtk_toolbar_new();
    GTK_WIDGET_UNSET_FLAGS(toolbar,GTK_CAN_FOCUS);
	
	LoadUIDefinition(in,"toolbar", (void (*)(PARAMETER *, void *)) AddToolbar, toolbar);
	
	fclose(in);
	 
    return toolbar;
 }

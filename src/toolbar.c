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
	  char	*parm;
	  void (*action)(GtkWidget *w, gpointer data);
 } PARAMETER;

/*---[ Implement ]------------------------------------------------------------*/

 static void ReleaseString(gpointer string)
 {
 	if(string)
 	{
 		DBGPrintf("Releasing \"%s\"",(char *) string);
 		g_free(string);
 	}
 }

 static void SetObjectParameter(GObject *obj, const char *name, const char *string)
 {
	 DBGPrintf("Allocating \"%s\"",string);
     g_object_set_data_full(obj,name,g_strdup((char *) string),ReleaseString);
 }

 static void ProcessDefinition(PARAMETER *prm, GObject * (*proc)(PARAMETER *p, void *data), void *data)
 {
	 GObject *obj;

	 if(!prm->item)
		 return;

//	 DBGPrintf("%s=%p",prm->item,prm->action);

	 obj = proc(prm,data);

	 if(prm->parm)
	 {
		if(obj)
		   SetObjectParameter(G_OBJECT(obj),"g3270.parameter",prm->parm);
	 	free(prm->parm);
	 }

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

 static void Parm(PARAMETER *prm, const char *parm)
 {
	 if(prm->parm)
		 free(prm->parm);
	 prm->parm = strdup(parm);
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

 int LoadUIDefinition(FILE *in, const char *id, GObject * (*proc)(PARAMETER *p, void *data), void *data)
 {
	 static const struct _cmd
 	 {
 		const char *key;
 		void (*exec)(PARAMETER *, const char *);
 	 } cmd[] =
  	 {
 		{ "action",			Action		},
		{ "type",			Type		},
 		{ "script",			Parm		},
 		{ "command",		Parm		},
 		{ "option",			Parm		}

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
//		      DBGMessage(ln);
		      *ptr++ = 0;
		      for(f=0;f< (sizeof(cmd)/sizeof(struct _cmd));f++)
		      {
			     if(!strcmp(cmd[f].key,ln))
				 {
//		            DBGPrintf("%s set to %s in (%s)",ln,ptr,parm.item);
			        cmd[f].exec(&parm,ptr);
				 }
		      }
		   }
		}
	}

	ProcessDefinition(&parm,proc,data);

	return 0;
 }

#ifdef __GTK_TOOL_ITEM_H__

 static GObject *AddToolbar(PARAMETER *p, GtkWidget *toolbar)
 {
	 GtkToolItem *item = 0;

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

     if(item)
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(item),-1);

	 return G_OBJECT(item);
 }
#endif

 GtkWidget	*LoadToolbar(GtkWidget *window)
 {
#ifdef DATADIR
    static const char	*filename = DATADIR "/toolbar.conf";
#else
    static const char	*filename = "./toolbar.conf";
#endif

	GtkWidget	*toolbar = 0;
	FILE 		*in		 = fopen(filename,"r");

	DBGPrintf("%s: %p",filename,in);

	if(!in)
		return 0;

#ifdef __GTK_TOOL_ITEM_H__
    toolbar = gtk_toolbar_new();
    GTK_WIDGET_UNSET_FLAGS(toolbar,GTK_CAN_FOCUS);
	LoadUIDefinition(in,"toolbar", (GObject * (*)(PARAMETER *, void *)) AddToolbar, toolbar);
#endif

	fclose(in);

    return toolbar;
 }

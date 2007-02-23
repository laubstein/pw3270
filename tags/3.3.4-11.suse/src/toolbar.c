
 #include "g3270.h"

/*---[ Stock buttons ]--------------------------------------------------------*/

 // http://developer.gnome.org/doc/API/2.0/gtk/gtk-Stock-Items.html	 

 static const struct _action
 {
	  void	*item;
	  void (*callback)(GtkWidget *w, gpointer data);
 } action[] =
 {
	 { GTK_STOCK_QUIT, 			action_exit			},
	 { GTK_STOCK_SELECT_ALL, 	action_select_all	},
	 { GTK_STOCK_COPY, 			action_copy			},
	 { GTK_STOCK_PASTE, 		action_paste		},
	 { GTK_STOCK_PRINT, 		action_print		},
	 { GTK_STOCK_CLEAR, 		action_clear		},
	 { GTK_STOCK_CONNECT, 		action_connect		},
	 { GTK_STOCK_DISCONNECT,	action_disconnect	}
 };

/*---[ Implement ]------------------------------------------------------------*/

 GtkWidget	*LoadToolbar(GtkWidget *window)
 {
	GtkWidget	*toolbar = 0;

#ifdef DEBUG
     int f;	 
	 GtkToolItem *item;
	 
	 toolbar = gtk_toolbar_new();
     GTK_WIDGET_UNSET_FLAGS(toolbar,GTK_CAN_FOCUS);
	 
	 for(f=0;f<(sizeof(action)/sizeof(struct _action));f++)
	 {
		item = gtk_tool_button_new_from_stock(action[f].item);
		if(action[f].callback)
 		   g_signal_connect(G_OBJECT(item), "clicked", G_CALLBACK(action[f].callback), 0);
	    gtk_toolbar_insert(GTK_TOOLBAR(toolbar),GTK_TOOL_ITEM(item),-1);
	 }
	 
#endif

	 
	 return toolbar;
 }

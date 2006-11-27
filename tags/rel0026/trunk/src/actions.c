
 #include <gdk/gdkkeysyms.h>
 #include "g3270.h"

 #include "lib/hostc.h"
 #include "lib/kybdc.h"
 #include "lib/actionsc.h"
 #include "lib/3270ds.h"
 #include "lib/tablesc.h"
 #include "lib/screenc.h"

/*---[ Callback table ]-------------------------------------------------------*/

 const struct action_callback action_callbacks[] =
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
	{ "disconnect",			action_disconnect		},
	{ "backtab",			action_BackTab			},
	{ "tab",				action_Tab				},
	{ "home",				action_Home				},
	{ "eraseeof",			action_EraseEOF			},
	{ "left",				action_Left				},
	{ "up",					action_Up				},
	{ "right",				action_Right			},
	{ "down",				action_Down				},
	{ "reset",				action_Reset			},
	{ "delete",				action_Delete			},
	{ "erase",				action_Erase			},
	{ "enter",				action_Enter			},
	{ "insert",				action_Insert			},
	{ "redraw",				action_Redraw			}
 };

 const int action_callback_counter = (sizeof(action_callbacks)/sizeof(struct action_callback));

/*---[ Keyboard processing ]--------------------------------------------*/

 // Replace the Alt-Key mas for one with a more "acceptable" name.
 #define GDK_ALT_MASK GDK_MOD1_MASK

 #ifdef DEBUG
     #define DECLARE_KEYPROC(key, state, action) { key, state, #key " (" #state ")", action, #action }
 #else
     #define DECLARE_KEYPROC(key, state, action) { key, state, action }
 #endif

 #ifdef DEBUG
     #define DECLARE_ACTION(key, state, action, cause, parm1, parm2) { key, state, #key " (" #state ")", action, #action, cause, parm1, parm2 }
 #else
     #define DECLARE_ACTION(key, state, action, cause, parm1, parm2) { key, state, action, cause, parm1, parm2 }
 #endif

 struct WindowActions
 {
   guint		 			keyval;
   guint					state;

#ifdef DEBUG
   const char				*trace;
#endif

   GtkItemFactoryCallback	exec;

#ifdef DEBUG
   const char				*action_trace;
#endif

 };

/*---[ Implement ]------------------------------------------------------------*/

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
    static const struct WindowActions keyproc[] =
    {
     	DECLARE_KEYPROC( GDK_Page_Up, 			0,		action_F7		),
     	DECLARE_KEYPROC( GDK_Page_Down,			0,		action_F8		),

     	DECLARE_KEYPROC( GDK_Print,				0,		action_print	),
     	DECLARE_KEYPROC( GDK_3270_PrintScreen,	0,		action_print	),

     	DECLARE_KEYPROC( GDK_ISO_Left_Tab,		0,		action_BackTab	),
     	DECLARE_KEYPROC( GDK_KP_Subtract,		0,		action_BackTab	),

     	DECLARE_KEYPROC( GDK_KP_Add,			0,		action_Tab		),
     	DECLARE_KEYPROC( GDK_Tab,				0,		action_Tab		),

     	DECLARE_KEYPROC( GDK_Home,				0,		action_Home		),
     	DECLARE_KEYPROC( GDK_End,				0,		action_EraseEOF	),

     	DECLARE_KEYPROC( GDK_Left,				0,		action_Left		),
     	DECLARE_KEYPROC( GDK_Up,				0,		action_Up		),
     	DECLARE_KEYPROC( GDK_Right,				0,		action_Right	),
     	DECLARE_KEYPROC( GDK_Down,				0,		action_Down		),

     	DECLARE_KEYPROC( GDK_KP_Left,			0,		action_Left		),
     	DECLARE_KEYPROC( GDK_KP_Up,				0,		action_Up		),
     	DECLARE_KEYPROC( GDK_KP_Right,			0,		action_Right	),
     	DECLARE_KEYPROC( GDK_KP_Down,			0,		action_Down		),

        DECLARE_KEYPROC( GDK_Clear,				0,		action_Clear	),
        DECLARE_KEYPROC( GDK_3270_Reset,		0,		action_Reset	),

        DECLARE_KEYPROC( GDK_Delete,			0,		action_Delete	),
        DECLARE_KEYPROC( GDK_BackSpace,			0,		action_Erase	),

        DECLARE_KEYPROC( GDK_Return,			0,		action_Enter	),
        DECLARE_KEYPROC( GDK_KP_Enter,			0,		action_Enter	),

        DECLARE_KEYPROC( GDK_Insert,			0,		action_Insert	),

        DECLARE_KEYPROC( GDK_Control_L,			0,		action_Redraw	)



    };

	char			ks[6];
    int				f;

#ifndef USE_GTKIMCONTEXT
    gchar 			*string		= 0;
#endif

    DBGTracex(event->state);

    /* Check for Function keys */
    if(event->keyval >= GDK_F1 && event->keyval <=  GDK_F12)
    {
    	snprintf(ks,5,"%d",(event->keyval - GDK_F1)+1);
    	DBGPrintf("Function-%s",ks);
		action_internal(PF_action, IA_DEFAULT, ks, CN);
    	return TRUE;
    }

    /* Check for special keyproc actions */
    for(f=0; f < (sizeof(keyproc)/sizeof(struct WindowActions));f++)
    {
    	if(keyproc[f].keyval == event->keyval && (event->state & keyproc[f].state) == keyproc[f].state)
    	{
#ifdef DEBUG
		   DBGPrintf("Key: %s\tAction: %s",keyproc[f].trace,keyproc[f].action_trace);
#endif
		   keyproc[f].exec(0,0);
           return TRUE;
    	}
    }

#ifndef USE_GTKIMCONTEXT
    string = event->string;

    DBGPrintf("Keyval: %d (%s) Keychar: \"%s\" State: %04x %s%s%s%s%s%s%s%s%s%s%s%s%s%s",
						event->keyval,
						gdk_keyval_name(event->keyval),
						string,
						event->state,
						event->state & GDK_SHIFT_MASK	? " GDK_SHIFT_MASK"		: "",
						event->state & GDK_LOCK_MASK	? " GDK_LOCK_MASK"		: "",
						event->state & GDK_CONTROL_MASK	? " GDK_CONTROL_MASK"	: "",
						event->state & GDK_MOD1_MASK	? " GDK_MOD1_MASK"		: "",
						event->state & GDK_MOD2_MASK	? " GDK_MOD2_MASK"		: "",
						event->state & GDK_MOD3_MASK	? " GDK_MOD3_MASK"		: "",
						event->state & GDK_MOD4_MASK	? " GDK_MOD4_MASK"		: "",
						event->state & GDK_MOD5_MASK	? " GDK_MOD5_MASK"		: "",
						event->state & GDK_BUTTON1_MASK	? " GDK_BUTTON1_MASK"	: "",
						event->state & GDK_BUTTON2_MASK	? " GDK_BUTTON2_MASK"	: "",
						event->state & GDK_BUTTON3_MASK	? " GDK_BUTTON3_MASK"	: "",
						event->state & GDK_BUTTON4_MASK	? " GDK_BUTTON4_MASK"	: "",
						event->state & GDK_BUTTON5_MASK	? " GDK_BUTTON5_MASK"	: "",
						event->state & GDK_RELEASE_MASK	? " GDK_RELEASE_MASK"	: ""
						);

    /* Check for regular key */
    if(string && *string && !(event->state & (GDK_ALT_MASK|GDK_CONTROL_MASK)) )
    {
 	   // Standard char, use it.
 	   ParseInput(string);
 	   return TRUE;
	}
#endif

    return FALSE;

 }

 void ParseInput(const gchar *string)
 {
    gchar *input;

    input = g_convert(string, -1, "ISO-8859-1", "UTF-8", NULL, NULL, NULL);
    if(!string)
    {
    	Log("Error converting string \"%s\" to ISO-8859-1",string);
    	return;
    }

    // NOTE (perry#1#): Is it the best way?
    emulate_input(input, strlen(input), False);

    g_free(input);
 }

 static int connecting = 0;

 static gpointer ConnectThread(gpointer host)
 {
    connecting++;
    DBGMessage("Connect thread has started");
    Log("Trying \"%s\"",host);
    reconnect = TRUE;
    host_connect(host);
    DBGMessage("Connect thread has ended");
    connecting--;

    UpdateWindowTitle();


    return 0;
 }

 void action_connect(GtkWidget *w, gpointer data)
 {
#if GTK == 2
    GThread   *thd = 0;
#else
    pthread_t  thd = 0;
#endif

    if(connecting)
       return;

    // TODO (perry#9#): Show a dialog box to select the hostname/port.
    if(!cl_hostname)
       return;

#if GTK == 2
    thd =  g_thread_create( ConnectThread, (gpointer) cl_hostname, 0, NULL);
#else
	pthread_create(&thd, NULL, (void * (*)(void *)) ConnectThread, cl_hostname);
#endif

 }

 void action_disconnect(GtkWidget *w, gpointer data)
 {
 	if(connecting)
 	   return;

 	Log("Disconnecting");
 	reconnect = FALSE;
	host_disconnect(False);
 }

 void action_clear(GtkWidget *w, gpointer data)
 {
    action_internal(EraseInput_action, IA_DEFAULT, CN, CN);
 }

 void action_print(GtkWidget *w, gpointer data)
 {
 	char 			filename[1024];
 	int				rows;
 	int				cols;
 	int				row;
 	int				col;
 	const struct ea *trm;
 	FILE			*arq;
 	int				fd;

    snprintf(filename,1023,"%s/%s.XXXXXX",TMPPATH,TARGET);
    fd = mkstemp(filename);

    DBGMessage(filename);

 	arq = fdopen(fd,"w");

 	if(arq)
 	{
       trm = Get3270DeviceBuffer(&rows, &cols);
       if(trm)
       {
          for(row = 0; row < rows; row++)
          {
    	     for(col = 0; col < cols; col++)
    	     {
    	     	fprintf(arq,"%c",ebc2asc[trm->cc]);
    	     	trm++;
    	     }
    	     fprintf(arq,"\n");
          }
       }
       fclose(arq);
       PrintTemporaryFile(filename);
 	}
 	else
 	{
 		Error("Unable to open \"%s\" for writing",filename);
 	}

 }

 void action_F7(GtkWidget *w, gpointer data)
 {
    action_internal(PF_action, IA_DEFAULT, "7", CN);
 }

 void action_F8(GtkWidget *w, gpointer data)
 {
    action_internal(PF_action, IA_DEFAULT, "8", CN);
 }

 void action_BackTab(GtkWidget *w, gpointer data)
 {
    action_internal(BackTab_action, IA_DEFAULT, CN, CN);
 }

 void action_Tab(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Tab_action, IA_DEFAULT, CN, CN);
 }

 void action_Home(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Home_action, IA_DEFAULT, CN, CN);
 }

 void action_EraseEOF(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(EraseEOF_action, IA_DEFAULT, CN, CN);
 }

 void action_Left(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Left_action, IA_DEFAULT, CN, CN);
 }

 void action_Up(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Up_action, IA_DEFAULT, CN, CN);
 }

 void action_Right(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Right_action, IA_DEFAULT, CN, CN);
 }

 void action_Down(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Down_action, IA_DEFAULT, CN, CN);
 }

 void action_Clear(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Clear_action, IA_DEFAULT, CN, CN);
 }

 void action_Reset(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Reset_action, IA_DEFAULT, CN, CN);
 }

 void action_Delete(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Delete_action, IA_DEFAULT, CN, CN);
 }

 void action_Erase(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Erase_action, IA_DEFAULT, CN, CN);
 }

 void action_Enter(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Enter_action, IA_DEFAULT, CN, CN);
 }

 void action_Insert(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(ToggleInsert_action, IA_DEFAULT, CN, CN);
 }

 void action_Newline(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Newline_action, IA_DEFAULT, CN, CN);
 }

 void action_Redraw(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(Redraw_action, IA_DEFAULT, CN, CN);
 }


/*
 void action_(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(_action, IA_DEFAULT, CN, CN);
 }
*/



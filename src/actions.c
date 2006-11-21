
 #include <gdk/gdkkeysyms.h>
 #include "g3270.h"

 #include "lib/hostc.h"
 #include "lib/kybdc.h"
 #include "lib/actionsc.h"
 #include "lib/3270ds.h"
 #include "lib/tablesc.h"

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
   guint		 	keyval;
   guint			state;

#ifdef DEBUG
   const char		*trace;
#endif

   void (*exec)(GtkWidget *, gpointer);

#ifdef DEBUG
   const char		*action_trace;
#endif

 };

 struct TerminalActions
 {
    guint		 	keyval;
    guint			state;

#ifdef DEBUG
    const char		*trace;
#endif

	XtActionProc 	action;

#ifdef DEBUG
    const char		*action_trace;
#endif
	enum iaction 	cause;
	const char 		*parm1;
	const char 		*parm2;
 };

/*---[ Statics ]--------------------------------------------------------------*/

/*---[ Implement ]------------------------------------------------------------*/

 // FIXME (perry#1#): Replace structures with GTK acelerators loaded from configuration file
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


    };

    static const struct TerminalActions actions[] =
    {
		// /opt/gnome/include/gtk-2.0/gdk/gdkkeysyms.h
		// http://www.koders.com/c/fidA3A9523D24A70BAFCE05733E73D558365D103DB3.aspx

		DECLARE_ACTION( GDK_Home,		0,	Home_action, 			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_End,		0,	EraseEOF_action,		IA_DEFAULT, CN, CN ),

		DECLARE_ACTION( GDK_Left,		0,	Left_action,			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Up,			0,	Up_action,				IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Right,		0,	Right_action, 			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Down,		0,	Down_action,			IA_DEFAULT, CN, CN ),

		DECLARE_ACTION( GDK_KP_Left,	0,	Left_action,			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_KP_Up,		0,	Up_action,				IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_KP_Right,	0,	Right_action, 			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_KP_Down,	0,	Down_action,			IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Clear,		0, 	Clear_action, 			IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_3270_Reset,	0,	Reset_action,			IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Tab,	    0,	Tab_action,				IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_KP_Add,	    0,	Tab_action,				IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Delete,		0,	Delete_action,			IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_BackSpace,	0,	Erase_action,			IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Return,		0,	Enter_action,			IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_KP_Enter,	0,	Enter_action,			IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Insert,		0,	ToggleInsert_action,	IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Linefeed,	0,	Newline_action,			IA_DEFAULT, CN, CN )

    };


	char			ks[6];
    int				f;

#ifndef USE_GTKIMCONTEXT
    gchar 			*string		= 0;
#endif

    DBGTracex(event->state);

    for(f=0; f < (sizeof(actions)/sizeof(struct TerminalActions));f++)
    {
    	if(actions[f].keyval == event->keyval && (event->state & actions[f].state) == actions[f].state)
    	{
#ifdef DEBUG
		   DBGPrintf("Key: %s\tAction: %s",actions[f].trace,actions[f].action_trace);
#endif
           action_internal(	actions[f].action,
							actions[f].cause,
							actions[f].parm1,
							actions[f].parm2 );
           return TRUE;
    	}
    }

    /* Check for Function keys */
    if(event->keyval >= GDK_F1 && event->keyval <=  GDK_F12)
    {
    	snprintf(ks,5,"%d",(event->keyval - GDK_F1)+1);
    	DBGPrintf("Function-%s",ks);
		action_internal(PF_action, IA_DEFAULT, ks, CN);
    	return TRUE;
    }

    /* Check for keyproc actions */
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
    	Log("Error converting string to ISO-8859-1");
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

 void action_exit(GtkWidget *w, gpointer data)
 {
 	action_disconnect(0,0);
 	Log("Exiting");
 	gtk_main_quit();
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







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

   void (*exec)(void);

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

 static void Page_Up(void)
 {
    action_internal(PF_action, IA_DEFAULT, "7", CN);
 }

 static void Page_Down(void)
 {
    action_internal(PF_action, IA_DEFAULT, "8", CN);
 }

 // FIXME (perry#1#): Replace structures with GTK acelerators loaded from configuration file
 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {

    static const struct WindowActions keyproc[] =
    {
     	DECLARE_KEYPROC( GDK_Page_Up, 	0, 				Page_Up				),
     	DECLARE_KEYPROC( GDK_Page_Down,	0, 				Page_Down			),
    };

    static const struct TerminalActions actions[] =
    {
		// /opt/gnome/include/gtk-2.0/gdk/gdkkeysyms.h
		// http://www.koders.com/c/fidA3A9523D24A70BAFCE05733E73D558365D103DB3.aspx

		DECLARE_ACTION( GDK_Home,		0,	Home_action, 			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_End,		0,	EraseInput_action,		IA_DEFAULT, CN, CN ),

		DECLARE_ACTION( GDK_Left,		0,	Left_action,			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Up,			0,	Up_action,				IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Right,		0,	Right_action, 			IA_DEFAULT, CN, CN ),
		DECLARE_ACTION( GDK_Down,		0,	Down_action,			IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_Clear,		0, 	Clear_action, 			IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_3270_Reset,	0,	Reset_action,			IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Tab,	    0,	Tab_action,				IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_Delete,		0,	Delete_action,			IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_BackSpace,	0,	BackSpace_action,		IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Return,		0,	Enter_action,			IA_DEFAULT, CN, CN ),
        DECLARE_ACTION( GDK_KP_Enter,	0,	Enter_action,			IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Insert,		0,	ToggleInsert_action,	IA_DEFAULT, CN, CN ),

        DECLARE_ACTION( GDK_Linefeed,	0,	Newline_action,			IA_DEFAULT, CN, CN )

    };


	char			ks[6];
    int				f;
    gchar 			*string		= 0;

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
		   keyproc[f].exec();
           return TRUE;
    	}
    }

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

	/* Unknown key, ignore-it */

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

 void action_connect(GtkWidget *w, gpointer data)
 {
    host_connect(cl_hostname);
 }

 void action_disconnect(GtkWidget *w, gpointer data)
 {
 	Log("Disconnecting");
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
    // TODO (perry#9#): Replace with mkstemp
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





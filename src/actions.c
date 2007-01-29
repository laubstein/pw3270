
 #include <gdk/gdkkeysyms.h>

 #include "g3270.h"

 #include "lib/hostc.h"
 #include "lib/kybdc.h"
 #include "lib/actionsc.h"
 #include "lib/3270ds.h"
 #include "lib/tablesc.h"
 #include "lib/screenc.h"

/*---[ Internal actions ]-----------------------------------------------------*/

 static void action_toogle(GtkWidget *w, gpointer data);

/*---[ Callback table ]-------------------------------------------------------*/

 const struct action_callback action_callbacks[] =
 {
	{ "print",					action_print					},
	{ "toogle",					action_toogle					},
#ifdef __GTK_ABOUT_DIALOG_H__
	{ "about",					action_AboutBox					},
#endif
	{ "print_selection",		action_print_selection			},
	{ "exec_with_selection",	action_exec_with_selection		},
	{ "exec_with_copy",			action_exec_with_copy			},
	{ "exec_with_screen",		action_exec_with_screen			},
	{ "print_copy",				action_print_copy				},
	{ "exit",					action_exit						},
	{ "copy",					action_copy						},
	{ "append",					action_append					},
	{ "paste",					action_paste					},
	{ "clear",					action_clear					},
	{ "select_all",				action_select_all				},
	{ "remove_selection",		action_remove_selection			},
	{ "crosshair",				action_crosshair				},
	{ "connect",				action_connect					},
	{ "F8",						action_F7						},
	{ "F7",						action_F8						},
	{ "disconnect",				action_disconnect				},
	{ "backtab",				action_BackTab					},
	{ "tab",					action_Tab						},
	{ "home",					action_Home						},
	{ "eraseeof",				action_EraseEOF					},
	{ "left",					action_Left						},
	{ "up",						action_Up						},
	{ "right",					action_Right					},
	{ "down",					action_Down						},
	{ "reset",					action_Reset					},
	{ "delete",					action_Delete					},
	{ "erase",					action_Erase					},
	{ "enter",					action_Enter					},
	{ "insert",					action_Insert					},
	{ "redraw",					action_Redraw					},
	{ "selectleft",				action_SelectLeft				},
	{ "selectup",				action_SelectUp					},
	{ "selectright",			action_SelectRight				},
	{ "selectdown",				action_SelectDown				},
	{ "selectionleft",			action_SelectionLeft			},
	{ "selectionup",			action_SelectionUp				},
	{ "selectionright",			action_SelectionRight			},
	{ "selectiondown",			action_SelectionDown			},
	{ "previousword",			action_PreviousWord				},
	{ "nextword",				action_NextWord					},
	{ "deleteword",				action_DeleteWord				},
	{ "deletefield",			action_DeleteField				}

 };

 const int action_callback_counter = (sizeof(action_callbacks)/sizeof(struct action_callback));

/*---[ Keyboard processing ]--------------------------------------------*/

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
     	DECLARE_KEYPROC( GDK_KP_Left,			GDK_SHIFT_MASK,		action_SelectLeft	),
     	DECLARE_KEYPROC( GDK_KP_Up,				GDK_SHIFT_MASK,		action_SelectUp		),
     	DECLARE_KEYPROC( GDK_KP_Right,			GDK_SHIFT_MASK,		action_SelectRight	),
     	DECLARE_KEYPROC( GDK_KP_Down,			GDK_SHIFT_MASK,		action_SelectDown	),

     	DECLARE_KEYPROC( GDK_Left,				GDK_SHIFT_MASK,		action_SelectLeft	),
     	DECLARE_KEYPROC( GDK_Up,				GDK_SHIFT_MASK,		action_SelectUp		),
     	DECLARE_KEYPROC( GDK_Right,				GDK_SHIFT_MASK,		action_SelectRight	),
     	DECLARE_KEYPROC( GDK_Down,				GDK_SHIFT_MASK,		action_SelectDown	),

     	DECLARE_KEYPROC( GDK_Left,				GDK_CONTROL_MASK,	action_SelectionLeft	),
     	DECLARE_KEYPROC( GDK_Up,				GDK_CONTROL_MASK,	action_SelectionUp		),
     	DECLARE_KEYPROC( GDK_Right,				GDK_CONTROL_MASK,	action_SelectionRight	),
     	DECLARE_KEYPROC( GDK_Down,				GDK_CONTROL_MASK,	action_SelectionDown	),

     	DECLARE_KEYPROC( GDK_KP_Left,			GDK_CONTROL_MASK,	action_SelectionLeft	),
     	DECLARE_KEYPROC( GDK_KP_Up,				GDK_CONTROL_MASK,	action_SelectionUp		),
     	DECLARE_KEYPROC( GDK_KP_Right,			GDK_CONTROL_MASK,	action_SelectionRight	),
     	DECLARE_KEYPROC( GDK_KP_Down,			GDK_CONTROL_MASK,	action_SelectionDown	),

        DECLARE_KEYPROC( GDK_l,					GDK_CONTROL_MASK,	action_Redraw			),

        DECLARE_KEYPROC( GDK_w,					GDK_CONTROL_MASK,	action_DeleteWord		),
        DECLARE_KEYPROC( GDK_u,					GDK_CONTROL_MASK,	action_DeleteField		),

        DECLARE_KEYPROC( GDK_r,					GDK_CONTROL_MASK,	action_Reset			),

     	DECLARE_KEYPROC( GDK_KP_Add,			GDK_NUMLOCK_MASK,	action_Tab			),

     	DECLARE_KEYPROC( GDK_Page_Up, 			0,					action_F7			),
     	DECLARE_KEYPROC( GDK_Page_Down,			0,					action_F8			),

     	DECLARE_KEYPROC( GDK_Print,				0,					action_print		),
     	DECLARE_KEYPROC( GDK_3270_PrintScreen,	0,					action_print		),

     	DECLARE_KEYPROC( GDK_ISO_Left_Tab,		0,					action_BackTab		),

     	DECLARE_KEYPROC( GDK_Tab,				0,					action_Tab			),

     	DECLARE_KEYPROC( GDK_Home,				0,					action_Home			),
     	DECLARE_KEYPROC( GDK_End,				0,					action_EraseEOF		),

     	DECLARE_KEYPROC( GDK_Left,				0,					action_Left			),
     	DECLARE_KEYPROC( GDK_Up,				0,					action_Up			),
     	DECLARE_KEYPROC( GDK_Right,				0,					action_Right		),
     	DECLARE_KEYPROC( GDK_Down,				0,					action_Down			),

     	DECLARE_KEYPROC( GDK_KP_Left,			0,					action_Left			),
     	DECLARE_KEYPROC( GDK_KP_Up,				0,					action_Up			),
     	DECLARE_KEYPROC( GDK_KP_Right,			0,					action_Right		),
     	DECLARE_KEYPROC( GDK_KP_Down,			0,					action_Down			),

        DECLARE_KEYPROC( GDK_Clear,				0,					action_Clear		),

        DECLARE_KEYPROC( GDK_3270_Reset,		0,					action_Reset		),

        DECLARE_KEYPROC( GDK_Delete,			0,					action_Delete		),
        DECLARE_KEYPROC( GDK_BackSpace,			0,					action_Erase		),

        DECLARE_KEYPROC( GDK_Return,			0,					action_Enter		),
        DECLARE_KEYPROC( GDK_KP_Enter,			0,					action_Enter		),

        DECLARE_KEYPROC( GDK_Escape,			0,					action_Reset		),

        DECLARE_KEYPROC( GDK_Insert,			0,					action_Insert		),


    };

//	char			ks[6];
    int				f;

#ifndef USE_GTKIMCONTEXT
    gchar 			*string		= 0;
#endif


    DBGPrintf("Keyval: %d (%s) Keychar: \"%s\" State: %04x %s%s%s%s%s%s%s%s%s%s%s%s%s%s",
						event->keyval,
						gdk_keyval_name(event->keyval),
						event->string,
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
    if(!input)
    {
    	Log("Error converting string \"%s\" to ISO-8859-1",string);
    	return;
    }

    // NOTE (perry#1#): Is it the best way?
    Input_String((const unsigned char *) input);
    /*
    if(!emulate_input(input, strlen(input), False))
    {
    	DBGMessage("Input failed!!");
    }
    */

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

 static gpointer exec_with_screen_thread(gpointer cmd)
 {
 	char 			filename[1024];
 	char			buffer[1024];
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

	   snprintf(buffer,1023,cmd,filename);
	   DBGMessage(buffer);
       system(buffer);
       remove(filename);

 	}
 	else
 	{
 		Error("Unable to open \"%s\" for writing",filename);
 	}
    return 0;
 }

 void action_exec_with_screen(GtkWidget *w, gpointer data)
 {
#if GTK == 2
    GThread   *thd = 0;
    thd =  g_thread_create( exec_with_screen_thread, (gpointer) data, 0, NULL);
#else
    pthread_t  thd = 0;
    pthread_create(&thd, NULL, (void * (*)(void *)) exec_with_screen_thread, data);
#endif
 }

 void action_print(GtkWidget *w, gpointer data)
 {
    action_exec_with_screen(w,data ? data : "kprinter --nodialog -t " TARGET " %s");
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

 void action_PreviousWord(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(PreviousWord_action, IA_DEFAULT, CN, CN);
 }

 void action_NextWord(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(NextWord_action, IA_DEFAULT, CN, CN);
 }

 void action_DeleteWord(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(DeleteWord_action, IA_DEFAULT, CN, CN);
 }

 void action_DeleteField(GtkWidget *w, gpointer data)
 {
 	CHKPoint();
    action_internal(DeleteField_action, IA_DEFAULT, CN, CN);
 }

 void action_AboutBox(GtkWidget *w, gpointer data)
 {

 #ifdef __GTK_ABOUT_DIALOG_H__

 	static const char *authors[] = {	"Paul Mattes <Paul.Mattes@usa.net>",
										"Perry Werneck <perry.werneck@gmail.com>",
										NULL};

	static const char license[] = "This program is free software; you can redistribute it and/or\n"
		"modify it under the terms of the GNU General Public License as\n"
 		"published by the Free Software Foundation; either version 2 of the\n"
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307\n"
		"USA";

 	CHKPoint();

 	gtk_show_about_dialog(	GTK_WINDOW(top_window),
							"name",    	"g3270",
							"authors", 	authors,
							"license", 	license,
							"logo",		icon,
							"comments",	_("3270 Terminal emulator for Gnome/GTK."),
#ifdef VERSION
							"version", 	VERSION,
#endif
							NULL
						);
/*
gtk_show_about_dialog (NULL,
					"name", "GChemPaint",
					"authors", authors,
					"artists", artists,
					"comments", _("GChemPaint is a 2D chemical structures editor for Gnome"),
					"copyright", _("(C) 2001-2005 by Jean Bréfort"),
					"license", license,
					"logo", pixbuf,
					"icon-name", "gchempaint",
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ?
											(const char *)translator_credits : NULL,
					"version", VERSION,
					"website", "http://www.nongnu.org/gchempaint",
					NULL);
*/
#endif

 }

 static void action_toogle(GtkWidget *w, gpointer data)
 {
 	if(!data)
 	   return;

 	DBGPrintf("Toogle \"%s\" using widget %p",(char *) data,w);
 }


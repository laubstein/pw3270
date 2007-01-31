
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "g3270.h"
#include "lib/kybdc.h"

/*---[ Defines ]--------------------------------------------------------------*/

#define TCNT    7
#define LUCNT   8
#define CM     (60 * 10)

/*---[ Prototipes ]-----------------------------------------------------------*/

static void screen_init(void);
static void screen_disp(Boolean erasing, const struct ea *display);
static void screen_suspend(void);

static void screen_resume(void);
static void screen_type(const char *model_name, int maxROWS, int maxCOLS);

static void cursor_move(int baddr);
static void toggle_monocase(struct toggle *t, enum toggle_type tt);
static void screen_changed(int first, int last);

static void status_ctlr_done(void);
static void status_insert_mode(Boolean on);
static void status_minus(void);
static void status_reset(void);
static void status_reverse_mode(Boolean on);
static void status_syswait(void);
static void status_twait(void);
static void status_typeahead(Boolean on);
static void status_compose(Boolean on, unsigned char c, enum keytype keytype);
static void status_lu(const char *lu);
static void status_oerr(int error_type);
static void ring_bell(void);
static void screen_flip(void);
static void screen_width(int width);
static void error_popup(const char *msg);
static void Redraw_action(Widget w, XEvent *event, String *params, Cardinal *num_params);
static void status_timing(struct timeval *t0, struct timeval *t1);

/*---[ Globals ]--------------------------------------------------------------*/

 char oia_cursor[8]					= "";
 char oia_LUName[12]				= "          ";
 char oia_Timet[TCNT+1]				= "";
 char oia_Timer[OIA_TIMER_COUNT+1]	= "";
 char oia_undera					= 1;
 char oia_Typeahead[2] 				= " ";

/*---[ 3270 Screen callback table ]-------------------------------------------*/

const SCREEN_CALLBACK g3270_screen_callbacks =
{
    sizeof(SCREEN_CALLBACK),
	SCREEN_MAGIC,

    screen_init,
    screen_disp,
    screen_suspend,

	screen_resume,
	screen_type,
    screen_changed,

	cursor_move,
	toggle_monocase,

	status_ctlr_done,
	status_insert_mode,
	status_minus,
	status_oerr,
	status_reset,
	status_reverse_mode,
	status_syswait,
	status_twait,
	status_typeahead,
	status_compose,
	status_lu,

	ring_bell,
	screen_flip,
	screen_width,

	error_popup,

	Redraw_action,

	status_timing,
	status_untiming

};

/*---[ Keyboard tables ]------------------------------------------------------*/

 static const KEYTABLE keys[] =
 {
//	{ "BREAK",	KEY_BREAK },
	{ "DOWN",		GDK_Down 		},
	{ "UP",			GDK_Up 			},
	{ "LEFT",		GDK_Left		},
	{ "RIGHT",		GDK_Right 		},
//	{ "HOME",	KEY_HOME },
	{ "BACKSPACE",	GDK_BackSpace	},
//	{ "F0",		KEY_F0 },
//	{ "DL",		KEY_DL },
//	{ "IL",		KEY_IL },
//	{ "DC",		KEY_DC },
//	{ "IC",		KEY_IC },
//	{ "EIC",	KEY_EIC },
//	{ "CLEAR",	KEY_CLEAR },
//	{ "EOS",	KEY_EOS },
//	{ "EOL",	KEY_EOL },
//	{ "SF",		KEY_SF },
//	{ "SR",		KEY_SR },
//	{ "NPAGE",	KEY_NPAGE },
//	{ "PPAGE",	KEY_PPAGE },
//	{ "STAB",	KEY_STAB },
//	{ "CTAB",	KEY_CTAB },
//	{ "CATAB",	KEY_CATAB },
//	{ "ENTER",	KEY_ENTER },
//	{ "SRESET",	KEY_SRESET },
//	{ "RESET",	KEY_RESET },
//	{ "PRINT",	KEY_PRINT },
//	{ "LL",		KEY_LL },
//	{ "A1",		KEY_A1 },
//	{ "A3",		KEY_A3 },
//	{ "B2",		KEY_B2 },
//	{ "C1",		KEY_C1 },
//	{ "C3",		KEY_C3 },
//	{ "BTAB",	KEY_BTAB },
//	{ "BEG",	KEY_BEG },
//	{ "CANCEL",	KEY_CANCEL },
//	{ "CLOSE",	KEY_CLOSE },
//	{ "COMMAND",	KEY_COMMAND },
//	{ "COPY",	KEY_COPY },
//	{ "CREATE",	KEY_CREATE },
//	{ "END",	KEY_END },
//	{ "EXIT",	KEY_EXIT },
//	{ "FIND",	KEY_FIND },
//	{ "HELP",	KEY_HELP },
//	{ "MARK",	KEY_MARK },
//	{ "MESSAGE",	KEY_MESSAGE },
//	{ "MOVE",	KEY_MOVE },
//	{ "NEXT",	KEY_NEXT },
//	{ "OPEN",	KEY_OPEN },
//	{ "OPTIONS",	KEY_OPTIONS },
//	{ "PREVIOUS",	KEY_PREVIOUS },
//	{ "REDO",	KEY_REDO },
//	{ "REFERENCE",	KEY_REFERENCE },
//	{ "REFRESH",	KEY_REFRESH },
//	{ "REPLACE",	KEY_REPLACE },
//	{ "RESTART",	KEY_RESTART },
//	{ "RESUME",	KEY_RESUME },
//	{ "SAVE",	KEY_SAVE },
//	{ "SBEG",	KEY_SBEG },
//	{ "SCANCEL",	KEY_SCANCEL },
//	{ "SCOMMAND",	KEY_SCOMMAND },
//	{ "SCOPY",	KEY_SCOPY },
//	{ "SCREATE",	KEY_SCREATE },
//	{ "SDC",	KEY_SDC },
//	{ "SDL",	KEY_SDL },
//	{ "SELECT",	KEY_SELECT },
//	{ "SEND",	KEY_SEND },
//	{ "SEOL",	KEY_SEOL },
//	{ "SEXIT",	KEY_SEXIT },
//	{ "SFIND",	KEY_SFIND },
//	{ "SHELP",	KEY_SHELP },
//	{ "SHOME",	KEY_SHOME },
//	{ "SIC",	KEY_SIC },
//	{ "SLEFT",	KEY_SLEFT },
//	{ "SMESSAGE",	KEY_SMESSAGE },
//	{ "SMOVE",	KEY_SMOVE },
//	{ "SNEXT",	KEY_SNEXT },
//	{ "SOPTIONS",	KEY_SOPTIONS },
//	{ "SPREVIOUS",	KEY_SPREVIOUS },
//	{ "SPRINT",	KEY_SPRINT },
//	{ "SREDO",	KEY_SREDO },
//	{ "SREPLACE",	KEY_SREPLACE },
//	{ "SRIGHT",	KEY_SRIGHT },
//	{ "SRSUME",	KEY_SRSUME },
//	{ "SSAVE",	KEY_SSAVE },
//	{ "SSUSPEND",	KEY_SSUSPEND },
//	{ "SUNDO",	KEY_SUNDO },
//	{ "SUSPEND",	KEY_SUSPEND },
//	{ "UNDO",	KEY_UNDO },
	{ CN, 0 }

 };

 const KEYBOARD_INFO g3270_keyboard_info =
 {
	sizeof(KEYBOARD_INFO),
	KEYBOARD_MAGIC,

	ring_bell,
	gsource_addfile,
	gsource_removefile,

	keys

 };

/*---[ Implement screen callbacks ]-------------------------------------------*/

static void screen_init(void)
{
	CHKPoint();
}

static void screen_disp(Boolean erasing, const struct ea *display)
{
	RedrawTerminalContents();
}

static void screen_suspend(void)
{
	CHKPoint();
}

static void screen_resume(void)
{
	RedrawTerminalContents();
}

static void screen_type(const char *model_name, int maxROWS, int maxCOLS)
{
	DBGPrintf("Screen type: %s (%dx%d)",model_name,maxROWS,maxCOLS);
}

static void cursor_move(int baddr)
{
	int		row;
	int		col;
 	int		cols;

 	Get3270DeviceBuffer(0, &cols);

 	row = baddr/cols;
 	col = baddr - (row*cols);

    SetCursorPosition(row,col);
    snprintf(oia_cursor,7,"%02d/%03d",row+1,col+1);

}

static void toggle_monocase(struct toggle *t, enum toggle_type tt)
{
	CHKPoint();
}

/* Done with controller confirmation */
static void status_ctlr_done(void)
{
	oia_undera = 1;
	RedrawStatusLine();
}

static void status_insert_mode(Boolean on)
{
	if(on)
	   SetCursorType(CURSOR_TYPE_INSERT);
	else
	   SetCursorType(CURSOR_TYPE_OVER);
}

/* Lock the keyboard (X -f) */
static void status_minus(void)
{
	SetOIAStatus(STATUS_MINUS);
}

/* Unlock the keyboard */
static void status_reset(void)
{
   if (GetKeyboardStatus() & KL_ENTER_INHIBIT)
      SetOIAStatus(STATUS_INHIBIT);
   else if (GetKeyboardStatus() & KL_DEFERRED_UNLOCK)
      SetOIAStatus(STATUS_NONSPECIFIC);
   else
      SetOIAStatus(STATUS_BLANK);
}

/* Toggle reverse mode */
static void status_reverse_mode(Boolean on)
{
	NotImplemented();
}

/* Lock the keyboard (X SYSTEM) */
static void status_syswait(void)
{
	SetOIAStatus(STATUS_SYSWAIT);
}

/* Lock the keyboard (twait) */
static void status_twait(void)
{
	oia_undera = 0;
    SetOIAStatus(STATUS_TWAIT);
}

/* Toggle typeahead */
static void status_typeahead(Boolean on)
{
	DBGPrintf("Typeahead: %s",on ? "Yes" : "No");
	*oia_Typeahead = on ? 'T' : ' ';
    RedrawStatusLine();
}

/* Set compose character */
static void status_compose(Boolean on, unsigned char c, enum keytype keytype)
{
	DBGPrintf("Compose: %s Char: %02x \"%c\"",on ? "Yes" : "No", c, c);
	NotImplemented();

    RedrawStatusLine();
}

/* Set LU name */
static void status_lu(const char *lu)
{
    if(!lu)
       lu = "";

    g3270_log("lib3270", "Using LU \"%s\"",lu);
    snprintf(oia_LUName,11,"%-10s",lu);
    UpdateWindowTitle();
    RedrawStatusLine();

}

static void ring_bell(void)
{
#ifdef DEBUG
    DBGMessage("Beep!");
	if(terminal && terminal->window)
	   gdk_beep();
#endif
}

static void screen_flip(void)
{
	CHKPoint();
}

static void screen_width(int width)
{
    // FIXME (perry#1#): Recalculate font size!
   DBGTrace(width);
   RedrawTerminalContents();
}

static void error_popup(const char *msg)
{
	Log(msg);
}

static void Redraw_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
	RedrawTerminalContents();
}

static void screen_changed(int first, int last)
{
   WaitForScreen = FALSE;
//   action_remove_selection(0,0);
   RedrawTerminalContents();
}

static void status_oerr(int error_type)
{
   switch(error_type)
   {
   case KL_OERR_PROTECTED:
      SetOIAStatus(STATUS_PROTECTED);
      break;

   case KL_OERR_NUMERIC:
      SetOIAStatus(STATUS_NUMERIC);
      break;

   case KL_OERR_OVERFLOW:
      SetOIAStatus(STATUS_OVERFLOW);
      break;

   case KL_OERR_DBCS:
      SetOIAStatus(STATUS_DBCS);
      break;

   }

}

#ifdef DEBUG
   static int dbgTimerEnabled = 0;
#endif

static void status_timing(struct timeval *t0, struct timeval *t1)
{
   unsigned long cs;       // centiseconds

#ifdef DEBUG
   if(!dbgTimerEnabled)
   {
      dbgTimerEnabled = 1;
      DBGMessage("Timer ON!");
   }
#endif

   if (t1->tv_sec - t0->tv_sec > (99*60))
   {
      strncpy(oia_Timer,":??.?",OIA_TIMER_COUNT);
   }
   else
   {
      cs = (t1->tv_sec - t0->tv_sec) * 10 +
           (t1->tv_usec - t0->tv_usec + 50000) / 100000;

      if (cs < CM)
         snprintf(oia_Timer, OIA_TIMER_COUNT, ":%02ld.%ld", cs / 10, cs % 10);
      else
         snprintf(oia_Timer, OIA_TIMER_COUNT, "%02ld:%02ld", cs / CM, (cs % CM) / 10);
   }

   RedrawStatusLine();

}

void status_untiming(void)
{
#ifdef DEBUG
   if(dbgTimerEnabled)
   {
      dbgTimerEnabled = 0;
      DBGMessage("Timer Off!");
   }
#endif

   *oia_Timer = 0;
   RedrawStatusLine();
}


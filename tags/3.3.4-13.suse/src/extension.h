

#ifndef G3270_EXTENSION_H_INCLUDED

 #include <gtk/gtk.h>
 #include <sys/time.h> 
 #include <g3270/localdefs.h>
 #include <g3270/lib3270.h>

 #define G3270_EXTENSION_H_INCLUDED

 enum status_codes
 {
        STATUS_DISCONNECTED,           /*  0 X Not Connected */
        STATUS_RESOLVING,              /*  1 X Resolving */
        STATUS_CONNECTING,             /*  2 X Connecting */
        STATUS_NONSPECIFIC,            /*  3 X */
        STATUS_INHIBIT,                /*  4 X Inhibit */
        STATUS_BLANK,                  /*  5 (blank) */
        STATUS_TWAIT,                  /*  6 X Wait */
        STATUS_SYSWAIT,                /*  7 X SYSTEM */
        STATUS_PROTECTED,              /*  8 X Protected */
        STATUS_NUMERIC,                /*  9 X Numeric */
        STATUS_OVERFLOW,               /* 10 X Overflow */
        STATUS_DBCS,                   /* 11 X DBCS */
        STATUS_SCROLLED,               /* 12 X Scrolled */
        STATUS_MINUS,                  /* 13 X -f */
        STATUS_AWAITING_FIRST,	       /* 14 X Wait */
	    STATUS_CONNECTED,			   /* 15 Connected */

		STATUS_RECONNECTING
 };

 enum cursor_types
 {
 	CURSOR_TYPE_OVER,
 	CURSOR_TYPE_INSERT,

 	CURSOR_TYPE_CROSSHAIR // Must be the last one!
 };
 
 #define CURSOR_TYPE_NONE	-1

 enum _STATUS_COLORS
 {
    STATUS_COLOR_BACKGROUND,
    STATUS_COLOR_SEPARATOR,
	STATUS_COLOR_CURSOR_POSITION,
	STATUS_COLOR_LUNAME,
	STATUS_COLOR_ERROR,
	STATUS_COLOR_TIME,
	STATUS_COLOR_WARNING,
	STATUS_COLOR_NORMAL,
	STATUS_COLOR_TOOGLE,
	STATUS_COLOR_SSL,
	STATUS_COLOR_CONNECTED,
	STATUS_COLOR_KEYBOARD,
	STATUS_COLOR_CONNECT_ICON,

	STATUS_COLORS	// Must be the last one
 };

 #define STATUS_COLOR_CNCT 		STATUS_COLOR_CONNECT_ICON
 #define STATUS_COLOR_TYPEAHEAD STATUS_COLOR_TOOGLE

 #ifndef GDK_NUMLOCK_MASK
    #define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

 #ifndef GDK_ALT_MASK
     #define GDK_ALT_MASK GDK_MOD1_MASK
 #endif
 
 #define LockThreads()		gdk_lock()
 #define UnlockThreads()	gdk_unlock()

#ifdef __cplusplus
 extern "C" {
#endif

/*---[ Macros ]---------------------------------------------------------------*/

 #define Log(...)               g3270_log(MODULE, __VA_ARGS__)
 #define WriteLog(...)          g3270_log(MODULE, __VA_ARGS__)
 #define WriteError(e,...)      g3270_logRC(MODULE, e, __VA_ARGS__)

 #define Error(...)             g3270_logRC(MODULE, -1, __VA_ARGS__)
 #define ErrorPopup(...)        g3270_popup(MODULE, -1, __VA_ARGS__)

 #define Exec(...)              g3270_logExec(MODULE, __VA_ARGS__)
 #define NOT_IMPLEMENTED( ... ) g3270_log(MODULE, "*** NOT IMPLEMENTED CALL ***: " __FILE__ " " __VA_ARGS__)

 #if defined( DEBUG )

    #define DBGFILE stderr

    #define CHKPoint()        	   fprintf(DBGFILE,"%s(%d):\t%s\t\t(" __DATE__ " " __TIME__")\n",__FILE__,__LINE__,__FUNCTION__);fflush(DBGFILE);

    #define DBGMessage(x)     	   fprintf(DBGFILE,"%s(%d):\t%s\n",__FILE__,__LINE__,x);fflush(DBGFILE);
    #define DBGTrace(x)       	   fprintf(DBGFILE,"%s(%d):\t%s = %ld\n",__FILE__,__LINE__,#x, (unsigned long) x);fflush(DBGFILE);
    #define DBGTracex(x)      	   fprintf(DBGFILE,"%s(%d):\t%s = %08lx\n",__FILE__,__LINE__,#x, (unsigned long) x);fflush(DBGFILE);
    #define DBGPrintf(x, ...) 	   fprintf(DBGFILE,"%s(%d):\t" x "\n",__FILE__,__LINE__, __VA_ARGS__);fflush(DBGFILE);

 #else

    #define DBGMessage(x) /* x */
    #define DBGTrace(x) /* x */
    #define DBGTracex(x) /* x */
    #define CHKPoint()  /* */
    #define DBGPrintf(x, ...) /* */

 #endif
 
 #ifndef BUILD
    #define BUILD 20070425
 #endif


/*---[ Prototipes ]-----------------------------------------------------------*/

 int	g3270_logRC(const char *, int, const char *, ...);
 int	g3270_popup(const char *, int, const char *, ...);

 int	g3270_log(const char *, const char *, ...);
 int	g3270_logExec(const char *, const char *, ...);
 int	g3270_logName(const char *);

 int	g3270_lock(void);
 int	g3270_unlock(void);

 void	gdk_lock(void);
 void	gdk_unlock(void);

/*---[ Extension entry points ]-----------------------------------------------*/

 int  g3270OpenExtension(GtkWidget *TopWindow);
 int  g3270CloseExtension(GtkWidget *TopWindow);
 void g3270LUChanged(GtkWidget *widget, const char *lu);
 void g3270ServerChanged(GtkWidget *widget, const char *server);

#ifdef __cplusplus
 }
#endif

 
#endif

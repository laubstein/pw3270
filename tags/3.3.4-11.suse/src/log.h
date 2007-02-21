
#ifndef LOG_H_INCLUDED

#define LOG_H_INCLUDED

#include "trace.h"

#ifdef __cplusplus
 extern "C" {
#endif

/*---[ Macros ]---------------------------------------------------------------*/

 #include <stdio.h>

 #define Log(...)               g3270_log(MODULE, __VA_ARGS__)
 #define WriteLog(...)          g3270_log(MODULE, __VA_ARGS__)
 #define WriteError(e,...)      g3270_logRC(MODULE, e, __VA_ARGS__)

 #define Error(...)             g3270_logRC(MODULE, -1, __VA_ARGS__)
 #define ErrorPopup(...)        g3270_popup(MODULE, -1, __VA_ARGS__)

 #define Exec(...)              g3270_logExec(MODULE, __VA_ARGS__)
 #define NOT_IMPLEMENTED( ... ) g3270_log(MODULE, "*** NAO IMPLEMENTADO EM " __FILE__ ": " __VA_ARGS__)

 #ifndef BUILD
    #define BUILD 20060919
 #endif

/*---[ Prototipos ]-----------------------------------------------------------*/

 FILE *                 g3270_openLog(void);
 FILE *                 g3270_logPrefix(const char *);
 void                   g3270_closeLog(FILE *);

 int                    g3270_logRC(const char *, int, const char *, ...);
 int                    g3270_popup(const char *, int, const char *, ...);

 int                    g3270_log(const char *, const char *, ...);
 int                    g3270_logExec(const char *, const char *, ...);
 int                    g3270_logName(const char *);

 int                   	g3270_lock(void);
 int 					g3270_unlock(void);


#ifdef __cplusplus
 }
#endif

#endif // LOG_H_INCLUDED

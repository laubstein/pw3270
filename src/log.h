
#ifndef LOG_H_INCLUDED

#define LOG_H_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

/*---[ Macros ]---------------------------------------------------------------*/

 #include <stdio.h>

 #ifdef DEBUG
    #define TRACE "debug"
 #endif

 #ifdef TRACE

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

 #ifdef DEBUG

    #define NDBGExec(x, ...)      fprintf(DBGFILE,"%s(%d):\tExecutar: [",__FILE__,__LINE__);fprintf(DBGFILE,x,__VA_ARGS__);fprintf(DBGFILE,"]\n");fflush(DBGFILE);
    #define MKDir(...)  		  fprintf(DBGFILE,"%s(%d):\tCriar diretorio: [",__FILE__,__LINE__);fprintf(DBGFILE,__VA_ARGS__);fprintf(DBGFILE,"]\n");fflush(DBGFILE);

 #else

	#define NDBGExec(x, ...)      g3270_logExec(MODULE, x, __VA_ARGS__)
	#define MKDir(...) 			  g3270_logExec(MODULE, ini->Get("install","mkdir","mkdir -p \"%s\""), __VA_ARGS__)

 #endif


 #ifndef PACKET
    #define PACKET __FILE__
 #endif

 #ifndef MODULE
    #define MODULE PACKET
 #endif

 #define WriteLog(...)          g3270_log(MODULE, __VA_ARGS__)
 #define WriteError(e,...)      g3270_logRC(MODULE, e, __VA_ARGS__)
 #define Error(...)             g3270_logRC(MODULE, -1, __VA_ARGS__)
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
 int                    g3270_log(const char *, const char *, ...);
 int                    g3270_logExec(const char *, const char *, ...);
 int                    g3270_logName(const char *);

 int                   	g3270_lock(void);
 int 					g3270_unlock(void);

#ifdef __cplusplus
 }
#endif

#endif // LOG_H_INCLUDED

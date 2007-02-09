
#ifndef TRACE_H_INCLUDED

#define TRACE_H_INCLUDED

#include <stdio.h>

#ifdef __cplusplus
 extern "C" {
#endif


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



#ifdef __cplusplus
 }
#endif

#endif // TRACE_H_INCLUDED

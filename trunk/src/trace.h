
#ifndef TRACE_H_INCLUDED

#define TRACE_H_INCLUDED

#include <stdio.h>

#ifdef __cplusplus
 extern "C" {
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

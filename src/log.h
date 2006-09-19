/*
Este arquivo é parte do programa Instalador

Copyright (C) 2003 Techisa do Brasil

Instalador é um software livre; você pode redistribui-lo e/ou
modifica-lo dentro dos termos da Licença Pública Geral Menor GNU como
publicada pela Fundação do Software Livre (FSF); na versão 2 da
Licença, ou (à sua opção) qualquer versão.

Este programa é distribuido na esperança que possa ser util,
mas SEM NENHUMA GARANTIA; sem uma garantia implicita de ADEQUAÇÂO
a qualquer MERCADO ou APLICAÇÃO EM PARTICULAR.
Veja a Licença Pública Geral GNU para maiores detalhes.

Você deve ter recebido uma cópia da Licença Pública Geral Menor GNU
junto com este programa, se não, escreva para a Fundação do Software
Livre(FSF) Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Definicoes e variaveis globais

*/

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

    #define WriteConsole(x, ...)   /* fprintf(stderr,x,__VA_ARGS__);fflush(stderr) */

 #else

    #define DBGMessage(x) /* x */
    #define DBGTrace(x) /* x */
    #define DBGTracex(x) /* x */
    #define CHKPoint()  /* */
    #define DBGPrintf(x, ...) /* */

    #define WriteConsole(x, ...)  fprintf(stderr,x,__VA_ARGS__);fflush(stderr)

 #endif

 #ifdef DEBUG

    #define NDBGExec(x, ...)      fprintf(DBGFILE,"%s(%d):\tExecutar: [",__FILE__,__LINE__);fprintf(DBGFILE,x,__VA_ARGS__);fprintf(DBGFILE,"]\n");fflush(DBGFILE);
    #define MKDir(...)  		  fprintf(DBGFILE,"%s(%d):\tCriar diretorio: [",__FILE__,__LINE__);fprintf(DBGFILE,__VA_ARGS__);fprintf(DBGFILE,"]\n");fflush(DBGFILE);

 #else

	#define NDBGExec(x, ...)      install_logExec(MODULE, x, __VA_ARGS__)
	#define MKDir(...) 			  install_logExec(MODULE, ini->Get("install","mkdir","mkdir -p \"%s\""), __VA_ARGS__)

 #endif


 #ifndef PACKET
    #define PACKET __FILE__
 #endif

 #ifndef MODULE
    #define MODULE PACKET
 #endif

 #define WriteLog(...)          install_log(MODULE, __VA_ARGS__)
 #define WriteError(e,...)      install_logRC(MODULE, e, __VA_ARGS__)
 #define Error(...)             install_logRC(MODULE, -1, __VA_ARGS__)
 #define Exec(...)              install_logExec(MODULE, __VA_ARGS__)
 #define NOT_IMPLEMENTED( ... ) install_log(MODULE, "*** NAO IMPLEMENTADO EM " __FILE__ ": " __VA_ARGS__)

 #ifndef BUILD
    #define BUILD 20051122
 #endif

/*---[ Publicas ]-------------------------------------------------------------*/

 #define LOGEXEC_MESSAGE_SIZE 80

 extern char logExecLastMessage[LOGEXEC_MESSAGE_SIZE];
 extern char logExecLastError[LOGEXEC_MESSAGE_SIZE];

/*---[ Prototipos ]-----------------------------------------------------------*/

 FILE *                 install_openLog(void);
 FILE *                 install_logPrefix(const char *);
 void                   install_closeLog(FILE *);

 int                    install_logRC(const char *, int, const char *, ...);
 int                    install_log(const char *, const char *, ...);
 int                    install_logExec(const char *, const char *, ...);
 int                    install_logName(const char *);

 int                   	install_lock(void);
 int 					install_unlock(void);

 int                    CopyFile(const char *, const char *, const char *);

#ifdef __cplusplus
 }
#endif

#endif // LOG_H_INCLUDED

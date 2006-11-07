
 #define _USE_BSD
 #include <string.h>
 #include <stdio.h>
 #include <time.h>
 #include <stdarg.h>
 #include <errno.h>

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/resource.h>
 #include <sys/wait.h>
 #include <sys/file.h>
 #include <pthread.h>
 #include <errno.h>
 #include <stdlib.h>

 #include <unistd.h>

 #include "log.h"
 #include "trace.h"

/*---[ Statics e globais ]----------------------------------------------------------------------------------*/

#ifdef DEBUG
 static char   			logfile[0x0100]		= "g3270.log";
#else
 static char   			logfile[0x0100]		= "/var/log/g3270.log";
#endif

 static pthread_mutex_t	lock				= PTHREAD_MUTEX_INITIALIZER;

/*---[ Prototipos ]-----------------------------------------------------------------------------------------*/

 static void writetime(FILE *, const char *);

/*---[ Implementacao ]--------------------------------------------------------------------------------------*/

 /**
  * Bloqueia acesso de outras threads.
  *
  * "locka" semaforo para serializacao de acesso
  */
 int g3270_lock(void)
 {
    return pthread_mutex_lock(&lock);
 }

 /**
  * Libera outras threads.
  *
  * Libera semaforo para serializacao de acesso
  */
 int g3270_unlock(void)
 {
    return pthread_mutex_unlock(&lock);
 }

 /**
  * Abre um arquivo de log
  * @return Handle para escrita no arquivo
  */
 FILE *g3270_openLog(void)
 {
 	const char *ptr;
    FILE       *ret = NULL;

    ret = fopen(logfile,"a");
    if(ret)
       return ret;

 	ptr = getenv("USER");
	if(ptr)
	{
		snprintf(logfile,0xFF,"/home/%s/" TARGET, ptr);
        ret = fopen(logfile,"a");
        if(ret)
		   return ret;
	}

    ret = fopen(TARGET ".log","a");
    if(ret)
       return ret;

 	ptr = getenv("TMPDIR");
	if(ptr)
	{
		snprintf(logfile,0xFF,"/home/%s/" TARGET, ptr);
        ret = fopen(logfile,"a");
        if(ret)
		   return ret;
	}

    strncpy(logfile,"/tmp/" TARGET, 0xFF);

    ret = fopen(logfile,"a");
    if(ret)
       return ret;

    return fopen("/dev/null","a");
 }

 /**
  * Fechar arquivo de log
  * @param	arq	Handle do arquivo de log retornado por g3270_openLog() ou g3270_logPrefix()
  */
 void g3270_closeLog(FILE *arq)
 {
    if(arq)
       fclose(arq);
 }

 /**
  * Abre arquivo de log gravando o inicio da linha.
  *
  * Abre um arquivo de log ja gravando a data/hora e identificacao do modulo
  * no arquivo.
  *
  * @param	module	Identificador do modulo para gravacao no arquivo
  * @return handle para escrita no arquivo
  */
 FILE *g3270_logPrefix(const char *module)
 {
    FILE *out = g3270_openLog();
	writetime(out,module);
	return out;
 }

 static void writetime(FILE *out, const char *module)
 {
    time_t      ltime;
    char        wrk[40];

    time(&ltime);
    strftime(wrk, 39, "%d/%m/%Y %H:%M:%S", localtime(&ltime));
    fprintf(out,"%s ",wrk);
 }

 /**
  * Grava uma entrada no arquivo de log.
  *
  * @param	module	Identificador do modulo para gravacao no arquivo
  * @param	fmt		String de formatacao para a mensagem no mesmo formato da funcao printf()
  * @param	...		Argumentos de acordo com a string de formatacao
  */
 int g3270_log(const char *module, const char *fmt, ...)
 {
    char    string[0x0100];
    va_list arg_ptr;
    FILE    *out;

    va_start(arg_ptr, fmt);
    vsnprintf(string, 0xFF, fmt, arg_ptr);
    va_end(arg_ptr);

	g3270_lock();

    out = g3270_logPrefix(module);

    if(!out)
       return -1;

    fprintf(out,"%s\n",string);
	DBGMessage(string);

    g3270_closeLog(out);
	g3270_unlock();

    return 0;
 }

 /**
  * Grava mensagem de erro.
  *
  * Grava uma mensagem de erro no arquivo de log.
  *
  * @param	module	Identificador do modulo para gravacao no arquivo
  * @param	rc		Codigo de erro ou -1 para usar o valor de errno
  * @param	fmt		String de formatacao para a mensagem no mesmo formato da funcao printf()
  * @param	...		Argumentos de acordo com a string de formatacao
  */
 int g3270_logRC(const char *module, int rc, const char *fmt, ...)
 {
    FILE    *out;
    char    string[0x0100];
    va_list arg_ptr;

	if(rc == -1)
	   rc = errno;

    if(!rc)
       return 0;

	g3270_lock();

    va_start(arg_ptr, fmt);
    vsprintf(string, fmt, arg_ptr);
    va_end(arg_ptr);

    out = g3270_logPrefix(module);

	if(rc > 0)
	{
       fprintf(out,"%s: %s (rc=%d)\n",string,strerror(rc),rc);
       DBGPrintf("%s: %s (rc=%d)\n",string,strerror(rc),rc);
	}
	else
	{
       fprintf(out,"%s: (rc=%d)\n",string,rc);
	}

    g3270_closeLog(out);

	g3270_unlock();

#ifdef DEBUG
	if(rc > 0)
	{
       DBGPrintf("%s: %s (rc=%d)\n",string,strerror(rc),rc);
	}
	else
	{
       DBGPrintf("%s: (rc=%d)\n",string,rc);
	}
#endif

    return rc;
 }


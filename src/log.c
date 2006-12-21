
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
 #include <gtk/gtk.h>


 #include <unistd.h>

 #include "g3270.h"
 #include "log.h"
 #include "trace.h"

/*---[ Statics e globais ]----------------------------------------------------------------------------------*/

 static char   			logFileName[0x0100]		= TARGET "_%u.log";
 static pthread_mutex_t	lock					= PTHREAD_MUTEX_INITIALIZER;

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

 static FILE *CheckFileName(char *dir)
 {
    time_t      ltime;
    struct stat	fs;
    char 	masc[0x0100];
    char	fileName[0x0100];

    if(!dir)
       return 0;

    strncpy(masc,dir,0xFF);
    strncat(masc,"/",0xFF);
    strncat(masc,logFileName,0xFF);

    time(&ltime);
    strftime(fileName, 0xFF, masc, localtime(&ltime));

    memset(&fs,0,sizeof(fs));
    if(stat(fileName,&fs) != -1 && fs.st_atime > 10000)
    {
       if( difftime(ltime,fs.st_atime) > 86400 )
          remove(fileName);
    }

    return fopen(fileName,"a");
 }

 /**
  * Abre um arquivo de log
  * @return Handle para escrita no arquivo
  */
 FILE *g3270_openLog(void)
 {
    FILE       	*ret	= NULL;
    char		*home	= getenv("HOME");
    char		dir[0x0100];

#ifdef LOGPATH
    ret = CheckFileName(LOGPATH);
    if(ret)
       return ret;
#endif

    if(home)
    {
       snprintf(dir, 0xFF,"%s/" TARGET, home);
       ret = CheckFileName(dir);
       if(ret)
          return ret;

       snprintf(dir, 0xFF,"%s/log", home);
       ret = CheckFileName(dir);
       if(ret)
          return ret;

       snprintf(dir, 0xFF,"%s/." TARGET, home);
       ret = CheckFileName(dir);
       if(ret)
          return ret;

       snprintf(dir, 0xFF,"%s/tmp", home);
       ret = CheckFileName(dir);
       if(ret)
          return ret;

    }

    ret = CheckFileName(getenv("TMPDIR"));
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
	DBGPrintf("%s\n",string);

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


 /**
  * Grava mensagem de erro e avisa o usuario.
  *
  * Grava uma mensagem de erro no arquivo de log.
  *
  * @param	module	Identificador do modulo para gravacao no arquivo
  * @param	rc		Codigo de erro ou -1 para usar o valor de errno
  * @param	fmt		String de formatacao para a mensagem no mesmo formato da funcao printf()
  * @param	...		Argumentos de acordo com a string de formatacao
  */
 int g3270_popup(const char *module, int rc, const char *fmt, ...)
 {
    FILE      *out;
    char      string[0x0100];
    va_list   arg_ptr;
    GtkWidget *widget;

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

    widget = gtk_message_dialog_new_with_markup(
						GTK_WINDOW(top_window),
						GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                        rc ? GTK_MESSAGE_ERROR : GTK_MESSAGE_WARNING,
                        rc ? GTK_BUTTONS_CLOSE : GTK_BUTTONS_OK,
                        string );

    gtk_dialog_run(GTK_DIALOG(widget));

    gtk_widget_destroy (widget);

    return rc;
 }


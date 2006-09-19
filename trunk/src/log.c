/*
Este arquivo é parte do programa Instalador

Copyright (C) 2003 Banco do Brasil S/A

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

*/

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

 #include <unistd.h>

 #include <log.h>

/*---[ Statics e globais ]----------------------------------------------------------------------------------*/

 static char   			logfile[0x0100]		= "/tmp/install.log";
 static pthread_mutex_t	lock				= PTHREAD_MUTEX_INITIALIZER;

/*---[ Prototipos ]-----------------------------------------------------------------------------------------*/

 static void writetime(FILE *, const char *);

/*---[ Implementacao ]--------------------------------------------------------------------------------------*/

 /**
  * Bloqueia acesso de outras threads.
  *
  * "locka" semaforo para serializacao de acesso
  */
 int install_lock(void)
 {
    return pthread_mutex_lock(&lock);
 }

 /**
  * Libera outras threads.
  *
  * Libera semaforo para serializacao de acesso
  */
 int install_unlock(void)
 {
    return pthread_mutex_unlock(&lock);
 }

 /**
  * Abre um arquivo de log
  * @return Handle para escrita no arquivo
  */
 FILE *install_openLog(void)
 {
    FILE *ret = NULL;

    ret = fopen(logfile,"a");

	if(!ret)
       return stderr;

    return ret;
 }

 /**
  * Fechar arquivo de log
  * @param	arq	Handle do arquivo de log retornado por install_openLog() ou install_logPrefix()
  */
 void install_closeLog(FILE *arq)
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
 FILE *install_logPrefix(const char *module)
 {
    FILE *out = install_openLog();
	writetime(out,module);
	return out;
 }

 static void writetime(FILE *out, const char *module)
 {
    time_t      ltime;
    char        wrk[40];

    time(&ltime);
    strftime(wrk, 39, "%d/%m/%Y %H:%M:%S", localtime(&ltime));
    fprintf(out,"%s %-8s ",wrk,module);
 }

 /**
  * Grava uma entrada no arquivo de log.
  *
  * @param	module	Identificador do modulo para gravacao no arquivo
  * @param	fmt		String de formatacao para a mensagem no mesmo formato da funcao printf()
  * @param	...		Argumentos de acordo com a string de formatacao
  */
 int install_log(const char *module, const char *fmt, ...)
 {
    char    string[0x0100];
    va_list arg_ptr;
    FILE    *out;

    va_start(arg_ptr, fmt);
    vsnprintf(string, 0xFF, fmt, arg_ptr);
    va_end(arg_ptr);

	install_lock();

    out = install_logPrefix(module);

    fprintf(out,"%s\n",string);
#ifdef DEBUG
	fprintf(stderr,"%s\n",string);
	fflush(stderr);
#endif

	DBGMessage(string);

    install_closeLog(out);
	install_unlock();

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
 int install_logRC(const char *module, int rc, const char *fmt, ...)
 {
    FILE    *out;
    char    string[0x0100];
    va_list arg_ptr;

	if(rc == -1)
	   rc = errno;

    if(!rc)
       return 0;

	install_lock();

    va_start(arg_ptr, fmt);
    vsprintf(string, fmt, arg_ptr);
    va_end(arg_ptr);

    out = install_logPrefix(module);

	if(rc > 0)
	{
       fprintf(out,"%s: %s (rc=%d)\n",string,strerror(rc),rc);
#ifndef DEBUG
       fprintf(stderr,"%s: %s (rc=%d)\n",string,strerror(rc),rc);
	   fflush(stderr);
#endif
	}
	else
	{
       fprintf(out,"%s: (rc=%d)\n",string,rc);
	}

    install_closeLog(out);

	install_unlock();

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
  * Muda o arquivo de log.
  *
  * Muda o arquivo de log em uso, copiando inclusive o conteudo anterior.
  * Esta funcao e utilizada para efetuar a transferencia do arquivo de log
  * da ramdisk para o disco real logo apos a formatacao do HD.
  *
  * @param	filename	path completo para o novo arquivo de log
  */
 int install_logName(const char *filename)
 {
	FILE *in;
	FILE *out;
	int  rc = 0;
	char buffer[0x0100];

    strncpy(buffer,filename,0xFF);
	out = fopen(buffer,"a");

	if(!out)
	{
       install_logRC(MODULE,-1,"Nao foi possivel ativar %s",filename);
	   return -1;
	}

    WriteLog("Mudando arquivo de log para %s",filename);
	install_lock();
	in = fopen(logfile,"r");
	if(in)
	{
       while(!feof(in) && !rc)
       {
          int   sz;
		  char *ptr;

		  sz = fread(ptr=buffer,1,0xFF,in);

          while(sz > 0 && !rc)
          {
             int szWrite = fwrite(ptr,1,sz,out);

             if(szWrite < 1)
             {
			    fprintf(out,"** Erro ao copiar arquivo de entrada\n");
				fflush(out);
				rc = -1;
             }
             ptr += szWrite;
             sz  -= szWrite;
          }
       }
	   fclose(in);
    }
    fclose(out);

	if(!rc)
	{
	   remove(logfile);
       link(logfile,filename);
       strncpy(logfile,filename,0xFF);
	}
    install_unlock();
	return rc;
 }

 /**
  * Copia arquivo
  *
  * Efetua a copia de um arquivo entre dois diretorios. O parametro root
  * e fornecido em separado para facilitar o uso da macro IMAGE_ROOT
  *
  * @param	src		Path completo para o arquivo fonte
  * @param	root	Diretorio para o arquivo destino
  * @param	dst		Nome do arquivo destino
  *
  * @see	IMAGE_ROOT
  */
 int CopyFile(const char *src, const char *root, const char *dst)
 {
    char buffer[1024];
	FILE *in;
	FILE *out;
	int  rc = 0;

	if(root)
	   snprintf(buffer,1023,"%s/%s",root,dst);
	else
	   strncpy(buffer,dst,1023);

	WriteLog("Copiando arquivo %s para %s",src,dst);

	in = fopen(src,"r");
	if(!in)
	{
	   Error("Nao foi possivel abrir %s para leitura",src);
	   return -1;
	}

	out = fopen(buffer,"w");
	if(!out)
	{
	   Error("Nao foi possivel criar %s",buffer);
	   fclose(in);
	   return -1;
	}

    while(!feof(in) && !rc)
    {
       int   sz;
	   char *ptr;

	   sz = fread(ptr=buffer,1,1023,in);

       while(sz > 0 && !rc)
       {
          int szWrite = fwrite(ptr,1,sz,out);

          if(szWrite < 1)
          {
		     Error("Erro na leitura do arquivo %s",src);
			 rc = -1;
          }
          ptr += szWrite;
          sz  -= szWrite;
       }
    }

	fclose(out);
	fclose(in);

    return rc;
 }

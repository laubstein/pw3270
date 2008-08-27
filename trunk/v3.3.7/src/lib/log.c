/*
* Copyright 2008, Banco do Brasil S.A.
*
* This file is part of g3270
*
* This program file is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program in a file named COPYING; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
*
* Authors:
*
* Perry Werneck<perry.werneck@gmail.com>
*
*/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <lib3270/config.h>
#include <lib3270/api.h>

/*---[ Constants ]------------------------------------------------------------------------------------------*/

 const char *logfile = PACKAGE_NAME ".log";

/*---[ Implementacao ]--------------------------------------------------------------------------------------*/

 static void writetime(FILE *out, const char *module)
 {
    time_t		ltime;
    char		wrk[40];

    time(&ltime);
    strftime(wrk, 39, "%d/%m/%Y %H:%M:%S", localtime(&ltime));
    fprintf(out,"%s %-8s\t",wrk,module);
 }

 static FILE *prefix(const char *module)
 {
    FILE *out = fopen(logfile,"a");
    if(out)
		writetime(out,module);
	return out;
 }

 /**
  * Grava uma entrada no arquivo de log.
  *
  * @param	module	Identificador do modulo para gravacao no arquivo
  * @param	fmt		String de formatacao para a mensagem no mesmo formato da funcao printf()
  * @param	...		Argumentos de acordo com a string de formatacao
  */
 int WriteLog(const char *module, const char *fmt, ...)
 {
    va_list arg_ptr;
    FILE    *out;

    out = prefix(module);
    if(!out)
       return -1;

    va_start(arg_ptr, fmt);
    vfprintf(out, fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(out,"\n");

    fclose(out);

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
 int WriteRCLog(const char *module, int rc, const char *fmt, ...)
 {
    FILE    *out;
    va_list arg_ptr;

	if(rc == -1)
	   rc = errno;

    if(!rc)
       return 0;

    out = prefix(module);
    if(!out)
		return -1;

    va_start(arg_ptr, fmt);
    vfprintf(out, fmt, arg_ptr);
    va_end(arg_ptr);

	fprintf(out,": %s (rc=%d)\n",strerror(rc),rc);

    fclose(out);

    return rc;
 }



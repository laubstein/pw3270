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

/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como main.c e possui 351 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */


 #include "rx3270.h"

 #include <gmodule.h>

/*---[ Structs ]----------------------------------------------------------------------------------*/

/*---[ Globals ]----------------------------------------------------------------------------------*/

 GtkWidget 	*program_window	= NULL;

/*---[ Implement ]--------------------------------------------------------------------------------*/

 #include "calls.h"

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270LoadFuncs                                    */
/*                                                                            */
/* Description: Register all functions in this library.                       */
/*                                                                            */
/* Rexx Args:   None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
ULONG APIENTRY rx3270LoadFuncs(PSZ Name, LONG Argc, RXSTRING Argv[], PSZ Queuename, PRXSTRING Retstr)
{
	int	 f;

	program_window = 0;

 	// Load rexx calls
	for(f=0; f < G_N_ELEMENTS(rexx_exported_calls); f++)
		RexxRegisterFunctionExe((char *) rexx_exported_calls[f].name,rexx_exported_calls[f].call);

	return RetValue(Retstr,0);
}


 ULONG RetConvertedString(PRXSTRING Retstr, const char *str)
 {
 	#warning Charset conversion isnt implemented yet

 	if(!str)
 	{
 		// Empty string doesn't need to be converted
 		strcpy(Retstr->strptr,"");
 	}
 	else
 	{
 		// Convert received string to UTF-8
		if(strlen(str) > (RXAUTOBUFLEN-1))
		{
			Retstr->strptr = RexxAllocateMemory(strlen(str)+1);
			strcpy(Retstr->strptr,str);
		}
		else
		{
			strncpy(Retstr->strptr,str,RXAUTOBUFLEN-1);
		}
 	}

    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 ULONG RaiseHaltSignal(void)
 {
#ifdef WIN32
	return RexxSetHalt(getpid(),GetCurrentThreadId());
#else
	return RexxSetHalt(getpid(),pthread_self());
#endif

 }

 int IsHalted(void)
 {
 	return 0;
 }

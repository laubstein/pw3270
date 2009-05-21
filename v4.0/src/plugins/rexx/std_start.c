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

#ifdef HAVE_ICONV
 #include <iconv.h>
#endif

 #include <gmodule.h>

#if defined WIN32
	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd);
#else
	int librx3270_loaded(void) __attribute__((constructor));
	int librx3270_unloaded(void) __attribute__((destructor));
#endif

/*---[ Structs ]----------------------------------------------------------------------------------*/

/*---[ Globals ]----------------------------------------------------------------------------------*/

 GtkWidget 	*program_window	= NULL;

#ifdef HAVE_ICONV
 static iconv_t outputConv = (iconv_t) (-1);
 static iconv_t inputConv = (iconv_t) (-1);
#else
 #warning Charset translation disabled in rexx standalone scripts
#endif

/*---[ Implement ]--------------------------------------------------------------------------------*/

 #include "calls.h"


/* Loading/Unloading */
int librx3270_loaded(void)
{
	Trace("%s - Library loaded",__FUNCTION__);

#ifdef HAVE_ICONV

	outputConv = iconv_open(REXX_DEFAULT_CHARSET, CHARSET);
	inputConv = iconv_open(CHARSET, REXX_DEFAULT_CHARSET);

#endif

    return 0;
}

int librx3270_unloaded(void)
{
#ifdef HAVE_ICONV

 	if(outputConv != (iconv_t) (-1))
		iconv_close(outputConv);

 	if(inputConv != (iconv_t) (-1))
		iconv_close(inputConv);
#endif

    return 0;
}

#if defined WIN32

BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd)
{
//	Trace("%s - Library %s",__FUNCTION__,(dwcallpurpose == DLL_PROCESS_ATTACH) ? "Loaded" : "Unloaded");

    switch(dwcallpurpose)
    {
	case DLL_PROCESS_ATTACH:
		librx3270_loaded();
		break;

	case DLL_PROCESS_DETACH:
		librx3270_unloaded();
		break;
    }

    return TRUE;
}

#endif



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

static void setString(PRXSTRING rx, const char *str)
{
	if(strlen(str) > (RXAUTOBUFLEN-1))
	{
		rx->strptr = RexxAllocateMemory(strlen(str)+1);
		strcpy(rx->strptr,str);
	}
	else
	{
		strncpy(rx->strptr,str,RXAUTOBUFLEN-1);
	}
    rx->strlength = strlen(rx->strptr);
}

 char *GetStringArg(PRXSTRING arg)
 {
	char *ret;

 	if(!(arg->strptr && *arg->strptr))
		return strdup("");

#ifdef HAVE_ICONV

	// Convert received string to 3270 encoding
	if(inputConv != (iconv_t)(-1))
	{
		size_t	in = strlen(arg->strptr);
		size_t out = (in << 1);
		char *ptr;
		char *buffer = malloc(out);

		memset(ptr=buffer,0,out);

		iconv(inputConv,NULL,NULL,NULL,NULL);	// Reset state

		if(iconv(inputConv,(char **) &arg->strptr,&in,&ptr,&out) == ((size_t) -1))
			ret = strdup(arg->strptr);
		else
			ret = strdup(buffer);

		free(buffer);
	}
	else
	{
		ret = strdup(arg->strptr);
	}

#else

		ret = strdup(arg->strptr);

#endif

	return ret;

 }

 void ReleaseStringArg(char *str)
 {
 	free(str);
 }


ULONG RetConvertedString(PRXSTRING Retstr, const char *str)
{

 	if(!str)
 	{
 		// Empty string doesn't need to be converted
 		strcpy(Retstr->strptr,"");
 		Retstr->strlength = 0;
 	}
 	else
 	{
#ifdef HAVE_ICONV
 		// Convert received string to rexx encoding
		if(outputConv != (iconv_t)(-1))
		{
			size_t	in = strlen(str);
			size_t out = (in << 1);
			char *ptr;
			char *buffer = malloc(out);

			memset(ptr=buffer,0,out);

			iconv(outputConv,NULL,NULL,NULL,NULL);	// Reset state

			if(iconv(outputConv,(char **) &str,&in,&ptr,&out) == ((size_t) -1))
				setString(Retstr,str);
			else
				setString(Retstr,buffer);

			free(buffer);
		}
		else
		{
			setString(Retstr,str);
		}
#else
		setString(Retstr,str);
#endif
 	}

    return RXFUNC_OK;
}

 ULONG APIENTRY rx3270QueryRunMode(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	strncpy(Retstr->strptr,"STANDALONE",RXAUTOBUFLEN-1);
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

 ULONG APIENTRY rx3270SetCharset(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(!Argc)
		return RXFUNC_BADCALL;

#ifdef HAVE_ICONV

 	if(outputConv != (iconv_t) (-1))
		iconv_close(outputConv);

 	if(inputConv != (iconv_t) (-1))
		iconv_close(inputConv);

	outputConv = iconv_open(Argv->strptr, CHARSET);

	inputConv = iconv_open(CHARSET, Argv->strptr);

    return RetValue(Retstr,0);

#else

    return RetValue(Retstr,EINVAL);

#endif

 }

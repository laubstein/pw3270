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
 * Este programa está nomeado como calls.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <windows.h>
 #include <lib3270.h>
 #include <malloc.h>
 #include <string.h>
 #include <errno.h>
 #include <pw3270/hllapi.h>
 #include <stdio.h>
 #include <lib3270/log.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 HMODULE 	  hModule	= NULL;
 H3270		* hSession	= NULL;

 static H3270			* (*session_new)(const char *model)											= NULL;
 static void			  (*session_free)(H3270 *h)													= NULL;
 static const char		* (*get_revision)(void)														= NULL;
 static int				  (*host_connect)(H3270 *h,const char *n, int wait)							= NULL;
 static int 			  (*wait_for_ready)(H3270 *h, int seconds)									= NULL;
 static void 			  (*host_disconnect)(H3270 *h)												= NULL;
 static int 			  (*script_sleep)(H3270 *h, int seconds)									= NULL;
 static LIB3270_MESSAGE	  (*get_message)(H3270 *h)													= NULL;
 static char 			* (*get_text)(H3270 *h, int row, int col, int len)							= NULL;
 static void  			* (*release_memory)(void *p)												= NULL;
 static int  			  (*action_enter)(H3270 *h)													= NULL;
 static int 			  (*set_text_at)(H3270 *h, int row, int col, const unsigned char *str)		= NULL;
 static int 			  (*cmp_text_at)(H3270 *h, int row, int col, const char *text)				= NULL;
 static int				  (*pfkey)(H3270 *hSession, int key)										= NULL;
 static int				  (*pakey)(H3270 *hSession, int key)										= NULL;

 static const struct _entry_point
 {
	void		**call;
	const char	* name;
 } entry_point[] =
 {
	{ (void **) &session_new,		"lib3270_session_new" 			},
	{ (void **) &session_free,		"lib3270_session_free"			},
	{ (void **) &get_revision,		"lib3270_get_revision"			},
	{ (void **) &host_connect,		"lib3270_connect"				},
	{ (void **) &host_disconnect,	"lib3270_disconnect"			},
	{ (void **) &wait_for_ready,	"lib3270_wait_for_ready"		},
	{ (void **) &script_sleep,		"lib3270_wait"					},
	{ (void **) &get_message,		"lib3270_get_program_message"	},
	{ (void **) &get_text,			"lib3270_get_text_at"			},
	{ (void **) &release_memory,	"lib3270_free"					},
	{ (void **) &action_enter,		"lib3270_enter"					},
	{ (void **) &set_text_at,		"lib3270_set_string_at"			},
	{ (void **) &cmp_text_at,		"lib3270_cmp_text_at"			},
	{ (void **) &pfkey,				"lib3270_pfkey"					},
	{ (void **) &pakey,				"lib3270_pakey"					},

	{ NULL, NULL }
 };

#undef trace

#ifdef DEBUG
	#define trace(...) { FILE *__dbg = fopen("c:\\users\\perry\\debug.txt","a"); if(__dbg) { fprintf(__dbg,__VA_ARGS__); fclose(__dbg); }; }
#else
	#define trace(...) /* */
#endif // DEBUG

#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
	#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS	0x00001000
#endif // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS

/*--[ Implement ]------------------------------------------------------------------------------------*/

 __declspec (dllexport) DWORD __stdcall hllapi_init(LPSTR mode)
 {
 	if(!mode)
		return EINVAL;

	if(hModule)
		return EBUSY;

	if(!(mode && *mode))
	{
		// Direct mode, load lib3270.dll, get pointers to the calls
		int 		f;
		HKEY 		hKey		= 0;
		HMODULE		kernel		= LoadLibrary("kernel32.dll");
		HANDLE		cookie		= NULL;
		DWORD		rc;
		HANDLE 		(*AddDllDirectory)(PCWSTR NewDirectory) = (HANDLE (*)(PCWSTR)) GetProcAddress(kernel,"AddDllDirectory");
		BOOL 	 	(*RemoveDllDirectory)(HANDLE Cookie)	= (BOOL (*)(HANDLE)) GetProcAddress(kernel,"RemoveDllDirectory");

		// Notify user in case of error loading protocol DLL
		UINT 		errorMode	= SetErrorMode(0);

		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\pw3270",0,KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS)
		{
			char			data[4096];
			unsigned long	datalen	= sizeof(data);		// data field length(in), data returned length(out)
			unsigned long	datatype;					// #defined in winnt.h (predefined types 0-11)
			if(RegQueryValueExA(hKey,"datadir",NULL,&datatype,(LPBYTE) data,&datalen) == ERROR_SUCCESS)
			{
				// Datadir is set, add it to DLL load path
				wchar_t path[4096];
				mbstowcs(path, data, 4095);
				trace("Datadir=[%s] AddDllDirectory=%p RemoveDllDirectory=%p\n",data,AddDllDirectory,RemoveDllDirectory);
				if(AddDllDirectory)
					cookie = AddDllDirectory(path);
			}
			RegCloseKey(hKey);
		}

		hModule = LoadLibraryEx("lib3270.dll.5.0",NULL,LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
		rc = GetLastError();

		SetErrorMode(errorMode);

		if(cookie && RemoveDllDirectory)
			RemoveDllDirectory(cookie);

		if(kernel)
			FreeLibrary(kernel);

		if(!hModule)
			return rc;

		// Get library entry pointers
		for(f=0;entry_point[f].name;f++)
		{
			void *ptr = (void *) GetProcAddress(hModule,entry_point[f].name);

			trace("%d %s=%p\n",f,entry_point[f].name,ptr);

			if(!ptr)
			{
				fprintf(stderr,"Can´t load \"%s\"\n",entry_point[f].name);
				hllapi_deinit();
				return ENOENT;
			}
			*entry_point[f].call = ptr;
		}

		// Get session handle
		hSession = session_new("");

		trace("%s ok hSession=%p\n",__FUNCTION__,hSession);

		return 0;
	}

	// Set entry points to pipe based calls


 	return -1;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_deinit(void)
 {
 	int f;

	// Release session
 	if(hSession && session_free)
		session_free(hSession);

	for(f=0;entry_point[f].name;f++)
		*entry_point[f].call = NULL;

 	if(hModule != NULL)
 	{
		FreeLibrary(hModule);
		hModule = NULL;
 	}

 	return 0;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_revision(void)
 {
	if(!get_revision)
		return 0;
	return (DWORD) atoi(get_revision());
 }

 __declspec (dllexport) DWORD __stdcall hllapi_connect(LPSTR uri)
 {
 	if(!(host_connect && hSession && uri))
		return EINVAL;

 	return host_connect(hSession,uri,1);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_disconnect(void)
 {
 	if(!(host_disconnect && hSession))
		return EINVAL;

	host_disconnect(hSession);

	return 0;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_wait_for_ready(WORD seconds)
 {
 	if(!(wait_for_ready && hSession))
		return EINVAL;

	trace("%s seconds=%d\n", __FUNCTION__, (int) seconds);

	return (DWORD) wait_for_ready(hSession,(int) seconds);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_wait(WORD seconds)
 {
 	if(!(script_sleep && hSession))
		return EINVAL;

	return (DWORD) script_sleep(hSession,(int) seconds);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_message_id(void)
 {
	if(!(get_message && hSession))
		return EINVAL;
	return (DWORD) get_message(hSession);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_screen_at(WORD row, WORD col, LPSTR buffer)
 {
	char	* text;
	int		  len;

	if(!(get_text && release_memory && hSession))
		return EINVAL;

	trace("%s row=%d col=%d buffer=%p",__FUNCTION__,row,col,buffer);
	len = strlen(buffer);

	trace(" len=%d",len);

	text = get_text(hSession,row,col,len);

	trace(" text=%p errno=%d %s\n",text,errno,strerror(errno));

	if(!text)
		return EINVAL;

	strncpy(buffer,text,len);
	release_memory(text);

	trace("text:\n%s\n",buffer);

 	return 0;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_enter(void)
 {
	if(!(action_enter && hSession))
		return EINVAL;

	return (DWORD) action_enter(hSession);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_set_text_at(WORD row, WORD col, LPSTR text)
 {
	if(!(set_text_at && hSession))
		return EINVAL;

	return (DWORD) set_text_at(hSession,row,col,(const unsigned char *) text);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_cmp_text_at(WORD row, WORD col, LPSTR text)
 {
	if(!(cmp_text_at && hSession))
		return EINVAL;

	return (DWORD) cmp_text_at(hSession,row,col,(const char *) text);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_pfkey(WORD key)
 {
	if(!(pfkey && hSession))
		return EINVAL;

	return (DWORD) pfkey(hSession,key);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_pakey(WORD key)
 {
	if(!(pfkey && hSession))
		return EINVAL;

	return (DWORD) pakey(hSession,key);
 }
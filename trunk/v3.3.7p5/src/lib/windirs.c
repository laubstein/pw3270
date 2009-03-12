/* 
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe.
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
 * Este programa está nomeado como windirs.c e possui 108 linhas de código.
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

/*
 *	windirs.c
 *		A Windows console-based 3270 Terminal Emulator
 *		Find common directory paths.
 */

#include <windows.h>
#include <stdio.h>

#include "windirsc.h"

/* Locate the desktop and session directories from the Windows registry. */
int
get_dirs(char *desktop, char *appdata)
{
	HRESULT hres;
	HKEY hkey;
	DWORD index;

	/* Get some paths from Windows. */
	hres = RegOpenKeyEx(HKEY_CURRENT_USER,
    "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
    		0, KEY_QUERY_VALUE, &hkey);
	if (hres != ERROR_SUCCESS) {
	    	printf("Sorry, I can't figure out where your Desktop or "
			"Application Data directories are, Windows error "
			"%ld.\n", hres);
		return -1;
	}

	if (desktop != NULL)
		desktop[0] = '\0';
	if (appdata != NULL)
		appdata[0] = '\0';

	/*
	 * Iterate to find Desktop and AppData.
	 * We can't just go for them individually, because we can't use
	 * ReqQueryValueEx on Win98, and ReqQueryValue doesn't work.
	 */
	for (index = 0; ; index++) {
		char name[MAX_PATH];
		DWORD nlen = MAX_PATH;
		char value[MAX_PATH];
		DWORD vlen = MAX_PATH;
		DWORD type;

		hres = RegEnumValue(hkey, index, name, &nlen, 0, &type, value,
			&vlen);
		if (hres != ERROR_SUCCESS)
			break;

		if (desktop != NULL && !strcmp(name, "Desktop"))
		    	strcpy(desktop, value);
		else if (appdata != NULL && !strcmp(name, "AppData")) {
		    	strcpy(appdata, value);
			strcat(appdata, "\\wc3270\\");
		}

		if ((desktop == NULL || desktop[0]) &&
		    (appdata == NULL || appdata[0]))
		    	break;

	}
	RegCloseKey(hkey);

	if ((desktop != NULL && !desktop[0]) ||
	    (appdata != NULL && !appdata[0])) {

	    	printf("Sorry, I can't figure out where your Desktop or "
			"Application Data directories are.\n");
		return -1;
	}

	return 0;
}

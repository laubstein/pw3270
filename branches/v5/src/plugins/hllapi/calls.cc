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
 * Este programa está nomeado como calls.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <exception>
 #include <pw3270/class.h>
 #include <pw3270/hllapi.h>
 #include "client.h"

 using namespace std;
 using namespace PW3270_NAMESPACE;

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static session	* hSession 			= NULL;
 static time_t	  hllapi_timeout	= 120;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 HLLAPI_API_CALL hllapi_init(LPSTR mode)
 {
 	trace("%s(%s)",__FUNCTION__,mode);

	try
	{
		if(hSession)
			delete hSession;
		hSession = session::create(mode);
		trace("hSession=%p",hSession);
	}
	catch(std::exception &e)
	{
		trace("Error \"%s\"",e.what());
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return hSession ? HLLAPI_STATUS_SUCCESS : HLLAPI_STATUS_SYSTEM_ERROR;
 }

 HLLAPI_API_CALL hllapi_deinit(void)
 {
 	trace("%s()",__FUNCTION__);

 	try
 	{
		if(hSession)
		{
			delete hSession;
			hSession = NULL;
		}
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_get_revision(void)
 {
 	try
 	{
		return atoi(session::get_default()->get_revision().c_str());
	}
	catch(std::exception &e)
	{
		return -1;
	}
	return (DWORD) -1;
 }

 HLLAPI_API_CALL hllapi_connect(LPSTR uri, WORD wait)
 {
 	int rc = HLLAPI_STATUS_SUCCESS;

 	try
 	{
		rc = session::get_default()->connect(uri,hllapi_timeout);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

	return rc;
 }

 HLLAPI_API_CALL hllapi_is_connected(void)
 {
	return session::get_default()->is_connected();
 }

 HLLAPI_API_CALL hllapi_get_state(void)
 {
	switch(hllapi_get_message_id())
	{
	case LIB3270_MESSAGE_NONE:				/* 0 - No message */
		return HLLAPI_STATUS_SUCCESS;

	case LIB3270_MESSAGE_DISCONNECTED:		/* 4 - Disconnected from host */
		return HLLAPI_STATUS_DISCONNECTED;

	case LIB3270_MESSAGE_MINUS:
	case LIB3270_MESSAGE_PROTECTED:
	case LIB3270_MESSAGE_NUMERIC:
	case LIB3270_MESSAGE_OVERFLOW:
	case LIB3270_MESSAGE_INHIBIT:
	case LIB3270_MESSAGE_KYBDLOCK:
		return HLLAPI_STATUS_KEYBOARD_LOCKED;

	case LIB3270_MESSAGE_SYSWAIT:
	case LIB3270_MESSAGE_TWAIT:
	case LIB3270_MESSAGE_AWAITING_FIRST:
	case LIB3270_MESSAGE_X:
	case LIB3270_MESSAGE_RESOLVING:
	case LIB3270_MESSAGE_CONNECTING:
		return HLLAPI_STATUS_WAITING;
	}

	return HLLAPI_STATUS_SYSTEM_ERROR;
 }

 HLLAPI_API_CALL hllapi_disconnect(void)
 {
	session::get_default()->disconnect();
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_wait_for_ready(WORD seconds)
 {
	return session::get_default()->wait_for_ready(seconds);
 }

 HLLAPI_API_CALL hllapi_wait(WORD seconds)
 {
	return session::get_default()->wait(seconds);
 }

 HLLAPI_API_CALL hllapi_get_message_id(void)
 {
	return session::get_default()->get_cstate();
 }

 HLLAPI_API_CALL hllapi_get_screen_at(WORD row, WORD col, LPSTR buffer)
 {
	if(!(buffer && *buffer))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	try
	{
		size_t	  sz = strlen(buffer);
		string	  str = session::get_default()->get_string_at(row,col,sz);
		strncpy(buffer,str.c_str(),sz);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_enter(void)
 {
	return session::get_default()->enter();
 }

 HLLAPI_API_CALL hllapi_set_text_at(WORD row, WORD col, LPSTR text)
 {
	try
	{
		session::get_default()->set_string_at(row,col,text);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_cmp_text_at(WORD row, WORD col, LPSTR text)
 {
 	int rc = HLLAPI_STATUS_SYSTEM_ERROR;
	try
	{
		rc = session::get_default()->cmp_string_at(row,col,text);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return rc;
 }

 HLLAPI_API_CALL hllapi_pfkey(WORD key)
 {
	return session::get_default()->pfkey(key);
 }

 HLLAPI_API_CALL hllapi_pakey(WORD key)
 {
	return session::get_default()->pakey(key);
 }

 HLLAPI_API_CALL hllapi_get_datadir(LPSTR datadir)
 {
	HKEY 			hKey	= 0;
 	unsigned long	datalen = strlen(datadir);

	*datadir = 0;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\pw3270",0,KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS)
	{
		unsigned long datatype;					// #defined in winnt.h (predefined types 0-11)
		if(RegQueryValueExA(hKey,"datadir",NULL,&datatype,(LPBYTE) datadir,&datalen) != ERROR_SUCCESS)
			*datadir = 0;
		RegCloseKey(hKey);
	}

	return *datadir;
 }

 HLLAPI_API_CALL hllapi_setcursor(WORD pos)
 {
	return session::get_default()->set_cursor_addr(pos-1);
 }

 HLLAPI_API_CALL hllapi_getcursor()
 {
	return session::get_default()->get_cursor_addr()+1;
 }

 HLLAPI_API_CALL hllapi_get_screen(WORD offset, LPSTR buffer, WORD len)
 {
	int rc = HLLAPI_STATUS_SYSTEM_ERROR;

	if(!(buffer && *buffer))
		return rc;

	try
	{
		size_t szBuffer = strlen(buffer);

		if(len < szBuffer && len > 0)
			szBuffer = len;

		string str = session::get_default()->get_string(offset,szBuffer);
		strncpy(buffer,str.c_str(),szBuffer);
		rc = HLLAPI_STATUS_SUCCESS;
	}
	catch(std::exception &e)
	{
		rc = HLLAPI_STATUS_SYSTEM_ERROR;
	}

	return rc;
 }

 HLLAPI_API_CALL hllapi_emulate_input(LPSTR buffer, WORD len, WORD pasting)
 {
	session::get_default()->emulate_input(buffer);
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_erase_eof(void)
 {
	session::get_default()->erase_eof();
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_print(void)
 {
	return session::get_default()->print();
 }

 char * hllapi_get_string(int offset, size_t len)
 {
	try
	{
		string str = session::get_default()->get_string(offset-1,len);
		char * ret = strdup(str.c_str());
		return ret;
	}
	catch(std::exception &e)
	{
	}

	return NULL;
 }

 void hllapi_free(void *p)
 {
 	free(p);
 }

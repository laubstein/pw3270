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
 * Este programa está nomeado como connection.cxx e possui - linhas de código.
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

#include "ooo3270.hpp"
#include <osl/thread.hxx>
#include <errno.h>
#include <lib3270/api.h>
#include <string.h>
#include <time.h>
#include <malloc.h>

/*---[ Conection Thread ]----------------------------------------------------------------------------------*/

 static oslThread hThread = NULL;

/**
 * Network threads.
 *
 * Separated thread to takes care of all network communication to avoid
 * "hangs" on user interface.
 *
 */
 void network_thread(char *hostinfo)
 {
 	Trace("%s starts",__FUNCTION__);

	if(!host_connect(hostinfo,1))
	{
		while(QueryCstate() != NOT_CONNECTED)
			RunPendingEvents(1);
	}

	free(hostinfo);

	hThread = NULL;
 	Trace("%s ends",__FUNCTION__);
 }

/*---[ Connection related calls ]--------------------------------------------------------------------------*/

sal_Int16 SAL_CALL pw3270::uno_impl::getConnectionState() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	return (sal_Int16) QueryCstate();
}

sal_Int16 SAL_CALL pw3270::uno_impl::Connect( const OUString& hostinfo, ::sal_Int16 timeout) throw (RuntimeException)
{
	char *ptr;
	time_t end;

	OString str = rtl::OUStringToOString( hostinfo , RTL_TEXTENCODING_ASCII_US );

	if(hThread)
		return EBUSY;

	Trace("%s(%s)",__FUNCTION__,str.getStr());

	if(QueryCstate() != NOT_CONNECTED)
		return EINVAL;


	ptr = strdup(str.getStr());
	hThread = osl_createThread((oslWorkerFunction) network_thread, ptr);

	if(!hThread)
	{
		free(ptr);
		return -1;
	}

	if(timeout < 1)
		return 0;

	end = time(0)+timeout;

	Trace("Waiting for connected state (timeout: %d)",timeout);

	while(time(0) < end)
	{
		if( (CONNECTED) && query_3270_terminal_status() == STATUS_CODE_BLANK)
		{
			Trace("%s: Connected and ready",__FUNCTION__);
			return 0;
		}

		yeld();
	}

	return ETIMEDOUT;
}

sal_Int16 SAL_CALL pw3270::uno_impl::Disconnect() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	if(QueryCstate() <= NOT_CONNECTED)
		return EINVAL;

	host_disconnect(0);

	return 0;
}

::sal_Bool SAL_CALL pw3270::uno_impl::isConnected(  ) throw (::com::sun::star::uno::RuntimeException)
{
	return (CONNECTED) ? true : false;
}


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
 * Este programa está nomeado como get.cxx e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "globals.hpp"
 #include <string.h>

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

sal_Int16 SAL_CALL pw3270::uno_impl::getRevision() throw (RuntimeException)
{
	return hSession->get_revision();
}

sal_Int16 SAL_CALL pw3270::uno_impl::getConnectionState(  ) throw (::com::sun::star::uno::RuntimeException)
{
	return hSession->get_state();
}

::rtl::OUString SAL_CALL pw3270::uno_impl::getTextAt( ::sal_Int16 row, ::sal_Int16 col, ::sal_Int16 size ) throw (::com::sun::star::uno::RuntimeException)
{
	char		* ptr = hSession->get_text_at(row,col,size);
	OUString	  ret;

	if(!ptr)
		return OUString( RTL_CONSTASCII_USTRINGPARAM( "" ) );

	ret = OUString(ptr, strlen(ptr), hSession->get_encoding(), RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

	hSession->mem_free(ptr);

	return ret;
}



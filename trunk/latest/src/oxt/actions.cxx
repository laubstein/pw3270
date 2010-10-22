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
 * Este programa está nomeado como actions.cxx e possui - linhas de código.
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
#include <time.h>
#include <lib3270/api.h>

/*---[ Macros ]--------------------------------------------------------------------------------------------*/

 #define CHECK_FOR_TERMINAL_STATUS	if(!PCONNECTED) \
										return ENOTCONN; \
									else if(query_3270_terminal_status() != STATUS_CODE_BLANK) \
										return EBUSY;

/*---[ Action related calls ]------------------------------------------------------------------------------*/

::sal_Int16 SAL_CALL pw3270::uno_impl::sendEnterKey() throw (::com::sun::star::uno::RuntimeException)
{
	CHECK_FOR_TERMINAL_STATUS

	return lib3270_send_enter();
}

::sal_Int16 SAL_CALL pw3270::uno_impl::setStringAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& str ) throw (::com::sun::star::uno::RuntimeException)
{
//	int		rc;
	OString vlr = rtl::OUStringToOString( str , getEncoding() );

	CHECK_FOR_TERMINAL_STATUS

	if(row < 1 || col < 1)
		return EINVAL;

	row--;
	col--;

	Trace("Inserting \"%s\" at %d,%d",vlr.getStr(),row,col);

	cursor_set_addr((row * ctlr_get_cols()) + col);

	Input_String((unsigned char *) vlr.getStr());

	return 0;
}

::sal_Int16 SAL_CALL pw3270::uno_impl::sendPFKey( ::sal_Int16 key ) throw (::com::sun::star::uno::RuntimeException)
{
	CHECK_FOR_TERMINAL_STATUS

	return lib3270_send_pfkey(key);
}

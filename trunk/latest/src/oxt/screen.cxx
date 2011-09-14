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
 * Este programa está nomeado como screen.cxx e possui - linhas de código.
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
#include <errno.h>
#include <string.h>
#include <time.h>
#include <lib3270/api.h>

/*---[ Screen related calls ]------------------------------------------------------------------------------*/

::rtl::OUString SAL_CALL pw3270::uno_impl::getScreenContentAt( ::sal_Int16 row, ::sal_Int16 col, ::sal_Int16 size ) throw (::com::sun::star::uno::RuntimeException)
{
	OUString ret;
	int start, rows, cols;
	char *buffer;

	row--;
	col--;

	Trace("Reading %d bytes at %d,%d",size,row,col);

	get_3270_terminal_size(this->hSession,&rows,&cols);

	if(row < 0 || row > rows || col < 0 || col > cols || size < 1)
		return OUString( RTL_CONSTASCII_USTRINGPARAM( "" ) );

	start = ((row) * cols) + col;

	buffer = (char *) malloc(size+1);

	screen_read(buffer, start, size);
	*(buffer+size) = 0;

	Trace("Read\n%s\n",buffer);

	ret = OUString(buffer,strlen(buffer), getEncoding(), RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

	free(buffer);

	return ret;

}

::rtl::OUString SAL_CALL pw3270::uno_impl::getScreenContent() throw (::com::sun::star::uno::RuntimeException)
{
	OUString ret;
	int sz, rows, cols;
	char *buffer;
	char *ptr;
	int  pos = 0;

	get_3270_terminal_size(this->hSession,&rows,&cols);

	Trace("Reading screen with %dx%d",rows,cols);

	sz = rows*(cols+1)+1;
	ptr = buffer = (char *) malloc(sz);
	memset(buffer,0,sz);

	for(int row = 0; row < rows;row++)
	{
		screen_read(ptr,pos,cols);
		pos += cols;
		ptr += cols;
		*(ptr++) = '\n';
	}
	*ptr = 0;

	Trace("Read\n%s\n",buffer);

	ret = OUString(buffer, strlen(buffer), getEncoding(), RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

	free(buffer);

	return ret;

}

::sal_Int16 SAL_CALL pw3270::uno_impl::waitForStringAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& key, ::sal_Int16 timeout ) throw (::com::sun::star::uno::RuntimeException)
{
	OString str = rtl::OUStringToOString( key , getEncoding() );
	int 	rc = ETIMEDOUT;
	int 	end = time(0)+timeout;
	int 	sz, rows, cols, start;
	char 	*buffer;
	int 	last = -1;

	get_3270_terminal_size(this->hSession,&rows,&cols);

	row--;
	col--;
	start = (row * cols) + col;

	sz = strlen(str.getStr());
	buffer = (char *) malloc(sz+1);

	Trace("Waiting for \"%s\" at %d",str.getStr(),start);

	wait();

	while( (rc == ETIMEDOUT) && (time(0) <= end) )
	{
		if(!CONNECTED)
		{
			// Disconnected, return
			rc = ENOTCONN;
		}
		else if(query_3270_terminal_status() == STATUS_CODE_BLANK)
		{
			// Screen contents are ok. Check.
			Trace("Waiting (last=%d)...",last);
			Trace("%d",query_screen_change_counter());

			if(last != query_screen_change_counter())
			{
				last = query_screen_change_counter();
				screen_read(buffer,start,sz);
				Trace("Found \"%s\"",buffer);
				if(!strncmp(buffer,str.getStr(),sz))
				{
					free(buffer);
					return 0;
				}
			}
		}

		wait();
	}

	free(buffer);

	return rc;
}

::sal_Bool SAL_CALL pw3270::uno_impl::queryStringAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& key ) throw (::com::sun::star::uno::RuntimeException)
{
	OString str = rtl::OUStringToOString( key , getEncoding() );
	int 	sz, rows, cols, start;
	char 	*buffer;
	bool	rc;

	get_3270_terminal_size(this->hSession,&rows,&cols);

	row--;
	col--;
	start = (row * cols) + col;

	sz = strlen(str.getStr());
	buffer = (char *) malloc(sz+1);

	screen_read(buffer,start,sz);

	rc = (strncmp(buffer,str.getStr(),sz) == 0);

	free(buffer);

	return rc;
}



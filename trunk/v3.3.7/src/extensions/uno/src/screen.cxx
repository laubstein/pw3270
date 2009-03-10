
#include "ooo3270.hpp"
#include <osl/thread.h>
#include <errno.h>
#include <lib3270/api.h>

/*---[ Screen related calls ]------------------------------------------------------------------------------*/

::rtl::OUString SAL_CALL I3270Impl::getScreenContentAt( ::sal_Int16 row, ::sal_Int16 col, ::sal_Int16 size ) throw (::com::sun::star::uno::RuntimeException)
{
	OUString ret;
	int start, rows, cols;
	char *buffer;

	row--;
	col--;

	Trace("Reading %d bytes at %d,%d",size,row,col);

	screen_size(&rows,&cols);

	if(row < 0 || row > rows || col < 0 || col > cols || size < 1)
		return OUString( RTL_CONSTASCII_USTRINGPARAM( "" ) );

	start = ((row) * cols) + col;

	buffer = (char *) malloc(size+1);

	screen_read(buffer, start, size);
	*(buffer+size) = 0;

	Trace("Read\n%s\n",buffer);

	ret = OUString::createFromAscii( buffer );

	free(buffer);

	return ret;

}

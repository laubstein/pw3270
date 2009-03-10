
#include "ooo3270.hpp"
// #include <osl/thread.h>
#include <errno.h>
#include <string.h>
#include <time.h>
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

	ret = OUString(buffer,strlen(buffer), RTL_TEXTENCODING_ISO_8859_1, RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

	free(buffer);

	return ret;

}

::rtl::OUString SAL_CALL I3270Impl::getScreenContent() throw (::com::sun::star::uno::RuntimeException)
{
	OUString ret;
	int sz, rows, cols;
	char *buffer;
	char *ptr;
	int  pos = 0;

	screen_size(&rows,&cols);

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

	ret = OUString(buffer,strlen(buffer), RTL_TEXTENCODING_ISO_8859_1, RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

	free(buffer);

	return ret;

}

::sal_Int16 SAL_CALL I3270Impl::waitForScreen( ::sal_Int16 timeout ) throw (::com::sun::star::uno::RuntimeException)
{
	int rc = 0;
	int last = query_screen_change_counter();
	int end = time(0)+timeout;

	while(!rc && last == query_screen_change_counter())
	{
		if(!CONNECTED)
			rc = ENOTCONN;
		else if(time(0) > end)
			rc = ETIMEDOUT;
		RunPendingEvents(1);
	}

	return rc;
}

::sal_Int16 SAL_CALL I3270Impl::waitForStringAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& key, ::sal_Int16 timeout ) throw (::com::sun::star::uno::RuntimeException)
{
	OString str = rtl::OUStringToOString( key , RTL_TEXTENCODING_ASCII_US );
	int 	rc = ETIMEDOUT;
	int 	end = time(0)+timeout;
	int 	sz, rows, cols, start;
	char 	*buffer;
	int 	last = -1;

	screen_size(&rows,&cols);

	row--;
	col--;
	start = (row * cols) + col;

	sz = strlen(str.getStr());
	buffer = (char *) malloc(sz+1);

	Trace("Waiting for \"%s\" at %d",str.getStr(),start);

	while( (rc == ETIMEDOUT) && (time(0) <= end) )
	{
		if(!CONNECTED)
		{
			rc = ENOTCONN;
		}
		else
		{
			Trace("Waiting (last=%d)...",last);
			RunPendingEvents(1);
			Trace("%d",query_screen_change_counter());

			if(last != query_screen_change_counter())
			{
				last = query_screen_change_counter();
				screen_read(buffer,start,sz);
				Trace("Found \"%s\"",buffer);
				if(!strncmp(buffer,str.getStr(),sz))
					rc = 0;
			}
		}
	}

	free(buffer);

	return rc;
}


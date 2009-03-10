
#include "ooo3270.hpp"
#include <osl/thread.h>
#include <errno.h>
#include <lib3270/api.h>

/*---[ Screen related calls ]------------------------------------------------------------------------------*/

::rtl::OUString SAL_CALL I3270Impl::getScreenContentAt( ::sal_Int16 row, ::sal_Int16 col, ::sal_Int16 size ) throw (::com::sun::star::uno::RuntimeException)
{
	Trace("Reading %d bytes at %d,%d",size,row,col);

	return OUString( RTL_CONSTASCII_USTRINGPARAM( "String de teste" ) );

}

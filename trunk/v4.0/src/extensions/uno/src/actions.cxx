

#include "ooo3270.hpp"
#include <time.h>
#include <lib3270/api.h>

/*---[ Macros ]--------------------------------------------------------------------------------------------*/

 #define CHECK_FOR_TERMINAL_STATUS	if(!PCONNECTED) \
										return ENOTCONN; \
									else if(query_3270_terminal_status() != STATUS_CODE_BLANK) \
										return EINVAL;

/*---[ Action related calls ]------------------------------------------------------------------------------*/

::sal_Int16 SAL_CALL I3270Impl::sendEnterKey() throw (::com::sun::star::uno::RuntimeException)
{
	CHECK_FOR_TERMINAL_STATUS

	return action_Enter();
}

::sal_Int16 SAL_CALL I3270Impl::setStringAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& str ) throw (::com::sun::star::uno::RuntimeException)
{
	int		rc;
	OString vlr = rtl::OUStringToOString( str , RTL_TEXTENCODING_ASCII_US );

	if(!PCONNECTED)
		return ENOTCONN;

	if(row < 1 || col < 1)
		return EINVAL;

	row--;
	col--;

	Trace("Inserting \"%s\" at %d,%d",vlr.getStr(),row,col);

	cursor_set_addr((row * ctlr_get_cols()) + col);

	Input_String((unsigned char *) vlr.getStr());

	return 0;
}

::sal_Int16 SAL_CALL I3270Impl::sendPFKey( ::sal_Int16 key ) throw (::com::sun::star::uno::RuntimeException)
{
	CHECK_FOR_TERMINAL_STATUS

	return action_PFKey(key);
}

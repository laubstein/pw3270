
#include "ooo3270.hpp"
#include <osl/thread.h>
#include <errno.h>
#include <lib3270/api.h>

/*---[ Statics ]-------------------------------------------------------------------------------------------*/

// static oslThread hThread = NULL;

/*---[ Connection related calls ]--------------------------------------------------------------------------*/

sal_Int16 SAL_CALL g3270::uno_impl::getConnectionState() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	return (sal_Int16) QueryCstate();
}

sal_Int16 SAL_CALL g3270::uno_impl::Connect( const OUString& hostinfo, ::sal_Int16 wait) throw (RuntimeException)
{
	int		rc;
	OString str = rtl::OUStringToOString( hostinfo , RTL_TEXTENCODING_ASCII_US );

	Trace("%s(%s)",__FUNCTION__,str.getStr());

	if(QueryCstate() != NOT_CONNECTED)
		return EINVAL;

	rc = host_connect(str.getStr(),wait);

	Trace("%s(%s): %d (IN_ANSI: %d IN_3270: %d)",__FUNCTION__,str.getStr(),rc,IN_ANSI,IN_3270);

	return rc;
}

sal_Int16 SAL_CALL g3270::uno_impl::Disconnect() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	if(QueryCstate() <= NOT_CONNECTED)
		return EINVAL;

	host_disconnect(0);

	return 0;
}

::sal_Bool SAL_CALL g3270::uno_impl::isConnected(  ) throw (::com::sun::star::uno::RuntimeException)
{
	return (CONNECTED) ? true : false;
}

::sal_Int16 SAL_CALL g3270::uno_impl::waitForEvents( ) throw (::com::sun::star::uno::RuntimeException)
{
	if(QueryCstate() == NOT_CONNECTED)
		return EINVAL;

	RunPendingEvents(1);

	return 0;
}

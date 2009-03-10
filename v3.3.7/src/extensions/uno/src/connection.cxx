
#include "ooo3270.hpp"
#include <osl/thread.h>
#include <errno.h>
#include <lib3270/api.h>

/*---[ Statics ]-------------------------------------------------------------------------------------------*/

// static oslThread hThread = NULL;

/*---[ Connection related calls ]--------------------------------------------------------------------------*/

sal_Int16 SAL_CALL I3270Impl::getConnectionState() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	return (sal_Int16) QueryCstate();
}

sal_Int16 SAL_CALL I3270Impl::Connect( const OUString& hostinfo ) throw (RuntimeException)
{
	int		rc;
	OString str = rtl::OUStringToOString( hostinfo , RTL_TEXTENCODING_ASCII_US );

	Trace("%s(%s)",__FUNCTION__,str.getStr());

	if(QueryCstate() != NOT_CONNECTED)
		return EINVAL;

	rc = host_connect(str.getStr());

	Trace("host_connect: %d",rc);

	if(rc >= 0)
	{
			RunPendingEvents(1);

			rc = 0;
			while(!IN_ANSI && !IN_3270 && !rc)
			{
					RunPendingEvents(1);
					if(!PCONNECTED)
						rc = ENOTCONN;
			}
	}

	Trace("%s(%s): %d (IN_ANSI: %d IN_3270: %d)",__FUNCTION__,str.getStr(),rc,IN_ANSI,IN_3270);

	return rc;
}

sal_Int16 SAL_CALL I3270Impl::Disconnect() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	if(QueryCstate() <= NOT_CONNECTED)
		return EINVAL;

	host_disconnect(0);

	return 0;
}


#include "ooo3270.hpp"
#include <lib3270/api.h>

/*---[ Connection related calls ]--------------------------------------------------------------------------*/

sal_Int16 SAL_CALL I3270Impl::getConnectionState() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	return (sal_Int16) QueryCstate();
}

sal_Int16 SAL_CALL I3270Impl::Connect( const OUString& hostinfo ) throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	if(QueryCstate() != NOT_CONNECTED)
		return EINVAL;

// int host_connect(const char *n);


	return -1;
}

sal_Int16 SAL_CALL I3270Impl::Disconnect() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	if(QueryCstate() <= NOT_CONNECTED)
		return EINVAL;

	host_disconnect(0);

	return 0;
}

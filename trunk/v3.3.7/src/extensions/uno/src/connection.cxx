
#include "ooo3270.hpp"

/*---[ Connection related calls ]--------------------------------------------------------------------------*/

sal_Int16 SAL_CALL I3270Impl::getConnectionState() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);

	return -1;
}

sal_Int16 SAL_CALL I3270Impl::Connect( const OUString& hostinfo ) throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);


	return -1;
}

sal_Int16 SAL_CALL I3270Impl::Disconnect() throw (RuntimeException)
{
	Trace("%s",__FUNCTION__);


	return -1;
}


#include "ooo3270.hpp"
#include <rtl/uuid.h>

#include <lib3270/api.h>

// http://wiki.services.openoffice.org/wiki/Counter_Example
// http://openoffice.org.sourcearchive.com/documentation/1.1.3/classframework_1_1BackingComp_69f84955c44b2e8be98cdbb6f4b675e7.html#69f84955c44b2e8be98cdbb6f4b675e7
// http://wiki.services.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/C%2B%2B_Component

/*---[ Implement XInterface ]------------------------------------------------------------------------------*/

/**
 * Function to create a new component instance; is needed by factory helper implementation.
 * @param xMgr service manager to if the components needs other component instances
 */
Reference< XInterface > SAL_CALL I3270Impl_create( const Reference< XMultiServiceFactory > & xMgr )
{
	Trace("%s",__FUNCTION__);
	return Reference< I3270 >( new I3270Impl( xMgr ) );
}

/*---[ Implement XServiceInfo ]----------------------------------------------------------------------------*/

OUString SAL_CALL I3270Impl::getImplementationName(  ) throw(RuntimeException)
{
	Trace("%s",__FUNCTION__);
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}

sal_Bool SAL_CALL I3270Impl::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
	Sequence< OUString > aSNL = getSupportedServiceNames();
	const OUString * pArray = aSNL.getArray();

	Trace("%s",__FUNCTION__);

	for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
		if( pArray[i] == ServiceName )
			return sal_True;

	return sal_False;
}

Sequence<OUString> SAL_CALL I3270Impl::getSupportedServiceNames(  ) throw(RuntimeException)
{
	Trace("%s",__FUNCTION__);
	return getSupportedServiceNames_Static();
}

Sequence<OUString> SAL_CALL I3270Impl::getSupportedServiceNames_Static(  )
{
	OUString aName( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME ) );

	Trace("%s returns: \"%s\"",__FUNCTION__,SERVICENAME);

	return Sequence< OUString >( &aName, 1 );
}

/*---[ Implement exported calls ]--------------------------------------------------------------------------*/
/**
 * Gives the environment this component belongs to.
 */
extern "C" void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)
{
	Trace("%s",LANGUAGE_BINDING_NAME);
	*ppEnvTypeName = LANGUAGE_BINDING_NAME;
}

/**
 * This function creates an implementation section in the registry and another subkey
 *
 * for each supported service.
 * @param pServiceManager   the service manager
 * @param pRegistryKey      the registry key
 */
extern "C" sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey)
{
	Trace("%s",__FUNCTION__);

	sal_Bool result = sal_False;

	if (pRegistryKey)
	{
		try
		{
			Reference< XRegistryKey > xNewKey(
				reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
					OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

			const Sequence< OUString > & rSNL =
				I3270Impl::getSupportedServiceNames_Static();
			const OUString * pArray = rSNL.getConstArray();
			for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
				xNewKey->createKey( pArray[nPos] );

			return sal_True;
		}
		catch (InvalidRegistryException &)
		{
			// we should not ignore exceptions
		}
	}
	return result;
}

/**
 * This function is called to get service factories for an implementation.
 *
 * @param pImplName       name of implementation
 * @param pServiceManager a service manager, need for component creation
 * @param pRegistryKey    the registry key for this component, need for persistent data
 * @return a component factory
 */
extern "C" void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey)
{
	void * pRet = 0;

	Trace("%s",__FUNCTION__);

	if (rtl_str_compare( pImplName, IMPLNAME ) == 0)
	{
		Reference< XSingleServiceFactory > xFactory( createSingleFactory(
			reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
			OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
			I3270Impl_create,
			I3270Impl::getSupportedServiceNames_Static() ) );

		if (xFactory.is())
		{
			xFactory->acquire();
			pRet = xFactory.get();
		}
	}

	return pRet;
}

/*---[ XInterface implementation ]-------------------------------------------------------------------------*/

void SAL_CALL I3270Impl::acquire() throw ()
{
	++m_nRefCount;
}

void SAL_CALL I3270Impl::release() throw ()
{
	if (! --m_nRefCount)
		delete this;
}

Any I3270Impl::queryInterface( Type const & type ) throw (RuntimeException)
{
	if (type.equals( ::cppu::UnoType< Reference< XInterface > >::get()))
	{
		// return XInterface interface (resolve ambiguity caused by multiple inheritance from
		// XInterface subclasses by casting to lang::XTypeProvider)
		Trace("%s(type).equals %s",__FUNCTION__,"XInterface");
		Reference< XInterface > x( static_cast< lang::XTypeProvider * >( this ) );
		return makeAny( x );

	}
	if (type.equals( ::cppu::UnoType< Reference< lang::XTypeProvider > >::get()))
	{
		// return XInterface interface
		Trace("%s(type).equals %s",__FUNCTION__,"lang::XTypeProvider");
		Reference< lang::XTypeProvider > x( static_cast< lang::XTypeProvider * >( this ) );
		return makeAny( x );

	}
	if (type.equals( ::cppu::UnoType< Reference< lang::XServiceInfo > >::get()))
	{
		// return XServiceInfo interface
		Trace("%s(type).equals %s",__FUNCTION__,"lang::XServiceInfo");
		Reference< lang::XServiceInfo > x( static_cast< lang::XServiceInfo * >( this ) );
		return makeAny( x );

	}
	if (type.equals( ::cppu::UnoType< Reference< I3270 > >::get()))
	{
		// return sample interface
		Trace("%s(type).equals %s",__FUNCTION__,"I3270");
		Reference< I3270 > x( static_cast< I3270 * >( this ) );
		return makeAny( x );

	}

	// querying for unsupported type
	Trace("%s(type).equals %s",__FUNCTION__,"Unsupported");

	return Any();

}

/*---[ Implement XTypeProvider ]---------------------------------------------------------------------------*/

Sequence<sal_Int8> SAL_CALL I3270Impl::getImplementationId(void) throw (RuntimeException)
{
	static Sequence< sal_Int8 > * s_pId = 0;

	Trace("%s",__FUNCTION__);

	if(! s_pId)
	{
		// create unique id
		Sequence< sal_Int8 > id( 16 );

		::rtl_createUuid( (sal_uInt8 *)id.getArray(), 0, sal_True );

		// guard initialization with some mutex
		::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
		if (! s_pId)
		{
			static Sequence< sal_Int8 > s_id( id );
			s_pId = &s_id;
		}
	}
	return *s_pId;

}

Sequence< Type > SAL_CALL I3270Impl::getTypes(  ) throw (RuntimeException)
{
	// http://wiki.services.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Implementing_without_Helpers
	Sequence< Type > seq( 3 );

	Trace("%s begin",__FUNCTION__);

	seq[ 0 ] = ::cppu::UnoType< Reference< lang::XTypeProvider > >::get();
	seq[ 1 ] = ::cppu::UnoType< Reference< lang::XServiceInfo > >::get();
	seq[ 2 ] = ::cppu::UnoType< Reference< I3270 > >::get();

	Trace("%s ends",__FUNCTION__);
	return seq;
}

/*---[ Implement I3270 ]-----------------------------------------------------------------------------------*/

static bool started = false;

I3270Impl::I3270Impl( const Reference< XMultiServiceFactory > & xServiceManager ) : m_nRefCount(0)
{
	Trace("Object created %s",__FUNCTION__);

	if(!started)
	{
		started = true;
		lib3270_init(OFFICE_PROGRAM);
	}



}

I3270Impl::~I3270Impl()
{
	Trace("Object destroyed %s",__FUNCTION__);

}

OUString I3270Impl::SAL_CALL getVersion() throw (RuntimeException)
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM( PACKAGE_VERSION ) );
}




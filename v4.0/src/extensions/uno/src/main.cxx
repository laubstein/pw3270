
#include "ooo3270.hpp"
// #include <rtl/uuid.h>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

// #include <comphelper/componentmodule.hxx>

#include <lib3270/api.h>

// http://wiki.services.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/C%2B%2B_Component
// http://wiki.services.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Providing_a_Single_Factory_Using_a_Helper_Method

using namespace com::sun::star::registry; // for XRegistryKey
using namespace com::sun::star::lang; // for XSingleComponentFactory

/*---[ Statics ]-------------------------------------------------------------------------------------------*/

static Sequence< OUString > getSupportedServiceNames_g3270()
{
	Sequence<OUString> names(1);

	Trace("%s returns: %s",__FUNCTION__, SERVICENAME);
	names[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME ) );

	return names;
}

static OUString getImplementationName_g3270()
{
	Trace("%s returns: %s",__FUNCTION__, IMPLNAME);

	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}


static Reference< XInterface > SAL_CALL CreateInstance_g3270( const Reference< XComponentContext > & xContext )
{
	return static_cast< lang::XTypeProvider * >( new g3270::uno_impl( xContext ) );
}


/*---[ Implement exported calls ]--------------------------------------------------------------------------*/

/**************************************************************
 * Function to determine the environment of the implementation.
 *
 * If the environment is NOT session specific
 * (needs no additional context), then this function
 * should return the environment type name and leave ppEnv (0).
 *
 * @param ppEnvTypeName	environment type name;
 *							string must be constant
 * @param ppEnv			function returns its environment
 *							if the environment is session specific,
 *							i.e. has special context
 */
extern "C" void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)
{
	Trace("%s set envtype to %s",__FUNCTION__,LANGUAGE_BINDING_NAME);
	*ppEnvTypeName = LANGUAGE_BINDING_NAME;
}

/************************************************************
 * Optional function to retrieve a component description.
 *
 * @return an XML formatted string containing a short
 *         component description
 */
// typedef const sal_Char * (SAL_CALL * component_getDescriptionFunc)(void);

/**********************************************************
 * Writes component registry info, at least writing the
 * supported service names.
 *
 * @param pServiceManager	a service manager
 *								(the type is XMultiServiceFactory
 *								to be used by the environment
 *								returned by
 *								component_getImplementationEnvironment)
 *
 * @param pRegistryKey		a registry key
 *								(the type is XRegistryKey to be used
 *								by the environment returned by
 *								component_getImplementationEnvironment)
 *
 * @return	true if everything went fine
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

			const Sequence< OUString > & rSNL = getSupportedServiceNames_g3270();
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

/*********************************************************
 * Retrieves a factory to create component instances.
 *
 * @param pImplName			desired implementation name
 *
 * @param pServiceManager 	a service manager
 *								(the type is XMultiServiceFactory
 *								to be used by the environment
 *								returned by
 *								component_getImplementationEnvironment)
 *
 * @param pRegistryKey		a registry key
 *								(the type is XRegistryKey to be used
 *								by the environment returned by
 *								component_getImplementationEnvironment)
 *
 * @return						acquired component factory
 *								(the type is XInterface to be used by the
 *								environment returned by
 *								component_getImplementationEnvironment)
 */
extern "C" void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey)
{
	void * pRet = 0;

	Trace("%s",__FUNCTION__);

	if(pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
	{
		Reference< XSingleComponentFactory > xFactory( ::cppu::createSingleComponentFactory(
					CreateInstance_g3270, OUString::createFromAscii( IMPLNAME ), getSupportedServiceNames_g3270() ));


		if (xFactory.is())
		{
			xFactory->acquire();
			pRet = xFactory.get();
		}
	}

	return pRet;
}

/*---[ Implement XInitialization ]-------------------------------------------------------------------------*/

void SAL_CALL g3270::uno_impl::initialize( Sequence< Any > const & args ) throw (Exception)
{
	Trace("%s",__FUNCTION__);
}

/*---[ Implement XServiceInfo ]----------------------------------------------------------------------------*/

OUString SAL_CALL g3270::uno_impl::getImplementationName(  ) throw(RuntimeException)
{
	Trace("%s",__FUNCTION__);
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}

sal_Bool SAL_CALL g3270::uno_impl::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
	Trace("%s",__FUNCTION__);
	return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("IMPLNAME") );
}

Sequence< OUString > g3270::uno_impl::getSupportedServiceNames() throw (RuntimeException)
{
	return getSupportedServiceNames_g3270();
}

/*---[ Implement I3270 ]-----------------------------------------------------------------------------------*/

namespace g3270
{

static bool started = false;

uno_impl::uno_impl( const Reference< XComponentContext > & xContext )
{
	Trace("Object created %s",__FUNCTION__);

	if(!started)
	{
		started = true;
		lib3270_init(OFFICE_PROGRAM);
	}

}

uno_impl::~uno_impl()
{
	Trace("Object deleted %s",__FUNCTION__);
}


OUString SAL_CALL uno_impl::getVersion() throw (RuntimeException)
{
	return OUString( RTL_CONSTASCII_USTRINGPARAM( PACKAGE_VERSION ) );
}

};

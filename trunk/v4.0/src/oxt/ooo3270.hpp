

#include "globals.hpp"
#include <osl/thread.hxx>

#include <cppuhelper/implbase3.hxx> // "3" implementing three interfaces
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implementationentry.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

/*---[ Object implementation ]-----------------------------------------------------------------------------*/

using namespace ::rtl; // for OUString
using namespace ::com::sun::star; // for sdk interfaces
using namespace ::com::sun::star::uno; // for basic types

namespace pw3270
{

class uno_impl : public ::cppu::WeakImplHelper3< br::com::bb::pw3270intf, lang::XServiceInfo, lang::XInitialization >
{
public:

	uno_impl( const Reference< XComponentContext > & xContext );
	virtual ~uno_impl();

	// XInitialization will be called upon createInstanceWithArguments[AndContext]()
	virtual void SAL_CALL initialize( Sequence< Any > const & args ) throw (Exception);

	// XInterface implementation
//	virtual void SAL_CALL acquire() throw ();
//	virtual void SAL_CALL release() throw ();
//	virtual Any SAL_CALL queryInterface( const Type & rType ) throw (RuntimeException);

	// XServiceInfo	implementation
    virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
	virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

	// XTypeProvider implementation
//	virtual Sequence<sal_Int8> SAL_CALL getImplementationId(void) throw (RuntimeException);
//	virtual Sequence< Type > SAL_CALL getTypes() throw (RuntimeException);

	// pw3270 implementation - Main
	virtual OUString SAL_CALL getVersion() throw (RuntimeException);
	virtual OUString SAL_CALL getRevision() throw (RuntimeException);

	// pw3270 implementation - Connect/Disconnect
	virtual sal_Int16 SAL_CALL getConnectionState() throw (RuntimeException);
	virtual sal_Int16 SAL_CALL Connect( const OUString& hostinfo, ::sal_Int16 wait ) throw (RuntimeException);
	virtual sal_Int16 SAL_CALL Disconnect() throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL isConnected(  ) throw (::com::sun::star::uno::RuntimeException);

	// pw3270 implementation - Screen
    virtual ::rtl::OUString SAL_CALL getScreenContentAt( ::sal_Int16 row, ::sal_Int16 col, ::sal_Int16 size ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getScreenContent() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL waitForStringAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& key, ::sal_Int16 timeout ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL queryStringAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& key ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isTerminalReady(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL waitForTerminalReady( ::sal_Int16 timeout ) throw (::com::sun::star::uno::RuntimeException);

	// pw3270 implementation - Actions
    virtual ::sal_Int16 SAL_CALL sendEnterKey() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL setStringAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& str ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL sendPFKey( ::sal_Int16 key ) throw (::com::sun::star::uno::RuntimeException);

private:

	void network_loop(void);
	static void start_thread(uno_impl *obj);

	rtl_TextEncoding getEncoding()
	{
		return RTL_TEXTENCODING_ISO_8859_1;
	}

	void yeld()
	{
		osl_yieldThread();
	}

	void wait()
	{
		osl_yieldThread();
	}

	oslThread	hThread;
	char		*hostinfo;

//	sal_Int32 m_nRefCount;

};

};

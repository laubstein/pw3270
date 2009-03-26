
#include <stdio.h>

#ifndef WIN32
    #include <unistd.h>
#endif

#include <rtl/ustring.hxx>

#include <osl/diagnose.h>

#include <cppuhelper/bootstrap.hxx>

// generated c++ interfaces
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <br/com/bb/I3270.hpp>

#include "config.hpp"

using namespace br::com::bb;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;

using namespace ::rtl;


#ifndef Trace
	#define Trace( fmt, ... )		fprintf(stderr, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fflush(stderr)
#endif

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

int SAL_CALL main(int argc, char **argv)
{
	Trace("Calling %s","createSimpleRegistry");
	printf("%s(%d)\n",__FILE__,__LINE__);
	Reference< XSimpleRegistry > xReg = createSimpleRegistry();
	OSL_ENSURE( xReg.is(), "### cannot get service instance of \"com.sun.star.regiystry.SimpleRegistry\"!" );

//	printf("%s(%d)\n",__FILE__,__LINE__);
//	xReg->open(OUString::createFromAscii("Ooo3270.uno.rdb"), sal_False, sal_False);
	xReg->open(OUString::createFromAscii("src/Ooo3270.uno.rdb"), sal_False, sal_False);

	OSL_ENSURE( xReg->isValid(), "### cannot open test registry \"Ooo3270.uno.rdb\"!" );

	Trace("Calling %s","bootstrap_InitialComponentContext");
	Reference< XComponentContext > xContext = bootstrap_InitialComponentContext(xReg);
	OSL_ENSURE( xContext.is(), "### cannot creage intial component context!" );

	Trace("Calling %s","getServiceManager");
	Reference< XMultiComponentFactory > xMgr = xContext->getServiceManager();
	OSL_ENSURE( xMgr.is(), "### cannot get initial service manager!" );

	// register my component
	Trace("Calling %s","createInstanceWithContext");

	Reference< XImplementationRegistration > xImplReg(
		xMgr->createInstanceWithContext(OUString::createFromAscii("com.sun.star.registry.ImplementationRegistration"), xContext), UNO_QUERY);
	OSL_ENSURE( xImplReg.is(), "### cannot get service instance of \"com.sun.star.registry.ImplementationRegistration\"!" );

	if (xImplReg.is())
	{
#if defined( WIN32 )
        const char *libname = "bin\\windbg\\Ooo3270.uno.dll";
#else
        const char *libname = "bin/Debug/Ooo3270.uno.so";
#endif
        Trace("Loading %s",libname);

		xImplReg->registerImplementation(
                OUString::createFromAscii("com.sun.star.loader.SharedLibrary"), // loader for component
                OUString::createFromAscii(libname),		// component location
                Reference< XSimpleRegistry >()	        // registry omitted,
                                                        // defaulting to service manager registry used
			);

		// get an object instance
		Trace("Calling createInstanceWithContext(%s)",IMPLNAME);

		Reference< XInterface > xx ;
		xx = xMgr->createInstanceWithContext(OUString::createFromAscii(IMPLNAME), xContext);

        Trace("Instance: %p",xx);

		Reference< I3270 > srv( xx, UNO_QUERY );

		OSL_ENSURE( srv.is(), "### cannot get service instance!");

		Trace("object.is(): %d",srv.is());

		if(srv.is())
		{
			// Wait for commands
			char	buffer[80];
			OString	str;

			Trace("getConnectionState: %d", srv->getConnectionState());
			Trace("Connect(): %d" , srv->Connect(OUString::createFromAscii("L:3270.df.bb:9023"),1));

			//str	=  OUStringToOString( srv->getScreenContentAt(20,39,5),RTL_TEXTENCODING_UTF8);
			//Trace("ContentsAt(20,39): \"%s\"",str.pData->buffer);
			Trace("waitForStringAt(SISBB) returned %d",srv->waitForStringAt(20,39,OUString::createFromAscii("SISBB"),20));
			Trace("sendEnterKey() returned %d",srv->sendEnterKey());
			Trace("waitForStringAt(Senha) returned %d",srv->waitForStringAt(14,2,OUString::createFromAscii("Senha"),20));
			Trace("setStringAt returned %d",srv->setStringAt(13,21,OUString::createFromAscii("c1103788")));

			str	=  OUStringToOString( srv->getScreenContent(),RTL_TEXTENCODING_UTF8);
			Trace("Entire screen:\n%s\n",str.pData->buffer);

			printf("Enter to exit...\n");
			fgets(buffer,80,stdin);

			Trace("Disconnect(): %d" , srv->Disconnect());

		}
	}


	Reference< XComponent >::query( xContext )->dispose();
	return 0;
}

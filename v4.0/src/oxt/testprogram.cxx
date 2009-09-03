
#include "globals.hpp"

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

int SAL_CALL main(int argc, char **argv)
{
	Trace("Calling %s","createSimpleRegistry");
	printf("%s(%d)\n",__FILE__,__LINE__);
	Reference< XSimpleRegistry > xReg = createSimpleRegistry();
	OSL_ENSURE( xReg.is(), "### cannot get service instance of \"com.sun.star.regiystry.SimpleRegistry\"!" );

	printf("%s(%d)\n",__FILE__,__LINE__);
	xReg->open(OUString::createFromAscii("uno/pw3270.uno.rdb"), sal_False, sal_False);

	OSL_ENSURE( xReg->isValid(), "### cannot open test registry \"pw3270.uno.rdb\"!" );

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
        const char *libname = "bin\\Debug\\pw3270.uno.dll";
#else
        const char *libname = "bin/Debug/pw3270.uno.so";
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

        Trace("Instance: %p",&xx);

		Reference< pw3270intf > srv( xx, UNO_QUERY );

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

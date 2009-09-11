/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como testprogram.cxx e possui 1088 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

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
			OString	str;
			char	buffer[80];
			Trace("getConnectionState: %d", srv->getConnectionState());

			str = OUStringToOString( srv->getVersion(),RTL_TEXTENCODING_UTF8);
			Trace("Version:\t%s",str.pData->buffer);

			str = OUStringToOString( srv->getRevision(),RTL_TEXTENCODING_UTF8);
			Trace("Revision:\t%s",str.pData->buffer);

			Trace("Connect(): %d" , srv->Connect(OUString::createFromAscii("L:3270.df.bb:9023"),10));

			sleep(5);

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
			sleep(5);

		}
	}


	Reference< XComponent >::query( xContext )->dispose();
	return 0;
}

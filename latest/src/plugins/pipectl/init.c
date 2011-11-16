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
 * Este programa está nomeado como init.c e possui - linhas de código.
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

 #include "pipectl.h"

/*---[ Globals ]----------------------------------------------------------------------------------*/

 EXPORT_PW3270_PLUGIN_INFORMATION("Pipe controller");

/*---[ Statics ]----------------------------------------------------------------------------------*/

/*---[ Implement ]--------------------------------------------------------------------------------*/

 void popup_lasterror(const gchar *fmt, ...)
 {
 	char 		buffer[4096];
	va_list		arg_ptr;
	int			sz;
 	DWORD 		errcode = GetLastError();
 	char		*ptr;
    LPVOID 		lpMsgBuf = 0;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	for(ptr=lpMsgBuf;*ptr && *ptr != '\n';ptr++);
	*ptr = 0;

	va_start(arg_ptr, fmt);
	vsnprintf(buffer,4095,fmt,arg_ptr);
	va_end(arg_ptr);

	sz = strlen(buffer);
	snprintf(buffer+sz,4096-sz,": %s\n(rc=%d)",lpMsgBuf,(int) errcode);

	printf("%s\n",buffer);

#ifdef DEBUG
	fprintf(stderr,"%s\n",buffer);
	fflush(stderr);
#endif

	LocalFree(lpMsgBuf);
 }

 PW3270_PLUGIN_ENTRY void pw3270_plugin_start(GtkWidget *topwindow)
 {
	// Create processing thread
	static const LPTSTR	lpszRequest	= TEXT("\\\\.\\pipe\\" PACKAGE_NAME );
	HANDLE					hPipe;


	hPipe = CreateNamedPipe(	lpszRequest,								// pipe name
								PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,	// read access
								PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE,	// pipe modes
								1,											// max. instances
								PIPE_BUFFER_LENGTH,							// output buffer size
								PIPE_BUFFER_LENGTH,							// input buffer size
								0,											// client time-out
								NULL);										// default security attribute

	Trace("%s: %p",(char *) lpszRequest, hPipe);

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		popup_lasterror("Falha ao criar pipe %s",lpszRequest);
		return;
	}

	init_source_pipe(hPipe);

 }

 PW3270_PLUGIN_ENTRY void pw3270_plugin_stop(GtkWidget *topwindow)
 {

 }


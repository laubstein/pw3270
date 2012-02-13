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

 #define PIPE_BUFFER_LENGTH 512

/*---[ Globals ]----------------------------------------------------------------------------------*/

 EXPORT_PW3270_PLUGIN_INFORMATION("Pipe controller");

/*---[ Statics ]----------------------------------------------------------------------------------*/

 static HANDLE 	  hPipe			= INVALID_HANDLE_VALUE;
 static GThread 	* hThread		= NULL;
 static gboolean	  enabled		= TRUE;
 static OVERLAPPED	  overlap_data;

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static void popup_lasterror(const gchar *fmt, ...)
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

 static void wait_for_client(void)
 {
	memset(&overlap_data,0,sizeof(overlap_data));

	if(ConnectNamedPipe(hPipe, &overlap_data))
	{
		popup_lasterror("%s", _( "Can't connect pipe" ) );
		enabled = FALSE;
	}
	else
	{
		switch (GetLastError())
		{
		// The overlapped connection in progress.
		case ERROR_IO_PENDING:
			Trace("Connection pending in pipe %p",hPipe);
			break;

		// Client is already connected, so signal an event.
		case ERROR_PIPE_CONNECTED:
			Trace("Client connected in pipe %p",hPipe);
			break;

		// If an error occurs during the connect operation...
		default:
			popup_lasterror("%s", _( "ConnectNamedPipe failed" ) );
			enabled = FALSE;
		}

	}
 }

 static gpointer pipe_processor_thread(gpointer dunno)
 {
 	static enum _state
 	{
 		PIPE_STATE_CONNECTING,
 		PIPE_STATE_WRITING,
 		PIPE_STATE_READING
 	} state = PIPE_STATE_CONNECTING;

	char buffer[4096];

 	Trace("%s starts",__FUNCTION__);
	wait_for_client();

	while(enabled)
	{
		if(WaitForSingleObject(hPipe,100) == WAIT_OBJECT_0)
		{
			DWORD cbRet, cbRead;

			if(GetOverlappedResult(hPipe,&overlap_data,&cbRet,FALSE))
			{
				// Data received
				switch(state)
				{
				case PIPE_STATE_CONNECTING:
					Trace("Connection received on pipe %p",hPipe);
					state = PIPE_STATE_READING;
					break;

				case PIPE_STATE_READING:
					if(ReadFile(hPipe,buffer, 4095, &cbRead,&overlap_data))
					{
						gchar *response;
						*(buffer+cbRead) = 0;
						response = run_commands(buffer);
						if(response)
						{
							WriteFile(hPipe,response,strlen(response),&cbRead,&overlap_data);
							g_free(response);
						}
						else
						{
							static const char *msg = "error";
							WriteFile(hPipe,msg,strlen(msg),&cbRead,&overlap_data);
						}
						state = PIPE_STATE_WRITING;
					}
					break;

				case PIPE_STATE_WRITING:
					DisconnectNamedPipe(hPipe);
					wait_for_client();
					state = PIPE_STATE_CONNECTING;
					break;

				default:
					Trace("Invalid state %d on %p",(int) state,hPipe);
					DisconnectNamedPipe(hPipe);
					wait_for_client();
					state = PIPE_STATE_CONNECTING;
				}
			}
			else
			{
				ULONG rc = GetLastError();

				Trace("GetOverlappedResult: %d (%s)",(int) rc, rc == ERROR_BROKEN_PIPE ? "Broken pipe" : "Unexpected");

				if(rc == ERROR_BROKEN_PIPE)
				{
					Trace("Pipe %p disconnected",hPipe);
					DisconnectNamedPipe(hPipe);
					wait_for_client();
					state = PIPE_STATE_CONNECTING;
				}
				else
				{
					enabled = FALSE;
				}
			}
		}
	}


 	hThread = NULL;
 	Trace("%s finished",__FUNCTION__);

 	return 0;
 }

 PW3270_PLUGIN_ENTRY void pw3270_plugin_start(GtkWidget *topwindow)
 {
	// Create processing thread
	static const LPTSTR lpszRequest	= TEXT("\\\\.\\pipe\\" PACKAGE_NAME );

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

	hThread = g_thread_create(pipe_processor_thread,0,TRUE,0);
 }

 PW3270_PLUGIN_ENTRY void pw3270_plugin_stop(GtkWidget *topwindow)
 {
	enabled = FALSE;

	if(hThread)
		g_thread_join(hThread);

	if(	hPipe != INVALID_HANDLE_VALUE)
	{
		Trace("Pipe %p closed",hPipe);
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
 }


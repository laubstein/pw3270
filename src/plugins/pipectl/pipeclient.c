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
 * Este programa está nomeado como pipeclient.c e possui - linhas de código.
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

 #include <windows.h>
 #include <stdio.h>


/*---[ Statics ]----------------------------------------------------------------------------------*/

 static HANDLE 	  hPipe		= INVALID_HANDLE_VALUE;

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static int show_lasterror(const char *fmt, ...)
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
	snprintf(buffer+sz,4096-sz,"\n%s\n(rc=%d)",(char *) lpMsgBuf,(int) errcode);

	fprintf(stderr,"%s\n",buffer);

	LocalFree(lpMsgBuf);

	return errcode;
 }

 static void run_query(HANDLE hPipe, const char *query)
 {
 	static char buffer[32768];
 	DWORD cbSize = 0;

//	printf("query(\"%s\")....\n",query);

	// http://msdn.microsoft.com/en-us/library/windows/desktop/aa365592(v=vs.85).aspx
/*
	if(!WriteFile(hPipe,query,strlen(query),&cbSize,NULL))
	{
		show_lasterror("Can´t write %s",query);
		return;
	}
*/

	if(!TransactNamedPipe(hPipe,(LPVOID) query,strlen(query),buffer,32768,&cbSize,NULL))
	{
		show_lasterror("Can't send message \"%s\"",query);
		return;
	}

	if(cbSize < 1)
	{
		printf("Empty response to message \"%s\"\n",query);
		return;
	}

	*(buffer+((int) cbSize)) = 0;

	printf("%s\n",buffer);
 }

 int main(int numpar, char *param[])
 {
 	char buffer[0x0100];

	static DWORD dwMode = PIPE_READMODE_MESSAGE;

	static const LPTSTR lpszPipeName	= TEXT( "\\\\.\\pipe\\pw3270" );

	printf("Waiting %s ....\r",lpszPipeName);
	while(!WaitNamedPipe(lpszPipeName,10000))
	{
		printf("Waiting %s ....\r",lpszPipeName);
		Sleep(1);
	}

	printf("Connecting %s ....\n",lpszPipeName);
	hPipe = CreateFile(	lpszPipeName,   						// pipe name
						GENERIC_WRITE|GENERIC_READ,			// Read/Write access
						0,              					// no sharing
						NULL,           					// default security attributes
						OPEN_EXISTING,  					// opens existing pipe
						0,									// Attributes
						NULL);          					// no template file

	if(hPipe == INVALID_HANDLE_VALUE)
		return show_lasterror("CreateFile(%s)",lpszPipeName);

	printf("Connected %s ....\n",lpszPipeName);

	if(!SetNamedPipeHandleState(hPipe,&dwMode,NULL,NULL))
		return show_lasterror("SetNamedPipeHandleState(%s)",lpszPipeName);

/*
	while(--numpar > 0)
	{
		param++;
		run_query(hPipe, *param);
	}
*/
	while(fgets(buffer,0x0FF,stdin))
	{
		char *ptr = strchr(buffer,'\n');
		if(ptr)
			*ptr = 0;

		if(*buffer)
			run_query(hPipe,buffer);
	}
	printf("%s\n","Disconnected....");
	CloseHandle(hPipe);

	return 0;
 }


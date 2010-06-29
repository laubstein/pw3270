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
 * Este programa está nomeado como pw3270_jni.c e possui - linhas de código.
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


#include <jni.h>
#include <time.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <lib3270.h>
#include <lib3270/config.h>

#ifndef WIN32
	#include <sys/types.h>
	#include <unistd.h>
#endif

/*---[ Statics ]-------------------------------------------------------------------------------------------*/

 static H3270 *hSession = NULL;

/*---[ Prototipes & Defines ]------------------------------------------------------------------------------*/

#if defined WIN32

	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd);

	static int lib3270_jni_init(void);
	static int lib3270_jni_fini(void);

#else

	int lib3270_jni_init(void) __attribute__((constructor));
	int lib3270_jni_fini(void) __attribute__((destructor));

#endif

#define CHECK_FOR_TERMINAL_STATUS	if(!PCONNECTED) \
										return ENOTCONN; \
									else if(query_3270_terminal_status() != STATUS_CODE_BLANK) \
										return EBUSY;


/*---[ Implement ]-----------------------------------------------------------------------------------------*/

#if defined WIN32

	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd)
	{
		switch(dwcallpurpose)
		{
		case DLL_PROCESS_ATTACH:
			lib3270_jni_init();
			break;

		case DLL_PROCESS_DETACH:
			lib3270_jni_fini();
			break;
		}

		return TRUE;
	}

#endif

int lib3270_jni_init(void)
{
#ifdef WIN32
	Trace("Running %s on %s %s %s",__FUNCTION__,__FILE__,__DATE__,__TIME__);
#else
	Trace("Running %s on %s %s %s (pid: %d)",__FUNCTION__,__FILE__,__DATE__,__TIME__,getpid());
#endif

	hSession = new_3270_session();
    Trace("%s ends",__FUNCTION__);
	return 0;
}

int lib3270_jni_fini(void)
{
	Trace("Running %s",__FUNCTION__);
	return 0;
}

/*---[ JNI Calls ]-----------------------------------------------------------------------------------------*/


JNIEXPORT jstring JNICALL Java_pw3270_terminal_getVersion(JNIEnv *env, jobject obj)
{
	return (*env)->NewStringUTF(env, PACKAGE_VERSION);
}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_getRevision(JNIEnv *env, jobject obj)
{
	return (*env)->NewStringUTF(env, PACKAGE_REVISION);
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_Connect(JNIEnv *env, jobject obj, jstring j_hostinfo, jint j_timeout)
{
	int rc;
	const char *hostinfo = (*env)->GetStringUTFChars(env, j_hostinfo, 0);

	if(QueryCstate() != NOT_CONNECTED)
		return EINVAL;

	rc = host_connect(hostinfo,1);

	(*env)->ReleaseStringUTFChars(env, j_hostinfo, hostinfo);

	if(rc)
		return rc;

	if(j_timeout > 0)
	{
		time_t tm = time(0) + ((int) j_timeout);

		while(time(0) < tm)
		{
			if( (CONNECTED) && query_3270_terminal_status() == STATUS_CODE_BLANK)
				return 0;

			RunPendingEvents(1);
		}
	}

	return ETIMEDOUT;
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_Disconnect(JNIEnv *env, jobject obj)
{
	if(QueryCstate() <= NOT_CONNECTED)
		return EINVAL;

	host_disconnect(hSession,0);

	return 0;
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_getConnectionState(JNIEnv *env, jobject obj)
{
	return (jint) QueryCstate();
}

JNIEXPORT jboolean JNICALL Java_pw3270_terminal_isConnected(JNIEnv *env, jobject obj)
{
	return (CONNECTED) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_pw3270_terminal_isTerminalReady(JNIEnv *env, jobject obj)
{
	if(!CONNECTED || query_3270_terminal_status() != STATUS_CODE_BLANK)
		return JNI_FALSE;
	return JNI_TRUE;
}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_getScreenContentAt(JNIEnv *env, jobject obj, jint row, jint col, jint size)
{
	jstring ret;

	int start, rows, cols;
	char *buffer;
	row--;
	col--;

	screen_size(&rows,&cols);

	if(row < 0 || row > rows || col < 0 || col > cols || size < 1)
		return (*env)->NewStringUTF(env,"");

	start = ((row) * cols) + col;

	buffer = (char *) malloc(size+1);

	screen_read(buffer, start, size);
	*(buffer+size) = 0;

	ret = (*env)->NewStringUTF(env,buffer);

	free(buffer);

	return ret;
}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_getScreenContent(JNIEnv *env, jobject obj)
{
	jstring ret;
	int sz, rows, cols, row;
	char *buffer;
	char *ptr;
	int  pos = 0;

	screen_size(&rows,&cols);

	sz = rows*(cols+1)+1;
	ptr = buffer = (char *) malloc(sz);
	memset(buffer,0,sz);

	for(row = 0; row < rows;row++)
	{
		screen_read(ptr,pos,cols);
		pos += cols;
		ptr += cols;
		*(ptr++) = '\n';
	}
	*ptr = 0;

	ret = (*env)->NewStringUTF(env,buffer);

	free(buffer);

	return ret;
}

JNIEXPORT jboolean JNICALL Java_pw3270_terminal_queryStringAt(JNIEnv *env, jobject obj, jint row, jint col, jstring j_key)
{
	const char	*key = (*env)->GetStringUTFChars(env, j_key, 0);

	int 	sz, rows, cols, start;
	char 	*buffer;
	jboolean	rc;

	screen_size(&rows,&cols);

	row--;
	col--;
	start = (row * cols) + col;

	sz = strlen(key);
	buffer = (char *) malloc(sz+1);

	screen_read(buffer,start,sz);

	rc = (strncmp(buffer,key,sz) == 0 ? JNI_TRUE : JNI_FALSE);

	(*env)->ReleaseStringUTFChars(env, j_key, key);

	free(buffer);

	return rc;
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_sendEnterKey(JNIEnv *env, jobject obj)
{
	CHECK_FOR_TERMINAL_STATUS
	return action_Enter();
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_setStringAt(JNIEnv *env, jobject obj, jint row, jint col, jstring j_str)
{
	const char *str;

	CHECK_FOR_TERMINAL_STATUS

	if(row < 1 || col < 1)
		return EINVAL;

	row--;
	col--;

	cursor_set_addr((row * ctlr_get_cols()) + col);

	str = (*env)->GetStringUTFChars(env, j_str, 0);
	Input_String((unsigned char *) str);
	(*env)->ReleaseStringUTFChars(env, j_str, str);

	return 0;
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_sendPFKey(JNIEnv *env, jobject obj, jint key)
{
	CHECK_FOR_TERMINAL_STATUS

	return action_PFKey(key);
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_waitForTerminalReady(JNIEnv *env, jobject obj, jint timeout)
{
	time_t end = time(0) + ((int) timeout);

	while(time(0) < end)
	{
		if(!CONNECTED)
		{
			return ENOTCONN;
		}
		else if(query_3270_terminal_status() == STATUS_CODE_BLANK)
		{
			return 0;
		}

		RunPendingEvents(1);
	}

	return ETIMEDOUT;
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_waitForStringAt(JNIEnv *env, jobject obj, jint row, jint col, jstring j_key, jint timeout)
{
	const char	*key = (*env)->GetStringUTFChars(env, j_key, 0);

	int		rc = ETIMEDOUT;
	int 	end = time(0)+timeout;
	int 	sz, rows, cols, start;
	char 	*buffer;
	int 	last = -1;

	screen_size(&rows,&cols);

	row--;
	col--;
	start = (row * cols) + col;

	sz = strlen(key);
	buffer = (char *) malloc(sz+1);

	while( (rc == ETIMEDOUT) && (time(0) <= end) )
	{
		if(!CONNECTED)
		{
			// Disconnected, ret
			rc = ENOTCONN;
		}
		else if(query_3270_terminal_status() == STATUS_CODE_BLANK)
		{
			// Screen contents are ok. Check.
			if(last != query_screen_change_counter())
			{
				last = query_screen_change_counter();
				screen_read(buffer,start,sz);
				if(!strncmp(buffer,key,sz))
				{
					(*env)->ReleaseStringUTFChars(env, j_key, key);
					free(buffer);
					return 0;
				}
			}
		}

		RunPendingEvents(1);
	}

	(*env)->ReleaseStringUTFChars(env, j_key, key);
	free(buffer);

	return rc;
}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_getEncoding(JNIEnv *env, jobject obj)
{
	return (*env)->NewStringUTF(env,"ISO-8859-1");
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_wait(JNIEnv *env, jobject obj, jint seconds)
{
	int end = time(0)+seconds;

	while((time(0) <= end) )
	{
		if(!CONNECTED)
			return ENOTCONN;
		RunPendingEvents(1);
	}

	return 0;
}

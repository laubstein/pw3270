/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
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
 * Este programa está nomeado como plugins.h e possui 45 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#ifndef PLUGINS_H_INCLUDED

	#define PLUGINS_H_INCLUDED 1

	#include <gtk/gtk.h>

	#if defined(_WIN32)
		#include <windows.h>

		#define PW3270_PLUGIN_ENTRY	__declspec (dllexport)
		#define LOCAL_EXTERN		extern

	#else
		#include <stdarg.h>

		// http://gcc.gnu.org/wiki/Visibility
		#if defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
				#define LOCAL_EXTERN __hidden extern
		#elif defined (__GNUC__) && defined (HAVE_GNUC_VISIBILITY)
				#define LOCAL_EXTERN __attribute__((visibility("hidden"))) extern
		#else
				#define LOCAL_EXTERN extern
		#endif

		#define PW3270_PLUGIN_ENTRY extern

	#endif


	PW3270_PLUGIN_ENTRY void pw3270_plugin_start(GtkWidget *topwindow, const gchar *script);
	PW3270_PLUGIN_ENTRY void pw3270_plugin_stop(GtkWidget *topwindow);
	PW3270_PLUGIN_ENTRY void pw3270_plugin_update_luname(GtkWidget *topwindow, const gchar *luname);
	PW3270_PLUGIN_ENTRY void pw3270_plugin_update_hostname(GtkWidget *topwindow, const gchar *servername);

#endif // PLUGINS_H_INCLUDED


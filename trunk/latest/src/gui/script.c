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
 * Este programa está nomeado como script.c e possui - linhas de código.
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


 #include <errno.h>
 #include <glib.h>
 #include <lib3270/config.h>

 #include "gui.h"

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 void script_interpreter( const gchar *script_type, const gchar *script_name, const gchar *script_text, int argc, gchar **argv )
 {
	void (*interpret)( const gchar *script_name, const gchar *script_text, int argc, gchar **argv );

	if(script_type)
	{
		if(get_symbol_by_name(NULL,(gpointer) &interpret,"pw3270_script_interpreter_%s",script_type))
		{
			interpret(script_name,script_text,argc,argv);
		}
		else
		{
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_WARNING,
												GTK_BUTTONS_OK_CANCEL,
												_(  "Can't find a valid interpreter for \"%s\"" ), script_type);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't run script" ) );

			if(script_name)
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), _( "The script \"%s\" can't be started.\nDo you have the required plugin installed?" ),script_name);

			if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
				gtk_main_quit();
			gtk_widget_destroy(dialog);

		}
		return;
	}

	// No type
	#warning Implement "no type" scripts

 }

/**
 * Call a list of scripts.
 *
 * @param scripts List of scripts to run in the format script1(arg1,arg2);script2(arg);...
 *
 */
 void run_script_list( const gchar *scripts )
 {
	// Call scripts
	int str;
	gchar **script = g_strsplit(scripts,";",-1);

	for(str=0;script[str];str++)
	{
		gchar *begin_arg = g_strstr_len(script[str],-1,"(");

		if(begin_arg)
		{
			gint	   argc;
			gchar	** argv  = 0;
			GError	 * error = NULL;
			gchar	*  end_arg = g_strstr_len(begin_arg,-1,")");

			*(begin_arg++) = 0;

			if(end_arg)
				*(end_arg) = 0;

			Trace("Calling script %s(%s)",script[str],begin_arg);

			if(!g_shell_parse_argv(begin_arg,&argc,&argv,&error))
			{
				g_warning("%s",error->message);
				g_error_free(error);
			}
			else
			{
				gchar *type = g_strrstr(script[str],".");
				if(type)
					type++;
				script_interpreter(type,script[str],NULL,argc,argv);
				g_strfreev(argv);
			}
		}
		else
		{
			gchar *type = g_strrstr(script[str],".");
			if(type)
				type++;
			script_interpreter(type,script[str],NULL,0,NULL);
		}

	}
	g_strfreev(script);

 }

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
 * Este programa está nomeado como main.c e possui 351 linhas de código.
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


 #include "rx3270.h"

/*---[ Rexx entry points ]------------------------------------------------------------------------*/

 const EXPORTED_CALL_ENTRY rexx_exported_calls[] =
 {
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270SetWidgetData				),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270GetWidgetData				),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270Popup 				   	),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270Actions				   	),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270Quit						),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270SetVisible				),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270Popup						),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270runDialog					),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270DestroyDialog				),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270FileChooserNew			),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270FileChooserGetFilename	),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270SetDialogTitle			),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270MessageDialogNew			),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270ProgressDialogNew			),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270ProgressDialogSetCurrent	),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270ProgressDialogSetTotal	),
	EXPORTED_PLUGIN_REXX_ENTRY( rx3270SetWindowDefaultSize		),

/*
	EXPORTED_STANDALONE_REXX_ENTRY( rx3270LoadFuncs				),
	EXPORTED_STANDALONE_REXX_ENTRY( rx3270Init				    )
*/

	EXPORTED_COMMON_REXX_ENTRY( rx3270Version				    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270Connect				    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270Disconnect				),
	EXPORTED_COMMON_REXX_ENTRY( rx3270QueryScreenAttribute    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270ToggleON			    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270ToggleOFF			    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270Toggle				    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270Toggled				    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270Log					    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270QueryCState			    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270WaitForEvents		    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270Sleep				    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270InputString			    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270FindFieldAttribute	    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270FindFieldLength		    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270GetCursorPosition	    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270SetCursorPosition		),
	EXPORTED_COMMON_REXX_ENTRY( rx3270ReadScreen			    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270SendPFKey			    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270WaitForChanges		    ),
	EXPORTED_COMMON_REXX_ENTRY( rx3270SendENTERKey			),
	EXPORTED_COMMON_REXX_ENTRY( rx3270WaitForString			),
	EXPORTED_COMMON_REXX_ENTRY( rx3270WaitForStringAt			),
	EXPORTED_COMMON_REXX_ENTRY( rx3270IsTerminalReady			),
	EXPORTED_COMMON_REXX_ENTRY( rx3270WaitForTerminalReady	),

 };

 int rexx_common_calls_count		= (sizeof(rexx_common_calls) / sizeof(EXPORTED_CALL_ENTRY));
 int rexx_standalone_calls_count 	= (sizeof(rexx_standalone_calls) / sizeof(EXPORTED_CALL_ENTRY));
 int rexx_plugin_calls_count		= (sizeof(rexx_plugin_calls) / sizeof(EXPORTED_CALL_ENTRY));

/*---[ Implement ]--------------------------------------------------------------------------------*/

 ULONG RetValue(PRXSTRING Retstr, int value)
 {
	g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%d",value);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 ULONG RetString(PRXSTRING Retstr, const char *value)
 {
 	if(!value)
 	{
 		strcpy(Retstr->strptr,"");
 	}
 	else if(strlen(value) > (RXAUTOBUFLEN-1))
 	{
 		Retstr->strptr = RexxAllocateMemory(strlen(value)+1);
 		strcpy(Retstr->strptr,value);
 	}
 	else
 	{
		g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%s",value);
 	}

    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }


ULONG RetPointer(PRXSTRING Retstr, gpointer value)
{
	g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%p",value);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
}

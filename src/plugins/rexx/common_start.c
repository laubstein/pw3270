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

/*---[ Globals ]----------------------------------------------------------------------------------*/

/*---[ Implement ]--------------------------------------------------------------------------------*/

 RexxReturnCode RetValue(PRXSTRING Retstr, int value)
 {
	g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%d",value);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 RexxReturnCode RetString(PRXSTRING Retstr, const char *value)
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


RexxReturnCode RetPointer(PRXSTRING Retstr, gpointer value)
{
	g_snprintf(Retstr->strptr,RXAUTOBUFLEN-1,"%p",value);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
}




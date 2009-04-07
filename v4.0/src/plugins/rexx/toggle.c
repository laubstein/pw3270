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
 * Este programa está nomeado como toggle.c e possui 104 linhas de código.
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

/*---[ Implement ]--------------------------------------------------------------------------------*/

 ULONG APIENTRY rx3270ToggleON(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			set_toggle(toggle,TRUE);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270ToggleOFF(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			set_toggle(toggle,FALSE);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Toggle(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			do_toggle(toggle);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Toggled(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	int toggle;

	if(Argc != 1)
		return RXFUNC_BADCALL;

	toggle = get_toggle_by_name(Argv[0].strptr);

	if(toggle < 0)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,Toggled(toggle));
 }



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
 * Este programa está nomeado como macros.c e possui - linhas de código.
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

 #include <stdio.h>
 #include <lib3270/api.h>
 #include <lib3270/macros.h>

 #define LIB3270_MACRO_ENTRY( name )  { #name, lib3270_macro_ ## name }

 static const LIB3270_MACRO_LIST macro_list[] =
 {
 	LIB3270_MACRO_ENTRY( encoding	),
 	LIB3270_MACRO_ENTRY( get		),
 	LIB3270_MACRO_ENTRY( set		),
 	LIB3270_MACRO_ENTRY( status		),

 	{NULL, NULL}
 };

 LIB3270_EXPORT const LIB3270_MACRO_LIST * get_3270_calls(void)
 {
 	return macro_list;
 }

 static char * value_as_string(int val)
 {
 	char buffer[10];
 	snprintf(buffer,9,"%d",val);
 	return strdup(buffer);
 }

 LIB3270_MACRO( encoding )
 {
 	return strdup("ISO-8859-1");
 }

 LIB3270_MACRO( get )
 {
	int start, qtd, rows, cols, row, col;
	char *buffer = NULL;

	switch(argc)
	{
	case 1:	// Get entire screen
		get_3270_terminal_size(hSession,&rows,&cols);
		qtd = (rows*(cols+1)+1);
		buffer = malloc(qtd+2);

		Trace("Screen buffer size: %d (%dx%d)",qtd,rows,cols);

		memset(buffer,0,qtd+1);
		start = qtd = 0;
		for(row = 0; row < rows;row++)
		{
			screen_read(buffer+qtd,start,cols);
			qtd += cols;
			start += cols;
			buffer[qtd++] = '\n';
		}
		buffer[qtd] = 0;

		Trace("Bytes read: %d",qtd);
		return buffer;

	case 2:	// Just size, get current cursor position
		start	= 0;
		qtd 	= atoi(argv[1]);
		break;

	case 3:	// Use start position
		start	= atoi(argv[1]);
		qtd 	= atoi(argv[2]);
		break;

	case 4:	// Get start position from row/col
		get_3270_terminal_size(hSession,&rows,&cols);

		row = atoi(argv[1])-1;
		col = atoi(argv[2])-1;

		if(row < 0 || row > rows || col < 0 || col > cols)
		{
			errno = EINVAL;
			return NULL;
		}

		start 	= (row * cols) + col;
		qtd 	= atoi(argv[3]);
		break;

	default:
		errno = EINVAL;
		return NULL;
	}

	if(qtd < 1)
	{
		errno = EINVAL;
		return NULL;
	}

	buffer = malloc(qtd+1);
	screen_read(buffer, start, qtd);

	return buffer;
 }

 LIB3270_MACRO( set )
 {
 	const char *str = NULL;

	if(query_3270_terminal_status() != STATUS_CODE_BLANK)
	{
		errno = EBUSY;
		return NULL;
	}

	switch(argc)
	{
	case 1:
		lib3270_enter();
		break;

    case 2:
		str = argv[1];
		break;

	case 3:
        cursor_set_addr(atoi(argv[1]));
		str = argv[2];
        break;

    case 4:
        cursor_set_addr((atoi(argv[1])-1) * ctlr_get_cols() + (atoi(argv[2])-1));
		str = argv[3];
        break;

	default:
		errno = EINVAL;
		return NULL;
	}

	if(str)
		Input_String((unsigned char *) str);

	return value_as_string(query_3270_terminal_status());
 }

 LIB3270_MACRO( status )
 {
 	Trace("Status: %d",query_3270_terminal_status());
	return value_as_string(query_3270_terminal_status());
 }

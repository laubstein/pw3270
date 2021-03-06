/*
 * "Software PW3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
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
 * Este programa está nomeado como actions.h e possui - linhas de código.
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

	#define DECLARE_LIB3270_ACTION( name )  				LIB3270_EXPORT int lib3270_ ## name (void);
	#define LIB3270_ACTION( name )  						LIB3270_EXPORT int lib3270_ ## name (void)

	// Clear actions - When called the selected area is cleared
	#define DECLARE_LIB3270_CLEAR_SELECTION_ACTION( name )  LIB3270_EXPORT int lib3270_ ## name (void);
	#define LIB3270_CLEAR_SELECTION_ACTION( name )			LIB3270_EXPORT int lib3270_ ## name (void)

	// Single key actions
	#define DECLARE_LIB3270_KEY_ACTION( name )				LIB3270_EXPORT int lib3270_ ## name (void);
	#define LIB3270_KEY_ACTION( name )						LIB3270_EXPORT int lib3270_ ## name (void)

	// Cursor actions
	#define DECLARE_LIB3270_CURSOR_ACTION( name )			LIB3270_EXPORT int lib3270_cursor_ ## name (void);
	#define LIB3270_CURSOR_ACTION( name )					LIB3270_EXPORT int lib3270_cursor_ ## name (void)

	// PF & PA key actions
	#define DECLARE_LIB3270_FKEY_ACTION( name )				LIB3270_EXPORT int lib3270_ ## name (int key);
	#define LIB3270_FKEY_ACTION( name )						LIB3270_EXPORT int lib3270_ ## name (int key)


	// Load action table entries
	#include <lib3270/action_table.h>


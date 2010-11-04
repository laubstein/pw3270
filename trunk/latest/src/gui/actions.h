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
 * Este programa está nomeado como actions.h e possui - linhas de código.
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

#ifndef ACTIONS_H_INCLUDED

	#define ACTIONS_H_INCLUDED 2

	/* Macros */
	#define DECLARE_PW3270_ACTION( name, attr )	LOCAL_EXTERN void action_ ## name (GtkAction *action);
	#define PW3270_ACTION( name )				LOCAL_EXTERN void action_ ## name (GtkAction *action)

	/* Globals */
	LOCAL_EXTERN GtkActionGroup	* action_group[ACTION_GROUP_MAX+1];
	LOCAL_EXTERN const gchar	* action_group_name[ACTION_GROUP_MAX];
	LOCAL_EXTERN GtkAction		* action_by_id[ACTION_ID_MAX];

	#define ACTION_SCROLL_MAX 4
	LOCAL_EXTERN GtkAction 		* action_scroll[ACTION_SCROLL_MAX];

	/* API Prototipes */
	LOCAL_EXTERN void init_actions(void);
	LOCAL_EXTERN void action_group_set_sensitive(ACTION_GROUP_ID id, gboolean status);

	/* Deprecated */
	LOCAL_EXTERN void			  update_3270_toggle_action(int toggle, int value) __attribute__ ((deprecated));
	LOCAL_EXTERN GtkAction 		* get_action_by_name(const gchar *name) __attribute__ ((deprecated));


	/* Actions */
	#include "action_table.h"

#endif // ACTION_H_INCLUDED

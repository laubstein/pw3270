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

	#define ACTIONS_H_INCLUDED

	/* Macros */
	#define PW3270_ACTION( name )				LOCAL_EXTERN void action_ ## name (GtkAction *action)
	#define DECLARE_PW3270_ACTION( name, attr )	LOCAL_EXTERN void action_ ## name (GtkAction *action)

	LOCAL_EXTERN void			  init_actions(void);
	LOCAL_EXTERN GCallback 		  get_action_callback_by_name(const gchar *name);
	LOCAL_EXTERN GtkAction 		* get_action_by_name(const gchar *name);
	LOCAL_EXTERN void			  update_3270_toggle_action(int toggle, int value);

	LOCAL_EXTERN void			  action_group_set_sensitive(ACTION_GROUP_ID id, gboolean status);
	LOCAL_EXTERN void			  set_action_sensitive_by_name(const gchar *name, gboolean sensitive);


	LOCAL_EXTERN void	action_SelectAll(GtkWidget *w, gpointer user_data);
	LOCAL_EXTERN void	action_SelectLeft(GtkWidget *w, gpointer user_data);
	LOCAL_EXTERN void	action_SelectUp(GtkWidget *w, gpointer user_data);
	LOCAL_EXTERN void	action_SelectRight(GtkWidget *w, gpointer user_data);
	LOCAL_EXTERN void	action_SelectDown(GtkWidget *w, gpointer user_data);
	LOCAL_EXTERN void	action_SelectionLeft(GtkWidget *w, gpointer user_data);
	LOCAL_EXTERN void	action_SelectionUp(GtkWidget *w, gpointer user_data);
	LOCAL_EXTERN void	action_SelectionRight(GtkWidget *w, gpointer user_data);
	LOCAL_EXTERN void	action_SelectionDown(GtkWidget *w, gpointer user_data);

	LOCAL_EXTERN void	action_Save(void);
	LOCAL_EXTERN void	action_Restore(void);
	LOCAL_EXTERN void	action_Redraw(void);
	LOCAL_EXTERN void	action_SelectColors(void);

	LOCAL_EXTERN void	action_Paste(void);
	LOCAL_EXTERN void	action_PasteSelection(void);
	LOCAL_EXTERN void	action_PasteNext(void);
	LOCAL_EXTERN void	action_PasteTextFile(void);
	LOCAL_EXTERN void	action_Append(void);
	LOCAL_EXTERN void	action_Copy(void);
	LOCAL_EXTERN void	action_CopyAsTable(void);
	LOCAL_EXTERN void	action_CopyAsImage(void);

	LOCAL_EXTERN void	action_SelectField(void);
	LOCAL_EXTERN void	action_ClearSelection(void);

	LOCAL_EXTERN void	action_Download(void);
	LOCAL_EXTERN void	action_Upload(void);

	#include "action_table.h"

#endif // ACTIONS_H_INCLUDED

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
 * Este programa está nomeado como actions.c e possui - linhas de código.
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

 #include "gui.h"
 #include "actions.h"

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkActionGroup *action_group[ACTION_GROUP_MAX+1] = { 0 };

 const gchar *action_group_name[ACTION_GROUP_MAX] =
 {
		"default",
		"online",
		"offline",
		"selection",
		"clipboard",
		"paste",
		"filetransfer"
 };

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 void init_actions(void)
 {
	int f;

	memset(action_group,0,sizeof(action_group));

	for(f=0;f<G_N_ELEMENTS(action_group_name);f++)
		action_group[f] = gtk_action_group_new(action_group_name[f]);


	g_object_set_data(G_OBJECT(topwindow),"ActionGroups",action_group);

 }

 void update_3270_toggle_action(int toggle, int value)
 {
	#warning Implementar
 }

 void action_group_set_sensitive(ACTION_GROUP_ID id, gboolean status)
 {
	// TODO (perry#1#): Replace action_group_set_sensitive() for a macro call to gtk_action_group_set_sensitive
	gtk_action_group_set_sensitive(action_group[id],status);
 }

 void set_action_sensitive_by_name(const gchar *name,gboolean sensitive)
 {
	int p;

	for(p = 0; action_group[p]; p++)
	{
		GtkAction *act = gtk_action_group_get_action(action_group[p],name);
		if(act)
		{
			Trace("%s: %s(%s)",__FUNCTION__,name,sensitive ? "sensitive" : "insensitive");
			gtk_action_set_sensitive(act,sensitive);
			return;
		}
	}

	Trace("%s: %s isn't available",__FUNCTION__,name);

 }



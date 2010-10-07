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
 * Este programa está nomeado como uiparser.h e possui - linhas de código.
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

#ifndef UIPARSER_H_INCLUDED

 #define UIPARSER_H_INCLUDED 1

 typedef enum _ui_callback_type
 {
	UI_CALLBACK_TYPE_DEFAULT,
	UI_CALLBACK_TYPE_TOGGLE,
	UI_CALLBACK_TYPE_SCROLL,
	UI_CALLBACK_TYPE_SCRIPT,

	UI_CALLBACK_TYPE_USER

 } UI_CALLBACK_TYPE;

 typedef struct _ui_callback
 {
	UI_CALLBACK_TYPE	  type;			/**< Action type */

	const gchar			* label;		/**< Default action label */
	GCallback		 	  callback;		/**< Callback for "activated/toggled" signal */
	gpointer			  user_data;	/**< User data for callback */

 } UI_CALLBACK;

 enum KEYPAD_POSITION
 {
 	KEYPAD_POSITION_TOP,	// 0x00	0 = First	0 = Vertical
 	KEYPAD_POSITION_LEFT,	// 0x01	0 = First	1 = Horizontal
 	KEYPAD_POSITION_BOTTOM,	// 0x02 1 = last	0 = Vertical
 	KEYPAD_POSITION_RIGHT	// 0x03 1 = last	1 = Horizontal
 };
 struct ui_attributes
 {
	gchar					* label;
	gchar					* tooltip;
 	gchar					* stock_id;
	gchar					* accel;
	gchar					* text;

	guint					  key_value;
	GdkModifierType 		  key_state;
 };

 struct action_script
 {
	gboolean 	enabled;
	gchar		*text;
 };

 struct action_descriptor
 {
	struct ui_attributes	  attr;

 	GtkUIManagerItemType	  ui_type;
	UI_CALLBACK_TYPE		  callback_type;	/**< Callback type (toggle, mouse_scroll, button, etc */
	guint				  	  sub;				/**< Type dependent info */
	guint				  	  group;


	GCallback		  		  callback;			/**< Callback for "activated" signal */
	gpointer		 		  user_data;		/**< User data for callback */

	GtkAction				* action;

	struct action_script	  script;

	gchar					  name[1];
 };

 LOCAL_EXTERN const gchar	* get_xml_attribute(const gchar **names, const gchar **values, const gchar *key);

 LOCAL_EXTERN int			  get_action_info_by_name(const gchar *search, const gchar **names, const gchar **values, gchar **name, UI_CALLBACK *info);
 LOCAL_EXTERN GtkUIManager	* load_application_ui(GtkWidget *window, GtkBox *toolbar, GtkBox *vbox, GtkBox *hbox);
 LOCAL_EXTERN GtkAction		* create_action_by_descriptor(const gchar *name, struct action_descriptor *data);

#endif // UIPARSER_H_INCLUDED

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
 * Este programa está nomeado como oia.h e possui - linhas de código.
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

#ifndef OIA_H_INCLUDED

	#define OIA_H_INCLUDED

	// OIA elements
	typedef enum _OIA_ELEMENT
	{

		OIA_ELEMENT_FOUR_IN_A_SQUARE,			/**< "4" in a square */
		OIA_ELEMENT_UNDERA,						/**< "A" underlined */
		OIA_ELEMENT_CONNECTION_STATUS, 			/**< solid box if connected, "?" in a box if not */
		OIA_ELEMENT_MESSAGE_AREA,	 			/**< message area */
		OIA_ELEMENT_SSL_STATE, 					/**< SSL Status */
		OIA_ELEMENT_ALT_STATE, 					/**< Alt indication ("A" or blank) */
		OIA_ELEMENT_SHIFT_STATE,	 			/**< Shift Status */
		OIA_ELEMENT_TYPEAHEAD_INDICATOR,		/**< Typeahead indication ("T" or blank) */
		OIA_ELEMENT_INSERT_INDICATOR,	 		/**< Insert mode indication (Special symbol/"I" or blank) */
		OIA_ELEMENT_SCRIPT_INDICATOR,	 		/**< Script indication ("S" or blank) */
		OIA_ELEMENT_LUNAME,			 			/**< LU Name */
		OIA_ELEMENT_COMMAND_TIMER,		 		/**< command timing (Clock symbol and m:ss, or blank) */
		OIA_ELEMENT_COMMAND_SPINNER,			/**< Spinner */
		OIA_ELEMENT_CURSOR_POSITION,	 		/**< cursor position (rrr/ccc or blank) */

#if GTK_CHECK_VERSION(2,16,0)
		OIA_ELEMENT_CAPS_INDICATOR, 			/**< Caps indications ("A" or blank) */
#endif

//		OIA_ELEMENT_META_INDICATOR, 			/**< Meta indication ("M" or blank) */
//		OIA_ELEMENT_COMPOSE_INDICATOR,			/**< Compose indication ("C" or blank) */
//		OIA_ELEMENT_COMPOSE_FIRST, 				/**< Compose first character */
//		OIA_ELEMENT_KEYMAP_INDICATOR,			/**< Alternate keymap indication ("K" or blank) */
//		OIA_ELEMENT_REVERSE_INPUT_INDICATOR,	/**< Reverse input mode indication ("R" or blank) */
//		OIA_ELEMENT_PRINTER_INDICATOR, 			/**< Printer indication ("P" or blank) */

		OIA_ELEMENT_COUNT						/**< Number of elements in the OIA */

	} OIA_ELEMENT;


	// Globals
	LOCAL_EXTERN gboolean		oia_flag[OIA_FLAG_USER];

	LOCAL_EXTERN gboolean		oia_shift_state;
	LOCAL_EXTERN gboolean		oia_alt_state;
#if GTK_CHECK_VERSION(2,16,0)
	LOCAL_EXTERN gboolean		oia_caps_state;
#endif

	LOCAL_EXTERN STATUS_CODE	terminal_message_id;

	LOCAL_EXTERN SCRIPT_STATE 	oia_script_state;
	LOCAL_EXTERN gboolean		oia_script_blink;
	LOCAL_EXTERN gint			oia_spinner_step;

	LOCAL_EXTERN gint			oia_timer;

	// Prototips
	LOCAL_EXTERN void update_oia_element(OIA_ELEMENT el);
	LOCAL_EXTERN void draw_oia(cairo_t *cr);
	LOCAL_EXTERN void oia_set_timer(long seconds);

#endif // OIA_H_INCLUDED


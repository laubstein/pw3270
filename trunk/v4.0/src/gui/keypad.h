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
 * Este programa está nomeado como keypad.h e possui 444 linhas de código.
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

#ifdef CONFIGURABLE_KEYPAD

	enum KEYPAD_POSITION
	{
		KEYPAD_POSITION_RIGHT,
		KEYPAD_POSITION_LEFT,
		KEYPAD_POSITION_TOP,
		KEYPAD_POSITION_BOTTOM
	};

	struct keypad_button
	{
		struct keypad_button	*next;
		GtkWidget 				*widget;
	};

	struct keypad_row
	{
		struct keypad_row	*next;
		int					cols;

		struct keypad_button *first_button;
		struct keypad_button *last_button;
	};

	struct keypad
	{
		struct 				keypad *next;
		int						rows;
		int						cols;

		enum KEYPAD_POSITION	position;

		struct 				keypad_row *first_row;
		struct 				keypad_row *last_row;

		GtkWidget				*widget;

		char 	name[1];
	};

	LOCAL_EXTERN struct keypad	* keypad_load(void);
	LOCAL_EXTERN void			  keypad_free(struct keypad *keypad);
	LOCAL_EXTERN void 			  keypad_pack(GtkBox *box, struct keypad *keypad, enum KEYPAD_POSITION position);

#else

	LOCAL_EXTERN GtkWidget	* keypad;
	LOCAL_EXTERN GtkWidget	* CreateKeypadWidget(void);

#endif // CONFIGURABLE_KEYPAD

	LOCAL_EXTERN void	  	  SetKeypadSensitive(gboolean state);

/* 
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe.
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
 * Este programa está nomeado como xio.c e possui 143 linhas de código.
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

/*
 *	xio.c
 *		Low-level I/O setup functions and exit code.
 */

#include "globals.h"

#include "actionsc.h"
#include "hostc.h"
#include "telnetc.h"
#include "togglesc.h"
#include "utilc.h"
#include "xioc.h"

/* Statics. */
static unsigned long ns_read_id;
static unsigned long ns_exception_id;
static Boolean reading = False;
static Boolean excepting = False;

/*
 * Called to set up input on a new network connection.
 */
void
x_add_input(int net_sock)
{
	ns_exception_id = AddExcept(net_sock, net_exception);
	excepting = True;
	ns_read_id = AddInput(net_sock, net_input);
	reading = True;
}

/*
 * Called when an exception is received to disable further exceptions.
 */
void
x_except_off(void)
{
	if (excepting) {
		RemoveInput(ns_exception_id);
		excepting = False;
	}
}

/*
 * Called when exception processing is complete to re-enable exceptions.
 * This includes removing and restoring reading, so the exceptions are always
 * processed first.
 */
void
x_except_on(int net_sock)
{
	if (excepting)
		return;
	if (reading)
		RemoveInput(ns_read_id);
	ns_exception_id = AddExcept(net_sock, net_exception);
	excepting = True;
	if (reading)
		ns_read_id = AddInput(net_sock, net_input);
}

/*
 * Called to disable input on a closing network connection.
 */
void
x_remove_input(void)
{
	if (reading) {
		RemoveInput(ns_read_id);
		reading = False;
	}
	if (excepting) {
		RemoveInput(ns_exception_id);
		excepting = False;
	}
}

/*
 * Application exit, with cleanup.
 */
void
x3270_exit(int n)
{
	static Boolean already_exiting = 0;

	/* Handle unintentional recursion. */
	if (already_exiting)
		return;
	already_exiting = True;

	/* Turn off toggle-related activity. */
	shutdown_toggles();

	/* Shut down the socket gracefully. */
	host_disconnect(False);

	/* Tell anyone else who's interested. */
	st_changed(ST_EXITING, True);

	exit(n);
}

void
Quit_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
	action_debug(Quit_action, event, params, num_params);
	if (!w || !CONNECTED) {
		x3270_exit(0);
	}
}

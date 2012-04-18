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
 * Este programa está nomeado como session.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef LIB3270_TRACE_H_INCLUDED

	#define LIB3270_TRACE_H_INCLUDED 1

	/**
	 * Set trace handle callback.
	 *
	 * @param handle	Callback to write in trace file or show trace window (NULL send all trace to stdout).
	 *
	 */
	LIB3270_EXPORT void lib3270_set_trace_handler( void (*handler)(H3270 *session, const char *fmt, va_list args) );

	/**
	 * Write on trace file.
	 *
	 * Write text on trace file, if DStrace is enabled.
	 *
	 * @param fmt 	String format.
	 * @param ...	Arguments.
	 *
	 */
	LIB3270_EXPORT void lib3270_write_dstrace(H3270 *session, const char *fmt, ...);

	/**
	 * Write on trace file.
	 *
	 * Write text on trace file, if event is enabled.
	 *
	 * @param fmt 	String format.
	 * @param ...	Arguments.
	 *
	 */
	LIB3270_EXPORT void lib3270_trace_event(H3270 *session, const char *fmt, ...);


#endif // LIB3270_TRACE_H_INCLUDED

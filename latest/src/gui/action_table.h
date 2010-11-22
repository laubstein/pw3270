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
 * Este programa está nomeado como action_table.h e possui - linhas de código.
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

	/* Keyboard actions */
	DECLARE_PW3270_ACTION( enter, "" )

	/* Network actions */
	DECLARE_PW3270_ACTION( sethostname, "" )
    DECLARE_PW3270_ACTION( connect, "host" )
    DECLARE_PW3270_ACTION( disconnect, "" )

	/* Configuration actions */
	DECLARE_PW3270_ACTION( save, "" )
	DECLARE_PW3270_ACTION( restore, "" )
	DECLARE_PW3270_ACTION( selectcolors, "" )

	/* Misc actions */
	DECLARE_PW3270_ACTION( about, "" )
	DECLARE_PW3270_ACTION( quit, "" )

	/* Screen actions */
	DECLARE_PW3270_ACTION( redraw, "" )

	/* File Transfer actions */
	DECLARE_PW3270_ACTION( download, "" )
	DECLARE_PW3270_ACTION( upload, "" )

	/* Text actions */
	DECLARE_PW3270_ACTION( savescreen, "")
	DECLARE_PW3270_ACTION( saveselected, "")
	DECLARE_PW3270_ACTION( saveclipboard, "")

	/* Print actions */
	DECLARE_PW3270_ACTION( printscreen, "")
	DECLARE_PW3270_ACTION( printselected, "")
	DECLARE_PW3270_ACTION( printclipboard, "")

	/* Clipboard actions */
	DECLARE_PW3270_ACTION( unselect, "" )
	DECLARE_PW3270_ACTION( reselect, "" )
	DECLARE_PW3270_ACTION( selectfield, "" )
	DECLARE_PW3270_ACTION( copy, "" )
	DECLARE_PW3270_ACTION( copyastable, "" )
	DECLARE_PW3270_ACTION( copyasimage, "" )
	DECLARE_PW3270_ACTION( paste, "" )
	DECLARE_PW3270_ACTION( pastenext, "" )
	DECLARE_PW3270_ACTION( pastetextfile, "" )
	DECLARE_PW3270_ACTION( append, "" )
	DECLARE_PW3270_ACTION( selectall, "" )

	DECLARE_PW3270_ACTION( selectright, ""  )
	DECLARE_PW3270_ACTION( selectleft, ""  )
	DECLARE_PW3270_ACTION( selectup, ""  )
	DECLARE_PW3270_ACTION( selectdown, ""  )

	DECLARE_PW3270_ACTION( selectionleft, ""  )
	DECLARE_PW3270_ACTION( selectionup, ""  )
	DECLARE_PW3270_ACTION( selectionright, ""  )
	DECLARE_PW3270_ACTION( selectiondown, ""  )

	/* Debug actions */
	DECLARE_PW3270_ACTION( dumpscreen, ""  )
	DECLARE_PW3270_ACTION( loadscreendump, ""  )

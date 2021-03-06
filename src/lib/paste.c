/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como paste.c e possui - linhas de código.
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

#include "globals.h"

#if defined(X3270_DISPLAY) /*[*/
#include <X11/Xatom.h>
#endif
#define XK_3270
#if defined(X3270_APL) /*[*/
#define XK_APL
#endif /*]*/
#include <X11/keysym.h>

#include <fcntl.h>
#include "3270ds.h"
#include "appres.h"
#include "ctlr.h"
#include "resources.h"

#include "actionsc.h"
#include "ansic.h"
#include "aplc.h"
#include "ctlrc.h"
#include "ftc.h"
#include "hostc.h"
// #include "idlec.h"
// #include "keymapc.h"
#include "keypadc.h"
#include "kybdc.h"
// #include "macrosc.h"
#include "popupsc.h"
#include "printc.h"
#include "screenc.h"
// #if defined(X3270_DISPLAY) /*[*/
// #include "selectc.h"
// #endif /*]*/
#include "statusc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "togglesc.h"
#include "trace_dsc.h"
#include "utf8c.h"
#include "utilc.h"
#if defined(X3270_DBCS) /*[*/
#include "widec.h"
#endif /*]*/
#include <lib3270/api.h>

/*---[ Struct ]-------------------------------------------------------------------------------------------------*/

 typedef struct _paste_data
 {
 	int qtd;
 	int row;
	int orig_addr;
	int orig_col;
 } PASTE_DATA;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

/*
 * Move the cursor back within the legal paste area.
 * Returns a Boolean indicating success.
 */
 static int remargin(int lmargin)
 {
	int ever = False;
	int baddr, b0 = 0;
	int faddr;
	unsigned char fa;

	if(toggled(MARGINED_PASTE))
	{
		baddr = h3270.cursor_addr;
		while(BA_TO_COL(baddr) < lmargin)
		{
			baddr = ROWCOL_TO_BA(BA_TO_ROW(baddr), lmargin);
			if (!ever)
			{
				b0 = baddr;
				ever = True;
			}

			faddr = find_field_attribute(&h3270,baddr);
			fa = ea_buf[faddr].fa;
			if (faddr == baddr || FA_IS_PROTECTED(fa))
			{
				baddr = next_unprotected(baddr);
				if (baddr <= b0)
					return 0;
			}

		}
		cursor_move(baddr);
	}

	return -1;
 }

 static int paste_char(PASTE_DATA *data, unsigned char c)
 {

	if(toggled(SMART_PASTE))
	{
		int faddr = find_field_attribute(&h3270,h3270.cursor_addr);
		if(FA_IS_PROTECTED(ea_buf[faddr].fa))
			h3270.cursor_addr++;
		else
			key_ACharacter(c, KT_STD, IA_PASTE, NULL);
	}
	else
	{
		key_ACharacter(c, KT_STD, IA_PASTE, NULL);
	}

	data->qtd++;

 	if(BA_TO_ROW(h3270.cursor_addr) != data->row)
 	{
 		Trace("Row changed from %d to %d",data->row,BA_TO_ROW(h3270.cursor_addr));
		if(!remargin(data->orig_col))
			return 0;
		data->row = BA_TO_ROW(h3270.cursor_addr);
 		return '\n';
 	}

 	return c;
 }

LIB3270_EXPORT int lib3270_set_string(H3270 *h, const unsigned char *str)
{
	PASTE_DATA data;
	unsigned char last = 1;
	int baddr;
	int faddr;
	unsigned char fa;

	CHECK_SESSION_HANDLE(h);

	memset(&data,0,sizeof(data));
 	data.row		= BA_TO_ROW(h->cursor_addr);
	data.orig_addr	= h->cursor_addr;
	data.orig_col	= BA_TO_COL(h->cursor_addr);

	if(kybdlock)
		return -EINVAL;

	screen_suspend(h);

	while(*str && last && !kybdlock && h->cursor_addr >= data.orig_addr)
	{
		switch(*str)
		{
		case '\t':
			last = paste_char(&data, ' ');
			break;

		case '\n':
			if(last != '\n')
			{
				baddr = (h->cursor_addr + h->cols) % (h->cols * h->rows);   /* down */
				baddr = (baddr / h->cols) * h->cols;               /* 1st col */
				faddr = find_field_attribute(h,baddr);
				fa = ea_buf[faddr].fa;
				if (faddr != baddr && !FA_IS_PROTECTED(fa))
					cursor_move(baddr);
				else
					cursor_move(next_unprotected(baddr));
				data.row = BA_TO_ROW(h->cursor_addr);
			}
			last = ' ';
			data.qtd++;
			break;

		default:
			last = paste_char(&data, *str);

		}
		str++;

		if(IN_3270 && toggled(MARGINED_PASTE) && BA_TO_COL(h->cursor_addr) < data.orig_col)
		{
			if(!remargin(data.orig_col))
				last = 0;
		}
	}

	screen_resume(h);
	return data.qtd;
}

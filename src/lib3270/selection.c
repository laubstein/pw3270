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
 * Este programa está nomeado como selection.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "globals.h"
// #include "ctlr.h"
 #include "appres.h"
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/selection.h>

 static void update_selection(H3270 *session);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void get_selected_addr(H3270 *session, int *begin, int *end)
{
	if(session->select.begin > session->select.end)
	{
		*end   = session->select.begin;
		*begin = session->select.end;
	}
	else
	{
		*begin = session->select.begin;
		*end   = session->select.end;
	}
}

static void update_selected_rectangle(H3270 *session)
{
	struct
	{
		int row;
		int col;
	} p[2];


	int begin, end, row, col, baddr;

	get_selected_addr(session,&begin,&end);

	// Get start & end posision
	p[0].row = (begin/session->cols);
	p[0].col = (begin%session->cols);
	p[1].row = (end/session->cols);
	p[1].col = (end%session->cols);

	// First remove unselected areas
	baddr = 0;
	for(row=0;row < session->rows;row++)
	{
		for(col = 0; col < session->cols;col++)
		{
			if(!(row >= p[0].row && row <= p[1].row && col >= p[0].col && col <= p[1].col) && (session->text[baddr].attr & LIB3270_ATTR_SELECTED))
			{
				session->text[baddr].attr &= ~LIB3270_ATTR_SELECTED;
				session->update(session,baddr,session->text[baddr].chr,session->text[baddr].attr,baddr == session->cursor_addr);
			}
			baddr++;
		}
	}

	// Then, draw selected ones
	baddr = 0;
	for(row=0;row < session->rows;row++)
	{
		for(col = 0; col < session->cols;col++)
		{
			if((row >= p[0].row && row <= p[1].row && col >= p[0].col && col <= p[1].col) && !(session->text[baddr].attr & LIB3270_ATTR_SELECTED))
			{
				session->text[baddr].attr |= LIB3270_ATTR_SELECTED;
				session->update(session,baddr,session->text[baddr].chr,session->text[baddr].attr,baddr == session->cursor_addr);
			}
			baddr++;
		}
	}

}

static void update_selected_region(H3270 *session)
{
	int baddr,begin,end;
	int len = session->rows*session->cols;

	get_selected_addr(session,&begin,&end);

	// First remove unselected areas
	for(baddr = 0; baddr < begin; baddr++)
	{
		if(session->text[baddr].attr & LIB3270_ATTR_SELECTED)
		{
			session->text[baddr].attr &= ~LIB3270_ATTR_SELECTED;
			session->update(session,baddr,session->text[baddr].chr,session->text[baddr].attr,baddr == session->cursor_addr);
		}
	}

	for(baddr = end+1; baddr < len; baddr++)
	{
		if(session->text[baddr].attr & LIB3270_ATTR_SELECTED)
		{
			session->text[baddr].attr &= ~LIB3270_ATTR_SELECTED;
			session->update(session,baddr,session->text[baddr].chr,session->text[baddr].attr,baddr == session->cursor_addr);
		}
	}

	// Then draw the selected ones
	for(baddr = begin; baddr <= end; baddr++)
	{
		if(!(session->text[baddr].attr & LIB3270_ATTR_SELECTED))
		{
			session->text[baddr].attr |= LIB3270_ATTR_SELECTED;
			session->update(session,baddr,session->text[baddr].chr,session->text[baddr].attr,baddr == session->cursor_addr);
		}
	}

}

void update_selection(H3270 *session)
{
	if(lib3270_get_toggle(session,LIB3270_TOGGLE_RECTANGLE_SELECT))
		update_selected_rectangle(session);
	else
		update_selected_region(session);
}

static void set_selected(H3270 *session)
{
	if(session->selected)
		return;

	session->selected = 1;
	session->set_selection(session,1);
}

void toggle_rectselect(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt)
{
	if(!session->selected)
		return;

	if(t->value)
		update_selected_rectangle(session);
	else
		update_selected_region(session);
}

LIB3270_ACTION(unselect)
{
	int a;

	CHECK_SESSION_HANDLE(hSession);

	if(!hSession->selected)
		return 0;

	hSession->selected = 0;

	for(a = 0; a < hSession->rows*hSession->cols; a++)
	{
		if(hSession->text[a].attr & LIB3270_ATTR_SELECTED)
		{
			hSession->text[a].attr &= ~LIB3270_ATTR_SELECTED;
			if(hSession->update)
				hSession->update(hSession,a,hSession->text[a].chr,hSession->text[a].attr,a == hSession->cursor_addr);
		}
	}

	hSession->set_selection(hSession,0);

	return 0;
}

LIB3270_EXPORT void lib3270_select_to(H3270 *session, int baddr)
{
	CHECK_SESSION_HANDLE(session);

	if(!lib3270_connected(session))
		return;

	if(!session->selected)
	{
		session->select.begin = session->cursor_addr;
		set_selected(session);
	}

	lib3270_set_cursor_address(session,session->select.end = baddr);

	update_selection(session);

}

LIB3270_EXPORT void lib3270_select_word(H3270 *session, int baddr)
{
	int pos, len;

	CHECK_SESSION_HANDLE(session);

	if(!lib3270_connected(session) || isspace(session->text[baddr].chr))
	{
		lib3270_ring_bell(session);
		return;
	}

	for(pos = baddr; pos > 0 && !isspace(session->text[pos].chr);pos--);
	session->select.begin = pos > 0 ? pos+1 : 0;

	len = session->rows * session->cols;
	for(pos = baddr; pos < len && !isspace(session->text[pos].chr);pos++);
	session->select.end = pos < len ? pos-1 : len;

	set_selected(session);
	update_selected_region(session);
}

LIB3270_EXPORT int lib3270_select_field_at(H3270 *session, int baddr)
{
	int start,len;

	CHECK_SESSION_HANDLE(session);

	if(!lib3270_connected(session))
	{
		lib3270_ring_bell(session);
		return -1;
	}

	start = lib3270_field_addr(session,baddr);

	if(start < 0)
	{
		lib3270_ring_bell(session);
		return -1;
	}

	session->select.begin	= (start+1);

	len = (session->rows * session->cols)-1;

	session->select.end	= start + lib3270_field_length(session,start);
	if(session->select.end > len)
		session->select.end = len;

	set_selected(session);
	update_selected_region(session);

	return 0;
}

LIB3270_ACTION( selectfield )
{
	CHECK_SESSION_HANDLE(hSession);
	lib3270_select_field_at(hSession,hSession->cursor_addr);
	return 0;
}

LIB3270_ACTION( selectall )
{
	int len, baddr;

	CHECK_SESSION_HANDLE(hSession);
	len = hSession->rows*hSession->cols;

	// First remove unselected areas
	for(baddr = 0; baddr < len; baddr++)
	{
		if(!(hSession->text[baddr].attr & LIB3270_ATTR_SELECTED))
		{
			hSession->text[baddr].attr |= LIB3270_ATTR_SELECTED;
			hSession->update(hSession,baddr,hSession->text[baddr].chr,hSession->text[baddr].attr,baddr == hSession->cursor_addr);
		}
	}

	set_selected(hSession);

	return 0;
}

LIB3270_ACTION( reselect )
{
	CHECK_SESSION_HANDLE(hSession);

	if(!lib3270_connected(hSession) || hSession->select.begin == hSession->select.end || hSession->selected)
		return 0;

	update_selection(hSession);
	set_selected(hSession);

	return 0;
}

static char * get_text(H3270 *hSession,unsigned char all)
{
	int	row, col, baddr;
	char *ret;
	size_t	  sz = 0;

	if(!lib3270_connected(hSession))
		return NULL;

	ret = malloc(hSession->rows * (hSession->cols+1));

	baddr = 0;
	for(row=0;row < hSession->rows;row++)
	{
		int cr = 0;

		for(col = 0; col < hSession->cols;col++)
		{
			if(all || hSession->text[baddr].attr & LIB3270_ATTR_SELECTED)
			{
				cr++;
				ret[sz++] = hSession->text[baddr].chr;
			}
			baddr++;
		}

		if(cr)
			ret[sz++] = '\n';
	}
	ret[sz] = 0;

	return realloc(ret,sz+1);
}

LIB3270_EXPORT char * lib3270_get_text(H3270 *hSession)
{
	return get_text(hSession,1);
}

LIB3270_EXPORT char * lib3270_get_selected(H3270 *hSession)
{
	if(!hSession->selected || hSession->select.begin == hSession->select.end)
		return NULL;

	return get_text(hSession,0);
}

LIB3270_EXPORT int lib3270_move_selection(H3270 *hSession, LIB3270_DIRECTION dir)
{
	if(!hSession->selected || hSession->select.begin == hSession->select.end)
		return ENOENT;

	switch(dir)
	{
	case LIB3270_DIR_UP:
		if(hSession->select.begin <= hSession->cols)
			return EINVAL;
		hSession->select.begin -= hSession->cols;
		hSession->select.end   -= hSession->cols;
		break;

	case LIB3270_DIR_DOWN:
		if(hSession->select.end >= (hSession->cols * (hSession->rows-1)))
			return EINVAL;
		hSession->select.begin += hSession->cols;
		hSession->select.end   += hSession->cols;
		break;

	case LIB3270_DIR_LEFT:
		if( (hSession->select.begin % hSession->cols) < 1)
			return EINVAL;
		hSession->select.begin--;
		hSession->select.end--;
		break;

	case LIB3270_DIR_RIGHT:
		if( (hSession->select.end % hSession->cols) >= (hSession->cols-1))
			return EINVAL;
		hSession->select.begin++;
		hSession->select.end++;
		break;

	default:
		return -1;
	}

	update_selection(hSession);
	lib3270_set_cursor_address(hSession,hSession->select.end);

	return 0;
}

LIB3270_EXPORT int lib3270_move_cursor(H3270 *hSession, LIB3270_DIRECTION dir, unsigned char sel)
{
	int cursor_addr = hSession->cursor_addr;

	if(!lib3270_connected(hSession))
		return -1;

	switch(dir)
	{
	case LIB3270_DIR_UP:
		if(cursor_addr <= hSession->cols)
			return EINVAL;
		cursor_addr -= hSession->cols;
		break;

	case LIB3270_DIR_DOWN:
		if(cursor_addr >= (hSession->cols * (hSession->rows-1)))
			return EINVAL;
		cursor_addr += hSession->cols;
		break;

	case LIB3270_DIR_LEFT:
		if( (cursor_addr % hSession->cols) < 1)
			return EINVAL;
		cursor_addr--;
		break;

	case LIB3270_DIR_RIGHT:
		if( (cursor_addr % hSession->cols) >= (hSession->cols-1))
			return EINVAL;
		cursor_addr++;
		break;

	default:
		return -1;
	}

	if(sel)
		lib3270_select_to(hSession,cursor_addr);
	else
		lib3270_set_cursor_address(hSession,cursor_addr);

}
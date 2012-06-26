/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como html.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include <string.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/html.h>

 #include "globals.h"
 #include "utilc.h"

 struct chr_xlat
 {
	unsigned char 	  chr;
	const char		* xlat;
 };

/*--[ Defines ]--------------------------------------------------------------------------------------*/

 enum html_element
 {
	HTML_ELEMENT_LINE_BREAK,
	HTML_ELEMENT_BEGIN_COLOR,
	HTML_ELEMENT_END_COLOR,
	HTML_ELEMENT_HEADER,
	HTML_ELEMENT_FOOTER,

	HTML_ELEMENT_COUNT
 };

 static const char * element_text[HTML_ELEMENT_COUNT] =
 {
#ifdef DEBUG
	"<br />\n",
#else
	"<br />",
#endif // Debug
	"<span style=\"color:%s;background-color:%s\">",
	"</span>",
	"<!DOCTYPE html><html><head><meta http-equiv=\"content-type\" content=\"text/html;charset=%s\"/></head><body style=\"font-family:courier;background-color:%s\">",
	"</body></html>"
 };

 static const char * html_color[] =
 {
		"black",
		"deepSkyBlue",
		"red",
		"pink",
		"green",
		"turquoise",
		"yellow",
		"white",
		"black",
		"blue",
		"orange",
		"purple",
		"paleGreen",
		"paleTurquoise",
		"grey",
		"white"
 };

 struct html_info
 {
	int				  szText;
	char			* text;
	unsigned short	  fg;
	unsigned short	  bg;
 };

 /*--[ Implement ]------------------------------------------------------------------------------------*/

 static void append_string(struct html_info *info, const char *text)
 {
 	int sz = strlen(info->text)+strlen(text);

	if(strlen(info->text)+sz <= info->szText)
	{
		info->szText	+= (100+sz);
		info->text		 = lib3270_realloc(info->text,info->szText);
	}

	strcat(info->text,text);

 }

 static void append_element(struct html_info *info, enum html_element id)
 {
	append_string(info,element_text[id]);
 }

 static update_colors(struct html_info *info, unsigned short attr)
 {
	unsigned short	  fg;
	unsigned short	  bg	= ((attr & 0x00F0) >> 4);
	char 			* txt;

	#warning Fix field colors
	if(attr & LIB3270_ATTR_FIELD)
		fg = (attr & 0x0003);
	else
		fg = (attr & 0x000F);

	if(fg == info->fg && bg == info->bg)
		return;

	if(info->fg != 0xFF)
		append_string(info,element_text[HTML_ELEMENT_END_COLOR]);

	txt = xs_buffer(element_text[HTML_ELEMENT_BEGIN_COLOR],html_color[fg],html_color[bg]);
	append_string(info,txt);
	lib3270_free(txt);

	info->fg = fg;
	info->bg = bg;
 }

 static const append_char(struct html_info *info, const struct chr_xlat *xlat, unsigned char chr)
 {
	char txt[] = { chr, 0 };
	int f;

	for(f=0;xlat[f].chr;f++)
	{
		if(xlat[f].chr == chr)
		{
			append_string(info,xlat[f].xlat);
			return;
		}
	}

	append_string(info,txt);

 }

 LIB3270_EXPORT char * lib3270_get_as_html(H3270 *session, LIB3270_HTML_OPTION option)
 {
	int	row, baddr;
	struct html_info info;

 	memset(&info,0,sizeof(info));
 	info.szText = session->rows * (session->cols + strlen(element_text[HTML_ELEMENT_LINE_BREAK])+1);
 	info.text	= lib3270_malloc(info.szText+1);
 	info.fg		= 0xFF;
 	info.bg		= 0xFF;

	if(option & LIB3270_HTML_OPTION_HEADERS)
	{
		char *txt = xs_buffer(element_text[HTML_ELEMENT_HEADER],lib3270_get_charset(session),html_color[0]);
		append_string(&info,txt);
		lib3270_free(txt);
	}

	baddr = 0;
	for(row=0;row < session->rows;row++)
	{
		int cr  = 0;
		int len = 0;
		int col;

		for(col = 0; col < session->cols;col++)
		{
			if( session->text[baddr+col].chr != ' ' || (session->text[baddr+col].attr & LIB3270_ATTR_CG))
				len = col;
		}

		for(col = 0; col <= len;col++)
		{
			if((option && LIB3270_HTML_OPTION_ALL) || (session->text[baddr+col].attr & LIB3270_ATTR_SELECTED))
			{
				cr++;
				update_colors(&info,session->text[baddr+col].attr);

				if(session->text[baddr+col].attr & LIB3270_ATTR_CG)
				{
					static const struct chr_xlat xlat[] =
					{
						{ 0xd3, "+"		}, // CG 0xab, plus
						{ 0xa2, "-"		}, // CG 0x92, horizontal line
						{ 0x85, "|"		}, // CG 0x184, vertical line
						{ 0xd4, "+"		}, // CG 0xac, LR corner
						{ 0xd5, "+"		}, // CG 0xad, UR corner
						{ 0xc5, "+"		}, // CG 0xa4, UL corner
						{ 0xc4, "+"		}, // CG 0xa3, LL corner
						{ 0xc6, "|"		}, // CG 0xa5, left tee
						{ 0xd6, "|"		}, // CG 0xae, right tee
						{ 0xc7, "-"		}, // CG 0xa6, bottom tee
						{ 0xd7, "-"		}, // CG 0xaf, top tee
						{ 0x8c, "&le;"	}, // CG 0xf7, less or equal "≤"
						{ 0xae, "&ge;"	}, // CG 0xd9, greater or equal "≥"
						{ 0xbe, "&ne;"	}, // CG 0x3e, not equal "≠"
						{ 0xad, "["		}, // "["
						{ 0xbd, "]"		}, // "]"

						{ 0x00, NULL	}
					};

					append_char(&info, xlat, session->text[baddr+col].chr);

				}
				else
				{
					static const struct chr_xlat xlat[] =
					{
						{ '"',	"&quot;"	},
						{ '&',	"&amp;"		},
						{ '<',	"&lt;"		},
						{ '>',	"&gt;"		},
						{ ' ',	"&nbsp;"	},

						{ 0x00, NULL		}
					};

					append_char(&info, xlat, session->text[baddr+col].chr);

				}

			}
		}

		baddr += session->cols;

		if(cr || (option && LIB3270_HTML_OPTION_ALL))
			append_element(&info,HTML_ELEMENT_LINE_BREAK);
	}

	if(info.fg != 0xFF)
		append_string(&info,element_text[HTML_ELEMENT_END_COLOR]);

	if(option & LIB3270_HTML_OPTION_HEADERS)
		append_element(&info,HTML_ELEMENT_FOOTER);

	return lib3270_realloc(info.text,strlen(info.text)+2);
 }


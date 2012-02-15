/*
 * "Software pw3270, desenvolvido com base nos c�digos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emula��o de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa � software livre. Voc� pode redistribu�-lo e/ou modific�-lo sob
 * os termos da GPL v.2 - Licen�a P�blica Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa � distribu�do na expectativa de  ser  �til,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia impl�cita de COMERCIALIZA��O ou  de  ADEQUA��O
 * A QUALQUER PROP�SITO EM PARTICULAR. Consulte a Licen�a P�blica Geral GNU para
 * obter mais detalhes.
 *
 * Voc� deve ter recebido uma c�pia da Licen�a P�blica Geral GNU junto com este
 * programa;  se  n�o, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa est� nomeado como testprogram.cxx e possui 1088 linhas de c�digo.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendon�a)
 * licinio@bb.com.br		(Lic�nio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aur�lio Caldas Miranda)
 *
 */

#include <stdio.h>
#ifdef WIN32
	#include <windows.h>
#endif

int main(int numpar, char *param[])
{
	FILE *out = fopen(param[1],"w");

	if(!out)
		return -1;

	fprintf(out,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(out,"<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n");
	fprintf(out,"<manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"description.xml\"/>\n");
	fprintf(out,"<manifest:file-entry manifest:media-type=\"application/vnd.sun.star.uno-component;type=native;platform=Windows\" manifest:full-path=\"windows/pw3270.uno.dll\"/>\n");
	fprintf(out,"<manifest:file-entry manifest:media-type=\"application/vnd.sun.star.uno-typelibrary;type=RDB\" manifest:full-path=\"pw3270.uno.rdb\"/>\n");
	fprintf(out,"</manifest:manifest>\n");

	fclose(out);

	out = fopen(param[2],"w");

	if(!out)
		return -1;

	fprintf(out,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(out,"<description xmlns=\"http://openoffice.org/extensions/description/2006\"\n");
	fprintf(out,"xmlns:d=\"http://openoffice.org/extensions/description/2006\"\n");
	fprintf(out,"xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n");

	fprintf(out,"  <version value=\"4.0\" />\n");

	fprintf(out,"  <identifier value=\"pw3270\" />\n");

	fprintf(out,"  <platform value=\"windows_x86\" />\n");

	fprintf(out,"  <dependencies>\n");
	fprintf(out,"    <OpenOffice.org-minimal-version value=\"2.2\" d:name=\"OpenOffice.org 2.2\"/>\n");
	fprintf(out,"  </dependencies>\n");

	fprintf(out,"  <display-name>\n");
	fprintf(out,"    <name lang=\"en\">3270 access extension</name>\n");
	fprintf(out,"  </display-name>\n");

	fprintf(out,"  <icon>\n");
	fprintf(out,"    <default xlink:href=\"pw3270.png\" />\n");
	fprintf(out,"  </icon>\n");

	fprintf(out,"  <extension-description>\n");
	fprintf(out,"    <src xlink:href=\"description.txt\" lang=\"en\" />\n");
	fprintf(out,"  </extension-description>\n");

	fprintf(out,"</description>\n");

	fclose(out);

	return 0;

}


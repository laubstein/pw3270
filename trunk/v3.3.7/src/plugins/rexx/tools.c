/*
* Copyright 2008, Banco do Brasil S.A.
*
* This file is part of g3270
*
* This program file is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program in a file named COPYING; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
*
* Authors:
*
* Perry Werneck<perry.werneck@gmail.com>
*
*/

 #include "rx3270.h"

/*---[ Statics ]----------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------*/

 ULONG APIENTRY rx3270Log(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	Log("%s",Argv[0].strptr);

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270QueryCState(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	#define DECLARE_XLAT_STATE( x ) { x, #x }

 	static const struct _xlat_state
 	{
 		enum cstate	state;
 		const gchar	*ret;
 	} xlat_state[] =
 	{
			DECLARE_XLAT_STATE( NOT_CONNECTED 		),
			DECLARE_XLAT_STATE( RESOLVING			),
			DECLARE_XLAT_STATE( PENDING				),
			DECLARE_XLAT_STATE( CONNECTED_INITIAL	),
			DECLARE_XLAT_STATE( CONNECTED_ANSI		),
			DECLARE_XLAT_STATE( CONNECTED_3270		),
			DECLARE_XLAT_STATE( CONNECTED_INITIAL_E	),
			DECLARE_XLAT_STATE( CONNECTED_NVT		),
			DECLARE_XLAT_STATE( CONNECTED_SSCP		),
			DECLARE_XLAT_STATE( CONNECTED_TN3270E	)
 	};

 	int				f;
 	enum cstate 	state = QueryCstate();

	if(Argc != 0)
		return RXFUNC_BADCALL;

	for(f=0;f < G_N_ELEMENTS(xlat_state); f++)
	{
		if(state == xlat_state[f].state)
		{
			return RetString(Retstr,xlat_state[f].ret);
		}
	}

	return RetString(Retstr,"UNEXPECTED");
 }

 ULONG APIENTRY rx3270Version(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	strncpy(Retstr->strptr,PACKAGE_VERSION,RXAUTOBUFLEN-1);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }


 ULONG APIENTRY rx3270UpdateScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc)
		return RXFUNC_BADCALL;

	while(gtk_events_pending())
		gtk_main_iteration();

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Sleep(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	clock_t end = clock();

	switch(Argc)
	{
	case 0:
		end += CLOCKS_PER_SEC;
		break;

	case 1:
		end += (atoi(Argv[0].strptr) * CLOCKS_PER_SEC);
		break;

	default:
		return RXFUNC_BADCALL;
	}

	Trace("Wait from %ld to %ld",(long) clock(),(long) end);

	while(clock() < end)
		gtk_main_iteration();

	Trace("Sleep ended (clock: %ld)",(long) clock());

	return RetValue(Retstr,0);
 }



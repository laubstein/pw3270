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

/*---[ Implement ]--------------------------------------------------------------------------------*/

 ULONG APIENTRY rx3270ToggleON(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			set_toggle(toggle,TRUE);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270ToggleOFF(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			set_toggle(toggle,FALSE);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Toggle(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int f;

 	for(f=0;f<Argc;f++)
 	{
 		int toggle = get_toggle_by_name(Argv[f].strptr);

		if(toggle < 0)
			return RXFUNC_BADCALL;
		else
			do_toggle(toggle);
 	}

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Toggled(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	int toggle;

	if(Argc != 1)
		return RXFUNC_BADCALL;

	toggle = get_toggle_by_name(Argv[0].strptr);

	if(toggle < 0)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,Toggled(toggle));
 }



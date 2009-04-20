/*-- REXX -----------------------------------------------------------------------*/
/*                                                                               */
/* "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270 */
/* (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a */
/* aplicativos mainframe. Registro no INPI sob o nome G3270.                     */
/*                                                                               */
/* Copyright (C) <2008> <Banco do Brasil S.A.>                                   */
/*                                                                               */
/* Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob */
/* os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela */
/* Free Software Foundation.                                                     */
/*                                                                               */
/* Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER */
/* GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO */
/* A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para */
/* obter mais detalhes.                                                          */
/*                                                                               */
/* Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este  */
/* programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple  */
/* Place, Suite 330, Boston, MA, 02111-1307, USA                                 */
/*                                                                               */
/* Contatos:                                                                     */
/*                                                                               */
/* perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)                   */
/* erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)                         */
/* licinio@bb.com.br		(Licínio Luis Branco)                                */
/* kraucer@bb.com.br		(Kraucer Fernandes Mazuco)                           */
/* macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)                       */
/*                                                                               */
/*-------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------*/
/* Library initialization code                                                   */
/*-------------------------------------------------------------------------------*/

if rxfuncquery('rx3270LoadFuncs') then do
   call rxfuncadd 'rx3270LoadFuncs', 'rx3270', 'rx3270LoadFuncs'
   retc = rx3270LoadFuncs()
   say 'Return code from loading rx3270 functions was' retc
end


/*-------------------------------------------------------------------------------*/
/* 3270 class                                                                    */
/*-------------------------------------------------------------------------------*/
::class rx3270 public subclass object

::method init
	if arg() > 0
		then raise syntax 40.4 array ('INIT', 0)
return

::method uninit
return

::method Connect
	use arg hostinfo, wait
return rx3270Connect(hostinfo,wait)

::method Disconnect
return rx3270Disconnect()

::method getConnectionState
return rx3270QueryCState()

::method isConnected
return self~getConnectionState() = "CONNECTED_TN3270E"

::method waitForEvents
return rx3270WaitForEvents()

::method getScreenContentAt
	use arg row, col, size
return rx3270ReadScreen(row,col,size)

::method getScreenContent
return rx3270ReadScreen()

::method waitForStringAt
	use arg row, col, key, timeout
	if datatype(timeout) <> "NUM"
		then timeout = 60
return rx3270WaitForStringAt(row,col,key,timeout)

::method queryStringAt
	use arg row, col, key
return self~getScreenContentAt(row,col,length(key)) == key

::method isTerminalReady
return rx3270IsTerminalReady()

::method waitForTerminalReady
	use arg timeout
return rx3270WaitForTerminalReady(timeout)

::method setStringAt
	use arg row, col, str
return rx3270InputString(row,col,str)

::method sendEnterKey
return rx3270SendENTERKey()

::method sendPFKey
	use arg key
return rx3270SendPFKey(key)

::method setCursorPosition
	use arg row, col
return rx3270SetCursorPosition(row,col)

/*-------------------------------------------------------------------------------*/
/* Common dialog box class                                                       */
/*-------------------------------------------------------------------------------*/
::class DialogBox public subclass object

::method init
	self~widget = 0
return

::method uninit
	ok = self~destroy()
return

::method destroy
	if self~widget <> 0
		then ok = rx3270DestroyDialog(self~widget)
	self~widget = 0;
return

::method run
	if self~widget = 0
		then return -22
return rx3270runDialog(self~widget)

::method widget
	expose widget
return widget

::method 'widget=' private
	expose widget
	use arg ptr
	widget = ptr
return

::method 'title='
	expose widget
	use arg ptr
return rx3270SetDialogTitle(self~widget,ptr)

/*-------------------------------------------------------------------------------*/
/* File selection dialog box                                                     */
/*-------------------------------------------------------------------------------*/
::class FileChooserDialog public subclass DialogBox

::method init
	use arg type, title
	self~widget = rx3270FileChooserNew(type,title)
return

::method filename
return rx3270FileChooserGetFilename(self~widget)



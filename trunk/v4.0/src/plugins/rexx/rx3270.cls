/*-- REXX -----------------------------------------------------------------------*/
/*                                                                               */
/* "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270 */
/* (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a */
/* aplicativos mainframe.                                                        */
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


::class rx3270 public subclass object

::method init
	if arg() > 0
		then raise syntax 40.4 array ('INIT', 0)
return

::method uninit
	ok = rx3270Disconnect()
return

::method Connect
	use arg hostinfo, wait
return rx3270Connect(hostinfo,wait)

::method Disconnect
return rx3270Disconnect()

::method getConnectionState
return rx3270QueryCState()

::method isConnected
return getConnectionState() = "CONNECTED_TN3270E"

::method waitForEvents
return rx3270WaitForEvents()

::method getScreenContentAt
	use arg row, col, size
return rx3270ReadScreen(row,col,size)

::method getScreenContent
return rx3270ReadScreen()

::method waitForStringAt
	use arg row, col, key, timeout
return rx3270WaitForStringAt(row,col,key,timeout)

::method queryStringAt
	use arg row, col, key
return getScreenContentAt(row,col,length(key)) == key

::method isTerminalReady
return rx3270IsTerminalReady()

::method waitForTerminalReady
	use arg timeout
return rx3270WaitForTerminalReady(timeout)

::method setStringAt
	use arg row, col, str
return rx3270InputString(row,col,str)

::method sendEnterKey()
return rx3270SendENTERKey()

::method sendPFKey
	use arg key
return rx3270SendPFKey(key)



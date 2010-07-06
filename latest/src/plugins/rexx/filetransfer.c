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
 * Este programa está nomeado como filetransfer.c e possui - linhas de código.
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


 #include "rx3270.h"

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static int begin_ft(unsigned short flags, int argc, RXSTRING argv[])
 {
	static const struct _ftoptions
	{
		unsigned int	 	flag;
		const gchar		*arg;
	} ft_options[] 			=		{	{	FT_FLAG_ASCII,			"TEXT"		},
										{	FT_FLAG_TSO,			"TSO" 		},
										{	FT_FLAG_CRLF,			"CRLF"		},
										{	FT_FLAG_APPEND,			"APPEND"	},
										{   FT_FLAG_REMAP_ASCII,	"REMAP"		}
									};

	static const char *opt_name[] = { "LRECL", "BLKSIZE", "PRIMSPACE", "SECSPACE", "DFT" };

	int opt[5] = { 0, 0, 0, 0, 4096 };
	int arg;

	if(argc < 2)
		return EINVAL;

	for(arg=2;arg<argc;arg++)
	{
		int f;

		for(f=0;f<G_N_ELEMENTS(ft_options);f++)
		{
			if(!g_ascii_strcasecmp(argv[f].strptr,ft_options[f].arg))
				flags |= ft_options[f].flag;
		}

		for(f=0;f<G_N_ELEMENTS(opt_name);f++)
		{
			int sz = strlen(opt_name[f]);

			if(!g_ascii_strncasecmp(argv[f].strptr,opt_name[f],sz))
			{
				char *ptr = strchr(argv[f].strptr,'=');
				if(!ptr)
					return EINVAL;
				opt[f] = atoi(ptr+1);
			}
		}
	}

	return BeginFileTransfer(flags, argv[0].strptr, argv[1].strptr,opt[0],opt[1],opt[2],opt[3],opt[4]);
 }

 RexxReturnCode REXXENTRY rx3270BeginFileSend(PSZ Name, LONG Argc, RXSTRING Argv[], PSZ Queuename, PRXSTRING Retstr)
 {
 	return RetValue(Retstr,begin_ft(0,(int) Argc, Argv));
 }

 RexxReturnCode REXXENTRY rx3270BeginFileRecv(PSZ Name, LONG Argc, RXSTRING Argv[], PSZ Queuename, PRXSTRING Retstr)
 {
 	return RetValue(Retstr,begin_ft(FT_FLAG_RECEIVE,(int) Argc, Argv));
 }

 RexxReturnCode REXXENTRY rx3270GetFTState(PSZ Name, LONG Argc, RXSTRING Argv[], PSZ Queuename, PRXSTRING Retstr)
 {
	static const struct _state
	{
		const gchar 	*name;
		enum ft_state	state;
	} state[] =
	{
			{ "NONE",		FT_NONE			},
			{ "AWAIT_ACK",	FT_AWAIT_ACK	},
			{ "RUNNING",	FT_RUNNING		},
			{ "ABORT_WAIT",	FT_ABORT_WAIT	},
			{ "ABORT_SENT",	FT_ABORT_SENT	}
	};

	enum ft_state current_state = GetFileTransferState();
	int f;

	for(f = 0;f < G_N_ELEMENTS(state); f++)
	{
		if(current_state == state[f].state)
			return RetString(Retstr,state[f].name);
	}

	return RetValue(Retstr,(int) current_state);
 }

 RexxReturnCode REXXENTRY rx3270WaitForFTComplete(PSZ Name, LONG Argc, RXSTRING Argv[], PSZ Queuename, PRXSTRING Retstr)
 {
 	if(Argc)
		return RXFUNC_BADCALL;

	while(GetFileTransferState() != FT_NONE)
	{
		RunPendingEvents(1);
		if(IsHalted())
				return RetValue(Retstr,ECANCELED);
	}

	return RetValue(Retstr,0);
 }

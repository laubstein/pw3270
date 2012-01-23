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
 * Este programa está nomeado como host.c e possui 1078 linhas de código.
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


/*
 *	host.c
 *		This module handles the ibm_hosts file, connecting to and
 *		disconnecting from hosts, and state changes on the host
 *		connection.
 */

#include "globals.h"
#include "appres.h"
#include "resources.h"

#include "actionsc.h"
#include "hostc.h"
// #include "macrosc.h"
#include "menubarc.h"
#include "popupsc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "xioc.h"

#include <errno.h>

#define RECONNECT_MS		2000	/* 2 sec before reconnecting to host */
#define RECONNECT_ERR_MS	5000	/* 5 sec before reconnecting to host */

#define MAX_RECENT	5

// enum cstate	cstate = NOT_CONNECTED;
Boolean			std_ds_host = False;
Boolean			no_login_host = False;
Boolean			non_tn3270e_host = False;
Boolean			passthru_host = False;
Boolean			ssl_host = False;
Boolean			ever_3270 = False;

// char           *full_current_host = CN;
//unsigned short  current_port;
//char	       *reconnect_host = CN;
//char	       *qualified_host = CN;

struct host *hosts = (struct host *)NULL;
static struct host *last_host = (struct host *)NULL;
// static Boolean auto_reconnect_inprogress = False;
// static int net_sock = -1;

// #if defined(X3270_DISPLAY)
// static void save_recent(const char *);
// #endif

static void try_reconnect(H3270 *session);

static char * stoken(char **s)
{
	char *r;
	char *ss = *s;

	if (!*ss)
		return NULL;
	r = ss;
	while (*ss && *ss != ' ' && *ss != '\t')
		ss++;
	if (*ss) {
		*ss++ = '\0';
		while (*ss == ' ' || *ss == '\t')
			ss++;
	}
	*s = ss;
	return r;
}


/*
 * Read the host file
 */
void
hostfile_init(void)
{
	FILE *hf;
	char buf[1024];
	static Boolean hostfile_initted = False;
	struct host *h;
	char *hostfile_name;

	if (hostfile_initted)
		return;

	hostfile_initted = True;
	hostfile_name = appres.hostsfile;
	if (hostfile_name == CN)
		hostfile_name = xs_buffer("%s/ibm_hosts", appres.conf_dir);
	else
		hostfile_name = do_subst(appres.hostsfile, True, True);
	hf = fopen(hostfile_name, "r");
	if (hf != (FILE *)NULL) {
		while (fgets(buf, sizeof(buf), hf)) {
			char *s = buf;
			char *name, *entry_type, *hostname;
			char *slash;

			if (strlen(buf) > (unsigned)1 &&
			    buf[strlen(buf) - 1] == '\n') {
				buf[strlen(buf) - 1] = '\0';
			}
			while (isspace(*s))
				s++;
			if (!*s || *s == '#')
				continue;
			name = stoken(&s);
			entry_type = stoken(&s);
			hostname = stoken(&s);
			if (!name || !entry_type || !hostname) {
				popup_an_error("Bad %s syntax, entry skipped",
				    ResHostsFile);
				continue;
			}
			h = (struct host *)Malloc(sizeof(*h));
			if (!split_hier(NewString(name), &h->name,
						&h->parents)) {
				Free(h);
				continue;
			}
			h->hostname = NewString(hostname);

			/*
			 * Quick syntax extension to allow the hosts file to
			 * specify a port as host/port.
			 */
			if ((slash = strchr(h->hostname, '/')))
				*slash = ':';

			if (!strcmp(entry_type, "primary"))
				h->entry_type = PRIMARY;
			else
				h->entry_type = ALIAS;
			if (*s)
				h->loginstring = NewString(s);
			else
				h->loginstring = CN;
			h->prev = last_host;
			h->next = (struct host *)NULL;
			if (last_host)
				last_host->next = h;
			else
				hosts = h;
			last_host = h;
		}
		(void) fclose(hf);
	} else if (appres.hostsfile != CN) {
		popup_an_errno(errno, "Cannot open " ResHostsFile " '%s'",
				appres.hostsfile);
	}
	Free(hostfile_name);

// #if defined(X3270_DISPLAY)
// 	save_recent(CN);
// #endif /*]*/
}

/*
 * Look up a host in the list.  Turns aliases into real hostnames, and
 * finds loginstrings.
 */
static int
hostfile_lookup(const char *name, char **hostname, char **loginstring)
{
	struct host *h;

	hostfile_init();
	for (h = hosts; h != (struct host *)NULL; h = h->next) {
		if (h->entry_type == RECENT)
			continue;
		if (!strcmp(name, h->name)) {
			*hostname = h->hostname;
			if (h->loginstring != CN) {
				*loginstring = h->loginstring;
			} else {
				*loginstring = appres.login_macro;
			}
			return 1;
		}
	}
	return 0;
}

#if defined(LOCAL_PROCESS) /*[*/
/* Recognize and translate "-e" options. */
static const char *
parse_localprocess(const char *s)
{
	int sl = strlen(OptLocalProcess);

	if (!strncmp(s, OptLocalProcess, sl)) {
		if (s[sl] == ' ')
			return(s + sl + 1);
		else if (s[sl] == '\0') {
			char *r;

			r = getenv("SHELL");
			if (r != CN)
				return r;
			else
				return "/bin/sh";
		}
	}
	return CN;
}
#endif /*]*/

/*
 * Strip qualifiers from a hostname.
 * Returns the hostname part in a newly-malloc'd string.
 * 'needed' is returned True if anything was actually stripped.
 * Returns NULL if there is a syntax error.
 */
static char *
split_host(char *s, Boolean *ansi, Boolean *std_ds, Boolean *passthru,
	Boolean *non_e, Boolean *secure, Boolean *no_login, char *xluname,
	char **port, Boolean *needed)
{
	char *lbracket = CN;
	char *at = CN;
	char *r = NULL;
	Boolean colon = False;

	*ansi = False;
	*std_ds = False;
	*passthru = False;
	*non_e = False;
	*secure = False;
	*xluname = '\0';
	*port = CN;

	*needed = False;

	/*
	 * Hostname syntax is:
	 *  Zero or more optional prefixes (A:, S:, P:, N:, L:, C:).
	 *  An optional LU name separated by '@'.
	 *  A hostname optionally in square brackets (which quote any colons
	 *   in the name).
	 *  An optional port name or number separated from the hostname by a
	 *  space or colon.
	 * No additional white space or colons are allowed.
	 */

	/* Strip leading whitespace. */
	while (*s && isspace(*s))
		s++;
	if (!*s) {
		popup_an_error("Empty hostname");
		goto split_fail;
	}

	/* Strip trailing whitespace. */
	while (isspace(*(s + strlen(s) - 1)))
		*(s + strlen(s) - 1) = '\0';

	/* Start with the prefixes. */
	while (*s && *(s + 1) && isalpha(*s) && *(s + 1) == ':') {
		switch (*s) {
		case 'a':
		case 'A':
			*ansi = True;
			break;
		case 's':
		case 'S':
			*std_ds = True;
			break;
		case 'p':
		case 'P':
			*passthru = True;
			break;
		case 'n':
		case 'N':
			*non_e = True;
			break;
#if defined(HAVE_LIBSSL) /*[*/
		case 'l':
		case 'L':
			*secure = True;
			break;
#endif /*]*/
		case 'c':
		case 'C':
			*no_login = True;
			break;
		default:
			popup_an_error("Hostname syntax error:\n"
					"Option '%c:' not supported", *s);
			goto split_fail;
		}
		*needed = True;
		s += 2;

		/* Allow whitespace around the prefixes. */
		while (*s && isspace(*s))
			s++;
	}

	/* Process the LU name. */
	lbracket = strchr(s, '[');
	at = strchr(s, '@');
	if (at != CN && lbracket != CN && at > lbracket)
		at = CN;
	if (at != CN) {
		char *t;
		char *lu_last = at - 1;

		if (at == s) {
			popup_an_error("Hostname syntax error:\n"
					"Empty LU name");
			goto split_fail;
		}
		while (lu_last < s && isspace(*lu_last))
			lu_last--;
		for (t = s; t <= lu_last; t++) {
			if (isspace(*t)) {
				char *u = t + 1;

				while (isspace(*u))
					u++;
				if (*u != '@') {
					popup_an_error("Hostname syntax "
							"error:\n"
							"Space in LU name");
					goto split_fail;
				}
				break;
			}
			if (t - s < LUNAME_SIZE) {
				xluname[t - s] = *t;
			}
		}
		xluname[t - s] = '\0';
		s = at + 1;
		while (*s && isspace(*s))
			s++;
		*needed = True;
	}

	/*
	 * Isolate the hostname.
	 * At this point, we've found its start, so we can malloc the buffer
	 * that will hold the copy.
	 */
	if (lbracket != CN) {
		char *rbracket;

		/* Check for junk before the '['. */
		if (lbracket != s) {
			popup_an_error("Hostname syntax error:\n"
					"Text before '['");
			goto split_fail;
		}

		s = r = NewString(lbracket + 1);

		/*
		 * Take whatever is inside square brackets, including
		 * whitespace, unmodified -- except for empty strings.
		 */
		rbracket = strchr(s, ']');
		if (rbracket == CN) {
			popup_an_error("Hostname syntax error:\n"
					"Missing ']'");
			goto split_fail;
		}
		if (rbracket == s) {
			popup_an_error("Empty hostname");
			goto split_fail;
		}
		*rbracket = '\0';

		/* Skip over any whitespace after the bracketed name. */
		s = rbracket + 1;
		while (*s && isspace(*s))
			s++;
		if (!*s)
			goto split_success;
		colon = (*s == ':');
	} else {
		char *name_end;

		/* Check for an empty string. */
		if (!*s || *s == ':') {
			popup_an_error("Empty hostname");
			goto split_fail;
		}

		s = r = NewString(s);

		/* Find the end of the hostname. */
		while (*s && !isspace(*s) && *s != ':')
			s++;
		name_end = s;

		/* If the terminator is whitespace, skip the rest of it. */
		while (*s && isspace(*s))
			s++;

		/*
		 * If there's nothing but whitespace (or nothing) after the
		 * name, we're done.
		 */
		if (*s == '\0') {
			*name_end = '\0';
			goto split_success;
		}
		colon = (*s == ':');
		*name_end = '\0';
	}

	/*
	 * If 'colon' is set, 's' points at it (or where it was).  Skip
	 * it and any whitespace that follows.
	 */
	if (colon) {
		s++;
		while (*s && isspace(*s))
			s++;
		if (!*s) {
			popup_an_error("Hostname syntax error:\n"
					"Empty port name");
			goto split_fail;
		}
	}

	/*
	 * Set the portname and find its end.
	 * Note that trailing spaces were already stripped, so the end of the
	 * portname must be a NULL.
	 */
	*port = s;
	*needed = True;
	while (*s && !isspace(*s) && *s != ':')
		s++;
	if (*s != '\0') {
		popup_an_error("Hostname syntax error:\n"
				"Multiple port names");
		goto split_fail;
	}
	goto split_success;

split_fail:
	Free(r);
	r = CN;

split_success:
	return r;
}

static int do_connect(H3270 *hSession, const char *n)
{
	char nb[2048];		/* name buffer */
	char *s;		/* temporary */
	const char *chost;	/* to whom we will connect */
	char *target_name;
	char *ps = CN;
	char *port = CN;
	Boolean resolving;
	Boolean pending;
	static Boolean ansi_host;
	const char *localprocess_cmd = NULL;
	Boolean has_colons = False;

	if (CONNECTED || hSession->auto_reconnect_inprogress)
		return 0;

	/* Skip leading blanks. */
	while (*n == ' ')
		n++;
	if (!*n) {
		popup_an_error("Invalid (empty) hostname");
		return -1;
	}

	/* Save in a modifiable buffer. */
	(void) strcpy(nb, n);

	/* Strip trailing blanks. */
	s = nb + strlen(nb) - 1;
	while (*s == ' ')
		*s-- = '\0';

	/* Remember this hostname, as the last hostname we connected to. */
	Replace(hSession->reconnect_host, NewString(nb));

// #if defined(X3270_DISPLAY)
// 	/* Remember this hostname in the recent connection list and file. */
// 	save_recent(nb);
// #endif

#if defined(LOCAL_PROCESS) /*[*/
	if ((localprocess_cmd = parse_localprocess(nb)) != CN) {
		chost = localprocess_cmd;
		port = appres.port;
	} else
#endif /*]*/
	{
		Boolean needed;

		/* Strip off and remember leading qualifiers. */
		if ((s = split_host(nb, &ansi_host, &std_ds_host,
		    &passthru_host, &non_tn3270e_host, &ssl_host,
		    &no_login_host, hSession->luname, &port,
		    &needed)) == CN)
			return -1;

		/* Look up the name in the hosts file. */
		if (!needed && hostfile_lookup(s, &target_name, &ps)) {
			/*
			 * Rescan for qualifiers.
			 * Qualifiers, LU names, and ports are all overridden
			 * by the hosts file.
			 */
			Free(s);
			if (!(s = split_host(target_name, &ansi_host,
			    &std_ds_host, &passthru_host, &non_tn3270e_host,
			    &ssl_host, &no_login_host, hSession->luname, &port,
			    &needed)))
				return -1;
		}
		chost = s;

		/* Default the port. */
		if (port == CN)
			port = appres.port;
	}

	/*
	 * Store the original name in globals, even if we fail the connect
	 * later:
	 *  current_host is the hostname part, stripped of qualifiers, luname
	 *   and port number
	 *  full_current_host is the entire string, for use in reconnecting
	 */
	if (n != hSession->full_current_host)
	{
		Replace(hSession->full_current_host, NewString(n));
	}

	Replace(hSession->current_host, CN);

	if (localprocess_cmd != CN) {
		if (full_current_host[strlen(OptLocalProcess)] != '\0')
			hSession->current_host = NewString(full_current_host + strlen(OptLocalProcess) + 1);
		else
			hSession->current_host = NewString("default shell");
	} else {
		hSession->current_host = s;
	}

	has_colons = (strchr(chost, ':') != NULL);

	Replace(hSession->qualified_host,
	    xs_buffer("%s%s%s%s:%s",
		    ssl_host? "L:": "",
		    has_colons? "[": "",
		    chost,
		    has_colons? "]": "",
		    port));


	/* Attempt contact. */
	ever_3270 = False;
	hSession->net_sock = net_connect(chost, port, localprocess_cmd != CN, &resolving,&pending);

	if (hSession->net_sock < 0 && !resolving)
	{
		/* Redundantly signal a disconnect. */
		lib3270_st_changed(hSession, ST_CONNECT, False);
		return -1;
	}

	/* Still thinking about it? */
	if (resolving)
	{
		hSession->cstate = RESOLVING;
		lib3270_st_changed(hSession, ST_RESOLVING, True);
		return 0;
	}

	/* Success. */

	/* Set pending string. */
	if (ps == CN)
		ps = appres.login_macro;

//	if (ps != CN)
//		login_macro(ps);

	/* Prepare Xt for I/O. */
	x_add_input(hSession,hSession->net_sock);

	/* Set state and tell the world. */
	if (pending)
	{
		hSession->cstate = PENDING;
		lib3270_st_changed(hSession, ST_HALF_CONNECT, True);
	}
	else
	{
		hSession->cstate = CONNECTED_INITIAL;
		lib3270_st_changed(hSession, ST_CONNECT, True);
#if defined(X3270_DISPLAY) /*[*/
		if (toggled(RECONNECT) && error_popup_visible())
			popdown_an_error();
#endif /*]*/
	}

	return 0;
}

int lib3270_connect(H3270 *h, const char *n, int wait)
{
	if(!h)
		h = &h3270;

	RunPendingEvents(0);

	if(h->auto_reconnect_inprogress)
		return EAGAIN;

	if(PCONNECTED)
		return EBUSY;

	if(do_connect(h,n))
		return -1;

	if(wait)
	{
		while(!IN_ANSI && !IN_3270)
		{
			RunPendingEvents(1);

			if(!PCONNECTED)
			{
				return ENOTCONN;
			}
		}
	}

	return 0;
}

/*
 * Called from timer to attempt an automatic reconnection.
 */
static void try_reconnect(H3270 *session)
{
	WriteLog("3270","Starting auto-reconnect (Host: %s)",session->reconnect_host ? session->reconnect_host : "-");
	session->auto_reconnect_inprogress = False;
	lib3270_reconnect(session,0);
}

void host_disconnect(H3270 *h, int failed)
{
	if(!h)
		h = &h3270;

	if (CONNECTED || HALF_CONNECTED) {
		x_remove_input(h);
		net_disconnect();
		h->net_sock = -1;

		Trace("Disconnected (Failed: %d Reconnect: %d in_progress: %d)",failed,toggled(RECONNECT),h->auto_reconnect_inprogress);
		if (toggled(RECONNECT) && !h->auto_reconnect_inprogress)
		{
			/* Schedule an automatic reconnection. */
			h->auto_reconnect_inprogress = True;
			(void) AddTimeOut(failed ? RECONNECT_ERR_MS: RECONNECT_MS, h, try_reconnect);
		}

		/*
		 * Remember a disconnect from ANSI mode, to keep screen tracing
		 * in sync.
		 */
#if defined(X3270_TRACE) /*[*/
		if (IN_ANSI && toggled(SCREEN_TRACE))
			trace_ansi_disc();
#endif /*]*/

		h->cstate = NOT_CONNECTED;

		/* Propagate the news to everyone else. */
		lib3270_st_changed(h,ST_CONNECT, False);
	}
}

/* The host has entered 3270 or ANSI mode, or switched between them. */
void
host_in3270(enum cstate new_cstate)
{
	Boolean now3270 = (new_cstate == CONNECTED_3270 ||
			   new_cstate == CONNECTED_SSCP ||
			   new_cstate == CONNECTED_TN3270E);

	h3270.cstate = new_cstate;
	ever_3270 = now3270;
	st_changed(ST_3270_MODE, now3270);
}

void
host_connected(void)
{
	h3270.cstate = CONNECTED_INITIAL;
	st_changed(ST_CONNECT, True);

#if defined(X3270_DISPLAY) /*[*/
	if (toggled(RECONNECT) && error_popup_visible())
		popdown_an_error();
#endif /*]*/
}

#if defined(X3270_DISPLAY) /*[*/
/* Comparison function for the qsort. */
static int
host_compare(const void *e1, const void *e2)
{
	const struct host *h1 = *(const struct host **)e1;
	const struct host *h2 = *(const struct host **)e2;
	int r;

	if (h1->connect_time > h2->connect_time)
		r = -1;
	else if (h1->connect_time < h2->connect_time)
		r = 1;
	else
		r = 0;
#if defined(CFDEBUG) /*[*/
	printf("%s %ld %d %s %ld\n",
	    h1->name, h1->connect_time,
	    r,
	    h2->name, h2->connect_time);
#endif /*]*/
	return r;
}
#endif /*]*/

#if defined(CFDEBUG) /*[*/
static void
dump_array(const char *when, struct host **array, int nh)
{
	int i;

	printf("%s\n", when);
	for (i = 0; i < nh; i++) {
		printf(" %15s %ld\n", array[i]->name, array[i]->connect_time);
	}
}
#endif /*]*/

/*
#if defined(X3270_DISPLAY)
static void
save_recent(const char *hn)
{
	char *lcf_name = CN;
	FILE *lcf = (FILE *)NULL;
	struct host *h;
	struct host *rest = (struct host *)NULL;
	int n_ent = 0;
	struct host *h_array[(MAX_RECENT * 2) + 1];
	int nh = 0;
	int i, j;
	time_t t = time((time_t *)NULL);

	// Allocate a new entry.
	if (hn != CN) {
		h = (struct host *)Malloc(sizeof(*h));
		h->name = NewString(hn);
		h->parents = NULL;
		h->hostname = NewString(hn);
		h->entry_type = RECENT;
		h->loginstring = CN;
		h->connect_time = t;
		h_array[nh++] = h;
	}

	// Put the existing entries into the array.
	for (h = hosts; h != (struct host *)NULL; h = h->next) {
		if (h->entry_type != RECENT)
			break;
		h_array[nh++] = h;
	}

	// Save the ibm_hosts entries for later.
	rest = h;
	if (rest != (struct host *)NULL)
		rest->prev = (struct host *)NULL;

	//
	// Read the last-connection file, to capture the any changes made by
	// other instances of x3270.
	//
	if (appres.connectfile_name != CN &&
	    strcasecmp(appres.connectfile_name, "none")) {
		lcf_name = do_subst(appres.connectfile_name, True, True);
		lcf = fopen(lcf_name, "r");
	}
	if (lcf != (FILE *)NULL) {
		char buf[1024];

		while (fgets(buf, sizeof(buf), lcf) != CN) {
			int sl;
			time_t connect_time;
			char *ptr;

			// Pick apart the entry.
			sl = strlen(buf);
			if (buf[sl - 1] == '\n')
				buf[sl-- - 1] = '\0';
			if (!sl ||
			    buf[0] == '#' ||
			    (connect_time = strtoul(buf, &ptr, 10)) == 0L ||
			    ptr == buf ||
			    *ptr != ' ' ||
			    !*(ptr + 1))
				continue;

			h = (struct host *)Malloc(sizeof(*h));
			h->name = NewString(ptr + 1);
			h->parents = NULL;
			h->hostname = NewString(ptr + 1);
			h->entry_type = RECENT;
			h->loginstring = CN;
			h->connect_time = connect_time;
			h_array[nh++] = h;
			if (nh > (MAX_RECENT * 2) + 1)
				break;
		}
		fclose(lcf);
	}

	// Sort the array, in reverse order by connect time.
#if defined(CFDEBUG)
	dump_array("before", h_array, nh);
#endif
	qsort(h_array, nh, sizeof(struct host *), host_compare);
#if defined(CFDEBUG)
	dump_array("after", h_array, nh);
#endif

	//
	// Filter out duplicate host names, and limit the array to
	//MAX_RECENT entries total.
	//
	hosts = (struct host *)NULL;
	last_host = (struct host *)NULL;
	for (i = 0; i < nh; i++) {
		h = h_array[i];
		if (h == (struct host *)NULL)
			continue;
		h->next = (struct host *)NULL;
		if (last_host != (struct host *)NULL)
			last_host->next = h;
		h->prev = last_host;
		last_host = h;
		if (hosts == (struct host *)NULL)
			hosts = h;
		n_ent++;

		// Zap the duplicates.
		for (j = i+1; j < nh; j++) {
			if (h_array[j] &&
			    (n_ent >= MAX_RECENT ||
			     !strcmp(h_array[i]->name, h_array[j]->name))) {
#if defined(CFDEBUG)
				printf("%s is a dup of %s\n",
				    h_array[j]->name, h_array[i]->name);
#endif
				Free(h_array[j]->name);
				Free(h_array[j]->hostname);
				Free(h_array[j]);
				h_array[j] = (struct host *)NULL;
			}
		}
	}

	// Re-attach the ibm_hosts entries to the end.
	if (rest != (struct host *)NULL) {
		if (last_host != (struct host *)NULL) {
			last_host->next = rest;
		} else {
			hosts = rest;
		}
		rest->prev = last_host;
	}

	// If there's been a change, rewrite the file.
	if (hn != CN &&
	    appres.connectfile_name != CN &&
	    strcasecmp(appres.connectfile_name, "none")) {
		lcf = fopen(lcf_name, "w");
		if (lcf != (FILE *)NULL) {
			fprintf(lcf, "# Created %s# by %s\n", ctime(&t), build);
			for (h = hosts; h != (struct host *)NULL; h = h->next) {
				if (h->entry_type != RECENT)
					break;
				(void) fprintf(lcf, "%lu %s\n", h->connect_time,
				    h->name);
			}
			fclose(lcf);
		}
	}
	if (lcf_name != CN)
		Free(lcf_name);
}
#endif
*/

/* Support for state change callbacks. */

/*
struct st_callback
{
	struct st_callback	* next;
	H3270				* session;
	void				* data;
	void (*func)(H3270 *, int, void *);
};

static struct st_callback *st_callbacks[N_ST];
static struct st_callback *st_last[N_ST];
*/

/* Register a function interested in a state change. */
LIB3270_EXPORT void lib3270_register_schange(H3270 *h,LIB3270_STATE_CHANGE tx, void (*func)(H3270 *, int, void *),void *data)
{
	struct lib3270_state_callback *st;

	if(!h)
		h = &h3270;

	st = (struct lib3270_state_callback *)Malloc(sizeof(*st));

	st->func	= func;
	st->next	= (struct lib3270_state_callback *)NULL;

	if (h->st_last[tx] != (struct lib3270_state_callback *)NULL)
		h->st_last[tx]->next = st;
	else
		h->st_callbacks[tx] = st;
	h->st_last[tx] = st;

}

/*
void register_schange(LIB3270_STATE_CHANGE tx, void (*func)(H3270 *, int))
{
	struct st_callback *st;

	st = (struct st_callback *)Malloc(sizeof(*st));

	st->func	= func;
	st->next	= (struct st_callback *)NULL;

	if (st_last[tx] != (struct st_callback *)NULL)
		st_last[tx]->next = st;
	else
		st_callbacks[tx] = st;
	st_last[tx] = st;
}
*/

/* Signal a state change. */
void lib3270_st_changed(H3270 *h, int tx, int mode)
{
	struct lib3270_state_callback *st;

	if(!h)
		h = &h3270;

	for (st = h->st_callbacks[tx];st != (struct lib3270_state_callback *)NULL;st = st->next)
	{
		(*st->func)(h,mode,st->data);
	}
}

LIB3270_EXPORT int lib3270_reconnect(H3270 *h,int wait)
{
	int rc;

	if(!h)
		h = &h3270;

	if (CONNECTED || HALF_CONNECTED)
		return EBUSY;

	if (h->current_host == CN)
		return ENOENT;

	if (h->auto_reconnect_inprogress)
		return EBUSY;

	rc = lib3270_connect(h,h->reconnect_host,wait);

	if(rc)
	{
		h->auto_reconnect_inprogress = False;
		return rc;
	}

	return 0;
}

LIB3270_EXPORT const char	* get_connected_lu(H3270 *h)
{
	if(h)
		return h->connected_lu;
	return h3270.connected_lu;
}

LIB3270_EXPORT const char	* get_current_host(H3270 *h)
{
	if(h)
		return h->current_host;
	return h3270.current_host;
}

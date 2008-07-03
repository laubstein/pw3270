/*
 * Modifications and original code Copyright 1993, 1994, 1995, 1996,
 *    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *   All Rights Reserved.  GTRC hereby grants public use of this software.
 *   Derivative works based on this software must incorporate this copyright
 *   notice.
 *
 * c3270 and wc3270 are distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	api.h
 *		lib3270 API calls
 */


#ifndef LIB3270_H_INCLUDED

		#define LIB3270_H_INCLUDED "2.0"

		/* Debug & log */
		#ifdef DEBUG
			#define Trace( fmt, ... )		WriteLog("TRACE", "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ )
		#else
			#define Trace( fmt, ... )	/* __VA_ARGS__ */
			#warning "So compilar como debug"
		#endif

		int WriteLog(const char *module, const char *fmt, ...);
		int WriteRCLog(const char *module, int rc, const char *fmt, ...);


		#define Log(fmt, ...)		WriteLog("MSG",fmt,__VA_ARGS__)

		/* I/O processing */
		struct lib3270_io_callbacks
		{
			unsigned short sz;

			unsigned long (*AddTimeOut)(unsigned long interval_ms, void (*proc)(void));
			void 			(*RemoveTimeOut)(unsigned long timer);

			unsigned long (*AddInput)(int source, void (*fn)(void));
			void			(*RemoveInput)(unsigned long id);

			unsigned long (*AddExcept)(int source, void (*fn)(void));

			#if !defined(_WIN32) /*[*/
				unsigned long (*AddOutput)(int source, void (*fn)(void));
			#endif /*]*/


		};

		int Register3270IOCallbacks(const struct lib3270_io_callbacks *cbk);

		/* Screen processing */
		typedef enum _CURSOR_MODE
		{
			CURSOR_MODE_NORMAL,
			CURSOR_MODE_WAITING,
			CURSOR_MODE_LOCKED,

			CURSOR_MODE_USER
		} CURSOR_MODE;

		typedef enum _STATUS_CODE
		{
			STATUS_CODE_BLANK,
			STATUS_CODE_SYSWAIT,
			STATUS_CODE_TWAIT,
			STATUS_CODE_CONNECTED,
			STATUS_CODE_DISCONNECTED,
			STATUS_CODE_AWAITING_FIRST,
			STATUS_CODE_MINUS,
			STATUS_CODE_PROTECTED,
			STATUS_CODE_NUMERIC,
			STATUS_CODE_OVERFLOW,
			STATUS_CODE_INHIBIT,


			STATUS_CODE_X,
			STATUS_CODE_USER
		} STATUS_CODE;

		typedef enum _OIA_FLAG
		{
			OIA_FLAG_BOXSOLID,
			OIA_FLAG_UNDERA,
			OIA_FLAG_SECURE,
		} OIA_FLAG;

		struct lib3270_screen_callbacks
		{
			unsigned short sz;

			void (*init)(void);
			void (*Error)(const char *s);
			void (*Warning)(const char *s);
			void (*setsize)(int rows, int cols);
			int  (*addch)(int row, int col, int c, unsigned short attr);
			void (*charset)(char *dcs);
			void (*title)(char *text);
			void (*changed)(int bstart, int bend);
			void (*ring_bell)(void);
			void (*redraw)(void);
			void (*move_cursor)(int row, int col);
			void (*suspend)(void);
			void (*resume)(void);
			void (*reset)(int lock, const char *msg);
			void (*status)(STATUS_CODE id);
			void (*typeahead)(int on);
			void (*printer)(int on);
			void (*compose)(int on, unsigned char c, int keytype);
			void (*oia_flag)(OIA_FLAG id, int on);
			void (*cursor)(CURSOR_MODE mode);

		};

		int Register3270ScreenCallbacks(const struct lib3270_screen_callbacks *cbk);

		/* Misc API calls */
		void Input_String(const unsigned char *str);


#endif // LIB3270_H_INCLUDED

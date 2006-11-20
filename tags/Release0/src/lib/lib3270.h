/*
 * Copyright 1999 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */


#ifndef LIB3270_INCLUDED

#define LIB3270_INCLUDED

#ifdef __cplusplus
 extern "C" {
#endif

#pragma pack(1)

#define SCREEN_MAGIC 	0x4871
#define KEYBOARD_MAGIC	0x4781

typedef struct _screen_callback
{
	unsigned short sz;
	unsigned short magic;

    void (*screen_init)(void);
    void (*screen_disp)(Boolean erasing, const struct ea *display);
    void (*screen_suspend)(void);
    void (*screen_resume)(void);
    void (*screen_type)(const char *model_name, int maxROWS, int maxCOLS);
    void (*screen_changed)(int first, int last);

    void (*cursor_move)(int baddr);
    void (*toggle_monocase)(struct toggle *t, enum toggle_type tt);

    void (*status_ctlr_done)(void);
    void (*status_insert_mode)(Boolean on);
    void (*status_minus)(void);
    void (*status_oerr)(int error_type);
    void (*status_reset)(void);
    void (*status_reverse_mode)(Boolean on);
    void (*status_syswait)(void);
    void (*status_twait)(void);
    void (*status_typeahead)(Boolean on);
    void (*status_compose)(Boolean on, unsigned char c, enum keytype keytype);
    void (*status_lu)(const char *lu);

    void (*ring_bell)(void);
    void (*screen_flip)(void);
    void (*screen_width)(int width);

    void (*error_popup)(const char *msg);

    void (*Redraw_action)(Widget w, XEvent *event, String *params, Cardinal *num_params);

	void (*status_timing)(struct timeval *t0, struct timeval *t1);
	void (*status_untiming)(void);

} SCREEN_CALLBACK;

typedef struct _keytable
{
	const char *name;
    int code;
} KEYTABLE;

/* Input events. */
#define InputReadMask	0x1
#define InputExceptMask	0x2
#define InputWriteMask	0x4

typedef struct input_3270
{
	struct input_3270 *next;
    int source;			/**< The file descriptor to poll */
    int condition;		/**< Bit field to indicate the type of handle (InputReadMask, InputExceptMask or InputWriteMask) */
    void (*proc)(void); /**< The function who process the file handle */
} INPUT_3270;

typedef struct timeout_3270 {
	struct timeout_3270 *next;
	struct timeval tv;
	void (*proc)(void);
	Boolean in_play;
} TIMEOUT_3270;

typedef struct _keyboard_info
{
	unsigned short sz;
	unsigned short magic;

	void (*beep)(void);

	void (*SourceAdded)(const INPUT_3270 *ip);
	void (*SourceRemoved)(const INPUT_3270 *ip);

	const KEYTABLE	*keys;
} KEYBOARD_INFO;

#pragma pack()

/*---[ Prototipes ]-----------------------------------------------------------*/

int set_3270_screen(const SCREEN_CALLBACK *scr);
int set_3270_keyboard(const KEYBOARD_INFO *kbd);

int Run_3270(const char	*cl_hostname);
int Initialize_3270(void);

const struct ea 	 *Get3270DeviceBuffer(int *rows, int *cols);
const struct ea      *QueryDeviceChar(int baddr);

const struct timeval *Check3270Timeouts(struct timeval *tp);


extern const INPUT_3270 * Query3270SourceList(void);

extern int   parse_3270_command_line(int argc, const char **argv, const char **cl_hostname);
extern void  register_3270_schange(int tx, void (*func)(Boolean));

extern const SCREEN_CALLBACK *screen_callbacks_3270;
extern const KEYBOARD_INFO   *keyboard_info_3270;

extern void  display_3270(Boolean erasing);
extern void  move3270Cursor(int baddr);
extern void  RemoveSelectionBox(void);

extern int   Get3270CursorRow(void);


#ifdef __cplusplus
 }
#endif

/* state changes */
#define ST_RESOLVING	 1
#define ST_HALF_CONNECT	 2
#define ST_CONNECT	     3
#define ST_3270_MODE	 4
#define ST_LINE_MODE	 5
#define ST_REMODEL	     6
#define ST_PRINTER	     7
#define ST_EXITING	     8
#define ST_CHARSET    	 9
#define N_ST		    10



#endif // LIB3270_INCLUDED

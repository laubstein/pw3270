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
    void (*screen_disp)(Boolean erasing);
    void (*screen_suspend)(void);
    void (*screen_resume)(void);

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

    void (*Redraw_action)(Widget w, XEvent *event, String *params, Cardinal *num_params);

} SCREEN_CALLBACK;

typedef struct _keytable
{
	const char *name;
    int code;
} KEYTABLE;

typedef struct _keyboard_info
{
	unsigned short sz;
	unsigned short magic;

	void (*beep)(void);

	const KEYTABLE	*keys;
} KEYBOARD_INFO;

#pragma pack()

int set_3270_screen(const SCREEN_CALLBACK *scr);
int set_3270_keyboard(const KEYBOARD_INFO *kbd);
int run_emulator(const char	*cl_hostname);


extern const SCREEN_CALLBACK *screen_callbacks_3270;
extern const KEYBOARD_INFO   *keyboard_info_3270;

#ifdef __cplusplus
 }
#endif


#endif // LIB3270_INCLUDED

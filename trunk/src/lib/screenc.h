/*
 * Copyright 1999, 2000, 2002 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * c3270 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/* c3270 version of screenc.h */


#pragma pack(1)

#define SCREEN_MAGIC 0x4871

typedef struct _screen_callback
{
	unsigned short sz;
	unsigned short magic;

    void (*screen_init)(void);
    void (*screen_disp)(Boolean erasing unused);
    void (*screen_suspend)(void);
    void (*screen_resume)(void);

    void (*cursor_move)(int baddr);
    void (*toggle_monocase)(struct toggle *t unused, enum toggle_type tt unused);

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

#pragma pack()

extern SCREEN_CALLBACK *screen_callbacks_3270;

#define blink_start()
#define display_heightMM()	100
#define display_height()	1
#define display_widthMM()	100
#define display_width()		1
#define mcursor_locked()
#define mcursor_normal()
#define mcursor_waiting()
#define screen_obscured()	False
#define screen_scroll()

extern void cursor_move(int baddr);
extern void cursor_move(int baddr);
extern void ring_bell(void);
extern void ring_bell(void);
extern void screen_132(void);
extern void screen_132(void);
extern void screen_80(void);
extern void screen_80(void);
extern void screen_disp(Boolean erasing);
extern void screen_disp(Boolean erasing);
extern void screen_init(void);
extern void screen_init(void);
extern void screen_flip(void);
extern void screen_flip(void);
extern void screen_resume(void);
extern void screen_suspend(void);
extern void toggle_monocase(struct toggle *t, enum toggle_type tt);

extern FILE *start_pager(void);

extern Boolean escaped;

extern void Escape_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
extern void Help_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
extern void Redraw_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
extern void Trace_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
extern void Show_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);


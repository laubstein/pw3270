

 #include "g3270.h"
 #include "lib/kybdc.h"
 #include "lib/actionsc.h"


/*---[ Defines ]--------------------------------------------------------------*/

 typedef struct _position
 {
 	unsigned char	updated;
 	long 			x;
 	long 			y;
 	long			row;
 	long			col;
 } POSITION;

 enum _selection_positions
 {
 	SELECTION_BEGIN,
 	SELECTION_END,

 	SELECTION_COUNT
 };

 typedef struct _cursor
 {
 	GdkCursor		*cursor;
 	GdkCursorType	type;
 } CURSOR;

/*---[ Prototipes ]-----------------------------------------------------------*/

/*---[ Constants ]------------------------------------------------------------*/

/*---[ Statics ]--------------------------------------------------------------*/

 static POSITION 	pos[SELECTION_COUNT];
 static GdkCursor 	*pointer	= 0;

 static int			selected_cursor = -1;
 static CURSOR		cursor[]		= {	{	0,	GDK_BOTTOM_RIGHT_CORNER }
										};

/*---[ Globals ]--------------------------------------------------------------*/

 unsigned short selecting = 0;

/*---[ Implement ]------------------------------------------------------------*/

 static void InvalidateSelectionBox(void)
 {
 	int rows;
 	int cols;
 	int f;

    if(!Get3270DeviceBuffer(&rows, &cols))
       return;

    for(f=0;f<SELECTION_COUNT;f++)
    {
    	if(pos[f].updated)
    	{
            pos[f].col = (pos[f].x - left_margin) / font->Width;

            if(pos[f].col < 0)
               pos[f].col = 0;

			if(pos[f].col > cols)
			   pos[f].col = cols;

            pos[f].row = (pos[f].y - top_margin)  / (font->Height + line_spacing);

			if(pos[f].row < 0)
			   pos[f].row = 0;

			if(pos[f].row > rows)
			   pos[f].row = rows;

			pos[f].x   = (pos[f].col * font->Width) + left_margin;
			pos[f].y   = (pos[f].row * (font->Height+line_spacing))+top_margin;

    		pos[f].updated = 0;

    	}
    }

 	RedrawTerminalContents();
 }

 void ConfigureSelectionBox(void)
 {
 	int f;

 	if(!selecting)
 	   return;

	DBGPrintf("Select from %ldx%ld to %ldx%ld",
						pos[0].row,pos[0].col,
						pos[1].row,pos[1].col);

    for(f=0;f<SELECTION_COUNT;f++)
    {
	   pos[f].updated 	= 1;
	   pos[f].x   		= (pos[f].col * font->Width) + left_margin;
	   pos[f].y   		= (pos[f].row * (font->Height+line_spacing))+top_margin;
    }

    InvalidateSelectionBox();
 }

 static void SelectCursor(int cr)
 {
 	if(cr == selected_cursor)
 	   return;

	selected_cursor = cr;

    if(terminal && terminal->window)
    {
 	   if(cr < 0)
 	   {
	      if(pointer)
             gdk_window_set_cursor(terminal->window,pointer);
	      return;
 	   }
       gdk_window_set_cursor(terminal->window,cursor[cr].cursor);
    }
 }

 void DrawSelectionBox(GdkDrawable *drawable, GdkGC *gc)
 {
 	int x[2];
 	int y[2];

 	if(!selecting)
 	   return;

    x[0] = min(pos[SELECTION_BEGIN].x,pos[SELECTION_END].x);
    x[1] = max(pos[SELECTION_BEGIN].x,pos[SELECTION_END].x);

    y[0] = min(pos[SELECTION_BEGIN].y,pos[SELECTION_END].y);
    y[1] = max(pos[SELECTION_BEGIN].y,pos[SELECTION_END].y);

    if( (x[0] == x[1]) || (y[0] == y[1]) )
       return;

    gdk_gc_set_foreground(gc,selection_cmap);
    gdk_draw_rectangle(drawable,gc,1,x[0],y[0],x[1]-x[0],y[1]-y[0]);

    gdk_gc_set_foreground(gc,selection_cmap+1);
    gdk_draw_rectangle(drawable,gc,0,x[0],y[0],x[1]-x[0],y[1]-y[0]);

 }

 gboolean mouse_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {

    DBGTracex(event->button);
    DBGTracex(event->type);

    switch( ((event->type & 0x0F) << 4) | (event->button & 0x0F))
 	{
	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 1:
	   DBGMessage("Button 1 Click");

	   pos[SELECTION_BEGIN].updated =
       pos[SELECTION_END].updated   = 1;

	   pos[SELECTION_BEGIN].x       =
       pos[SELECTION_END].x         = (unsigned long) event->x;

       pos[SELECTION_BEGIN].y       =
       pos[SELECTION_END].y         = (unsigned long) event->y;

       selecting = 1;
	   break;

	case ((GDK_2BUTTON_PRESS & 0x0F) << 4) | 1:
	   DBGMessage("Button 1 Double-Click");
	   break;

	case ((GDK_BUTTON_PRESS & 0x0F) << 4) | 3:
	   DBGMessage("Button 2 Click");
	   break;

	case ((GDK_2BUTTON_PRESS & 0x0F) << 4) | 3:
	   DBGMessage("Button 2 Double-Click");
	   break;

	default:
	   DBGPrintf("Unknown mouse click %d with button %d",event->type, event->button);
	   return 0;

 	}

    InvalidateSelectionBox();

 	return 0;
 }

 gboolean mouse_button_release(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
 {
    int rows, cols;

    switch(selecting)
    {
	case 1:	// Single click!

	   DBGPrintf("Move cursor position to %ld,%ld",pos[0].row,pos[0].col);

	   if(Get3270DeviceBuffer(&rows, &cols))
          move3270Cursor((pos[0].row * cols) + pos[0].col);

	   selecting = 0;

	   break;

	case 2:	// Selecting box
	   DBGMessage("Finish selection");
	   break;
    }

    SelectCursor(-1);
    return 0;
 }

 gboolean mouse_motion(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
 {
    switch(selecting)
    {
	case 1:	// Single click
	case 2: // Selecting
	   pos[SELECTION_END].updated	= 1;
       pos[SELECTION_END].x         = (unsigned long) event->x;
       pos[SELECTION_END].y         = (unsigned long) event->y;
       selecting = 2;
       SelectCursor(0);
       InvalidateSelectionBox();
	   break;

    }

    return 0;
 }

 #if GTK == 2

 gboolean mouse_scroll(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
 {
    // FIXME (perry#1#): Read associoation from scroll to function key from configuration file.
    static const char *scroll[] = { "8", "7" };

    CHKPoint();

    switch(selecting)
    {
	case 0:		// No selection in progress, act as page-up/page-down
 	   if(event->direction < 2 && !WaitForScreen)
 	   {
          WaitForScreen = TRUE;
          action_internal(PF_action, IA_DEFAULT, scroll[event->direction], CN);
 	   }
 	   return 0;

    case 1:		// It's clicked
    case 2:		// It's selection
       return 0;

    }

 	return 0;
 }

#endif

 void LoadMousePointers(GdkDrawable *drawable, GdkGC *gc)
 {
 	int f;
 	for(f = 0; f < (sizeof(cursor)/sizeof(CURSOR)); f++)
 		cursor[f].cursor = gdk_cursor_new(cursor[f].type);
 }

 void SetMousePointer(GdkCursor *cursor)
 {
 	if(!cursor)
 	   return;

    pointer = cursor;

    DBGTracex(pointer);

    if(!selecting)
    {
	   if(terminal && terminal->window && pointer)
	      gdk_window_set_cursor(terminal->window,pointer);
    }
 }

 void action_remove_selection(GtkWidget *w, gpointer data)
 {
 	selecting = 0;
 	InvalidateSelectionBox();
 }

 void action_select_all(GtkWidget *w, gpointer data)
 {
 	int rows;
 	int cols;

    if(!Get3270DeviceBuffer(&rows, &cols))
       return;

    pos[0].updated =
    pos[1].updated = 1;

    pos[0].x       =
    pos[0].y	   = 0;

	pos[1].x       = (cols * font->Width) + left_margin;
	pos[1].y       = (rows * (font->Height+line_spacing))+top_margin;

 	selecting = 2;
 	InvalidateSelectionBox();
 }

 #define GetTerminalSize()	int rows, cols ; \
							if(!Get3270DeviceBuffer(&rows, &cols)) return;


 void action_SelectRight(GtkWidget *w, gpointer data)
 {
 	GetTerminalSize();

    if(!selecting)
    {
       selecting  = 99;
       pos[0].col =
       pos[1].col = cursor_col;
       pos[0].row = cursor_row;
       pos[1].row = cursor_row+1;
    }

	if(pos[1].col < cols)
	   pos[1].col++;

    ConfigureSelectionBox();

 }

 void action_SelectDown(GtkWidget *w, gpointer data)
 {
 	GetTerminalSize();

    if(!selecting)
    {
       selecting  = 99;
       pos[0].col = cursor_col;
       pos[1].col = cursor_col+1;
       pos[0].row =
       pos[1].row = cursor_row;
    }

	if(pos[1].row < rows)
	   pos[1].row++;

    ConfigureSelectionBox();

 }

 void action_SelectLeft(GtkWidget *w, gpointer data)
 {
    if(!selecting)
    {
       selecting = 99;
       pos[0].col =
       pos[1].col = cursor_col;
       pos[0].row = cursor_row;
       pos[1].row = cursor_row+1;
    }

	if(pos[1].col > 0)
	   pos[1].col--;

    ConfigureSelectionBox();

 }

 void action_SelectUp(GtkWidget *w, gpointer data)
 {
 	GetTerminalSize();

    if(!selecting)
    {
       selecting  = 99;
       pos[0].col = cursor_col;
       pos[1].col = cursor_col+1;
       pos[0].row =
       pos[1].row = cursor_row;
    }

	if(pos[1].row > 0)
	   pos[1].row--;

    ConfigureSelectionBox();

 }

 void action_SelectionUp(GtkWidget *w, gpointer data)
 {
    GetTerminalSize();

	if(!selecting)
	   return;

    if(pos[0].row <= 1 || pos[1].row < 1)
       return;

	CHKPoint();

    pos[0].row--;
    pos[1].row--;
    ConfigureSelectionBox();

 }

 void action_SelectionDown(GtkWidget *w, gpointer data)
 {
    GetTerminalSize();

	if(!selecting)
	   return;

    if(pos[0].row >= rows || pos[1].row >= rows)
       return;

	CHKPoint();

    pos[0].row++;
    pos[1].row++;
    ConfigureSelectionBox();
 }

 void action_SelectionLeft(GtkWidget *w, gpointer data)
 {
    GetTerminalSize();

	if(!selecting)
	   return;

    if(pos[0].col < 1 || pos[1].col < 1)
       return;

	CHKPoint();

    pos[0].col--;
    pos[1].col--;
    ConfigureSelectionBox();
 }

 void action_SelectionRight(GtkWidget *w, gpointer data)
 {
    GetTerminalSize();

	if(!selecting)
	   return;

    if(pos[0].col >= cols || pos[1].col >= cols)
       return;

	CHKPoint();

    pos[0].col++;
    pos[1].col++;
    ConfigureSelectionBox();
 }


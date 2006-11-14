
 #include "g3270.h"
 #include "trace.h"

 #include "lib/kybdc.h"


/*---[ Globals ]--------------------------------------------------------------*/

 char *Clipboard  = 0;
 int  szClipboard = 0;

/*---[ Implement ]------------------------------------------------------------*/

 void InitClipboard(GtkWidget *w)
 {
#ifdef USE_SELECTION

#endif
 }

 int CopyToClipboard(int fromRow, int fromCol, int toRow, int toCol)
 {
 	DBGTracex(Clipboard);
 	if(Clipboard)
 	{
 		DBGMessage("Release old clipboard area");
 		szClipboard = 0;
 		g_free(Clipboard);
 		Clipboard = 0;
 	}
 	return AppendToClipboard(fromRow,fromCol,toRow,toCol);
 }


 int AppendToClipboard(int fromRow, int fromCol, int toRow, int toCol)
 {
#ifdef USE_CLIPBOARD
    gchar			*string;
#endif

 	char			*buffer;
 	int				rows;
 	int				cols;
 	const struct ea *screen;
 	const struct ea *trm;
 	char			*ptr;
 	char			*mark;
 	int				sz;

    int col;
    int row;

 	int bCol = min(fromCol,toCol);
 	int fCol = max(fromCol,toCol);

 	int bRow = min(fromRow,toRow);
 	int fRow = min(fromRow,toRow);

 	screen = Get3270DeviceBuffer(&rows, &cols);

 	DBGPrintf("Adding area from %d,%d to %d,%d to clipboard",bRow,bCol,fRow,fCol);

    buffer = g_malloc(((rows+1) * cols)+5);

    if(!buffer)
    {
    	Log("Can't allocate memory for temporary clipboard area");
    	return -1;
    }

    ptr = buffer;
    for(row=fromRow;row<toRow;row++)
    {
    	trm  = screen + ((row * cols)+fromCol);
    	mark = ptr;

    	for(col=fromCol;col<toCol;col++)
    	{
            *ptr = Ebc2ASC(trm->cc);
    		if(*ptr > ' ')
    		   mark = ptr;
    		ptr++;
    		trm++;
    	}
    	ptr = (mark+1);
    	*(ptr++) = '\n';

    }

    if(ptr == buffer)
    {
    	DBGMessage("No clipboard info");
    	return -1;
    }
    *ptr = 0;

    if(!strlen(buffer))
    {
    	DBGMessage("Empty buffer");
    	return -1;
    }

#ifdef USE_CLIPBOARD

    string = g_convert(buffer, -1,"UTF-8", "ISO-8859-1", NULL, NULL, NULL);
    if(!string)
    {
    	Log("Error converting clipboard string to UTF-8");
    	return -1;
    }

	sz = strlen(string);

    if(!Clipboard)
    {
    	ptr         =
    	Clipboard   = g_malloc((szClipboard = sz)+5);
    }
    else
    {
    	DBGPrintf("Resizing clipboard (%d+%d)",szClipboard,sz);
    	Clipboard    = g_realloc(Clipboard,szClipboard+sz+5);
    	ptr		     = Clipboard + szClipboard;
    	szClipboard += sz;
    }

    if(!Clipboard)
    {
    	Log("Memory allocation error when saving clipboard info");
        g_free(string);
    	return -1;
    }

	strcpy(ptr,string);

    g_free(string);

#else

	sz = strlen(buffer);

    if(!Clipboard)
    {
    	ptr         =
    	Clipboard   = g_malloc((szClipboard = sz)+5);
    }
    else
    {
    	DBGPrintf("Resizing clipboard (%d+%d)",szClipboard,sz);
    	Clipboard    = g_realloc(Clipboard,szClipboard+sz+5);
    	ptr		     = Clipboard + szClipboard;
    	szClipboard += sz;
    }

    if(!Clipboard)
    {
    	Log("Memory allocation error when saving clipboard info");
    	return -1;
    }

	strcpy(ptr,buffer);

#endif

#ifdef DEBUG
    DBGPrintf("Clipboard contents (%d bytes):",szClipboard);
    fprintf(DBGFILE,"%s\n",Clipboard);
    fflush(DBGFILE);
#endif

    g_free(buffer);

#ifdef USE_CLIPBOARD

    gtk_clipboard_set_text(	gtk_widget_get_clipboard(terminal,GDK_SELECTION_CLIPBOARD),
                            Clipboard,-1);
#else

    #error Clipboard is not implemented

#endif
    return 0;
 }

#ifdef USE_CLIPBOARD
 static void paste_clipboard(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
    gchar *string;

    string = g_convert(text, -1, "ISO-8859-1", "UTF-8", NULL, NULL, NULL);
    if(!string)
    {
    	Log("Error converting clipboard string to ISO-8859-1");
    	return;
    }

    emulate_input(string, strlen(string), True);
    g_free(string);

 }
#endif

 void action_paste(GtkWidget *w, gpointer data)
 {
#ifdef USE_CLIPBOARD
    gtk_clipboard_request_text(	gtk_widget_get_clipboard(terminal,GDK_SELECTION_CLIPBOARD),
								paste_clipboard,
								0 );
#else

    #error Clipboard is not implemented

#endif
 }



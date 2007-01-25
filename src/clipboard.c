
 #include "g3270.h"
 #include "trace.h"

 #include "lib/kybdc.h"
 #include "lib/tablesc.h"


/*---[ Globals ]--------------------------------------------------------------*/

 char *Clipboard  = 0;
 int  szClipboard = 0;

/*---[ Implement ]------------------------------------------------------------*/

 void InitClipboard(GtkWidget *w)
 {

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
    gchar			*string;
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
 	int fRow = max(fromRow,toRow);

 	screen = Get3270DeviceBuffer(&rows, &cols);

 	DBGPrintf("Adding area from %d,%d to %d,%d to clipboard",bRow,bCol,fRow,fCol);

    buffer = g_malloc(((rows+1) * cols)+5);

    if(!buffer)
    {
    	Log("Can't allocate memory for temporary clipboard area");
    	return -1;
    }

    ptr = buffer;
    for(row=bRow;row<fRow;row++)
    {
    	trm  = screen + ((row * cols)+fromCol);
    	mark = ptr;

    	for(col=bCol;col<fCol;col++)
    	{
            *ptr = ebc2asc[trm->cc];
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
	sz   = strlen(buffer);

    if(!sz)
    {
    	DBGMessage("Empty buffer");
    	return -1;
    }

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
  	    g_free(buffer);
    	return -1;
    }

	strcpy(ptr,buffer);
    g_free(buffer);

    // Convert contents to UTF-8 and send it to clipboard
    string = g_convert(Clipboard, -1,"UTF-8", "ISO-8859-1", NULL, NULL, NULL);
    if(!string)
    {
    	Log("Error converting clipboard string to UTF-8");
    	return -1;
    }

    gtk_clipboard_set_text(	gtk_widget_get_clipboard(terminal,GDK_SELECTION_CLIPBOARD),
                            string,-1);

#ifdef DEBUG
    DBGPrintf("Clipboard contents (%d bytes in UTF-8):",szClipboard);
    fprintf(DBGFILE,"%s\n",string);
    fflush(DBGFILE);
#endif

    g_free(string);

    return 0;
 }

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

 void action_paste(GtkWidget *w, gpointer data)
 {
    gtk_clipboard_request_text(	gtk_widget_get_clipboard(terminal,GDK_SELECTION_CLIPBOARD),
								paste_clipboard,
								0 );
 }

 static gpointer exec_with_copy_thread(gpointer cmd)
 {
 	char 			filename[1024];
 	char			buffer[1024];
 	FILE			*arq;
 	int				fd;

    if(!Clipboard)
       return 0;

    snprintf(filename,1023,"%s/%s.XXXXXX",TMPPATH,TARGET);
    fd = mkstemp(filename);

    DBGMessage(filename);

 	arq = fdopen(fd,"w");
 	if(arq)
 	{
	   fprintf(arq,"%s\n",Clipboard);
       fclose(arq);

	   snprintf(buffer,1023,cmd,filename);
	   DBGMessage(buffer);
       system(buffer);
       remove(filename);
 	}
 	else
 	{
 		Error("Unable to open \"%s\" for writing",filename);
 	}
    return 0;
 }

 void action_exec_with_copy(GtkWidget *w, gpointer data)
 {
#if GTK == 2
    GThread   *thd = 0;
    thd =  g_thread_create( exec_with_copy_thread, (gpointer) data, 0, NULL);
#else
    pthread_t  thd = 0;
    pthread_create(&thd, NULL, (void * (*)(void *)) exec_with_copy_thread, data);
#endif
 }

 void action_print_copy(GtkWidget *w, gpointer data)
 {
    action_exec_with_copy(w,data ? data : "kprinter --nodialog -t " TARGET " %s");
 }


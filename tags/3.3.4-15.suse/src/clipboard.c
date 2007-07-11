
 #include <errno.h>
 #include "g3270.h"
 #include "trace.h"

 #include "lib/kybdc.h"
 #include "lib/tablesc.h"
 #include "lib/3270ds.h"


 #define DUMP_COPY 1

/*---[ Globals ]--------------------------------------------------------------*/

 char *Clipboard  = 0;
 int  szClipboard = 0;

/*---[ Implement ]------------------------------------------------------------*/

 void InitClipboard(GtkWidget *w)
 {

 }

 #if defined(DUMP_COPY)

 static void DumpString(const char *msg, const char *string)
 {
	char 		 buffer[1024];
	const char *ptr;
    int			 ps = 0;
	 
    DBGPrintf("Dump of %s",msg);
	 
    memset(buffer,' ',78);
    *(buffer+78) = 0;

    for(ptr = string; *ptr; ptr++)
    {
    	char temp[5];
    	if(ps > 0x0F)
    	{
    		fprintf(stderr,"%s\n",buffer);
            memset(buffer,' ',78);
            *(buffer+78) = 0;
    		ps = 0;
    	}
    	*(buffer+50+ps) = *ptr >= ' ' ? *ptr : '.';
    	snprintf(temp,5,"%02x",(int) *ptr);
    	memcpy(buffer+(ps*3),temp,2);
    	ps++;
    }

    *(buffer+75) = 0;
	fprintf(stderr,"%s\n",buffer);
    fflush(stderr);
 }
#endif	

 
 gchar *CopyTerminalContents(int bRow, int bCol, int fRow, int fCol, int table)
 {
 	int 			cols;
 	int 			rows;

 	int 			len;
    int 			col;
    int 			row;

    int				mode = 0;
    unsigned char	chr  = ' ';
    char			*ptr;
	char 			*dst;
	char 			*mark;
	int 			copy = 0;

 	const struct ea *screen;
 	const struct ea *trm;

 	gchar			*buffer;

 	screen = Get3270DeviceBuffer(&rows,&cols);

 	if(fRow < 0)
 	   fRow = rows;

	if(fCol < 0)
	   fCol = cols;

 	if( (fCol <= bCol) || (fRow <= bRow) || !screen)
 	{
 		errno = EINVAL;
 		return 0;
 	}

 	len = (cols * rows) << 1;

 	DBGTrace(len);

 	buffer = g_malloc(len+1);

    if(!buffer)
       return 0;

    /* Copy selected area to transfer buffer */
    DBGPrintf("Reading %dx%d <-> %dx%d",bRow,bCol,fRow,fCol);

	ptr  = (char *) buffer;
   	trm  = screen;
	*ptr = 0;
    for(row=0;row < rows;row++)
    {
    	mark = ptr;
    	for(col=0;col<cols;col++)
    	{
		    chr = ' ';

			if(trm->fa)
            {
			   if(table)
			      *(ptr++) = '\t';

			   if( (trm->fa & (FA_INTENSITY|FA_INT_NORM_SEL|FA_INT_HIGH_SEL)) == (FA_INTENSITY|FA_INT_NORM_SEL|FA_INT_HIGH_SEL) )
                  mode = (trm->fa & FA_PROTECT) ? 1 : 2;
               else
                  mode = 0;
            }

            if(!mode)
               chr = ebc2asc[trm->cc];

            if(row >= bRow && row < fRow && col >= bCol && col < fCol)
            {
			   /* It's inside the selection box, copy it */
               *(ptr++) = chr;
               if(!isspace(chr))
                  mark = ptr;
            }

    		trm++;
    	}
    	ptr = mark;
        if(row >= bRow && row < fRow)
   	       *(ptr++) = '\n';

    }

    *ptr = 0;

#ifdef DUMP_COPY	
	DumpString("Pre-processed Buffer",buffer);
#endif
	
	if(table)
	{
	   // Remove unnecessary spaces
	   for(mark=ptr=dst=buffer;*ptr;ptr++)
       {
		   switch(*ptr)
		   {
		   case '\t':
			   copy     = 0;
		       dst      = mark;
			   *(dst++) = '\t';
		       mark     = dst;
			   break;
			   
		   case '\n':
			   copy     = 0;
		       dst      = mark;
			   *(dst++) = '\n';
		       mark     = dst;
		       break;
		   
		   default:
			  if(!isspace(*ptr))
			  {
				 mark = dst+1;
			     copy = 1;
			  }
			  
			  if(copy)
				 *(dst++) = *ptr;
		   }
       }
	   *dst = 0;
	   
#ifdef DUMP_COPY	
   	DumpString("Post-processed Buffer",buffer);
#endif
	}
	
	
    return buffer;
 }

 int AddToClipboard(int fromRow, int fromCol, int toRow, int toCol, int clear, int table)
 {
    gchar	*string;
 	gchar	*buffer;
 	char	*ptr;
 	int		sz;

    buffer = CopyTerminalContents(	min(fromRow,toRow), min(fromCol,toCol),
									max(fromRow,toRow), max(fromCol,toCol),
									table );

    if(!buffer)
    	return -1;

 	if(clear && Clipboard)
 	{
 		DBGMessage("Release old clipboard area");
 		szClipboard = 0;
 		g_free(Clipboard);
 		Clipboard = 0;
 	}

	sz = strlen(buffer);

    if(!sz)
    {
    	g_free(buffer);
    	DBGMessage("No clipboard info");
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

//#ifdef DEBUG
//   #define DUMP_PASTE
//#endif

 static void paste_clipboard(GtkClipboard *clipboard, const gchar *text, gpointer data)
 {
    gchar *string;
    char  *ptr;

#ifdef DUMP_PASTE
    char 	buffer[0x0100];
    int		ps = 0;
#endif

    string = g_convert(text, -1, "ISO-8859-1", "UTF-8", NULL, NULL, NULL);
    if(!string)
    {
    	Log("Error converting clipboard string to ISO-8859-1");
    	return;
    }

#ifdef DUMP_PASTE

    DBGMessage("Paste buffer:");
    memset(buffer,' ',78);
    *(buffer+78) = 0;

    for(ptr = (char *) string; *ptr; ptr++)
    {
    	char temp[5];
    	if(ps > 0x0F)
    	{
    		fprintf(stderr,"%s\n",buffer);
            memset(buffer,' ',78);
            *(buffer+78) = 0;
    		ps = 0;
    	}
    	*(buffer+50+ps) = *ptr >= ' ' ? *ptr : '.';
    	snprintf(temp,5,"%02x",(int) *ptr);
    	memcpy(buffer+(ps*3),temp,2);
    	ps++;
    }

    *(buffer+75) = 0;
	fprintf(stderr,"%s\n",buffer);
    fflush(stderr);

#else

    /* Remove TABS */
    for(ptr = string;*ptr;ptr++)
    {
    	if(*ptr == 0x09)
    	   *ptr = ' ';
    }

    /* Paste and release */
    emulate_input(string, strlen(string), True);

#endif

    g_free(string);

 }

  
 static gchar *paste_buffer = 0; /**< String with contains the remaining part of the pasted buffer */
 

 void action_paste(GtkWidget *w, gpointer data)
 {
	if(paste_buffer)
	{
		g_free(paste_buffer);
		paste_buffer = 0;
	}
	
    gtk_clipboard_request_text(	gtk_widget_get_clipboard(terminal,GDK_SELECTION_CLIPBOARD),
								paste_clipboard,
								0 );
 }

 void action_paste_next(GtkWidget *w, gpointer data)
 {
	 if(!paste_buffer)
		 return;
	 
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
#else
    pthread_t  thd = 0;
#endif

	if(!Clipboard)
	{
       GtkWidget *widget = gtk_message_dialog_new(
					    GTK_WINDOW(top_window),
                        GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_WARNING,
                        GTK_BUTTONS_OK,
                        _( "Não existe cópia armazenada" ) );

       gtk_dialog_run(GTK_DIALOG(widget));
       gtk_widget_destroy (widget);

       return;

	}

#if GTK == 2
    thd =  g_thread_create( exec_with_copy_thread, (gpointer) data, 0, NULL);
#else
    pthread_create(&thd, NULL, (void * (*)(void *)) exec_with_copy_thread, data);
#endif

 }

 void action_print_copy(GtkWidget *w, gpointer data)
 {
    action_exec_with_copy(w,data ? data : PRINT_COMMAND);
 }

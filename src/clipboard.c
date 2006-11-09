
 #include "g3270.h"
 #include "trace.h"


/*---[ Globals ]--------------------------------------------------------------*/

 char *Clipboard  = 0;
 int  szClipboard = 0;

/*---[ Implement ]------------------------------------------------------------*/

 int CopyToClipboard(int fromRow, int fromCol, int toRow, int toCol)
 {
 	if(Clipboard)
 	{
 		DBGMessage("Release old clipboard area");
 		szClipboard = 0;
 		g_free(Clipboard);
 	}
 	return AppendToClipboard(fromRow,fromCol,toRow,toCol);
 }


 int AppendToClipboard(int fromRow, int fromCol, int toRow, int toCol)
 {
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

#ifdef DEBUG
    DBGPrintf("Clipboard contents (%d bytes):",szClipboard);
    fprintf(DBGFILE,"%s\n",Clipboard);
    fflush(DBGFILE);
#endif

    g_free(buffer);
    return 0;
 }



 #include "g3270.h"

// TODO (perry#9#): Rewrite using GTK's printing system


/*---[ Constants ]------------------------------------------------------------*/

 static const char *exec = "kprinter -t " TARGET " %s";

/*---[ Lock/Unlock ]----------------------------------------------------------*/

/*---[ Implement ]------------------------------------------------------------*/


 static gpointer PrintThread(gpointer filename)
 {
 	char buffer[4096];

    DBGMessage( (char *) filename);
 	snprintf(buffer,4095,exec,(char *) filename);

 	DBGMessage(buffer);
    system(buffer);

    remove(filename);
    free(filename);
    return 0;
 }

 int PrintTemporaryFile(const char *filename)
 {
#if GTK == 2
    GThread   *thd = 0;
#else
    pthread_t  thd = 0;
#endif

    DBGMessage(filename);

#if GTK == 2
    thd =  g_thread_create( PrintThread, (gpointer) filename, 0, NULL);
#else
     pthread_create(&thd, NULL, (void * (*)(void *)) PrintThread, filename);
#endif

 	return 0;
 }

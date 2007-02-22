

 #include "g3270.h"

// TODO (perry#9#): Rewrite using GTK's printing system


/*---[ Constants ]------------------------------------------------------------*/

 static const char *exec = PRINT_COMMAND;

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
 	char *file = strdup(filename);
#if GTK == 2
    GThread   *thd = 0;
#else
    pthread_t  thd = 0;
#endif

    DBGMessage(file);

#if GTK == 2
    thd =  g_thread_create( PrintThread, (gpointer) file, 0, NULL);
#else
     pthread_create(&thd, NULL, (void * (*)(void *)) PrintThread, file);
#endif

 	return 0;
 }

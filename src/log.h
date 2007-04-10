
#ifndef LOG_H_INCLUDED

#define LOG_H_INCLUDED

 #include <stdio.h>

 FILE	*g3270_openLog(void);
 FILE	*g3270_logPrefix(const char *);
 void	g3270_closeLog(FILE *);

 #include "trace.h"

#endif // LOG_H_INCLUDED

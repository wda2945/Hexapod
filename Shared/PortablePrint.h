/***************************************************
	Platform Independent printf()
****************************************************/

#ifdef __XC32__

#include "SysLog/SysLog.h"

#define DebugPrint(...)  {char tmp[81];\
    snprintf(tmp,80,__VA_ARGS__);\
    tmp[80] = 0;\
    Syslog_write_string(tmp);}
    
#else

#include <stdio.h>

#define DebugPrint(...) printf(__VA_ARGS__)

#endif
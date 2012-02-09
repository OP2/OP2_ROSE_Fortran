#ifndef _REAL__H
#define _REAL__H

#ifdef REALISDOUBLE
typedef double REAL;
#define REAL_STRING "double"
#else
typedef float REAL;
#define REAL_STRING "float"
#endif



#endif

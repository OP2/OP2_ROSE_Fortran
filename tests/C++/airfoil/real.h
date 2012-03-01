#ifndef _REAL__H
#define _REAL__H

#ifdef REALISDOUBLE
#define REAL double
#define REAL_STRING "double"
#else
typedef float REAL;
#define REAL float
#define REAL_STRING "float"
#endif



#endif

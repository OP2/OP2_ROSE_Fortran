/* src/mpi2c++/mpi2c++_config.h.  Generated automatically by configure.  */
/* src/mpi2c++/mpi2c++_config.h.in.  Generated automatically from configure.in by autoheader.  */
// -*- c++ -*-
//
// This file is part of the University of Notre Dame implementation
// of the MPI 2 C++ bindings.  See the LICENSE file in the top level
// directory for details.
//

#ifndef _MPIPP_CONFIG_H
#define _MPIPP_CONFIG_H


/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

#define _MPIPP_USEEXCEPTIONS_ 0
#define _MPIPP_DEBUG_         0

#define MPI2CPP_HAVE_BOOL     1 
// Put this here, even though it is not used, so that autoheader does
// not complain
#define LSC_HAVE_BOOL         1

#define MPI2CPP_SIZEOF_INT    4
#define SIZEOF_MPI2CPP_BOOL_T 1

#if MPI2CPP_SIZEOF_INT != SIZEOF_MPI2CPP_BOOL_T
#define _MPIPP_BOOL_NE_INT_   1
#else
#define _MPIPP_BOOL_NE_INT_   0
#endif

// Does our compiler support namespaces?
#define _MPIPP_USENAMESPACE_  1

// Compile for Profiling?
#define _MPIPP_PROFILING_     1

// What kind of signals do we have?
#define MPI2CPP_BSD_SIGNAL    0
#define MPI2CPP_SYSV_SIGNAL   1

// Is the ERR_PENDING constant defined?
#define MPI2CPP_HAVE_PENDING  0

// Do we have MPI_GET_VERSION?
#define MPI2CPP_HAVE_MPI_GET_VERSION  1

// Is the MPI_STATUS_IGNORE constant defined?
#define MPI2CPP_HAVE_STATUS_IGNORE    0
#define MPI2CPP_HAVE_STATUSES_IGNORE  0

#if _MPIPP_PROFILING_
#define _REAL_MPI_ PMPI
#else
#define _REAL_MPI_ MPI
#endif

#if _MPIPP_USENAMESPACE_
#define _MPIPP_STATIC_
#define _MPIPP_EXTERN_ extern
#else
#define _MPIPP_STATIC_ static
#define _MPIPP_EXTERN_
#endif

#if MPI2CPP_HAVE_BOOL
typedef bool MPI2CPP_BOOL_T;
#define MPI2CPP_FALSE false
#define MPI2CPP_TRUE true
#else
enum MPI2CPP_BOOL_T { MPI2CPP_FALSE, MPI2CPP_TRUE };
#ifdef bool
/* #undef bool */
#endif
#ifdef false
/* #undef false */
#endif
#ifdef true
/* #undef true */
#endif
#define bool MPI2CPP_BOOL_T
#define false MPI2CPP_FALSE
#define true MPI2CPP_TRUE
#endif

#ifndef MPI2CPP_VIRTUAL_FUNC_RET
#define MPI2CPP_VIRTUAL_FUNC_RET 0
#endif 

#define MPI2CPP_BSD_SIGNAL 0
#define MPI2CPP_SYSV_SIGNAL 1

#define MPI2CPP_FORTRAN 1
#define MPI2CPP_ALL_OPTIONAL_FORTRAN 0
#define MPI2CPP_SOME_OPTIONAL_FORTRAN 1
#define MPI2CPP_OPTIONAL_C 0


#define MPI2CPP_LAM61 0
#define MPI2CPP_LAM631 0
#define MPI2CPP_LAM632 0
#define MPI2CPP_LAM633 0
#define MPI2CPP_LAM64 0
#define MPI2CPP_LAMUNKNOWN 0
#define MPI2CPP_LAM (MPI2CPP_LAM631 | MPI2CPP_LAM632 | MPI2CPP_LAM633 | MPI2CPP_LAM64 | MPI2CPP_LAMUNKNOWN)

#define MPI2CPP_MPICH120 0
#define MPI2CPP_MPICH121 1
#define MPI2CPP_MPICHUNKNOWN 0
#define MPI2CPP_MPICH (MPI2CPP_MPICH120 | MPI2CPP_MPICH121 | MPI2CPP_MPICHUNKNOWN) 

 
#define MPI2CPP_IBM21014 0
#define MPI2CPP_IBM21015 0
#define MPI2CPP_IBM21016 0
#define MPI2CPP_IBM21017 0
#define MPI2CPP_IBM21018 0
#define MPI2CPP_IBM2_3_0_0 0
#define MPI2CPP_IBM2_4_0_0 0
#define MPI2CPP_IBMUNKNOWN 0
#define MPI2CPP_IBM_SP (MPI2CPP_IBM21014 | MPI2CPP_IBM21015 | MPI2CPP_IBM21016 | MPI2CPP_IBM21017 | MPI2CPP_IBM21018 | MPI2CPP_IBM2_3_0_0 | MPI2CPP_IBMUNKNOWN| MPI2CPP_IBM2_4_0_0)

 
#define MPI2CPP_SGI20 0
#define MPI2CPP_SGI30 0
#define MPI2CPP_SGI31 0
#define MPI2CPP_SGI32 0
#define MPI2CPP_SGIUNKNOWN 0

#define MPI2CPP_HPUX_OS 0
#define MPI2CPP_HPUX0102 0
#define MPI2CPP_HPUX0103 0
#define MPI2CPP_HPUX0105 0
#define MPI2CPP_HPUXUNKNOWN 0

#define MPI2CPP_CRAY 0
#define MPI2CPP_CRAY1104 0
#define MPI2CPP_CRAYUNKNOWN 0

#define MPI2CPP_ATTR int

#define MPI2CPP_AIX 0

// Tell MPICH to use the right MPI_Handle_function definition

#if MPI2CPP_MPICH
#ifndef USE_STDARG
#define USE_STDARG
#endif
#endif

#endif

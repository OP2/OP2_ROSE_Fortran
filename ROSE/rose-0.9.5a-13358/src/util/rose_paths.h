#ifndef ROSE_PATHS_H
#define ROSE_PATHS_H

// DQ (4/21/2009): If this is not set then set it here.
// For most of ROSE usage this is set in sage3.h, but initial
// construction or ROSETTA used to generate ROSE requires 
// it as well.
#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif

// DQ (4/21/2009): This must be set before sys/stat.h is included by any other header file.
// Use of _FILE_OFFSET_BITS macro is required on 32-bit systems to controling size of "struct stat"
#if !(defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64))
#error "The _FILE_OFFSET_BITS macro should be set before any sys/stat.h is included by any other header file!"
#endif

#include <string>

extern const std::string ROSE_GFORTRAN_PATH;
extern const std::string ROSE_AUTOMAKE_TOP_SRCDIR;
extern const std::string ROSE_AUTOMAKE_TOP_BUILDDIR;
extern const std::string ROSE_AUTOMAKE_PREFIX;
extern const std::string ROSE_AUTOMAKE_DATADIR;
extern const std::string ROSE_AUTOMAKE_BINDIR;
extern const std::string ROSE_AUTOMAKE_INCLUDEDIR;
extern const std::string ROSE_AUTOMAKE_INFODIR;
extern const std::string ROSE_AUTOMAKE_LIBDIR;
extern const std::string ROSE_AUTOMAKE_LIBEXECDIR;
extern const std::string ROSE_AUTOMAKE_LOCALSTATEDIR;
extern const std::string ROSE_AUTOMAKE_MANDIR;

extern const std::string ROSE_AUTOMAKE_ABSOLUTE_PATH_TOP_SRCDIR;

/* Additional interesting data to provide */
extern const std::string ROSE_CONFIGURE_DATE;
extern const std::string ROSE_AUTOMAKE_BUILD_OS;
extern const std::string ROSE_AUTOMAKE_BUILD_CPU;

/* Numeric form of ROSE version -- assuming ROSE version x.y.zL (where */
/* x, y, and z are numbers, and L is a single lowercase letter from a to j), */
/* the numeric value is x * 1000000 + y * 10000 + z * 100 + (L - 'a') */
extern const int ROSE_NUMERIC_VERSION;


// DQ (5/2/2009): This is temporary while we work out the details of the new Graph IR nodes.
// #define USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY 0

//#ifdef ROSE_USE_NEW_GRAPH_NODES
//#ifndef ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY
//#warning "ROSE_USING_GRAPH_IR_NODES_FOR_BACKWARD_COMPATABILITY IS NOT SET"
//#endif
//#endif


#endif /* ROSE_PATHS_H */


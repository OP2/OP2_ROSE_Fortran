/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Helper class to get (opaque) variable references to variables
 * defined in the OpenMP run-time library
 */

#ifndef OPEN_MP_H
#define OPEN_MP_H

#include <rose.h>

namespace OpenMP
{
  std::string const numberOfThreads = "nthreads";
  std::string const threadIndex = "threadIndex";
  std::string const sliceStart = "sliceStart";
  std::string const sliceEnd = "sliceEnd";
  std::string const threadID = "threadID";
  std::string const sliceIterator = "sliceIterator";
  std::string const blockID = "blockID";
}

#endif

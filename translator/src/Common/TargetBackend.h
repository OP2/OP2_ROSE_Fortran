/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * The different backends supported:
 * 1) Unknown: primarily for debugging reasons
 * 2) CUDA
 * 3) OpenMP
 * 4) OpenCL
 */

#ifndef TARGET_BACKEND_H
#define TARGET_BACKEND_H

#include <string>

namespace TargetBackend
{
  enum BACKEND_VALUE
  {
    UNKNOWN, CUDA, OPENMP, OPENCL
  };

  std::string
  toString (BACKEND_VALUE backend);
}

#endif

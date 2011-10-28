/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * The different frontends supported:
 * 1) C++
 * 2) Fortran
 *
 * The different backends supported:
 * 1) CUDA
 * 2) OpenMP
 * 3) OpenCL
 */

#pragma once
#ifndef TARGET_LANGUAGE_H
#define TARGET_LANGUAGE_H

#include <string>

namespace TargetLanguage
{
  enum FRONTEND
  {
    FORTRAN, CPP, UNKNOWN_FRONTEND
  };

  enum BACKEND
  {
    CUDA, OPENMP, OPENCL, UNKNOWN_BACKEND
  };

  std::string
  toString (FRONTEND frontend);

  std::string
  toString (BACKEND backend);
}

#endif

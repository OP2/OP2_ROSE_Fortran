#pragma once
#ifndef CPP_OPENCL_DATA_SIZES_DECLARATION_DIRECT_LOOP_H
#define CPP_OPENCL_DATA_SIZES_DECLARATION_DIRECT_LOOP_H

#include <CPPOpenCLDataSizesDeclaration.h>

#include <string>

class CPPOpenCLDataSizesDeclarationDirectLoop: public CPPOpenCLDataSizesDeclaration
{
  private:

    virtual void
    addFields ();

  public:

    CPPOpenCLDataSizesDeclarationDirectLoop (
        std::string const & subroutineName, CPPParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif

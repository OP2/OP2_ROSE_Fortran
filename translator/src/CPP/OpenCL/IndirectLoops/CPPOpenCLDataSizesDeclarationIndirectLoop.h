#pragma once
#ifndef CPP_OPENCL_DATA_SIZES_DECLARATION_INDIRECT_LOOP_H
#define CPP_OPENCL_DATA_SIZES_DECLARATION_INDIRECT_LOOP_H

#include <CPPOpenCLDataSizesDeclaration.h>

class CPPOpenCLDataSizesDeclarationIndirectLoop: public CPPOpenCLDataSizesDeclaration
{
  private:

    virtual void
    addFields ();

  public:

    CPPOpenCLDataSizesDeclarationIndirectLoop (
        std::string const & subroutineName, CPPParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif

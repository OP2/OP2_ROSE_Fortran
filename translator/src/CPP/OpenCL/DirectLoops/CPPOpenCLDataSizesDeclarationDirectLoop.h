/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Models the data sizes declaration needed for a direct loop.
 * Its fields include the sizes of the OP_DAT arguments
 */

#ifndef CPP_OPENCL_DATA_SIZES_DECLARATION_DIRECT_LOOP_H
#define CPP_OPENCL_DATA_SIZES_DECLARATION_DIRECT_LOOP_H

#include <CPPOpenCLDataSizesDeclaration.h>

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

/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Models the data sizes declaration needed for a direct loop.
 * Its fields include the sizes of the OP_DAT arguments
 */

#ifndef DATA_SIZES_DECLARATION_OF_DIRECT_LOOP_H
#define DATA_SIZES_DECLARATION_OF_DIRECT_LOOP_H

#include <DataSizesDeclaration.h>

class DataSizesDeclarationOfDirectLoop: public DataSizesDeclaration
{
  private:

    virtual void
    addFields (ParallelLoop & parallelLoop);

  public:

    DataSizesDeclarationOfDirectLoop (ParallelLoop & parallelLoop,
        std::string const & subroutineName, SgScopeStatement * moduleScope);
};

#endif

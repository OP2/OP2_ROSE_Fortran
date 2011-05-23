/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef DATA_SIZES_DECLARATION_OF_INDIRECT_LOOP_H
#define DATA_SIZES_DECLARATION_OF_INDIRECT_LOOP_H

#include<DataSizesDeclaration.h>

class DataSizesDeclarationOfIndirectLoop: public DataSizesDeclaration
{
  private:

    virtual void
    addFields (ParallelLoop & parallelLoop);

  public:

    /*
     * ======================================================
     * Name of the field which represents the size of a
     * local to global renumbering (i.e. from local memory
     * into global device memory in the CUDA architecture)
     * argument used for an indirect OP_DAT.
     *
     * These are the 'ind_maps' variable sizes in the plan
     * function according to Mike Giles documentation
     * ======================================================
     */
    std::string
        getLocalToGlobalRenumberingSizeFieldName (
            unsigned int OP_DAT_ArgumentGroup);

    /*
     * ======================================================
     * Name of the field which represents the size of a
     * global to local renumbering (i.e. from global device memory
     * to local memory in the CUDA architecture)
     * argument used for an indirect OP_DAT.
     *
     * These are the 'maps' variable sizes in the plan function
     * according to Mike Giles documentation
     * ======================================================
     */
    std::string
        getGlobalToLocalRenumberingSizeFieldName (
            unsigned int OP_DAT_ArgumentGroup);

    DataSizesDeclarationOfIndirectLoop (ParallelLoop & parallelLoop,
        std::string const & subroutineName, SgScopeStatement * moduleScope);
};

#endif

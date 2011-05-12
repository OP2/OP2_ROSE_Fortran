/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models a variable declaration: it includes
 * fields for the sizes of data passed to the device.
 * These sizes are needed in Fortran to prevent segmentation
 * faults.
 *
 * These sizes must be passed in a struct-like data
 * structure as the number of kernel parameters can be become
 * unwieldly. In these cases, the code might not compile
 * with current-generation CUDA compilers due to the number of
 * parameters, so the solution is to instead pack them into
 * a struct.
 */

#ifndef DEVICE_DATA_SIZES_DECLARATION_H
#define DEVICE_DATA_SIZES_DECLARATION_H

#include<ParallelLoop.h>

class DeviceDataSizesDeclaration
{
  protected:
    /*
     * ======================================================
     * The type definition declaration statement
     * ======================================================
     */
    SgDerivedTypeStatement * deviceDatatypeStatement;

    /*
     * ======================================================
     * A mapping from an OP_DAT argument to its
     * declaration representing its size. This is a field inside
     * the above type definition declaration statement
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *> OP_DAT_Sizes;

    /*
     * ======================================================
     * A mapping from an indirect map argument to its
     * declaration representing its size. This is a field inside
     * the above type definition declaration statement
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *>
        localToGlobalRenumberingOfIndirectMappingSizes;

    /*
     * ======================================================
     * A mapping from an indirect map argument to its
     * declaration representing its size. This is a field inside
     * the above type definition declaration statement
     * ======================================================
     */
    std::map <unsigned int, SgVariableDeclaration *>
        globalToLocalRenumberingOfIndirectMappingSizes;

    /*
     * ======================================================
     * A mapping to the other fields inside the above type
     * definition declaration statement
     * ======================================================
     */
    std::map <std::string, SgVariableDeclaration *> otherFieldDeclarations;

  private:

    virtual void
    addFields (ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);

  public:

    SgClassType *
    getType ()
    {
      return deviceDatatypeStatement->get_type ();
    }

    SgVariableDeclaration *
    get_OP_DAT_SizeFieldDeclaration (unsigned int OP_DAT_ArgumentGroup)
    {
      return OP_DAT_Sizes[OP_DAT_ArgumentGroup];
    }

    SgVariableDeclaration *
    get_LocalToGlobalMappingSizeFieldDeclaration (
        unsigned int OP_DAT_ArgumentGroup)
    {
      return localToGlobalRenumberingOfIndirectMappingSizes[OP_DAT_ArgumentGroup];
    }

    SgVariableDeclaration *
    get_GlobalToLocalMappingSizeFieldDeclaration (
        unsigned int OP_DAT_ArgumentGroup)
    {
      return globalToLocalRenumberingOfIndirectMappingSizes[OP_DAT_ArgumentGroup];
    }

    SgVariableDeclaration *
    getPlanVariableSizeFieldDeclaration (std::string planVariableSizeName)
    {
      return otherFieldDeclarations[planVariableSizeName];
    }

    /*
     * ======================================================
     * This function calls the correct addFields subclass
     * implementation
     * ======================================================
     */

    void
    initialise (ParallelLoop & parallelLoop, SgScopeStatement * moduleScope)
    {
      addFields (parallelLoop, moduleScope);
    }

    DeviceDataSizesDeclaration (ParallelLoop & parallelLoop,
        std::string const & subroutineName, SgScopeStatement * moduleScope);
};

#endif

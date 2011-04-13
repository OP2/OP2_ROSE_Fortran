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

namespace DeviceDataSizesFields
{
  std::string const pindSizesSize = "pindSizesSize";
  std::string const pindOffsSize = "pindOffsSize";
  std::string const pblkMapSize = "pblkMapSize";
  std::string const poffsetSize = "poffsetSize";
  std::string const pnelemsSize = "pnelemsSize";
  std::string const pnthrcolSize = "pnthrcolSize";
  std::string const pthrcolSize = "pthrcolSize";
}

class DeviceDataSizesDeclaration
{
  private:

    SgDerivedTypeStatement * deviceDatatypeStatement;

  private:

    void
    addFields (ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);

  public:

    DeviceDataSizesDeclaration (ParallelLoop & parallelLoop,
        std::string const & subroutineName, SgScopeStatement * moduleScope);
};

#endif

#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <boost/lexical_cast.hpp>
#include <Subroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>
#include <DeviceDataSizesDeclaration.h>

class KernelSubroutine: public Subroutine
{
  protected:

    static std::string
    get_OP_DAT_VariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup);
    }

    static std::string
    get_OP_DAT_SizeVariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup)
          + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;
    }

    static std::string
    getLocalThread_OP_DAT_VariableName (unsigned int OP_DAT_ArgumentGroup)
    {
      using boost::lexical_cast;
      using std::string;

      return IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
          + lexical_cast <string> (OP_DAT_ArgumentGroup)
          + IndirectAndDirectLoop::Fortran::VariableSuffixes::local;
    }

    void
    createArgsSizesFormalParameter (
        DeviceDataSizesDeclaration & deviceDataSizesDeclaration);

    /*
     * ======================================================
     * Creates the argI_l local thread variables
     * for CUDA kernels of direct and indirect loops
     * ======================================================
     */

    void
    createLocalThreadVariables (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Creates the autoshared variable: only for real(8)
     * ======================================================
     */
    void
    createAutosharedVariable (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Initialises local thread variables
     * ======================================================
     */
    void
    initialiseLocalThreadVariables (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Support for reduction variables
     * ======================================================
     */
    void
    createAndAppendReductionSubroutineCall (ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * Generation of formal parameter for offset in shared
     * memory reserved to reductions
     * ======================================================
     */
    void
    createAndAppendSharedMemoryOffesetForReduction ();

    SgStatement *
    createUserSubroutineCall (UserDeviceSubroutine & userDeviceSubroutine,
        SgVariableDeclaration * variable_setElementCounter,
        SgVariableDeclaration * variable_offsetInThreadBlock,
        ParallelLoop & parallelLoop);

    KernelSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + SubroutineNames::kernelSuffix)
    {
    }
};

#endif

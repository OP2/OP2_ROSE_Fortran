/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models an OP_PAR_LOOP call discovered in the code
 */

#ifndef OP2_PARALLEL_LOOP_H
#define OP2_PARALLEL_LOOP_H

#include <rose.h>
#include <OP2CommonDefinitions.h>
#include <OP2DeclaredVariables.h>
#include <OP2Variables.h>
#include <DeviceSubroutines.h>

class OP2ParallelLoop
{
  private:

    /*
     * ======================================================
     * The name of the generated CUDA-Fortran module for this
     * OP_PAR_LOOP
     * ======================================================
     */
    std::string CUDAModuleName;

    /*
     * ======================================================
     * The actual arguments passed to the OP_PAR_LOOP
     * ======================================================
     */
    SgExpressionPtrList actualArguments;

    /*
     * ======================================================
     * Is this a direct or indirect loop?
     * ======================================================
     */
    bool isDirect;

    /*
     * ======================================================
     * The declarations corresponding to each distinct
     * OP_DAT argument. The key to this map is the name of the
     * actual argument passed to the OP_PAR_LOOP
     * ======================================================
     */
    std::map <std::string, OP_DAT_Declaration *> OP_DATs;

    /*
     * ======================================================
     * The same OP_DAT variable can be passed as an argument
     * to OP_PAR_LOOP. This map records how many times an OP_DAT
     * variable is passed
     * ======================================================
     */
    std::map <std::string, unsigned int> OP_DAT_Occurrences;

    /*
     * ======================================================
     * The host subroutine generated for this OP_PAR_LOOP
     * ======================================================
     */
    HostSubroutine * hostSubroutine;

    /*
     * ======================================================
     * The kernel subroutine generated for this OP_PAR_LOOP
     * ======================================================
     */
    KernelSubroutine * kernelSubroutine;

    /*
     * ======================================================
     * The user subroutine generated for this OP_PAR_LOOP
     * (which runs on the device)
     * ======================================================
     */
    UserDeviceSubroutine * userDeviceSubroutine;

    /*
     * ======================================================
     * The user subroutine passed to this OP_PAR_LOOP
     * (which originally runs on the host)
     * ======================================================
     */
    UserHostSubroutine * userHostSubroutine;

  private:

    /*
     * ======================================================
     * Discovers whether all OP_MAPs are direct, and hence
     * whether this OP_PAR_LOOP has direct or indirect access
     * to its data
     * ======================================================
     */
    void
    setDirectOrIndirectLoop (Declarations * op2DeclaredVariables);

    /*
     * ======================================================
     * Retrieves the declarations of the OP_DAT arguments
     * so that we can later retrieve their primitive types
     * and dimensions
     * ======================================================
     */
    void
    retrieve_OP_DAT_Declarations (Declarations * op2DeclaredVariables);

  public:

    OP2ParallelLoop (std::string userSubroutineName,
        SgExpressionPtrList & actualArguments,
        Declarations * op2DeclaredVariables);

    std::string
    getCUDAModuleName () const
    {
      return CUDAModuleName;
    }

    SgExpressionPtrList &
    getActualArguments ()
    {
      return actualArguments;
    }

    bool
    isDirectLoop () const
    {
      return isDirect;
    }

    HostSubroutine &
    getHostSubroutine () const
    {
      return *hostSubroutine;
    }

    KernelSubroutine &
    getKernelSubroutine () const
    {
      return *kernelSubroutine;
    }

    UserDeviceSubroutine &
    getUserDeviceSubroutine () const
    {
      return *userDeviceSubroutine;
    }

    UserHostSubroutine &
    getUserHostSubroutine () const
    {
      return *userHostSubroutine;
    }

    unsigned int
    getNumberOf_OP_DAT_ArgumentGroups () const
    {
      return (actualArguments.size () - OP2::NUMBER_OF_NON_OP_DAT_ARGUMENTS)
          / OP2::NUMBER_OF_ARGUMENTS_PER_OP_DAT;
    }

    std::map <std::string, OP_DAT_Declaration *>::const_iterator
    first_OP_DAT ()
    {
      return OP_DATs.begin ();
    }

    std::map <std::string, OP_DAT_Declaration *>::const_iterator
    last_OP_DAT ()
    {
      return OP_DATs.end ();
    }

    unsigned int
    get_OP_DAT_Occurrences (std::string const & variableName)
    {
      return OP_DAT_Occurrences[variableName];
    }
};

#endif

/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models an OP_PAR_LOOP call discovered in Fortran code
 */

#ifndef OP2_PARALLEL_LOOP_H
#define OP2_PARALLEL_LOOP_H

#include <rose.h>

#include "OP2CommonDefinitions.h"
#include "OP2DeclaredVariables.h"

class OP2ParallelLoop
{
  private:
    /*
     * ====================================================================================================
     * Private data
     * ====================================================================================================
     */

    /*
     * The name of the user function on the host
     */
    std::string userHostFunctionName;

    /*
     * The name of the user function on the device
     */
    std::string userDeviceFunctionName;

    /*
     * The name of the subroutine called from the host to launch
     * the kernel
     */
    std::string mainKernelDeviceName;

    /*
     * The name of the host subroutine that is called in place of
     * an OP_PAR_LOOP after the source-to-source transformation.
     * This is the subroutine that initiates the device code
     */
    std::string mainKernelHostName;

    /*
     * The name of the generated CUDA-Fortran module for this OP_PAR_LOOP
     */
    std::string CUDAModuleName;

    /*
     * The actual arguments passed to the OP_PAR_LOOP
     */
    SgExpressionPtrList actualArguments;

    /*
     * Is this a direct or indirect loop?
     */
    bool isDirect;

    /*
     * The formal OP_SET argument in the generated host subroutine
     */
    SgVariableDeclaration * OP_SET_Argument;

    /*
     * The formal indirection index arguments (idx0, ..., idxN-1) in the generated
     * host subroutine
     */
    std::vector <SgVariableDeclaration *> OP_INDIRECTION_Arguments;

    /*
     * The formal OP_DAT arguments (arg0, ..., argN-1) in the generated
     * host subroutine
     */
    std::vector <SgVariableDeclaration *> OP_DAT_Arguments;

    /*
     * The formal OP_MAP arguments (map0, ..., mapN-1) in the generated
     * host subroutine
     */
    std::vector <SgVariableDeclaration *> OP_MAP_Arguments;

    /*
     * The formal OP_ACCESS arguments (access0, ..., accessN-1) in the generated
     * host subroutine
     */
    std::vector <SgVariableDeclaration *> OP_ACCESS_Arguments;

    /*
     * Local variables (data0Size, ..., dataN-1Size) in the generated host subroutine
     */
    std::vector <SgVariableDeclaration *> hostSubroutineVariables;

    /*
     * Local variables (c2fPtr0, ..., c2fPtrN-1)
     * in the generated host subroutine to transform a C pointer into a Fortran pointer
     */
    std::vector <SgVariableDeclaration *> CToFortranPointers;

    /*
     * Device allocatable variables for the formal parameters of the user supplied
     * function
     */
    std::vector <SgVariableDeclaration *> UserFunctionArguments;

    /*
     * The actual data types of OP_DAT arguments
     */
    std::vector <SgType *> OP_DAT_ActualTypes;

    /*
     * The dimensions (number of data items per set element) of OP_DAT arguments
     */
    std::vector <unsigned int> OP_DAT_Dimensions;

  private:

    /*
     * Discovers whether all OP_MAPs are direct, and hence whether this
     * OP_PAR_LOOP has direct or indirect access to its data
     */
    void
    setDirectOrIndirectLoop (OP2DeclaredVariables * op2DeclaredVariables);

    /*
     * Retrieves the base types of OP_DAT variables.
     */
    void
    retrieve_OP_DAT_BaseTypes (OP2DeclaredVariables * op2DeclaredVariables);

  public:
    /*
     * ====================================================================================================
     * Public functions
     * ====================================================================================================
     */

    OP2ParallelLoop (std::string userFunctionName,
        SgExpressionPtrList & actualArguments,
        OP2DeclaredVariables * op2DeclaredVariables)
    {
      this->userHostFunctionName = userFunctionName;
      this->userDeviceFunctionName = userFunctionName + "_device";
      this->mainKernelDeviceName = userFunctionName + "_kernel";
      this->mainKernelHostName = userFunctionName + "_host";
      this->CUDAModuleName = userFunctionName + "_cudafor";
      this->actualArguments = actualArguments;

      setDirectOrIndirectLoop (op2DeclaredVariables);
      retrieve_OP_DAT_BaseTypes (op2DeclaredVariables);
    }

    std::string
    getUserHostFunctionName () const
    {
      return userHostFunctionName;
    }

    std::string
    getUserDeviceFunctionName () const
    {
      return userDeviceFunctionName;
    }

    std::string
    getMainKernelDeviceName () const
    {
      return mainKernelDeviceName;
    }

    std::string
    getMainKernelHostName () const
    {
      return mainKernelHostName;
    }

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

    unsigned int
    getNumberOf_OP_DAT_ArgumentGroups () const
    {
      return (actualArguments.size () - OP2::NUMBER_OF_NON_OP_DAT_ARGUMENTS)
          / OP2::NUMBER_OF_ARGUMENTS_PER_OP_DAT;
    }

    void
    set_OP_SET_Argument (SgVariableDeclaration * OP_SET_Argument)
    {
      this->OP_SET_Argument = OP_SET_Argument;
    }

    void
    set_OP_INDIRECTION_Argument (
        SgVariableDeclaration * OP_INDIRECTION_Argument)
    {
      OP_INDIRECTION_Arguments.push_back (OP_INDIRECTION_Argument);
    }

    void
    set_OP_DAT_Argument (SgVariableDeclaration * OP_DAT_Argument)
    {
      OP_DAT_Arguments.push_back (OP_DAT_Argument);
    }

    void
    set_OP_MAP_Argument (SgVariableDeclaration * OP_MAP_Argument)
    {
      OP_MAP_Arguments.push_back (OP_MAP_Argument);
    }

    void
    set_OP_ACESS_Argument (SgVariableDeclaration * OP_ACCESS_Argument)
    {
      OP_ACCESS_Arguments.push_back (OP_ACCESS_Argument);
    }

    void
    set_Host_Subroutine_Variable (
        SgVariableDeclaration * hostSubroutineVariable)
    {
      hostSubroutineVariables.push_back (hostSubroutineVariable);
    }

    void
    setCToFortranPointer (SgVariableDeclaration * cToFortranPointer)
    {
      CToFortranPointers.push_back (cToFortranPointer);
    }

    void
    setUserFunctionArgument (SgVariableDeclaration * argument)
    {
      UserFunctionArguments.push_back (argument);
    }

    SgVariableDeclaration * const
    get_OP_SET_Argument () const
    {
      return OP_SET_Argument;
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    get_OP_INDIRECTION_Arguments () const
    {
      return OP_INDIRECTION_Arguments.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    get_OP_DAT_Arguments () const
    {
      return OP_DAT_Arguments.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    get_OP_MAP_Arguments () const
    {
      return OP_MAP_Arguments.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    get_OP_ACCESS_Arguments () const
    {
      return OP_ACCESS_Arguments.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    get_Host_Subroutine_Variables () const
    {
      return hostSubroutineVariables.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    getCToFortranPointers () const
    {
      return CToFortranPointers.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    getUserFunctionArguments () const
    {
      return UserFunctionArguments.begin ();
    }

    SgType *
    get_OP_DAT_ActualType (int index)
    {
      return OP_DAT_ActualTypes[index];
    }

    unsigned int
    get_OP_DAT_Dimension (int index)
    {
      return OP_DAT_Dimensions[index];
    }
};

#endif

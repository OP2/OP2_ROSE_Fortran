/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models an OP_PAR_LOOP call discovered in Fortran code
 */

#ifndef OPPARLOOP_H
#define OPPARLOOP_H

#include <rose.h>

class OpParLoop
{
  private:
    /*
     * ====================================================================================================
     * Private data
     * ====================================================================================================
     */

    /*
     * The name of the kernel function on the host
     */
    std::string kernelHostName;

    /*
     * The name of the kernel function on the device
     */
    std::string kernelDeviceName;

    /*
     * Arguments to OP_PAR_LOOP are grouped into batches of 4 arguments, plus
     * another two, i.e. there are 2+4n parameters.
     * This variable stores the value n in the expression 2+4n
     */
    unsigned int OP_DAT_ArgumentGroups;

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
     * Local variables (argument0, ..., argumentN-1, c2fPtr0, ..., c2fPtrN-1)
     * in the generated host subroutine to transform a C pointer into a Fortran pointer
     */
    std::vector <SgVariableDeclaration *> CToFortranVariables;

    /*
     * The actual data types of an OP_DAT argument
     */
    std::vector <SgType *> OP_DAT_ActualTypes;

    /*
     * The dimension values passed to op_decl_dat calls
     */
    std::vector <unsigned int> OP_DAT_Dimensions;

  public:
    /*
     * ====================================================================================================
     * Public functions
     * ====================================================================================================
     */

    OpParLoop (std::string kernelHostName, unsigned int OP_DAT_ArgumentGroups)
    {
      this->kernelHostName = kernelHostName;
      this->kernelDeviceName = kernelHostName + "_device";
      this->OP_DAT_ArgumentGroups = OP_DAT_ArgumentGroups;
    }

    std::string
    getKernelHostName () const
    {
      return kernelHostName;
    }

    std::string
    getKernelDeviceName () const
    {
      return kernelDeviceName;
    }

    unsigned int
    getNumberOf_OP_DAT_ArgumentGroups () const
    {
      return OP_DAT_ArgumentGroups;
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
    set_C_To_Fortran_Variable (SgVariableDeclaration * cToFortranVariable)
    {
      CToFortranVariables.push_back (cToFortranVariable);
    }

    void
    set_OP_DAT_ActualType (SgType * type)
    {
      OP_DAT_ActualTypes.push_back (type);
    }

    void
    set_OP_DAT_Dimension (int dimension)
    {
      OP_DAT_Dimensions.push_back (dimension);
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
    get_C_To_Fortran_Variables () const
    {
      return CToFortranVariables.begin ();
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

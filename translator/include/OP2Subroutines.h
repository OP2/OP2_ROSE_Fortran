/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * These classes model three subroutines:
 *
 * 1) The host subroutine which allocates and deallocates OP_DAT data on the
 * device and calls the kernel
 *
 * 2) The main device subroutine (the subroutine with the "__global__" keyword
 * in CUDA C) which is launched from the host
 *
 * 3) The user subroutine which is passed to OP_PAR_LOOP and is launched from
 * the kernel (the subroutine with the "__device__" keyword in CUDA C)
 */

#ifndef DEVICE_SUBROUTINES_H
#define DEVICE_SUBROUTINES_H

#include <rose.h>
#include <OP2DeclaredVariables.h>

class Subroutine
{

  protected:

    std::string subroutineName;

    Subroutine (std::string const & subroutineName)
    {
      this->subroutineName = subroutineName;
    }

  public:

    std::string const &
    getSubroutineName () const
    {
      return subroutineName;
    }
};

class HostSubroutine: public Subroutine
{
  private:

    /*
     * ======================================================
     * The formal OP_SET parameter
     * ======================================================
     */
    SgVariableDeclaration * OP_SET_FormalParameter;

    /*
     * ======================================================
     * The formal indirection index parameters
     * ======================================================
     */
    std::vector <SgVariableDeclaration *> OP_INDIRECTION_FormalParameters;

    /*
     * ======================================================
     * The formal OP_DAT parameters
     * ======================================================
     */
    std::vector <SgVariableDeclaration *> OP_DAT_FormalParameters;

    /*
     * ======================================================
     * The formal OP_MAP parameters
     * ======================================================
     */
    std::vector <SgVariableDeclaration *> OP_MAP_FormalParameters;

    /*
     * ======================================================
     * The formal OP_ACCESS parameters
     * ======================================================
     */
    std::vector <SgVariableDeclaration *> OP_ACCESS_FormalParameters;

  public:

    HostSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + "_host")
    {
    }

    virtual
    ~HostSubroutine ()
    {
    }

    void
    set_OP_SET_Argument (SgVariableDeclaration * variableDeclaration)
    {
      OP_SET_FormalParameter = variableDeclaration;
    }

    void
    set_OP_INDIRECTION_Argument (SgVariableDeclaration * variableDeclaration)
    {
      OP_INDIRECTION_FormalParameters.push_back (variableDeclaration);
    }

    void
    set_OP_DAT_Argument (SgVariableDeclaration * variableDeclaration)
    {
      OP_DAT_FormalParameters.push_back (variableDeclaration);
    }

    void
    set_OP_MAP_Argument (SgVariableDeclaration * variableDeclaration)
    {
      OP_MAP_FormalParameters.push_back (variableDeclaration);
    }

    void
    set_OP_ACESS_Argument (SgVariableDeclaration * variableDeclaration)
    {
      OP_ACCESS_FormalParameters.push_back (variableDeclaration);
    }

    SgVariableDeclaration *
    get_OP_SET_Argument () const
    {
      return OP_SET_FormalParameter;
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    first_OP_INDIRECTION_Argument () const
    {
      return OP_INDIRECTION_FormalParameters.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    lastt_OP_INDIRECTION_Argument () const
    {
      return OP_INDIRECTION_FormalParameters.end ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    first_OP_DAT_Argument () const
    {
      return OP_DAT_FormalParameters.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    last_OP_DAT_Argument () const
    {
      return OP_DAT_FormalParameters.end ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    first_OP_MAP_Argument () const
    {
      return OP_MAP_FormalParameters.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    last_OP_MAP_Argument () const
    {
      return OP_MAP_FormalParameters.end ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    first_OP_ACCESS_Argument () const
    {
      return OP_ACCESS_FormalParameters.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    last_OP_ACCESS_Argument () const
    {
      return OP_ACCESS_FormalParameters.end ();
    }
};

class HostSubroutineOfDirectLoop: public HostSubroutine
{
  private:

    /*
     * ======================================================
     * Local variables which model the size of each OP_DAT
     * variable
     * ======================================================
     */
    std::vector <SgVariableDeclaration *> sizeOf_OP_DAT_Variables;

    /*
     * ======================================================
     * Local variables which enable transformation of a C
     * pointer into a Fortran pointer
     * ======================================================
     */
    std::vector <SgVariableDeclaration *> CToFortranPointers;

    /*
     * ======================================================
     * Local variables which are passed as actual parameters
     * to the kernel
     * ======================================================
     */
    std::vector <SgVariableDeclaration *> kernelSubroutineActualParameters;

  public:

    HostSubroutineOfDirectLoop (std::string const & subroutineName) :
      HostSubroutine (subroutineName)
    {
    }

    void
    setSizeOf_OP_DAT_Variable (SgVariableDeclaration * variableDeclaration)
    {
      sizeOf_OP_DAT_Variables.push_back (variableDeclaration);
    }

    void
    setCToFortranPointer (SgVariableDeclaration * variableDeclaration)
    {
      CToFortranPointers.push_back (variableDeclaration);
    }

    void
    setKernelSubroutineActualParameter (
        SgVariableDeclaration * variableDeclaration)
    {
      kernelSubroutineActualParameters.push_back (variableDeclaration);
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    first_SizeOf_OP_DAT_Variable ()
    {
      return sizeOf_OP_DAT_Variables.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    last_SizeOf_OP_DAT_Variable ()
    {
      return sizeOf_OP_DAT_Variables.end ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    first_CToFortranPointer ()
    {
      return CToFortranPointers.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    last_CToFortranPointer ()
    {
      return CToFortranPointers.end ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    first_KernelSubroutineActualParameter ()
    {
      return kernelSubroutineActualParameters.begin ();
    }

    std::vector <SgVariableDeclaration *>::const_iterator
    last_KernelSubroutineActualParameter ()
    {
      return kernelSubroutineActualParameters.end ();
    }
};

class HostSubroutineOfIndirectLoop: public HostSubroutine
{
  public:

    HostSubroutineOfIndirectLoop (std::string const & subroutineName) :
      HostSubroutine (subroutineName)
    {
    }
};

class KernelSubroutine: public Subroutine
{
  public:

    KernelSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + "_kernel")
    {
    }

};

class UserHostSubroutine: public Subroutine
{
  public:

    UserHostSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName)
    {
    }
};

class UserDeviceSubroutine: public Subroutine
{
  public:

    UserDeviceSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + "_device")
    {
    }

    /*
     * ======================================================
     * Copies the user function (launched by the kernel) and
     * applies some modifications so that it can run on the device
     * ======================================================
     */
    void
    copyAndModifySubroutine (SgScopeStatement * moduleScope,
        UserHostSubroutine & userHostSubroutine, Declarations & declarations);
};

#endif

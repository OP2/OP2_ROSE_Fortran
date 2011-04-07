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

#ifndef SUBROUTINES_H
#define SUBROUTINES_H

#include <rose.h>
#include <Declarations.h>

class Subroutine
{
  protected:

    std::string subroutineName;

    /*
     * ======================================================
     * Access to this statement allows ROSE to build function
     * call expressions to the generated subroutine. This
     * is needed, for example, when patching the user-supplied
     * code
     * ======================================================
     */
    SgProcedureHeaderStatement * subroutineHeaderStatement;

    /*
     * ======================================================
     * The parameters of the newly created subroutine
     * ======================================================
     */
    SgFunctionParameterList * formalParameters;

    /*
     * ======================================================
     * The scope of the newly created subroutine
     * ======================================================
     */
    SgScopeStatement * subroutineScope;

  protected:

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

    SgProcedureHeaderStatement *
    getSubroutineHeaderStatement ()
    {
      return subroutineHeaderStatement;
    }
};

#endif

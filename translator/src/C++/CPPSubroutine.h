#ifndef CPP_SUBROUTINE_H
#define CPP_SUBROUTINE_H

#include <Subroutine.h>

class CPPSubroutine: public Subroutine
{
  protected:

    /*
     * ======================================================
     * Access to this statement allows ROSE to build function
     * call expressions to the generated subroutine. This
     * is needed, for example, when patching the user-supplied
     * code
     * ======================================================
     */
    SgFunctionDeclaration * functionDeclaration;

  protected:

    /*
     * ======================================================
     * C++ subroutine constructor with name of the
     * subroutine to create
     * ======================================================
     */
    CPPSubroutine (std::string const & subroutineName);

  public:

    /*
     * ======================================================
     * Returns the function declaration statement used internally
     * by ROSE in its abstract syntax tree
     * ======================================================
     */
    SgFunctionDeclaration *
    getFunctionDeclaration ();
};

#endif

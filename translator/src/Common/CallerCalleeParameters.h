/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the parameter exchange between caller
 * and callee subroutines
 */

#ifndef CALLER_CALLEE_PARAMETERS_H
#define CALLER_CALLEE_PARAMETERS_H

#include <map>

class SgVariableDeclaration;

class CallerCalleeParameters
{
  private:

    /*
     * ======================================================
     * These maps model the variable declarations on the
     * caller and callee sides, respectively
     * ======================================================
     */

    std::map <unsigned int, SgVariableDeclaration *> actualParameters;

    std::map <unsigned int, SgVariableDeclaration *> formalParameters;

  public:

    CallerCalleeParameters ();
};

#endif

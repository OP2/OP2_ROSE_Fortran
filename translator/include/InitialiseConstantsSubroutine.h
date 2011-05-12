#ifndef INITIALISE_CONSTANTS_SUBROUTINE_H
#define INITIALISE_CONSTANTS_SUBROUTINE_H

#include <Subroutine.h>

class InitialiseConstantsSubroutine: public Subroutine
{
  private:

    std::string variableNamePrefix;

    std::map <std::string, SgVariableDeclaration *> constantDeclarations;

    std::vector <std::string> constantVariablesNamesBeforeTransformation;

    std::vector <std::string> constantVariablesNamesAfterTransformation;

  public:

    void
    generateSubroutineForAlreadyComputedValues (SgScopeStatement * moduleScope);

    /*
     * ======================================================
     * Generate the subroutine which initialises the constants
     * ======================================================
     */
    void
    generateSubroutine (SgScopeStatement * moduleScope);

    /*
     * ======================================================
     * Declare the constants at the scope of the generated
     * Fortran module so each included subroutine has visibility
     * ======================================================
     */
    void
    declareConstants (SgScopeStatement * moduleScope);

    std::map <std::string, SgVariableDeclaration *>
    getConstantDeclarations ()
    {
      return constantDeclarations;
    }

    std::vector <std::string>
    getConstantNamesBeforeTransformation ()
    {
      return constantVariablesNamesBeforeTransformation;
    }

    std::vector <std::string>
    getConstantNamesAfterTransformation ()
    {
      return constantVariablesNamesAfterTransformation;
    }

    InitialiseConstantsSubroutine (std::string const & subroutineName);
};

#endif

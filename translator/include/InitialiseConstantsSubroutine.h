#ifndef INITIALISE_CONSTANTS_SUBROUTINE_H
#define INITIALISE_CONSTANTS_SUBROUTINE_H

#include <Subroutine.h>

class InitialiseConstantsSubroutine: public Subroutine
{
  private:

    std::string variableNamePrefix;

    /*
     * ======================================================
     * Mapping from the constant names BEFORE the transformation
     * to the constant names AFTER the transformation
     * ======================================================
     */
    std::map <std::string, std::string> constantVariableNames;

  private:

    /*
     * ======================================================
     * Declare the constants at the scope of the generated
     * Fortran module so each included subroutine has visibility
     * ======================================================
     */
    void
    declareConstants (SgScopeStatement * moduleScope);

  public:

    /*
     * ======================================================
     * Generate the subroutine which initialises the constants
     * ======================================================
     */
    void
    generateSubroutine (SgScopeStatement * moduleScope);

    /*
     * ======================================================
     * An iterator over the constant names before and after
     * the transformation.
     * Points to the first constant name
     * ======================================================
     */
    std::map <std::string, std::string>::const_iterator
    getFirstConstantName ();

    /*
     * ======================================================
     * An iterator over the constant names before and after
     * the transformation.
     * Points to the last constant name
     * ======================================================
     */
    std::map <std::string, std::string>::const_iterator
    getLastConstantName ();

    InitialiseConstantsSubroutine (std::string const & subroutineName,
        SgScopeStatement * moduleScope);
};

#endif

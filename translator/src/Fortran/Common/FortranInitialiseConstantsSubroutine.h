#ifndef FORTRAN_INITIALISE_CONSTANTS_SUBROUTINE_H
#define FORTRAN_INITIALISE_CONSTANTS_SUBROUTINE_H

#include <Subroutine.h>
#include <FortranProgramDeclarationsAndDefinitions.h>

class FortranInitialiseConstantsSubroutine: public Subroutine <
    SgProcedureHeaderStatement>
{
  private:

    SgScopeStatement * moduleScope;

    FortranProgramDeclarationsAndDefinitions * declarations;

    /*
     * ======================================================
     * Mapping from the constant names BEFORE the transformation
     * to the constant names AFTER the transformation
     * ======================================================
     */
    std::map <std::string, std::string> constantVariableNames;

  private:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    /*
     * ======================================================
     * Generate the subroutine which initialises the constants
     * ======================================================
     */
    void
    generateSubroutine ();

    /*
     * ======================================================
     * Declare the constants at the scope of the generated
     * Fortran module so each included subroutine has visibility
     * ======================================================
     */
    void
    declareConstants ();

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

    FortranInitialiseConstantsSubroutine (SgScopeStatement * moduleScope,
        FortranProgramDeclarationsAndDefinitions * declarations);
};

#endif

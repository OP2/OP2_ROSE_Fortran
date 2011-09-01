#ifndef CPP_INITIALISE_CONSTANTS_SUBROUTINE_H
#define CPP_INITIALISE_CONSTANTS_SUBROUTINE_H

#include <Subroutine.h>
#include <CPPProgramDeclarationsAndDefinitions.h>

class CPPInitialiseConstantsSubroutine: 
  public Subroutine < SgProcedureHeaderStatement > // or SgFunctionDefinition? (in fortran = SgProcedureHeaderStatement
{
  private:

    SgScopeStatement * moduleScope;

    CPPProgramDeclarationsAndDefinitions * declarations;

    /*
     * ======================================================
     * Mapping from the constant names BEFORE the transformation
     * to the constant names AFTER the transformation
     * ======================================================
     */
    std::map <std::string, SgExpression *> constantVariableNames;

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
     * CPP module so each included subroutine has visibility
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
    std::map <std::string, SgExpression *>::const_iterator
    getFirstConstantName ();

    /*
     * ======================================================
     * An iterator over the constant names before and after
     * the transformation.
     * Points to the last constant name
     * ======================================================
     */
    std::map <std::string, SgExpression *>::const_iterator
    getLastConstantName ();

    CPPInitialiseConstantsSubroutine (SgScopeStatement * moduleScope,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif

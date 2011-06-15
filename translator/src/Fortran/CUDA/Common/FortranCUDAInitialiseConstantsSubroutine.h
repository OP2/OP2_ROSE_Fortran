#ifndef FORTRAN_CUDA_INITIALISE_CONSTANTS_SUBROUTINE_H
#define FORTRAN_CUDA_INITIALISE_CONSTANTS_SUBROUTINE_H

#include <Subroutine.h>
#include <FortranSubroutine.h>

class FortranCUDAInitialiseConstantsSubroutine: public Subroutine,
    public FortranSubroutine
{
  private:

    SgScopeStatement * moduleScope;

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
    declareConstants ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    void
    generateSubroutine ();

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

    FortranCUDAInitialiseConstantsSubroutine (
        std::string const & subroutineName, SgScopeStatement * moduleScope);
};

#endif

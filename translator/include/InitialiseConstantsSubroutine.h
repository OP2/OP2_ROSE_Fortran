#ifndef INITIALISE_CONSTANTS_SUBROUTINE_H
#define INITIALISE_CONSTANTS_SUBROUTINE_H

#include <Subroutine.h>

/*
 * ======================================================
 * Anonymous namespace so that they have internal linkage,
 * i.e. they cannot be accessed outside of the scope of
 * this header file
 * ======================================================
 */
namespace
{
  /*
   * ======================================================
   * Following are suffixes of symbolic constants needed
   * in each indirect loop. They are suffixes because
   * Fortran does not have a namespace concept, so we
   * prepend the name of the user device subroutine to make
   * them unique.
   *
   * Although these constants are used across all modules
   * implementing an OP_PAR_LOOP (for each indirect loop),
   * we cannot have a single module in which they are
   * included and initialised. This is because currently
   * the compilers generating the final binaries (e.g.
   * PGI) do not support calling functions/subroutines
   * outside of a single Fortran module.
   * ======================================================
   */

  std::string const variableNameSuffix_air_const = "_air_const";
  std::string const variableNameSuffix_alpha = "_alpha";
  std::string const variableNameSuffix_cfl = "_cfl";
  std::string const variableNameSuffix_eps = "_eps";
  std::string const variableNameSuffix_gam = "_gam";
  std::string const variableNameSuffix_gm1 = "_gm1";
  std::string const variableNameSuffix_mach = "_mach";
  std::string const variableNameSuffix_qinf = "_qinf";
}

class InitialiseConstantsSubroutine: public Subroutine
{
  private:

    std::string variableNamePrefix;

    std::map <std::string, SgVariableDeclaration *> constantDeclarations;

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
     * Declare the constants at the scope of the generated
     * Fortran module so each included subroutine has visibility
     * ======================================================
     */
    void
    declareConstants (SgScopeStatement * moduleScope);

    InitialiseConstantsSubroutine (std::string const & subroutineName);
};

#endif

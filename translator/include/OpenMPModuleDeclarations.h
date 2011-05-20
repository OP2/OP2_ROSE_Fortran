#ifndef OPEN_MP_MODULE_DECLARATIONS_H
#define OPEN_MP_MODULE_DECLARATIONS_H

#include <string>
#include <rose.h>

class OpenMPModuleDeclarations
{

  private:

    /*
     * ======================================================
     * The name of the user subroutine
     * ======================================================
     */
    std::string userSubroutineName;

    /*
     * ======================================================
     * Variables declared at module scope level
     * ======================================================
     */
    std::map <std::string, SgVariableDeclaration *> moduleDeclarations;

  private:

    void
    createFirstExecutionBooleanDeclaration (SgScopeStatement * moduleScope);

    std::string
    getFirstExecutionBooleanVariableName ();

  public:

    SgVarRefExp *
    getReferenceToFirstExecutionBoolean ();

    OpenMPModuleDeclarations (SgScopeStatement * moduleScope,
        std::string const & userSubroutineName);

};

#endif

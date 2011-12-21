#ifndef FORTRAN_USER_SUBROUTINE_H
#define FORTRAN_USER_SUBROUTINE_H

#include <UserSubroutine.h>
#include <vector>


class FortranParallelLoop;
class FortranProgramDeclarationsAndDefinitions;

using namespace std;

class FortranUserSubroutine: public UserSubroutine <SgProcedureHeaderStatement,
    FortranProgramDeclarationsAndDefinitions>
{
  protected:
    
   /*
    * ======================================================
    * List of other subroutines called by this subroutine
    * Used to avoid copying the same routine twice
    * ======================================================
    */
    vector < FortranUserSubroutine * > additionalSubroutines;
    
  public:

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

    virtual void appendAdditionalSubroutines ( SgScopeStatement * moduleScope,
      FortranParallelLoop * parallelLoop, FortranProgramDeclarationsAndDefinitions * declarations);
    
    vector < FortranUserSubroutine * > getAdditionalSubroutines()
    {
      return additionalSubroutines;
    }
    
    FortranUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations);

    FortranUserSubroutine (SgScopeStatement * moduleScope,
        FortranParallelLoop * parallelLoop,
        FortranProgramDeclarationsAndDefinitions * declarations,
        string subroutineName);
};

#endif

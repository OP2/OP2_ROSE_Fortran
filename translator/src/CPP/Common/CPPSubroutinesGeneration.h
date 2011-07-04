#ifndef CPP_SUBROUTINES_GENERATION_H
#define CPP_SUBROUTINES_GENERATION_H

#include <rose.h>
#include <SubroutinesGeneration.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPParallelLoop.h>
#include <CPPHostSubroutine.h>

class CPPSubroutinesGeneration: public SubroutinesGeneration <
    CPPProgramDeclarationsAndDefinitions, CPPParallelLoop, CPPHostSubroutine>
{
  protected:

    virtual CPPHostSubroutine
        *
        createSubroutines (CPPParallelLoop * parallelLoop,
            std::string const & userSubroutineName,
            SgScopeStatement * moduleScope) = 0;

    void
    patchCallsToParallelLoops (CPPParallelLoop & parallelLoop,
        std::string const & userSubroutineName,
        CPPHostSubroutine & hostSubroutine, SgScopeStatement * scope,
        SgFunctionCallExp * functionCallExp);

    /*
     * ======================================================
     * Creates the Fortran/C/C++ source file to be unparsed
     * that contains the generated subroutines and declarations
     * ======================================================
     */
    SgSourceFile &
    createSourceFile (CPPParallelLoop & parallelLoop);

    virtual void
    visit (SgNode * node);

  public:

    CPPSubroutinesGeneration (SgProject * project,
        CPPProgramDeclarationsAndDefinitions * declarations);
};

#endif

#ifndef CPP_SUBROUTINES_GENERATION_H
#define CPP_SUBROUTINES_GENERATION_H

#include <rose.h>
#include <SubroutinesGeneration.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPParallelLoop.h>
#include <CPPHostSubroutine.h>

class CPPSubroutinesGeneration: 
  public SubroutinesGeneration <CPPProgramDeclarationsAndDefinitions, CPPHostSubroutine>
{
  protected:

    virtual CPPHostSubroutine *
    createSubroutines (CPPParallelLoop * parallelLoop,
        std::string const & userSubroutineName) = 0;

    void
    patchCallsToParallelLoops (  std::string const & moduleName );

    SgSourceFile &
    createSourceFile ();


    void generate ();

  public:

    CPPSubroutinesGeneration (
        CPPProgramDeclarationsAndDefinitions * declarations,
        std::string const & fileSuffix);
};

#endif

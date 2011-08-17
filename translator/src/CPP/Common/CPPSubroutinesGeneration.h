#ifndef CPP_SUBROUTINES_GENERATION_H
#define CPP_SUBROUTINES_GENERATION_H

#include <rose.h>
#include <SubroutinesGeneration.h>
#include <CPPProgramDeclarationsAndDefinitions.h>
#include <CPPParallelLoop.h>
#include <CPPHostSubroutine.h>
#include <CPPOpDatDimensionsDeclaration.h>

class CPPSubroutinesGeneration: 
  public SubroutinesGeneration <CPPProgramDeclarationsAndDefinitions, CPPHostSubroutine>
{
  protected:
    
    std::map <std::string, CPPOpDatDimensionsDeclaration *>
        dimensionsDeclarations;

    virtual CPPHostSubroutine *
    createSubroutines ( ) = 0;

    void
    patchCallsToParallelLoops ( );

    SgSourceFile &
    createSourceFile ();


    void generate ();

  public:

    CPPSubroutinesGeneration (
        CPPProgramDeclarationsAndDefinitions * declarations,
        std::string const & fileSuffix) :
      SubroutinesGeneration <CPPProgramDeclarationsAndDefinitions, CPPHostSubroutine> (
          declarations, 
          fileSuffix)
    {
      SgSourceFile & sourceFile = createSourceFile ();

      moduleScope = sourceFile.get_globalScope ();
    }
};

#endif

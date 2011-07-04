#ifndef CPP_HOST_SUBROUTINE_H
#define CPP_HOST_SUBROUTINE_H

#include <HostSubroutine.h>
#include <CPPParallelLoop.h>
#include <CPPProgramDeclarationsAndDefinitions.h>

class CPPHostSubroutine: public HostSubroutine <SgFunctionDeclaration,
    CPPProgramDeclarationsAndDefinitions>
{
  protected:

    CPPHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        CPPParallelLoop * parallelLoop) :
      HostSubroutine <SgFunctionDeclaration,
          CPPProgramDeclarationsAndDefinitions> (subroutineName,
          userSubroutineName, kernelSubroutineName, parallelLoop)
    {
    }
};

#endif

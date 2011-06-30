#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename TSubroutineHeader, typename TDeclarations>
  class KernelSubroutine: public Subroutine <TSubroutineHeader>
  {
    protected:

      std::string userSubroutineName;

      ParallelLoop <TSubroutineHeader, TDeclarations> * parallelLoop;

    protected:

      virtual SgStatement *
      createUserSubroutineCallStatement () = 0;

      virtual void
      createExecutionLoopStatements () = 0;

      virtual void
      createOpDatFormalParameterDeclarations ()= 0;

      KernelSubroutine (std::string const & subroutineName,
          std::string const & userSubroutineName, ParallelLoop <
              TSubroutineHeader, TDeclarations> * parallelLoop) :
        Subroutine <TSubroutineHeader> (subroutineName + "_kernel"),
            userSubroutineName (userSubroutineName),
            parallelLoop (parallelLoop)
      {
      }
  };

#endif

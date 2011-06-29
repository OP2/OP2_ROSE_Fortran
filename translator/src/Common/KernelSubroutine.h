#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename T>
  class KernelSubroutine: public Subroutine <T>
  {
    protected:

      std::string userSubroutineName;

      ParallelLoop * parallelLoop;

    protected:

      virtual SgStatement *
      createUserSubroutineCallStatement () = 0;

      virtual void
      createExecutionLoopStatements () = 0;

      virtual void
      createOpDatFormalParameterDeclarations ()= 0;

      KernelSubroutine (std::string const & subroutineName,
          std::string const & userSubroutineName, ParallelLoop * parallelLoop) :
        Subroutine <T> (subroutineName + "_kernel"), userSubroutineName (
            userSubroutineName), parallelLoop (parallelLoop)
      {
      }
  };

#endif

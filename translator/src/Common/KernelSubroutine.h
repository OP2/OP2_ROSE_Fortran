#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename T1, typename T2>
  class KernelSubroutine: public Subroutine <T1>
  {
    protected:

      std::string userSubroutineName;

      ParallelLoop <T1, T2> * parallelLoop;

    protected:

      virtual SgStatement *
      createUserSubroutineCallStatement () = 0;

      virtual void
      createExecutionLoopStatements () = 0;

      virtual void
      createOpDatFormalParameterDeclarations ()= 0;

      KernelSubroutine (std::string const & subroutineName,
          std::string const & userSubroutineName,
          ParallelLoop <T1, T2> * parallelLoop) :
        Subroutine <T1> (subroutineName + "_kernel"), userSubroutineName (
            userSubroutineName), parallelLoop (parallelLoop)
      {
      }
  };

#endif

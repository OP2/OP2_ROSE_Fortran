
#pragma once
#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>

template <typename TSubroutineHeader>
  class ReductionSubroutines;

class ParallelLoop;

template <typename TSubroutineHeader>
  class KernelSubroutine: public Subroutine <TSubroutineHeader>
  {
    protected:

      std::string userSubroutineName;

      ParallelLoop * parallelLoop;

      ReductionSubroutines <TSubroutineHeader> * reductionSubroutines;

    protected:

      virtual SgStatement *
      createUserSubroutineCallStatement () = 0;

      virtual void
      createExecutionLoopStatements () = 0;

      virtual void
      createOpDatFormalParameterDeclarations () = 0;

      KernelSubroutine (std::string const & subroutineName,
          std::string const & userSubroutineName, ParallelLoop * parallelLoop) :
        Subroutine <TSubroutineHeader> (subroutineName + "_kernel"),
            userSubroutineName (userSubroutineName),
            parallelLoop (parallelLoop)
      {
      }
  };

#endif

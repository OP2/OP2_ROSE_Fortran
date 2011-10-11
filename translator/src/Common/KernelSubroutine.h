#pragma once
#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename TSubroutineHeader>
  class ReductionSubroutines;

template <typename TSubroutineHeader>
  class KernelSubroutine: public Subroutine <TSubroutineHeader>
  {
    protected:

      Subroutine <TSubroutineHeader> * userSubroutine;

      ParallelLoop * parallelLoop;

      ReductionSubroutines <TSubroutineHeader> * reductionSubroutines;

    protected:

      virtual SgStatement *
      createUserSubroutineCallStatement () = 0;

      virtual void
      createExecutionLoopStatements () = 0;

      virtual void
      createOpDatFormalParameterDeclarations () = 0;

      KernelSubroutine (Subroutine <TSubroutineHeader> * userSubroutine,
          ParallelLoop * parallelLoop) :
        Subroutine <TSubroutineHeader> (parallelLoop->getUserSubroutineName ()
            + "_kernel"), userSubroutine (userSubroutine), parallelLoop (
            parallelLoop)
      {
      }
  };

#endif

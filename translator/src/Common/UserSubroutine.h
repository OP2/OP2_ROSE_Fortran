#pragma once
#ifndef USER_SUBROUTINE_H
#define USER_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename TSubroutineHeader, typename TDeclarations>
  class UserSubroutine: public Subroutine <TSubroutineHeader>
  {
    protected:

      TDeclarations * declarations;

      ParallelLoop * parallelLoop;

      TSubroutineHeader * originalSubroutine;

    protected:

      UserSubroutine (ParallelLoop * parallelLoop, TDeclarations * declarations) :
        Subroutine <TSubroutineHeader> (parallelLoop->getUserSubroutineName ()
            + "_modified"), declarations (declarations), parallelLoop (
            parallelLoop)
      {
      }
  };

#endif

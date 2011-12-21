#pragma once
#ifndef USER_SUBROUTINE_H
#define USER_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>
#include <vector>

using namespace std;

template <typename TSubroutineHeader, typename TDeclarations>
  class UserSubroutine: public Subroutine <TSubroutineHeader>
  {
    
    protected:

      TDeclarations * declarations;

      ParallelLoop * parallelLoop;

      TSubroutineHeader * originalSubroutine;

      /*
       * ======================================================
       * List of functions called in the user subroutine
       * to be appended to the generated files
       * ======================================================
       */
  
      vector < TSubroutineHeader * > calledRoutines;
      
    protected:

      UserSubroutine (ParallelLoop * parallelLoop, TDeclarations * declarations) :
        Subroutine <TSubroutineHeader> (parallelLoop->getUserSubroutineName ()
            + "_modified"), declarations (declarations), parallelLoop (
            parallelLoop)
      {
      }

      /*
       * ======================================================
       * In this case we do not modify the name
       * + "_modified")
       * ======================================================
       */

      UserSubroutine (ParallelLoop * parallelLoop, TDeclarations * declarations, string subroutineName) :
        Subroutine <TSubroutineHeader> (subroutineName)
            , declarations (declarations), parallelLoop (
            parallelLoop)
      {
      }

  };

#endif

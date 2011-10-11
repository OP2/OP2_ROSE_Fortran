#pragma once
#ifndef HOST_SUBROUTINE_H
#define HOST_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

class SgStatement;

template <typename TSubroutineHeader>
  class HostSubroutine: public Subroutine <TSubroutineHeader>
  {
    protected:

      Subroutine <TSubroutineHeader> * calleeSubroutine;

      ParallelLoop * parallelLoop;

    protected:

      /*
       * ======================================================
       * Create the statement which calls the kernel function
       * ======================================================
       */
      virtual SgStatement *
      createKernelFunctionCallStatement () = 0;

      /*
       * ======================================================
       * Creates the statements needed by a reduction after a
       * kernel call
       * ======================================================
       */
      virtual void
      createReductionEpilogueStatements () = 0;

      /*
       * ======================================================
       * Creates the statements needed by a reduction before a
       * kernel call
       * ======================================================
       */
      virtual void
      createReductionPrologueStatements () = 0;

      /*
       * ======================================================
       * Creates the declarations used in a reduction
       * ======================================================
       */
      virtual void
      createReductionLocalVariableDeclarations () = 0;

      HostSubroutine (Subroutine <TSubroutineHeader> * calleeSubroutine,
          ParallelLoop * parallelLoop) :
        Subroutine <TSubroutineHeader> (parallelLoop->getUserSubroutineName ()
            + "_host"), calleeSubroutine (calleeSubroutine), parallelLoop (
            parallelLoop)
      {
      }
  };

#endif

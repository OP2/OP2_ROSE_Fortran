
#pragma once
#ifndef HOST_SUBROUTINE_H
#define HOST_SUBROUTINE_H

#include <Subroutine.h>

class SgStatement;
class ParallelLoop;

template <typename TSubroutineHeader>
  class HostSubroutine: 
    public Subroutine <TSubroutineHeader>
  {
    protected:

      std::string userSubroutineName;

      std::string kernelSubroutineName;

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

      HostSubroutine (std::string const & subroutineName,
          std::string const & userSubroutineName,
          std::string const & kernelSubroutineName, ParallelLoop * parallelLoop) :
        Subroutine <TSubroutineHeader> (subroutineName + "_host"),
            userSubroutineName (userSubroutineName), kernelSubroutineName (
                kernelSubroutineName), parallelLoop (parallelLoop)
      {
      }
  };

#endif

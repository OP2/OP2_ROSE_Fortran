#ifndef HOST_SUBROUTINE_H
#define HOST_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename T1, typename T2>
  class HostSubroutine: public Subroutine <T1>
  {
    protected:

      std::string userSubroutineName;

      std::string kernelSubroutineName;

      ParallelLoop <T1, T2> * parallelLoop;

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
          std::string const & kernelSubroutineName,
          ParallelLoop <T1, T2> * parallelLoop) :
        Subroutine <T1> (subroutineName + "_host"), userSubroutineName (
            userSubroutineName), kernelSubroutineName (kernelSubroutineName),
            parallelLoop (parallelLoop)
      {
      }
  };

#endif

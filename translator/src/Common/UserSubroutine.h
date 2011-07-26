#ifndef USER_SUBROUTINE_H
#define USER_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename TSubroutineHeader, typename TDeclarations>
  class UserSubroutine: public Subroutine <TSubroutineHeader>
  {
    protected:

      /*
       * ======================================================
       * The name of the user subroutine on the host
       * ======================================================
       */

      std::string hostSubroutineName;

      ParallelLoop <TSubroutineHeader> * parallelLoop;

      TDeclarations * declarations;

    protected:

      UserSubroutine (std::string const & subroutineName,
          TDeclarations * declarations,
          ParallelLoop <TSubroutineHeader> * parallelLoop) :
        Subroutine <TSubroutineHeader> (subroutineName + "_device"),
            hostSubroutineName (subroutineName), declarations (declarations),
            parallelLoop (parallelLoop)
      {
      }

    public:

      /*
       * ======================================================
       * The name of the user subroutine on the host
       * ======================================================
       */
      std::string const &
      getHostSubroutineName ()
      {
        return hostSubroutineName;
      }
  };

#endif

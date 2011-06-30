#ifndef USER_SUBROUTINE_H
#define USER_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename T1, typename T2>
  class UserSubroutine: public Subroutine <T1>
  {
    protected:

      /*
       * ======================================================
       * The name of the user subroutine on the host
       * ======================================================
       */

      std::string hostSubroutineName;

      ParallelLoop <T1, T2> * parallelLoop;

      T2 * declarations;

    protected:

      UserSubroutine (std::string const & subroutineName,
          ParallelLoop <T1, T2> * parallelLoop) :
        Subroutine <T1> (subroutineName + "_device"), hostSubroutineName (
            subroutineName), parallelLoop (parallelLoop)
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

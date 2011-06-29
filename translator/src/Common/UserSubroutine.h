#ifndef USER_SUBROUTINE_H
#define USER_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

template <typename T>
  class UserSubroutine: public Subroutine <T>
  {
    protected:

      /*
       * ======================================================
       * The name of the user subroutine on the host
       * ======================================================
       */

      std::string hostSubroutineName;

      ParallelLoop * parallelLoop;

    protected:

      UserSubroutine (std::string const & subroutineName,
          ParallelLoop * parallelLoop) :
        Subroutine <T> (subroutineName + "_device"), hostSubroutineName (
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

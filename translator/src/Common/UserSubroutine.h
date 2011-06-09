#ifndef USER_SUBROUTINE_H
#define USER_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

class UserSubroutine: public Subroutine
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
        ParallelLoop * parallelLoop);

  public:

    /*
     * ======================================================
     * The name of the user subroutine on the host
     * ======================================================
     */
    std::string const &
    getHostSubroutineName ();
};

#endif

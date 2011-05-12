#ifndef USER_DEVICE_SUBROUTINE_H
#define USER_DEVICE_SUBROUTINE_H

#include <Subroutine.h>
#include <InitialiseConstantsSubroutine.h>
#include <ParallelLoop.h>

class UserDeviceSubroutine: public Subroutine
{
  private:

    std::string userHostSubroutineName;

  private:

    /*
     * ======================================================
     * Copies the user function (launched by the kernel) and
     * applies some modifications so that it can run on the
     * device
     * ======================================================
     */
    void
    copyAndModifySubroutine (SgScopeStatement * moduleScope,
        InitialiseConstantsSubroutine * initialiseConstantsSubroutine,
        Declarations & declarations, ParallelLoop & parallelLoop);

  public:

    UserDeviceSubroutine (std::string const & subroutineName,
        SgScopeStatement * moduleScope,
        InitialiseConstantsSubroutine * initialiseConstantsSubroutine,
        Declarations & declarations, ParallelLoop & parallelLoop);

    /*
     * ======================================================
     * The name of the user subroutine on the host
     * ======================================================
     */
    std::string const &
    getUserHostSubroutineName ()
    {
      return userHostSubroutineName;
    }
};

#endif

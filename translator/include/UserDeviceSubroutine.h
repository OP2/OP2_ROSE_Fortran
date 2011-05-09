#ifndef USER_DEVICE_SUBROUTINE_H
#define USER_DEVICE_SUBROUTINE_H

#include <Subroutine.h>

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
        Declarations & declarations);

  public:

    UserDeviceSubroutine (std::string const & subroutineName,
        SgScopeStatement * moduleScope, Declarations & declarations);

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

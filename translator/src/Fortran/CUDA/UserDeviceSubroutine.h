#ifndef USER_DEVICE_SUBROUTINE_H
#define USER_DEVICE_SUBROUTINE_H

#include <Subroutine.h>
#include <InitialiseConstantsSubroutine.h>
#include <ParallelLoop.h>

class UserDeviceSubroutine: public Subroutine
{
  private:

    InitialiseConstantsSubroutine * initialiseConstantsSubroutine;

    Declarations * declarations;

    ParallelLoop * parallelLoop;

    SgScopeStatement * moduleScope;

    /*
     * ======================================================
     * The name of the user subroutine on the host
     * ======================================================
     */
    std::string hostSubroutineName;

  private:

    /*
     * ======================================================
     * Copies the user function (launched by the kernel) and
     * applies some modifications so that it can run on the
     * device
     * ======================================================
     */
    void
    copyAndModifySubroutine ();

    void
    createFormalParameterDeclarations ();

    void
    createLocalVariableDeclarations ();

    void
    createStatements ();

  public:

    /*
     * ======================================================
     * The name of the user subroutine on the host
     * ======================================================
     */
    std::string const &
    getHostSubroutineName ();

    UserDeviceSubroutine (std::string const & subroutineName,
        InitialiseConstantsSubroutine * initialiseConstantsSubroutine,
        Declarations * declarations, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif

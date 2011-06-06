#ifndef FORTRAN_KERNEL_SUBROUTINE_H
#define FORTRAN_KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <ParallelLoop.h>

class FortranKernelSubroutine: public Subroutine
{
  protected:

    std::string userSubroutineName;

    ParallelLoop * parallelLoop;

  protected:
    /*
     * ======================================================
     * Returns the name of the OP_DAT formal parameter
     * in this OP_DAT argument group
     * ======================================================
     */
    static std::string
    get_OP_DAT_FormalParameterName (unsigned int OP_DAT_ArgumentGroup);

    FortranKernelSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName, ParallelLoop * parallelLoop);
};

#endif

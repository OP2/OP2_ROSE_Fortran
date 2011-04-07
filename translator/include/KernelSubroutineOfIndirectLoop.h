/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class models the kernel subroutine for an indirect loop.
 * Its declarations and statements do the following:
 *
 */

#ifndef KERNEL_SUBROUTINE_OF_INDIRECT_LOOP_H
#define KERNEL_SUBROUTINE_OF_INDIRECT_LOOP_H

#include <KernelSubroutine.h>

class KernelSubroutineOfIndirectLoop: public KernelSubroutine
{
  public:

    KernelSubroutineOfIndirectLoop (std::string const & subroutineName,
        UserDeviceSubroutine & userDeviceSubroutine,
        ParallelLoop & parallelLoop, SgScopeStatement * moduleScope);

};

#endif

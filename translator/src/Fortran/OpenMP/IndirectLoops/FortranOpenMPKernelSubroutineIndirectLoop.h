#ifndef FORTRAN_OPENMP_KERNEL_SUBROUTINE_INDIRECT_LOOP_H
#define FORTRAN_OPENMP_KERNEL_SUBROUTINE_INDIRECT_LOOP_H

#include <FortranOpenMPKernelSubroutine.h>

class FortranOpenMPKernelSubroutineIndirectLoop: public FortranOpenMPKernelSubroutine
{
  private:

    virtual SgStatement *
    createUserSubroutineCallStatement ();

    virtual void
    createExecutionLoopStatements ();

    void
    createPlanFormalParameterDeclarations ();

    virtual void
    createOpDatFormalParameterDeclarations ();

    virtual void
    createStatements ();

    virtual void
    createLocalVariableDeclarations ();

    virtual void
    createFormalParameterDeclarations ();

  public:

    FortranOpenMPKernelSubroutineIndirectLoop (
        std::string const & subroutineName,
        std::string const & userSubroutineName,
        FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
        FortranReductionSubroutines * reductionSubroutines);
};

#endif

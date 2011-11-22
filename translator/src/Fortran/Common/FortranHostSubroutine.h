#pragma once
#ifndef FORTRAN_HOST_SUBROUTINE_H
#define FORTRAN_HOST_SUBROUTINE_H

#include <HostSubroutine.h>

class SgProcedureHeaderStatement;
class SgScopeStatement;
class SgBasicBlock;
class FortranKernelSubroutine;
class FortranParallelLoop;

class FortranHostSubroutine: public HostSubroutine <SgProcedureHeaderStatement>
{
  protected:

    virtual SgBasicBlock *
    createTransferOpDatStatements () = 0;

    virtual void
    createFormalParameterDeclarations ();

    FortranHostSubroutine (SgScopeStatement * moduleScope, Subroutine <
        SgProcedureHeaderStatement> * calleeSubroutine,
        FortranParallelLoop * parallelLoop);
};

#endif

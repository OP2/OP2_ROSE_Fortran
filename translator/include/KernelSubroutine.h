#ifndef KERNEL_SUBROUTINE_H
#define KERNEL_SUBROUTINE_H

#include <Subroutine.h>
#include <UserDeviceSubroutine.h>
#include <ParallelLoop.h>

class KernelSubroutine: public Subroutine
{
  protected:

    SgVariableDeclaration * formalParameter_setSize;

    SgExprListExp *
    createActualParametersForUserDeviceSubroutine (SgVarRefExp * iterSetVarRef,
        SgScopeStatement * subroutineScope, ParallelLoop & op2ParallelLoop);

    void
    setUp_OP_DAT_ArgumentTypes (std::vector <SgType *> & opDatArgumentTypes,
        SgVariableDeclaration * setSizeFormalParameter,
        ParallelLoop & op2ParallelLoop);

    void
    createFormalParameters (ParallelLoop & op2ParallelLoop);

    KernelSubroutine (std::string const & subroutineName) :
      Subroutine (subroutineName + "_kernel")
    {
    }
};

#endif

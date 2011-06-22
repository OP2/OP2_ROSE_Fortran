#ifndef FORTRAN_HOST_SUBROUTINE_H
#define FORTRAN_HOST_SUBROUTINE_H

#include <HostSubroutine.h>
#include <FortranSubroutine.h>
#include <ParallelLoop.h>

class FortranHostSubroutine: public FortranSubroutine, public HostSubroutine
{
  protected:

    virtual void
    createTransferOpDatStatements (SgScopeStatement * statementScope) = 0;

    void
    createOpDatDimensionsDeclaration (SgType * type);

    void
    createOpDatSizesDeclaration (SgType * type);

    virtual void
    createFormalParameterDeclarations ();

    FortranHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName, ParallelLoop * parallelLoop,
        SgScopeStatement * moduleScope);
};

#endif

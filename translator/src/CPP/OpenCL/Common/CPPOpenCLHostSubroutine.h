#pragma once
#ifndef CPP_OPENCL_HOST_SUBROUTINE_H
#define CPP_OPENCL_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPOpenCLDataSizesDeclaration;
class CPPOpDatDimensionsDeclaration;
class CPPOpenCLModuleDeclarations;

class CPPOpenCLHostSubroutine: public CPPHostSubroutine
{
  protected:

    CPPOpenCLDataSizesDeclaration * dataSizesDeclaration;

    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration;

    CPPOpenCLModuleDeclarations * moduleDeclarations;

  protected:

    virtual void
    createReductionPrologueStatements ();

    virtual void
    createReductionEpilogueStatements ();

    virtual void
    createReductionLocalVariableDeclarations ();

    SgStatement *
    createGetKernelStatement (std::string & kernelName);

    SgStatement *
    createKernelArgumentStatement (SgExpression * argumentNumber,
        SgExpression * size, SgExpression * data, SgScopeStatement * scope);

    SgStatement *
    createKernelCallStatement ();

    SgStatement *
    createErrorCheckStatement (std::string & message);

    SgStatement *
    createFinishStatement ();

    virtual SgStatement *
    createKernelFunctionCallStatement ();

    SgStatement *
    createKernelCallBlock (std::string & kernelName, std::vector <std::pair <
        SgExpression *, SgExpression *> > & argList, SgScopeStatement * scope);

    CPPOpenCLHostSubroutine (std::string const & subroutineName,
        std::string const & userSubroutineName,
        std::string const & kernelSubroutineName,
        CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope);
};

#endif

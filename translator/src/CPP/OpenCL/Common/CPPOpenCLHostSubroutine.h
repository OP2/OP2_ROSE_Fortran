#ifndef CPP_OPENCL_HOST_SUBROUTINE_H
#define CPP_OPENCL_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>
#include <CPPInitialiseConstantsSubroutine.h>
#include <CPPOpenCLKernelSubroutine.h>
#include <CPPOpenCLDataSizesDeclaration.h>
#include <CPPOpDatDimensionsDeclaration.h>
#include <CPPParallelLoop.h>
#include <CPPOpenCLModuleDeclarations.h>

class CPPOpenCLHostSubroutine: public CPPHostSubroutine
{
  protected:

  CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

  CPPOpenCLDataSizesDeclaration * dataSizesDeclaration;

  CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration;

  CPPOpenCLModuleDeclarations * moduleDeclarations;
  
  protected:

  SgStatement *
  createGetKernelStatement (
      std::string & kernelName );

  SgStatement *
  createKernelArgumentStatement (
      SgExpression * argumentNumber,
      SgExpression * size,
      SgExpression * data,
      SgScopeStatement * scope );

  SgStatement *
  createKernelCallStatement ();

  SgStatement *
  createErrorCheckStatement (
      std::string & message );

  SgStatement *
  createFinishStatement ();

  SgStatement *
  createKernelFunctionCallStatement ();
  
  void
  createKernelCallBlock ();


  CPPOpenCLHostSubroutine (
      std::string const & subroutineName,
      std::string const & userSubroutineName,
      std::string const & kernelSubroutineName,
      SgScopeStatement * moduleScope, 
      CPPParallelLoop * parallelLoop );
};

#endif

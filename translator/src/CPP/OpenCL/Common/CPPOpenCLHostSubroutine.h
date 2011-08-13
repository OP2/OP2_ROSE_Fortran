#ifndef CPP_OPENCL_HOST_SUBROUTINE_H
#define CPP_OPENCL_HOST_SUBROUTINE_H

#include <CPPHostSubroutine.h>

class CPPOpenCLHostSubroutine: public CPPHostSubroutine
{
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

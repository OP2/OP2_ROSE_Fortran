#pragma once
#ifndef FORTRAN_CUDA_INITIALISE_CONSTANTS_SUBROUTINE_H
#define FORTRAN_CUDA_INITIALISE_CONSTANTS_SUBROUTINE_H

#include <string>
#include <map>

#include <Subroutine.h>
#include <ParallelLoop.h>

class FortranProgramDeclarationsAndDefinitions;

class FortranCUDAInitialiseConstantsSubroutine: public Subroutine <
  SgProcedureHeaderStatement>
{
  private:
  
    FortranProgramDeclarationsAndDefinitions * declarations;

    std::map <std::string, std::string> oldNamesToNewNames;

    ScopedVariableDeclarations * constantDeclarations;

  protected:

    /*
     * ======================================================
     * Every created subroutine has a number of statements
     * ======================================================
     */
    
    virtual void
    createStatements ();

    /*
     * ======================================================
     * Every created subroutine has local variable declarations
     * ======================================================
     */
    
    virtual void
    createLocalVariableDeclarations ();

    /*
     * ======================================================
     * Every created subroutine has formal parameters
     * ======================================================
     */

    virtual void
    createFormalParameterDeclarations ();

  public:
    
    FortranCUDAInitialiseConstantsSubroutine (std::string subroutineName, SgScopeStatement * moduleScope,
      FortranProgramDeclarationsAndDefinitions * allDeclarations, std::map <std::string, std::string> oldNamesToNewNames,
      ScopedVariableDeclarations * _constantDeclarations);
};
  
#endif

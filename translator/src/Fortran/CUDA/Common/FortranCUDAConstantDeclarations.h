#pragma once
#ifndef FORTRAN_CUDA_CONSTANT_DECLARATIONS_H
#define FORTRAN_CUDA_CONSTANT_DECLARATIONS_H

#include <string>
#include <map>

class FortranProgramDeclarationsAndDefinitions;
class ScopedVariableDeclarations;
class SgScopeStatement;
class SgVarRefExp;
class SgProcedureHeaderStatement;
class FortranCUDAInitialiseConstantsSubroutine;

class FortranCUDAConstantDeclarations
{
  private:

    ScopedVariableDeclarations * variableDeclarations;

    FortranCUDAInitialiseConstantsSubroutine * initialisationRoutine;
    
  protected:

    std::map <std::string, std::string> oldNamesToNewNames;

  private:

    std::string
    getNewConstantVariableName (std::string const & originalName);

    void
    addDeclarations (FortranProgramDeclarationsAndDefinitions * declarations,
        SgScopeStatement * moduleScope);

  public:

    SgVarRefExp *
    getReferenceToNewVariable (std::string const & originalName);

    bool
    isCUDAConstant (std::string const & originalName);

    void
    patchReferencesToCUDAConstants (
        SgProcedureHeaderStatement * procedureHeader);

    void
    appendCUDAConstantInitialisationToModule ( SgScopeStatement * moduleScope,
        FortranProgramDeclarationsAndDefinitions * declarations);
        
    FortranCUDAConstantDeclarations (
        FortranProgramDeclarationsAndDefinitions * declarations,
        SgScopeStatement * moduleScope);
};

#endif

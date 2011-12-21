#include "FortranCUDAConstantDeclarations.h"
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "OP2Definitions.h"
#include "ScopedVariableDeclarations.h"
#include "Debug.h"
#include <rose.h>

SgVarRefExp *
FortranCUDAConstantDeclarations::getReferenceToNewVariable (
    std::string const & originalName)
{
  return variableDeclarations->getReference (getNewConstantVariableName (
      originalName));
}

bool
FortranCUDAConstantDeclarations::isCUDAConstant (
    std::string const & originalName)
{
  return oldNamesToNewNames.count (originalName) != 0;
}

std::string
FortranCUDAConstantDeclarations::getNewConstantVariableName (
    std::string const & originalName)
{
  return originalName + "_CUDA";
}

void
FortranCUDAConstantDeclarations::addDeclarations (
    FortranProgramDeclarationsAndDefinitions * declarations,
    SgScopeStatement * moduleScope)
{
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Adding variables with constant access specifiers to module",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (map <string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();

    Debug::getInstance ()->debugMessage ("Analysing OP_DECL_CONST with name '"
        + variableName + "'", Debug::OUTER_LOOP_LEVEL, __FILE__, __LINE__);

    string const & newVariableName = getNewConstantVariableName (variableName);

    oldNamesToNewNames[variableName] = newVariableName;

    variableDeclarations->add (newVariableName,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            newVariableName, type, moduleScope, 1, CUDA_CONSTANT));
  }
}

void
FortranCUDAConstantDeclarations::patchReferencesToCUDAConstants (
    SgProcedureHeaderStatement * procedureHeader)
{
  Debug::getInstance ()->debugMessage ("Patching references to CUDA constants",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  class TreeVisitor: public AstSimpleProcessing
  {
    private:

      FortranCUDAConstantDeclarations * CUDAConstants;

    public:

      TreeVisitor (FortranCUDAConstantDeclarations * CUDAConstants) :
        CUDAConstants (CUDAConstants)
      {
      }

      virtual void
      visit (SgNode * node)
      {
        using std::string;

        SgVarRefExp * variableReference = isSgVarRefExp (node);

        if (variableReference != NULL)
        {
          string const variableName =
              variableReference->get_symbol ()->get_name ();

          if (CUDAConstants->isCUDAConstant (variableName))
          {
            SgVarRefExp * newReference =
                CUDAConstants->getReferenceToNewVariable (variableName);

            variableReference->set_symbol (newReference->get_symbol ());
          }
        }
      }
  };

  TreeVisitor * visitor = new TreeVisitor (this);

  visitor->traverse (procedureHeader, preorder);
}

void
FortranCUDAConstantDeclarations::appendCUDAConstantInitialisationToModule ( SgScopeStatement * moduleScope,
    FortranProgramDeclarationsAndDefinitions * declarations)
{

  /*
   * ======================================================
   * This function appends a new subroutine to the CUDA
   * module which initialises CUDA constants to their
   * respective values. Its call is appended just after
   * the last call to op_decl_const in the user code
   * ======================================================
   */
  
  std::string subroutineName = "initCUDAConstants";
  
  initialisationRoutine = new FortranCUDAInitialiseConstantsSubroutine (subroutineName, moduleScope,
      declarations, oldNamesToNewNames, variableDeclarations);
}

FortranCUDAConstantDeclarations::FortranCUDAConstantDeclarations (
    FortranProgramDeclarationsAndDefinitions * declarations,
    SgScopeStatement * moduleScope)
{
  variableDeclarations = new ScopedVariableDeclarations ();

  addDeclarations (declarations, moduleScope);
}

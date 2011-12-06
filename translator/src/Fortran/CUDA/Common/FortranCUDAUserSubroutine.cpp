#include "FortranCUDAUserSubroutine.h"
#include "FortranCUDAConstantDeclarations.h"
#include "FortranParallelLoop.h"
#include "FortranProgramDeclarationsAndDefinitions.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "Debug.h"
#include "Exceptions.h"
#include "RoseHelper.h"
#include <boost/algorithm/string/predicate.hpp>
#include <algorithm>

void
FortranCUDAUserSubroutine::patchReferencesToCUDAConstants (
    FortranCUDAConstantDeclarations * CUDAConstants)
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
            variableReference->set_symbol (
                CUDAConstants->getReferenceToNewVariable (variableName)->get_symbol ());
          }
        }
      }
  };

  TreeVisitor * visitor = new TreeVisitor (CUDAConstants);

  visitor->traverse (subroutineHeaderStatement, preorder);
}

void
FortranCUDAUserSubroutine::createStatements ()
{
  using namespace SageInterface;
  using boost::iequals;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Outputting and modifying statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgFunctionParameterList * originalParameters =
      originalSubroutine->get_parameterList ();

  vector <SgStatement *> originalStatements =
      originalSubroutine->get_definition ()->get_body ()->get_statements ();

  for (vector <SgStatement *>::iterator it = originalStatements.begin (); it
      != originalStatements.end (); ++it)
  {
    SgVariableDeclaration * isVariableDeclaration = isSgVariableDeclaration (
        *it);

    if (isVariableDeclaration == NULL)
    {
      Debug::getInstance ()->debugMessage (
          "Appending (non-variable-declaration) statement",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      appendStatement (*it, subroutineScope);
    }
    else
    {
      Debug::getInstance ()->debugMessage ("Appending variable declaration",
          Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

      unsigned int OP_DAT_ArgumentGroup = 1;

      for (SgInitializedNamePtrList::iterator variableIt =
          isVariableDeclaration->get_variables ().begin (); variableIt
          != isVariableDeclaration->get_variables ().end (); ++variableIt)
      {
        string const variableName = (*variableIt)->get_name ().getString ();

        SgType * type = (*variableIt)->get_typeptr ();

        bool isFormalParamater = false;

        for (SgInitializedNamePtrList::iterator paramIt =
            originalParameters->get_args ().begin (); paramIt
            != originalParameters->get_args ().end (); ++paramIt, ++OP_DAT_ArgumentGroup)
        {
          string const formalParamterName =
              (*paramIt)->get_name ().getString ();

          if (iequals (variableName, formalParamterName))
          {
            isFormalParamater = true;

            if (parallelLoop->isIndirect (OP_DAT_ArgumentGroup)
                && parallelLoop->isRead (OP_DAT_ArgumentGroup))
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' is an INDIRECT formal parameter which is READ",
                  Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 1, CUDA_SHARED);
            }
            else if (parallelLoop->isGlobal (OP_DAT_ArgumentGroup)
                && !parallelLoop->isArray (OP_DAT_ArgumentGroup)
                && parallelLoop->isRead (OP_DAT_ArgumentGroup))
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' is a GLOBAL SCALAR formal parameter which is READ",
                  Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 1, VALUE);
            }
            else
            {
              Debug::getInstance ()->debugMessage ("'" + variableName
                  + "' is a formal parameter "
                  + parallelLoop->getOpDatInformation (OP_DAT_ArgumentGroup),
                  Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

              SgVariableDeclaration
                  * variableDeclaration =
                      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                          variableName, type, subroutineScope,
                          formalParameters, 1, CUDA_DEVICE);
            }
          }
        }

        if (isFormalParamater == false)
        {
          Debug::getInstance ()->debugMessage ("'" + variableName
              + "' is NOT a formal parameter", Debug::HIGHEST_DEBUG_LEVEL,
              __FILE__, __LINE__);

          SgVariableDeclaration
              * variableDeclaration =
                  FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                      variableName, type, subroutineScope);
        }
      }
    }
  }
}

void
FortranCUDAUserSubroutine::createLocalVariableDeclarations ()
{
}

void
FortranCUDAUserSubroutine::createFormalParameterDeclarations ()
{
}

FortranCUDAUserSubroutine::FortranCUDAUserSubroutine (
    SgScopeStatement * moduleScope, FortranParallelLoop * parallelLoop,
    FortranProgramDeclarationsAndDefinitions * declarations,
    FortranCUDAConstantDeclarations * CUDAConstants) :
  FortranUserSubroutine (moduleScope, parallelLoop, declarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaDevice ();

  createStatements ();

  patchReferencesToCUDAConstants (CUDAConstants);

  /*
   * ======================================================
   * We have to set each node in the AST representation of
   * this subroutine as compiler generated, otherwise chunks
   * of the user kernel are missing in the output
   * ======================================================
   */

  RoseHelper::forceOutputOfCodeToFile (subroutineHeaderStatement);
}

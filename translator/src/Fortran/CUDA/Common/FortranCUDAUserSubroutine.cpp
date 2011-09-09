#include <boost/algorithm/string/predicate.hpp>
#include <FortranCUDAUserSubroutine.h>
#include <Debug.h>
#include <algorithm>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
FortranCUDAUserSubroutine::findOriginalSubroutine ()
{
  using boost::iequals;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Searching for original user subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (vector <SgProcedureHeaderStatement *>::const_iterator it =
      declarations->firstSubroutineInSourceCode (); it
      != declarations->lastSubroutineInSourceCode (); ++it)
  {
    SgProcedureHeaderStatement * subroutine = *it;

    if (iequals (hostSubroutineName, subroutine->get_name ().getString ()))
    {
      originalSubroutine = subroutine;
      break;
    }
  }

  ROSE_ASSERT (originalSubroutine != NULL);
}

void
FortranCUDAUserSubroutine::createStatements ()
{
  using boost::iequals;
  using SageInterface::appendStatement;
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
    /*
     * ======================================================
     * Copy the statement if:
     * 1) It is NOT a variable declaration
     * 2) It is a variable declaration BUT it is not a formal
     * parameter declaration
     * ======================================================
     */
    SgVariableDeclaration * isVariableDeclaration = isSgVariableDeclaration (
        *it);

    if (isVariableDeclaration == NULL)
    {
      appendStatement (*it, subroutineScope);

      if (isSgImplicitStatement (*it) != NULL)
      {
        /*
         * ======================================================
         * Append the variable declarations immediately after the
         * 'IMPLICIT NONE' statement
         * We scan the parallel loop arguments to understand
         * if the parameter will be allocated on shared or device
         * memory. It is based on the assumption that the number
         * of kernel parameters is equal to the number of par.
         * loop argument lines
         * ======================================================
         */

        unsigned int OP_DAT_ArgumentGroup = 1;

        for (SgInitializedNamePtrList::iterator paramIt =
            originalParameters->get_args ().begin (); paramIt
            != originalParameters->get_args ().end (); ++paramIt)
        {
          string const variableName = (*paramIt)->get_name ().getString ();

          SgType * type = (*paramIt)->get_typeptr ();

          if (parallelLoop->isIndirect (OP_DAT_ArgumentGroup)
              && parallelLoop->isRead (OP_DAT_ArgumentGroup))
          {
            SgVariableDeclaration
                * variableDeclaration =
                    FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                        variableName, type, subroutineScope, formalParameters,
                        2, DEVICE, SHARED);
          }
          else if (parallelLoop->isGlobalScalar (OP_DAT_ArgumentGroup))
          {
            SgVariableDeclaration
                * variableDeclaration =
                    FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                        variableName, type, subroutineScope, formalParameters,
                        1, VALUE);
          }
          else
          {
            SgVariableDeclaration
                * variableDeclaration =
                    FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                        variableName, type, subroutineScope, formalParameters,
                        1, DEVICE);
          }

          ++OP_DAT_ArgumentGroup;
        }
      }
    }
    else
    {
      bool isFormalParameter = false;

      string const
          variableName =
              isVariableDeclaration->get_definition ()->get_vardefn ()->get_name ().getString ();

      for (SgInitializedNamePtrList::iterator paramIt =
          formalParameters->get_args ().begin (); paramIt
          != formalParameters->get_args ().end (); ++paramIt)
      {
        string const parameterName = (*paramIt)->get_name ().getString ();

        if (iequals (variableName, parameterName))
        {
          isFormalParameter = true;
        }
      }

      if (isFormalParameter == false)
      {
        /*
         * ======================================================
         * Append the statement to the new subroutine only if it
         * is not a formal parameter
         * ======================================================
         */
        appendStatement (*it, subroutineScope);
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

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDAUserSubroutine::FortranCUDAUserSubroutine (
    std::string const & subroutineName, SgScopeStatement * moduleScope,
    FortranProgramDeclarationsAndDefinitions * declarations,
    FortranParallelLoop * parallelLoop) :
  UserSubroutine <SgProcedureHeaderStatement,
      FortranProgramDeclarationsAndDefinitions> (subroutineName, declarations,
      parallelLoop)
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  appendStatement (subroutineHeaderStatement, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(device) ",
      AstUnparseAttribute::e_before);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  findOriginalSubroutine ();

  createStatements ();
}

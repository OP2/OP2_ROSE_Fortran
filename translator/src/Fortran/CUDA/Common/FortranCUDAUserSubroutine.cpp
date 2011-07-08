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
FortranCUDAUserSubroutine::patchReferencesToConstants ()
{
  using boost::iequals;
  using SageBuilder::buildVarRefExp;
  using std::map;
  using std::string;

  class ModifyReferencesToConstantVariables: public AstSimpleProcessing
  {
    public:

      FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

      ModifyReferencesToConstantVariables (
          FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine)
      {
        this->initialiseConstantsSubroutine = initialiseConstantsSubroutine;
      }

      virtual void
      visit (SgNode * node)
      {
        SgVarRefExp * oldVarRefExp = isSgVarRefExp (node);

        if (oldVarRefExp != NULL)
        {
          for (map <string, string>::const_iterator it =
              initialiseConstantsSubroutine->getFirstConstantName (); it
              != initialiseConstantsSubroutine->getLastConstantName (); ++it)
          {

            if (iequals (it->first,
                oldVarRefExp->get_symbol ()->get_name ().getString ()))
            {
              SgVarRefExp
                  * newVarRefExp =
                      buildVarRefExp (
                          initialiseConstantsSubroutine->getVariableDeclarations ()->get (
                              it->second));

              oldVarRefExp->set_symbol (newVarRefExp->get_symbol ());
            }
          }
        }

        SgLocatedNode * locatedNode = isSgLocatedNode (node);
        if (locatedNode != NULL)
        {
          locatedNode->setOutputInCodeGeneration ();
        }
      }
  };

  (new ModifyReferencesToConstantVariables (initialiseConstantsSubroutine))->traverse (
      subroutineHeaderStatement, preorder);
}

void
FortranCUDAUserSubroutine::createStatements ()
{
  using boost::iequals;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Modifying and outputting user subroutine to file",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgProcedureHeaderStatement * originalSubroutine = NULL;

  SgFunctionParameterList * originalParameters = NULL;

  /*
   * ======================================================
   * Find the original subroutine (otherwise the definition
   * would be empty)
   * ======================================================
   */

  for (vector <SgProcedureHeaderStatement *>::const_iterator it =
      declarations->firstSubroutineInSourceCode (); it
      != declarations->lastSubroutineInSourceCode (); ++it)
  {
    SgProcedureHeaderStatement * subroutine = *it;

    if (iequals (hostSubroutineName, subroutine->get_name ().getString ()))
    {
      /*
       * ======================================================
       * Grab the subroutine and its parameters
       * ======================================================
       */
      originalSubroutine = subroutine;
      originalParameters = subroutine->get_parameterList ();
      break;
    }
  }

  ROSE_ASSERT (originalSubroutine != NULL);

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

        unsigned int opDatCounter = 1;

        for (SgInitializedNamePtrList::iterator paramIt =
            originalParameters->get_args ().begin (); paramIt
            != originalParameters->get_args ().end (); ++paramIt)
        {
          std::string const variableName = (*paramIt)->get_name ().getString ();

          SgType * type = (*paramIt)->get_typeptr ();

          /*
           * ======================================================
           * Set the Fortran attributes of the declared variables:
           * either shared or device. As device is the default
           * attribute in Fortran CUDA, we do not need to define it
           * ======================================================
           */

          if (parallelLoop->getOpMapValue (opDatCounter) == INDIRECT
              && parallelLoop->getOpAccessValue (opDatCounter) == READ_ACCESS)
          {
            SgVariableDeclaration
                * variableDeclaration =
                    FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                        variableName, type, subroutineScope, formalParameters,
                        1, SHARED);
          }
          else
          {
            SgVariableDeclaration
                * variableDeclaration =
                    FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                        variableName, type, subroutineScope, formalParameters);
          }

          ++opDatCounter;
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
    FortranCUDAInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    FortranProgramDeclarationsAndDefinitions * declarations,
    FortranParallelLoop * parallelLoop) :
  UserSubroutine <SgProcedureHeaderStatement,
      FortranProgramDeclarationsAndDefinitions> (subroutineName, declarations,
      parallelLoop), initialiseConstantsSubroutine (
      initialiseConstantsSubroutine)
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

  createStatements ();

  patchReferencesToConstants ();
}

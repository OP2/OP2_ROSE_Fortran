#include <boost/algorithm/string/predicate.hpp>
#include <CPPOpenCLUserSubroutine.h>
#include <Debug.h>
#include <algorithm>
//#include <CPPStatementsAndExpressionsBuilder.h>
//#include <CPPTypesBuilder.h>
using namespace SageBuilder;
/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
CPPOpenCLUserSubroutine::patchReferencesToConstants()
{
  using boost::iequals;
  using SageBuilder::buildVarRefExp;
  using std::map;
  using std::string;

  class ModifyReferencesToConstantVariables : public AstSimpleProcessing
  {
  public:

    CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine;

    ModifyReferencesToConstantVariables(
        CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine ) :
      initialiseConstantsSubroutine( initialiseConstantsSubroutine )
    {
    }

    virtual void
    visit( SgNode * node )
    {
      SgVarRefExp * oldVarRefExp = isSgVarRefExp( node );

      if ( oldVarRefExp != NULL )
        {
          for ( map<string, string>::const_iterator it = initialiseConstantsSubroutine->getFirstConstantName(); 
              it != initialiseConstantsSubroutine->getLastConstantName(); 
              ++it )
            {

              if ( iequals(
                  it->first,
                  oldVarRefExp->get_symbol()->get_name().getString() ) )
                {
                  SgVarRefExp * newVarRefExp = buildVarRefExp(
                      initialiseConstantsSubroutine->getVariableDeclarations()->get( it->second ) );

                  oldVarRefExp->set_symbol( newVarRefExp->get_symbol() );
                }
            }
        }

      SgLocatedNode * locatedNode = isSgLocatedNode( node );
      if ( locatedNode != NULL )
        {
          locatedNode->setOutputInCodeGeneration();
        }
    }
  };

  ( new ModifyReferencesToConstantVariables( initialiseConstantsSubroutine ) )->traverse(
      subroutineHeaderStatement,
      preorder );
}

void
CPPOpenCLUserSubroutine::createStatements()
{
  using boost::iequals;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  Debug::getInstance()->debugMessage(
      "Modifying and outputting user subroutine to file",
      Debug::FUNCTION_LEVEL,
      __FILE__,
      __LINE__);

  SgFunctionDeclaration * originalSubroutine = NULL;

  SgFunctionParameterList * originalParameters = NULL;

  /*
   * ======================================================
   * Find the original subroutine (otherwise the definition
   * would be empty)
   * ======================================================
   */

  for ( vector<SgFunctionDeclaration *>::const_iterator it = declarations->firstSubroutineInSourceCode(); 
      it != declarations->lastSubroutineInSourceCode(); 
      ++it )
    {
      SgFunctionDeclaration * subroutine = *it;

      if ( iequals( hostSubroutineName, subroutine->get_name().getString() ) )
        {
          /*
           * ======================================================
           * Grab the subroutine and its parameters
           * ======================================================
           */
          originalSubroutine = subroutine;
          originalParameters = subroutine->get_parameterList();
          break;
        }
    }

  ROSE_ASSERT (originalSubroutine != NULL);

  vector<SgStatement *> originalStatements =
      originalSubroutine->get_definition()->get_body()->get_statements();
  

  for ( vector<SgStatement *>::iterator it = originalStatements.begin(); 
      it != originalStatements.end(); ++it )
    {
      /*
       * ======================================================
       * Copy the statement if:
       * 1) It is NOT a variable declaration
       * 2) It is a variable declaration BUT it is not a formal
       * parameter declaration
       * ======================================================
       */
      SgVariableDeclaration * isVariableDeclaration = isSgVariableDeclaration(
          *it );

      if ( isVariableDeclaration == NULL )
        {
          appendStatement( *it, subroutineScope );

          //FIXME
          if ( isSgImplicitStatement( *it ) != NULL )
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

              for ( SgInitializedNamePtrList::iterator paramIt = originalParameters->get_args().begin(); 
                  paramIt != originalParameters->get_args().end(); 
                  ++paramIt )
                {
                  std::string const variableName =
                      ( *paramIt )->get_name().getString();

                  SgType * type = ( *paramIt )->get_typeptr();

                  /*
                   * ======================================================
                   * Set the Fortran attributes of the declared variables:
                   * either shared or device. As device is the default
                   * attribute in Fortran CUDA, we do not need to define it
                   * ======================================================
                   */

                  if ( parallelLoop->getOpMapValue( opDatCounter ) == INDIRECT
                      && parallelLoop->getOpAccessValue( opDatCounter )
                          == READ_ACCESS )
                    {

                      SgVariableDeclaration * variableDeclaration =
                          CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter(
                              variableName,
                              type,
                              subroutineScope,
                              formalParameters,
                              1,
                              SHARED );
                    }
                  else
                    {
                      SgVariableDeclaration * variableDeclaration =
                          CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter(
                              variableName,
                              type,
                              subroutineScope,
                              formalParameters );
                      // XXX: DEVICE?
                    }

                  ++opDatCounter;
                }
            }
        }
      else
        {
          bool isFormalParameter = false;

          string const variableName =
              isVariableDeclaration->get_definition()->get_vardefn()->get_name().getString();

          for ( SgInitializedNamePtrList::iterator paramIt = formalParameters->get_args().begin(); 
              paramIt != formalParameters->get_args().end(); 
              ++paramIt )
            {
              string const parameterName = ( *paramIt )->get_name().getString();

              if ( iequals( variableName, parameterName ) )
                {
                  isFormalParameter = true;
                }
            }

          if ( isFormalParameter == false )
            {
              /*
               * ======================================================
               * Append the statement to the new subroutine only if it
               * is not a formal parameter
               * ======================================================
               */
              appendStatement( *it, subroutineScope );
            }
        }
    }
  
  SgVariableDeclaration * globalConstantsParameter =
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter(
          OpenCL::CPP::constants,
          type,
          subroutineScope,
          formalParameters,
          1,
          CONSTANT);
}

void
CPPOpenCLUserSubroutine::createLocalVariableDeclarations()
{
}

void
CPPOpenCLUserSubroutine::createFormalParameterDeclarations()
{
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLUserSubroutine::CPPOpenCLUserSubroutine(
    std::string const & subroutineName,
    SgScopeStatement * moduleScope,
    CPPInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    CPPProgramDeclarationsAndDefinitions * declarations,
    CPPParallelLoop * parallelLoop ) :
  UserSubroutine<SgFunctionDeclaration, CPPProgramDeclarationsAndDefinitions> (
      subroutineName,
      declarations,
      parallelLoop ), 
  initialiseConstantsSubroutine( initialiseConstantsSubroutine )
{
  using SageBuilder::buildDefiningFunctionDeclaration;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;


  subroutineHeaderStatement = buildDefiningFunctionDeclaration(
      this->subroutineName.c_str(),
      buildVoidType(),
      formalParameters,
      moduleScope );

  appendStatement( subroutineHeaderStatement, moduleScope );
  
  /*
   * Inline user kernel function
   */
  subroutineHeaderStatement->get_functionModifier().setInline();

  subroutineScope = subroutineHeaderStatement->get_definition()->get_body();

  createStatements();

  patchReferencesToConstants();
}

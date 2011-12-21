#include <OP2Definitions.h>
#include <FortranCUDAInitialiseConstantsSubroutine.h>
#include <FortranProgramDeclarationsAndDefinitions.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <CompilerGeneratedNames.h>
#include <FortranTypesBuilder.h>

void
FortranCUDAInitialiseConstantsSubroutine::createStatements ()
{
  using namespace SageInterface;
  using namespace SageBuilder;
  using namespace LoopVariableNames;

  /*
   * ======================================================
   * Create a list of assignment from constant names
   * to cuda constant names. Handle array assignment as
   * do-loops
   * ======================================================
   */

  Debug::getInstance ()->debugMessage ("Appending assignments for CUDA constants initialisation",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (std::map <std::string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    std::string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();
 
    if ( isSgArrayType (type) != NULL )
      {
	SgBasicBlock * initLoopBody = buildBasicBlock ();

	SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          constantDeclarations->getReference (oldNamesToNewNames[variableName]),
	  variableDeclarations->getReference (getIterationCounterVariableName (1)));

	SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          variableDeclarations->getReference (variableName),
	  variableDeclarations->getReference (getIterationCounterVariableName (1)));

	SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression1, arrayExpression2);

	appendStatement (assignmentStatement, initLoopBody);

	SgAssignOp * initLoopLowerBoundExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (1));	

	SgFortranDo * initialisationLoop =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
	      initLoopLowerBoundExpression, buildIntVal(constDefinition->getDimension()),
              buildIntVal (1), initLoopBody);

	appendStatement (initialisationLoop, subroutineScope);
      } 
    else
      {
	SgExprStatement * assignmentStatement = buildAssignStatement (
          constantDeclarations->getReference ( oldNamesToNewNames[it->first] ),
	  variableDeclarations->getReference ( it->first ));

	appendStatement (assignmentStatement, subroutineScope);
      }
  }
}

void
FortranCUDAInitialiseConstantsSubroutine::createLocalVariableDeclarations ()
{
  
}

void
FortranCUDAInitialiseConstantsSubroutine::createFormalParameterDeclarations ()
{
  using std::vector;
  using std::map;
  using std::string;
  using namespace SageInterface;
  using namespace SageBuilder;
  using namespace LoopVariableNames;
  
  Debug::getInstance ()->debugMessage ("Outputting formal parameters for CUDA constant initialisation routine",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

      
  for (std::map <std::string, OpConstDefinition *>::const_iterator it =
      declarations->firstOpConstDefinition (); it
      != declarations->lastOpConstDefinition (); ++it)
  {
    std::string const & variableName = it->first;

    OpConstDefinition * constDefinition = it->second;

    SgType * type = constDefinition->getType ();
    
    variableDeclarations->add (
      variableName,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
        variableName, type, subroutineScope, formalParameters));

  }

  /*
   * ======================================================
   * Append iteration variable for initialising 
   * constant arrays
   * ======================================================
   */
  variableDeclarations ->add (getIterationCounterVariableName (1),
    FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
        getIterationCounterVariableName (1),
        FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}



FortranCUDAInitialiseConstantsSubroutine::FortranCUDAInitialiseConstantsSubroutine (std::string subroutineName, SgScopeStatement * moduleScope,
  FortranProgramDeclarationsAndDefinitions * allDeclarations, std::map <std::string, std::string> _oldNamesToNewNames,
  ScopedVariableDeclarations * _constantDeclarations):
  Subroutine <SgProcedureHeaderStatement> (subroutineName), declarations(allDeclarations), oldNamesToNewNames(_oldNamesToNewNames),
  constantDeclarations(_constantDeclarations)
{
  using namespace SageInterface;
  using namespace SageBuilder;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
      
  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  appendStatement (subroutineHeaderStatement, moduleScope);

  appendStatement (
      FortranStatementsAndExpressionsBuilder::buildImplicitNoneStatement (),
      subroutineScope);

  createFormalParameterDeclarations ();
  
  createStatements (); 
}

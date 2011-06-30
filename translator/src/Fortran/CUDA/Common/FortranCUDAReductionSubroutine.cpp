#include <boost/lexical_cast.hpp>
#include <ROSEHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <FortranCUDAReductionSubroutine.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

SgStatement *
FortranCUDAReductionSubroutine::createSynchThreadsCallStatement ()
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (
          CUDA::Fortran::syncthreads, subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  return buildExprStatement (subroutineCall);
}

void
FortranCUDAReductionSubroutine::createStatements ()
{
  using SageInterface::appendStatement;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildGreaterThanOp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildVoidType;

  /*
   * ======================================================
   * Initialise local variables
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (buildDotExp (
      buildOpaqueVarRefExp (CUDA::Fortran::threadidx, subroutineScope),
      buildOpaqueVarRefExp (CUDA::Fortran::x, subroutineScope)),
      buildIntVal (1));

  SgExpression * tidInitialAssignExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::threadID)),
      subtractExpression1);

  appendStatement (buildExprStatement (tidInitialAssignExpression),
      subroutineScope);

  SgExpression * blockdimXExpr = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::blockidx, subroutineScope), buildOpaqueVarRefExp (
      CUDA::Fortran::x, subroutineScope));

  SgFunctionSymbol * shiftFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("ishft", subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (blockdimXExpr,
      buildIntVal (-1));

  SgFunctionCallExp * shiftFunctionCall = buildFunctionCallExp (
      shiftFunctionSymbol, actualParameters);

  SgExpression * initIterationCounter = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      shiftFunctionCall);

  appendStatement (buildExprStatement (initIterationCounter), subroutineScope);

  /*
   * ======================================================
   * startoffset is given in terms of bytes. it must be
   * recomputed in terms of the fortran kind type of the
   * reduction variable
   * ======================================================
   */

  SgExpression * autosharedKindSize =
      reductionVariableType->get_base_type ()->get_type_kind ();

  /*
   * ======================================================
   * If the user hasn't specified a Fortran kind, we have
   * assume standard ones: integer(4) and real(4)
   * ======================================================
   */

  if (autosharedKindSize == NULL && isSgTypeInt (
      reductionVariableType->get_base_type ()) != NULL)
  {
    autosharedKindSize = buildIntVal (4);
  }

  SgVarRefExp * autosharedOffsetReference = buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::sharedMemoryStartOffset));

  SgAssignOp * initStartOffset = buildAssignOp (autosharedOffsetReference,
      buildDivideOp (autosharedOffsetReference, autosharedKindSize));

  appendStatement (buildExprStatement (initStartOffset), subroutineScope);

  appendStatement (createSynchThreadsCallStatement (), subroutineScope);

  /*
   * ======================================================
   * Builds initial assignment of computed value to shared
   * memory position
   * ======================================================
   */
  SgExpression * arrayIndexExpression = buildAddOp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)), buildVarRefExp (
          variableDeclarations->get (ReductionSubroutine::threadID)));

  SgPntrArrRefExp * autosharedSubscriptExpression = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::autoshared)), arrayIndexExpression);

  SgAssignOp * initAutoshared = buildAssignOp (autosharedSubscriptExpression,
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::inputValue)));

  appendStatement (buildExprStatement (initAutoshared), subroutineScope);

  SgExpression * mainLoopCondition = buildGreaterThanOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (0));

  SgExpression * ifGuardExpression = buildLessThanOp (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::threadID)),
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

  // same expression as above
  SgPntrArrRefExp * autosharedMyThreadPosition = autosharedSubscriptExpression;

  // same expression as above plus the iteration counter
  SgExpression * autosharedOtherThreadPositionExpr = buildAddOp (
      arrayIndexExpression, buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

  SgPntrArrRefExp * autosharedOtherThreadPosition = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::autoshared)), autosharedOtherThreadPositionExpr);

  SgExpression * reductionPartialComputation = buildAssignOp (
      autosharedMyThreadPosition, buildAddOp (autosharedMyThreadPosition,
          autosharedOtherThreadPosition));

  SgBasicBlock * ifBlock = buildBasicBlock (buildExprStatement (
      reductionPartialComputation));

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  SgExprListExp * actualParametersItCountReassign = buildExprListExp (
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)), buildIntVal (-1));

  SgFunctionCallExp * shiftFunctionCallInsideMailLoop = buildFunctionCallExp (
      shiftFunctionSymbol, actualParametersItCountReassign);

  SgExpression * reassignIterationCounter = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      shiftFunctionCallInsideMailLoop);

  SgStatement * mainLoopBody = buildBasicBlock (
      createSynchThreadsCallStatement (), ifStatement, buildExprStatement (
          reassignIterationCounter));

  SgWhileStmt * reductionImplementation = buildWhileStmt (buildExprStatement (
      mainLoopCondition), mainLoopBody);

  reductionImplementation->set_has_end_statement (true);

  appendStatement (reductionImplementation, subroutineScope);

  /*
   * ======================================================
   * Finalisation: synchronisation plus final sum
   * ======================================================
   */
  appendStatement (createSynchThreadsCallStatement (), subroutineScope);

  SgExpression * ifThreadIdGreaterZero = buildEqualityOp (buildVarRefExp (
      variableDeclarations->get (ReductionSubroutine::threadID)), buildIntVal (
      0));

  SgPntrArrRefExp * deviceVariableUniquePosition = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::reductionResultOnDevice)), buildIntVal (1));

  SgPntrArrRefExp * autosharedFinalReductionResultPosition =
      buildPntrArrRefExp (buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::autoshared)), buildVarRefExp (
          variableDeclarations->get (
              ReductionSubroutine::sharedMemoryStartOffset)));

  SgExpression * reductionFinalComputation = buildAssignOp (
      deviceVariableUniquePosition, buildAddOp (deviceVariableUniquePosition,
          autosharedFinalReductionResultPosition));

  SgBasicBlock * finalIfBlock = buildBasicBlock (buildExprStatement (
      reductionFinalComputation));

  SgIfStmt * finalIfStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifThreadIdGreaterZero, finalIfBlock);

  appendStatement (finalIfStatement, subroutineScope);
}

void
FortranCUDAReductionSubroutine::createLocalVariableDeclarations ()
{
  using std::vector;
  using std::string;

  /*
   * ======================================================
   * Create autoshared variable declaration with the same
   * base type as the reduction variable
   * ======================================================
   */

  SgArrayType * arrayType = FortranTypesBuilder::getArray_RankOne (
      reductionVariableType->get_base_type (), 0, new SgAsteriskShapeExp (
          ROSEHelper::getFileInfo ()));

  variableDeclarations->add (CommonVariableNames::autoshared,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::autoshared, arrayType, subroutineScope, 1,
          SHARED));

  /*
   * ======================================================
   * Create thread ID and iteration counter variables
   * ======================================================
   */

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (CommonVariableNames::iterationCounter1);

  fourByteIntegers.push_back (ReductionSubroutine::threadID);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }
}

void
FortranCUDAReductionSubroutine::createFormalParameterDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating reduction procedure formal parameter", 2);

  /*
   * ======================================================
   * Declare the device variable on which the result of local
   * reductions is stored by the first thread in the block
   * ======================================================
   */
  variableDeclarations->add (
      ReductionSubroutine::reductionResultOnDevice,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::reductionResultOnDevice, reductionVariableType,
          subroutineScope, formalParameters, 1, DEVICE));

  /*
   * ======================================================
   * Declare the value of the reduction variable produced by
   * each thread which is passed by value.
   * WARNING: the type of the input data is for now limited
   * to a scalar value (does not manage reduction on arrays)
   * ======================================================
   */

  variableDeclarations->add (
      ReductionSubroutine::inputValue,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::inputValue,
          reductionVariableType->get_base_type (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the the warp size: currently, in CUDA Fortran
   * it is not possible to access the warpsize variable value
   * in a device subroutine (only possible in global ones)
   * ======================================================
   */

  variableDeclarations->add (
      ReductionSubroutine::warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the the offset from which the automatically allocated
   * shared memory variable space is reserved for use by
   * the reduction subroutine
   * ======================================================
   */

  variableDeclarations->add (
      ReductionSubroutine::sharedMemoryStartOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          ReductionSubroutine::sharedMemoryStartOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

/*
 * ======================================================
 * WARNING:
 * We assume that the subroutineAndVariableName include both
 * the user subroutine name plus the variable name to which
 * reduction is referred.
 * We suppose that there is only one possible instance of
 * OP_GBL on which reduction is performed passed to an
 * OP_PAR_LOOP.
 * ======================================================
 */

FortranCUDAReductionSubroutine::FortranCUDAReductionSubroutine (
    std::string const & subroutineAndVariableName,
    SgScopeStatement * moduleScope, SgArrayType * reductionVariableType) :
  Subroutine <SgProcedureHeaderStatement> (subroutineAndVariableName),
      reductionVariableType (reductionVariableType)
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(device) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}

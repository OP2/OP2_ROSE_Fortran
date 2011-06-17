#include <boost/lexical_cast.hpp>
#include <FortranCUDAReductionSubroutine.h>
#include <ROSEHelper.h>
#include <Debug.h>
#include <CommonNamespaces.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>

SgStatement *
FortranCUDAReductionSubroutine::createCallToSynchThreads ()
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine (
          CUDA::Fortran::SubroutineNames::syncthreads, subroutineScope);

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
      buildOpaqueVarRefExp (CUDA::Fortran::VariableNames::threadidx,
          subroutineScope), buildOpaqueVarRefExp (CUDA::Fortran::FieldNames::x,
          subroutineScope)), buildIntVal (1));

  SgExpression
      * tidInitialAssignExpression =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID)),
              subtractExpression1);

  appendStatement (buildExprStatement (tidInitialAssignExpression),
      subroutineScope);

  SgExpression * blockdimXExpr = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::VariableNames::blockidx, subroutineScope),
      buildOpaqueVarRefExp (CUDA::Fortran::FieldNames::x, subroutineScope));

  SgFunctionSymbol
      * shiftFunctionSymbol =
          FortranTypesBuilder::buildNewFortranFunction (
              IndirectAndDirectLoop::Fortran::ReductionSubroutine::SubroutineNames::shift,
              subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (blockdimXExpr,
      buildIntVal (-1));

  SgFunctionCallExp * shiftFunctionCall = buildFunctionCallExp (
      shiftFunctionSymbol, actualParameters);

  SgExpression
      * initIterationCounter =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter)),
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

  SgVarRefExp
      * autosharedOffsetReference =
          buildVarRefExp (
              variableDeclarations->getDeclaration (
                  IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset));

  SgAssignOp * initStartOffset = buildAssignOp (autosharedOffsetReference,
      buildDivideOp (autosharedOffsetReference, autosharedKindSize));

  appendStatement (buildExprStatement (initStartOffset), subroutineScope);

  appendStatement (createCallToSynchThreads (), subroutineScope);

  /*
   * ======================================================
   * Builds initial assignment of computed value to shared
   * memory position
   * ======================================================
   */
  SgExpression
      * arrayIndexExpression =
          buildAddOp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset)),
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID)));

  SgPntrArrRefExp * autosharedSubscriptExpression = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->getDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::autoshared)),
      arrayIndexExpression);

  SgAssignOp
      * initAutoshared =
          buildAssignOp (
              autosharedSubscriptExpression,
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::inputValue)));

  appendStatement (buildExprStatement (initAutoshared), subroutineScope);

  SgExpression
      * mainLoopCondition =
          buildGreaterThanOp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter)),
              buildIntVal (0));

  SgExpression
      * ifGuardExpression =
          buildLessThanOp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID)),
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter)));

  // same expression as above
  SgPntrArrRefExp * autosharedMyThreadPosition = autosharedSubscriptExpression;

  // same expression as above plus the iteration counter
  SgExpression
      * autosharedOtherThreadPositionExpr =
          buildAddOp (
              arrayIndexExpression,
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter)));

  SgPntrArrRefExp * autosharedOtherThreadPosition = buildPntrArrRefExp (
      buildVarRefExp (variableDeclarations->getDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::autoshared)),
      autosharedOtherThreadPositionExpr);

  SgExpression * reductionPartialComputation = buildAssignOp (
      autosharedMyThreadPosition, buildAddOp (autosharedMyThreadPosition,
          autosharedOtherThreadPosition));

  SgBasicBlock * ifBlock = buildBasicBlock (buildExprStatement (
      reductionPartialComputation));

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  SgExprListExp
      * actualParametersItCountReassign =
          buildExprListExp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter)),
              buildIntVal (-1));

  SgFunctionCallExp * shiftFunctionCallInsideMailLoop = buildFunctionCallExp (
      shiftFunctionSymbol, actualParametersItCountReassign);

  SgExpression
      * reassignIterationCounter =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter)),
              shiftFunctionCallInsideMailLoop);

  SgStatement * mainLoopBody = buildBasicBlock (createCallToSynchThreads (),
      ifStatement, buildExprStatement (reassignIterationCounter));

  SgWhileStmt * reductionImplementation = buildWhileStmt (buildExprStatement (
      mainLoopCondition), mainLoopBody);

  reductionImplementation->set_has_end_statement (true);

  appendStatement (reductionImplementation, subroutineScope);

  /*
   * ======================================================
   * Finalisation: synchronisation plus final sum
   * ======================================================
   */
  appendStatement (createCallToSynchThreads (), subroutineScope);

  SgExpression
      * ifThreadIdGreaterZero =
          buildEqualityOp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID)),
              buildIntVal (0));

  SgPntrArrRefExp
      * deviceVariableUniquePosition =
          buildPntrArrRefExp (
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionResultOnDevice)),
              buildIntVal (1));

  SgPntrArrRefExp
      * autosharedFinalReductionResultPosition =
          buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->getDeclaration (
                  IndirectAndDirectLoop::Fortran::VariableNames::autoshared)),
              buildVarRefExp (
                  variableDeclarations->getDeclaration (
                      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset)));

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

  variableDeclarations->addDeclaration (
      IndirectAndDirectLoop::Fortran::VariableNames::autoshared,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::autoshared, arrayType,
          subroutineScope, 1, SHARED));

  /*
   * ======================================================
   * Create thread ID and iteration counter variables
   * ======================================================
   */

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (
      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter);

  fourByteIntegers.push_back (
      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID);

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
  variableDeclarations->addDeclaration (
      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionResultOnDevice,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionResultOnDevice,
          reductionVariableType, subroutineScope, formalParameters, 1, DEVICE));

  /*
   * ======================================================
   * Declare the value of the reduction variable produced by
   * each thread which is passed by value.
   * WARNING: the type of the input data is for now limited
   * to a scalar value (does not manage reduction on arrays)
   * ======================================================
   */

  variableDeclarations->addDeclaration (
      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::inputValue,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::inputValue,
          reductionVariableType->get_base_type (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the the warp size: currently, in CUDA Fortran
   * it is not possible to access the warpsize variable value
   * in a device subroutine (only possible in global ones)
   * ======================================================
   */

  variableDeclarations->addDeclaration (
      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Declare the the offset from which the automatically allocated
   * shared memory variable space is reserved for use by
   * the reduction subroutine
   * ======================================================
   */

  variableDeclarations->addDeclaration (
      IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset,
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
  Subroutine (subroutineAndVariableName), FortranSubroutine (),
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

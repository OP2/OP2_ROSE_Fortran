#include <ReductionSubroutine.h>
#include <ROSEHelper.h>
#include <Debug.h>
#include <FortranTypesBuilder.h>
#include <CommonNamespaces.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <boost/lexical_cast.hpp>

//TODO: generate code for OP_MIN and OP_MAX as well

SgStatement *
ReductionSubroutine::createCallToSynchThreads ()
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
ReductionSubroutine::createFormalParameters ()
{
  Debug::getInstance ()->debugMessage (
      "Creating reduction procedure formal parameter", 2);

  /*
   * ======================================================
   * Declare the device variable on which the result of local
   * reductions is stored by the first thread in the block
   * ======================================================
   */
  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionResultOnDevice]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionResultOnDevice,
          reductionVariableType, subroutineScope);

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionResultOnDevice]->get_declarationModifier ().get_typeModifier ().setDevice ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionResultOnDevice]->get_variables ().begin ()));

  /*
   * ======================================================
   * Declare the value of the reduction variable produced by
   * each thread which is passed by value.
   * WARNING: the type of the input data is for now limited
   * to a scalar value (does not manage reduction on arrays)
   * ======================================================
   */

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::inputValue]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::inputValue,
          reductionVariableType->get_base_type (), subroutineScope);

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::inputValue]->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::inputValue]->get_variables ().begin ()));

  /*
   * ======================================================
   * Declare the the warp size: currently, in CUDA Fortran
   * it is not possible to access the warpsize variable value
   * in a device subroutine (only possible in global ones)
   * ======================================================
   */

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::warpSize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::warpSize]->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::warpSize]->get_variables ().begin ()));

  /*
   * ======================================================
   * Declare the the offset from which the automatically allocated
   * shared memory variable space is reserved for use by
   * the reduction subroutine
   * ======================================================
   */

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]->get_variables ().begin ()));
}

void
ReductionSubroutine::createLocalVariables ()
{
  using std::vector;
  using std::string;

  /*
   * ======================================================
   * Create autoshared variable declaration with the same
   * base type as the reduction variable
   * ======================================================
   */

  SgExpression * upperBound = new SgAsteriskShapeExp (
      ROSEHelper::getFileInfo ());

  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          IndirectAndDirectLoop::Fortran::VariableNames::autoshared,
          FortranTypesBuilder::getArray_RankOne (
              reductionVariableType->get_base_type (), 0, upperBound),
          subroutineScope);

  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]->get_declarationModifier ().get_typeModifier ().setShared ();

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
    localVariableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, FortranTypesBuilder::getFourByteInteger (), subroutineScope);
  }
}

void
ReductionSubroutine::createStatements ()
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
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID]),
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
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter]),
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
   * if the user hasn't specified a fortran kind, we have
   * assume standard ones: integer(4) and real(4)
   * ======================================================
   */

  if (autosharedKindSize == NULL)
  {

    if (isSgTypeInt (reductionVariableType->get_base_type ()) != NULL)
    {
      autosharedKindSize = buildIntVal (
          FortranVariableDeafultKinds::DEFAULT_KIND_INT);
    }
    else
    {
      autosharedKindSize = buildIntVal (
          FortranVariableDeafultKinds::DEFAULT_KIND_REAL);
    }
  }

  SgVarRefExp
      * autosharedOffsetReference =
          buildVarRefExp (
              formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]);

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
                  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]),
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID]));

  SgPntrArrRefExp
      * autosharedSubscriptExpression =
          buildPntrArrRefExp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]),
              arrayIndexExpression);

  SgAssignOp
      * initAutoshared =
          buildAssignOp (
              autosharedSubscriptExpression,
              buildVarRefExp (
                  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::inputValue]));

  appendStatement (buildExprStatement (initAutoshared), subroutineScope);

  SgExpression
      * mainLoopCondition =
          buildGreaterThanOp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter]),
              buildIntVal (0));

  SgExpression
      * ifGuardExpression =
          buildLessThanOp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID]),
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter]));

  // same expression as above
  SgPntrArrRefExp * autosharedMyThreadPosition = autosharedSubscriptExpression;

  // same expression as above plus the iteration counter
  SgExpression
      * autosharedOtherThreadPositionExpr =
          buildAddOp (
              arrayIndexExpression,
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter]));

  SgPntrArrRefExp
      * autosharedOtherThreadPosition =
          buildPntrArrRefExp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]),
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
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter]),
              buildIntVal (-1));

  SgFunctionCallExp * shiftFunctionCallInsideMailLoop = buildFunctionCallExp (
      shiftFunctionSymbol, actualParametersItCountReassign);

  SgExpression
      * reassignIterationCounter =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::iterationCounter]),
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
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::threadID]),
              buildIntVal (0));

  SgPntrArrRefExp
      * deviceVariableUniquePosition =
          buildPntrArrRefExp (
              buildVarRefExp (
                  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::reductionResultOnDevice]),
              buildIntVal (1));

  SgPntrArrRefExp
      * autosharedFinalReductionResultPosition =
          buildPntrArrRefExp (
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]),
              buildVarRefExp (
                  formalParameterDeclarations[IndirectAndDirectLoop::Fortran::ReductionSubroutine::VariableNames::sharedMemoryStartOffset]));

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

ReductionSubroutine::ReductionSubroutine (
    std::string const & subroutineAndVariableName,
    SgScopeStatement * moduleScope, SgArrayType * reductionVariableType) :
  Subroutine (subroutineAndVariableName)
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(device) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  this->reductionVariableType = reductionVariableType;

  createFormalParameters ();

  createLocalVariables ();

  createStatements ();
}

std::map <unsigned int, SgProcedureHeaderStatement *>
ReductionSubroutine::generateReductionSubroutines (ParallelLoop & parallelLoop,
    SgScopeStatement * scopeStatement)
{
  using boost::lexical_cast;
  using std::string;
  using std::map;

  map <unsigned int, SgProcedureHeaderStatement *> reductionSubroutines;

  if (parallelLoop.isReductionRequired () == true)
  {
    for (unsigned int i = 1; i
        <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
    {
      if (parallelLoop.isReductionRequired (i) == true)
      {
        /*
         * ======================================================
         * Generates the reduction subroutine name:
         * <userKernelName> + "_reduction" + "_type"
         * ======================================================
         */

        SgType * opDatType = parallelLoop.get_OP_DAT_Type (i);

        SgArrayType * isArrayType = isSgArrayType (opDatType);

        ROSE_ASSERT ( isArrayType != NULL );

        SgExpression * opDatKindSize =
            FortranStatementsAndExpressionsBuilder::getFortranKindOf_OP_DAT (
                isArrayType);

        SgIntVal * isKindIntVal = isSgIntVal (opDatKindSize);

        ROSE_ASSERT ( isKindIntVal != NULL );

        string typeName;

        if (isSgTypeInt (isArrayType->get_base_type ()) != NULL)
        {
          typeName = SubroutineNames::integerSuffix;
        }
        else if (isSgTypeFloat (isArrayType->get_base_type ()) != NULL)
        {
          typeName = SubroutineNames::floatSuffix;
        }
        else
        {
          Debug::getInstance ()->errorMessage (
              "Error: type for reduction variable is not supported");
        }

        /*
         * ======================================================
         * For now we distinguish between subroutines by also
         * appending the index of the related OP_DAT argument.
         * Eventually, the factorisation will solve this problem
         * ======================================================
         */
        string const reductionSubroutineName =
            IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
                + lexical_cast <string> (i) + SubroutineNames::reductionSuffix
                + typeName + lexical_cast <string> (isKindIntVal->get_value ());

        ReductionSubroutine * reductionSubroutine = new ReductionSubroutine (
            reductionSubroutineName, scopeStatement, isArrayType);

        /*
         * ======================================================
         * Generate one per reduction variable, eventually
         * we will have to factorise
         * ======================================================
         */
        reductionSubroutines[i]
            = reductionSubroutine->getSubroutineHeaderStatement ();
      }
    }
  }

  return reductionSubroutines;
}

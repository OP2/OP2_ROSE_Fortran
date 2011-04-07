#include <boost/lexical_cast.hpp>
#include <HostSubroutineOfIndirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
HostSubroutineOfIndirectLoop::createStatementsToConvertPositionsInExecutionPlanMaps (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildIntType;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildGreaterOrEqualOp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;
  using std::string;
  using std::map;

  map <unsigned int, SgVariableDeclaration *>::const_iterator it;

  for (it = localVariables_ExecutionPlan_OP_MAP_Size.begin (); it
      != localVariables_ExecutionPlan_OP_MAP_Size.end (); ++it)
  {
    /*
     * ======================================================
     * Build the guard of the if statement
     * ======================================================
     */

    SgVarRefExp * indsArrayReference = buildVarRefExp (
        localVariables_Others[variableName_inds]);

    SgExpression * indexExpression = buildIntVal (it->first);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        indsArrayReference, indexExpression);

    SgExpression * ifGuardExpression = buildGreaterOrEqualOp (
        arrayIndexExpression, buildIntVal (0));

    /*
     * ======================================================
     * Build the body of the if statement
     * ======================================================
     */

    /*
     * ======================================================
     * 1st statement
     * ======================================================
     */

    SgVarRefExp * executionPlanMapSizeReference = buildVarRefExp (it->second);

    SgVarRefExp * opSetFormalArgumentReference = buildVarRefExp (
        formalParameter_OP_SET);

    SgExpression * sizeField = buildDotExp (opSetFormalArgumentReference,
        buildOpaqueVarRefExp ("size", subroutineScope));

    SgExpression * assignmentExpression = buildAssignOp (
        executionPlanMapSizeReference, sizeField);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    /*
     * ======================================================
     * 2nd statement
     * ======================================================
     */

    SgVarRefExp * opMapArrayReference = buildVarRefExp (
        localVariables_Others[variableName_maps]);

    indexExpression = buildIntVal (it->first);

    arrayIndexExpression = buildPntrArrRefExp (opMapArrayReference,
        indexExpression);

    SgVarRefExp * executionPlanMapReference = buildVarRefExp (
        localVariables_ExecutionPlan_OP_MAP[it->first]);

    SgExpression * shapeExpression =
        FortranStatementsAndExpressionsBuilder::buildShapeExpression (
            it->second, subroutineScope);

    SgStatement * callStatement = createCallToC_F_POINTER (
        arrayIndexExpression, executionPlanMapReference, shapeExpression);

    /*
     * ======================================================
     * Add the if statement with a NULL else block
     * ======================================================
     */

    SgBasicBlock * ifBlock = buildBasicBlock (assignmentStatement,
        callStatement);

    SgIfStmt * ifStatement =
        FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
            ifGuardExpression, ifBlock);

    appendStatement (ifStatement, subroutineScope);
  }
}

void
HostSubroutineOfIndirectLoop::createPlanCToForttranPointerConversionStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;

  SgVarRefExp * planRetReference = buildVarRefExp (
      localVariables_Others[variableName_planRet]);

  SgVarRefExp * actualPlanReference = buildVarRefExp (
      localVariables_Others[variableName_actualPlan]);

  /*
   * ======================================================
   * First call to c_f_pointer
   * ======================================================
   */

  SgStatement * callStatement = createCallToC_F_POINTER (planRetReference,
      actualPlanReference);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * Second call to c_f_pointer
   * ======================================================
   */

  SgExpression * nindirectField = buildDotExp (actualPlanReference,
      buildOpaqueVarRefExp ("nindirect", subroutineScope));

  SgVarRefExp * pnindirectReference = buildVarRefExp (
      localVariables_Others[variableName_pnindirect]);

  SgExpression * shapeExpression =
      FortranStatementsAndExpressionsBuilder::buildShapeExpression (
          localVariables_Others[variableName_indsNumber], subroutineScope);

  callStatement = createCallToC_F_POINTER (nindirectField, pnindirectReference,
      shapeExpression);

  appendStatement (callStatement, subroutineScope);

  /*
   * ======================================================
   * Third call to c_f_pointer
   * ======================================================
   */

  /*
   * ======================================================
   * Fourth call to c_f_pointer
   * ======================================================
   */

  /*
   * ======================================================
   * Fifth call to c_f_pointer
   * ======================================================
   */
}

void
HostSubroutineOfIndirectLoop::createPlanFunctionCallStatement ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;
  using std::map;
  using std::make_pair;
  using std::string;

  SgVarRefExp * parameter1 = buildVarRefExp (formalParameter_SubroutineName);

  SgVarRefExp * parameter2 = buildVarRefExp (formalParameter_OP_SET);

  SgVarRefExp * parameter3 = buildVarRefExp (
      localVariables_Others[variableName_argsNumber]);

  SgVarRefExp * parameter4 = buildVarRefExp (
      localVariables_Others[variableName_args]);

  SgVarRefExp * parameter5 = buildVarRefExp (
      localVariables_Others[variableName_idxs]);

  SgVarRefExp * parameter6 = buildVarRefExp (
      localVariables_Others[variableName_maps]);

  SgVarRefExp * parameter7 = buildVarRefExp (
      localVariables_Others[variableName_accesses]);

  SgVarRefExp * parameter8 = buildVarRefExp (
      localVariables_Others[variableName_indsNumber]);

  SgVarRefExp * parameter9 = buildVarRefExp (
      localVariables_Others[variableName_inds]);

  SgExprListExp * cplan_ActualParameters = buildExprListExp (parameter1,
      parameter2, parameter3, parameter4, parameter5, parameter6, parameter7,
      parameter8, parameter9);

  SgFunctionSymbol * cplanFunctionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("c_f_pointer",
          subroutineScope);

  SgFunctionCallExp * cplanFunctionCall = buildFunctionCallExp (
      cplanFunctionSymbol, cplan_ActualParameters);

  SgVarRefExp * cplanFunctionReturnReference = buildVarRefExp (
      localVariables_Others[variableName_planRet]);

  SgExpression * assignmentExpression = buildAssignOp (
      cplanFunctionReturnReference, cplanFunctionCall);

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createDoLoopToCorrectIndexing (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNotEqualOp;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * The loop starts counting from 1
   * ======================================================
   */
  SgVarRefExp * iterationCounterReference = buildVarRefExp (
      localVariables_Others[variableName_IterationCounter]);

  SgExpression * initializationExpression = buildAssignOp (
      iterationCounterReference, buildIntVal (1));

  /*
   * ======================================================
   * The loop stops counting at the number of OP_DAT argument
   * groups
   * ======================================================
   */
  SgExpression * upperBoundExpression = buildIntVal (
      parallelLoop.getNumberOf_OP_DAT_ArgumentGroups ());

  /*
   * ======================================================
   * The stride of the loop counter is 1
   * ======================================================
   */
  SgExpression * strideExpression = buildIntVal (1);

  /*
   * ======================================================
   * Build the body of the do-loop
   * ======================================================
   */
  SgVarRefExp * opIndirectionArrayReference = buildVarRefExp (
      localVariables_Others[variableName_idxs]);

  SgExpression * arrayIndexExpression = buildPntrArrRefExp (
      opIndirectionArrayReference, iterationCounterReference);

  SgExpression * minusOneExpression = buildSubtractOp (arrayIndexExpression,
      buildIntVal (1));

  SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
      minusOneExpression);

  SgStatement * assignmentStatement = buildExprStatement (assignmentExpression);

  SgBasicBlock * thenBlock = buildBasicBlock (assignmentStatement);

  SgExpression * ifGuardExpression = buildNotEqualOp (arrayIndexExpression,
      buildIntVal (-1));

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, thenBlock);

  SgBasicBlock * loopBodyBlock = buildBasicBlock (ifStatement);

  /*
   * ======================================================
   * Add the do-loop statement
   * ======================================================
   */
  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initializationExpression, upperBoundExpression, strideExpression,
          loopBodyBlock);

  appendStatement (fortranDoStatement, subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createExecutionPlanStatements (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  unsigned int arrayIndexValue = 1;

  for (vector <SgVariableDeclaration *>::const_iterator it =
      formalParameters_OP_DAT_List.begin (); it
      != formalParameters_OP_DAT_List.end (); ++it)
  {
    SgVarRefExp * opDatFormalArgumentReference = buildVarRefExp (*it);

    SgExpression * indexField = buildDotExp (opDatFormalArgumentReference,
        buildOpaqueVarRefExp ("index", subroutineScope));

    SgVarRefExp * opDatArrayReference = buildVarRefExp (
        localVariables_Others[variableName_args]);

    SgExpression * indexExpression = buildIntVal (arrayIndexValue);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opDatArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        indexField);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);

    arrayIndexValue++;
  }

  arrayIndexValue = 1;

  for (vector <SgVariableDeclaration *>::const_iterator it =
      formalParameters_OP_INDIRECTION.begin (); it
      != formalParameters_OP_INDIRECTION.end (); ++it)
  {
    SgVarRefExp * opIndexFormalArgumentReference = buildVarRefExp (*it);

    SgVarRefExp * opIndirectionArrayReference = buildVarRefExp (
        localVariables_Others[variableName_idxs]);

    SgExpression * indexExpression = buildIntVal (arrayIndexValue);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opIndirectionArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        opIndexFormalArgumentReference);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);

    arrayIndexValue++;
  }

  createDoLoopToCorrectIndexing (parallelLoop);

  arrayIndexValue = 1;

  for (vector <SgVariableDeclaration *>::const_iterator it =
      formalParameters_OP_MAP.begin (); it != formalParameters_OP_MAP.end (); ++it)
  {
    SgVarRefExp * opMapFormalArgumentReference = buildVarRefExp (*it);

    SgExpression * indexField = buildDotExp (opMapFormalArgumentReference,
        buildOpaqueVarRefExp ("index", subroutineScope));

    SgVarRefExp * opMapArrayReference = buildVarRefExp (
        localVariables_Others[variableName_maps]);

    SgExpression * indexExpression = buildIntVal (arrayIndexValue);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opMapArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        indexField);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);

    arrayIndexValue++;
  }

  arrayIndexValue = 1;

  for (vector <SgVariableDeclaration *>::const_iterator it =
      formalParameters_OP_ACCESS.begin (); it
      != formalParameters_OP_ACCESS.end (); ++it)
  {
    SgVarRefExp * opAccessFormalArgumentReference = buildVarRefExp (*it);

    SgVarRefExp * opAccessArrayReference = buildVarRefExp (
        localVariables_Others[variableName_accesses]);

    SgExpression * indexExpression = buildIntVal (arrayIndexValue);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        opAccessArrayReference, indexExpression);

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        opAccessFormalArgumentReference);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);

    arrayIndexValue++;
  }

  arrayIndexValue = 1;

  for (unsigned int i = 0; i
      < parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    SgVarRefExp * indsArrayReference = buildVarRefExp (
        localVariables_Others[variableName_inds]);

    SgExpression * indexExpression = buildIntVal (arrayIndexValue);

    SgExpression * arrayIndexExpression = buildPntrArrRefExp (
        indsArrayReference, indexExpression);

    SgExpression * rhsExpression;

    if (parallelLoop.get_OP_MAP_Value (i) == DIRECT)
    {
      rhsExpression = buildIntVal (-1);
    }
    else
    {
      rhsExpression = buildIntVal (0);
    }

    SgExpression * assignmentExpression = buildAssignOp (arrayIndexExpression,
        rhsExpression);

    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, subroutineScope);

    arrayIndexValue++;
  }

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariables_Others[variableName_argsNumber], buildIntVal (
          parallelLoop.getNumberOf_OP_DAT_ArgumentGroups ()), subroutineScope);

  FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
      localVariables_Others[variableName_indsNumber], buildIntVal (
          parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments ()),
      subroutineScope);
}

void
HostSubroutineOfIndirectLoop::createExecutionPlanLocalVariables (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildPointerType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;
  using std::make_pair;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Creating execution plan variables", 2);

  map <string, OP_DAT_Declaration *>::const_iterator OP_DAT_iterator;

  /*
   * ======================================================
   * Create pointers to the execution plan on the C and
   * Fortran side
   * ======================================================
   */

  SgType * c_ptrType = FortranTypesBuilder::buildNewTypeDeclaration ("c_ptr",
      subroutineScope)->get_type ();

  SgType * op_planType = FortranTypesBuilder::buildNewTypeDeclaration (
      "op_plan", subroutineScope)->get_type ();

  SgVariableDeclaration * variableDeclaration_ExecutionPlan_CPointer =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_planRet, c_ptrType, subroutineScope);

  SgVariableDeclaration * variableDeclaration_ExecutionPlan_FortranPointer =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_actualPlan, op_planType, subroutineScope);

  localVariables_Others.insert (make_pair (variableName_planRet,
      variableDeclaration_ExecutionPlan_CPointer));

  localVariables_Others.insert (make_pair (variableName_actualPlan,
      variableDeclaration_ExecutionPlan_FortranPointer));

  /*
   * ======================================================
   * Create pointer to the
   * ======================================================
   */

  SgType * c_devptrType = FortranTypesBuilder::buildNewTypeDeclaration (
      "c_devptr", subroutineScope)->get_type ();

  SgVariableDeclaration * variableDeclaration_pindMaps =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_pindMaps,
          buildPointerType (FortranTypesBuilder::getArray_RankOne (
              c_devptrType, 1,
              parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments ())),
          subroutineScope);

  localVariables_Others.insert (make_pair (variableName_actualPlan,
      variableDeclaration_pindMaps));

  SgVariableDeclaration * variableDeclaration =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          variableName_pindMapsSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  localVariables_Others.insert (make_pair (variableName_pindMapsSize,
      variableDeclaration));

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments (); ++i)
  {
    string const variableName = "pindMaps" + lexical_cast <string> (i);

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ()), subroutineScope);

    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    localVariables_ExecutionPlan_IndirectMaps.insert (make_pair (i,
        variableDeclaration));
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOfDistinctIndirect_OP_DAT_Arguments (); ++i)
  {
    string const variableName = "pindMaps" + lexical_cast <string> (i) + "Size";

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope);

    localVariables_ExecutionPlan_IndirectMaps_Size.insert (make_pair (i,
        variableDeclaration));
  }

  /*
   * ======================================================
   * Create arrays for OP_DAT, OP_INDIRECTION,
   * OP_MAP, OP_ACCESS arguments. These arrays are filled up
   * with the actual values of the OP_DAT, OP_INDIRECTION,
   * OP_MAP, OP_ACCESS that are passed to the OP_PAR_LOOP;
   * these arrays are then given to the plan function.
   * Note, therefore, that the size of the arrays is exactly
   * the number of OP_DAT argument groups.
   * There is an additional array 'inds' storing which
   * OP_DAT arguments are accessed through an indirection
   * ======================================================
   */

  vector <string> fourByteIntegerArrays;
  fourByteIntegerArrays.push_back (variableName_args);
  fourByteIntegerArrays.push_back (variableName_idxs);
  fourByteIntegerArrays.push_back (variableName_maps);
  fourByteIntegerArrays.push_back (variableName_accesses);
  fourByteIntegerArrays.push_back (variableName_inds);

  for (vector <string>::iterator it = fourByteIntegerArrays.begin (); it
      != fourByteIntegerArrays.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger (), 1,
                parallelLoop.getNumberOf_OP_DAT_ArgumentGroups ()),
            subroutineScope);

    localVariables_Others.insert (make_pair (*it, variableDeclaration));
  }

  /*
   * ======================================================
   * Create device variables for each distinct OP_DAT
   * (which point to the actual data)
   * ======================================================
   */

  for (OP_DAT_iterator = parallelLoop.first_OP_DAT (); OP_DAT_iterator
      != parallelLoop.last_OP_DAT (); ++OP_DAT_iterator)
  {
    string const variableName = "pArg" + lexical_cast <string> (
        parallelLoop.getFirstOP_DATOccurrence (OP_DAT_iterator->first))
        + "DatD";

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, OP_DAT_iterator->second->getActualType (),
            subroutineScope);

    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    localVariables_ExecutionPlan_OP_DAT.insert (make_pair (
        OP_DAT_iterator->first, variableDeclaration));
  }

  /*
   * ======================================================
   * Create device integer variables for each distinct OP_DAT
   * which record the size of the array of the actual data
   * ======================================================
   */

  for (OP_DAT_iterator = parallelLoop.first_OP_DAT (); OP_DAT_iterator
      != parallelLoop.last_OP_DAT (); ++OP_DAT_iterator)
  {
    string const variableName = "pArg" + lexical_cast <string> (
        parallelLoop.getFirstOP_DATOccurrence (OP_DAT_iterator->first))
        + "DatDSize";

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope);

    localVariables_ExecutionPlan_OP_DAT_Size.insert (make_pair (
        OP_DAT_iterator->first, variableDeclaration));
  }

  /*
   * ======================================================
   * Create device integer variables for each OP_DAT
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "pMaps" + lexical_cast <string> (i);

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getTwoByteInteger ()), subroutineScope);

    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    localVariables_ExecutionPlan_OP_MAP.insert (make_pair (i,
        variableDeclaration));
  }

  /*
   * ======================================================
   * Create integer variables
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "pMaps" + lexical_cast <string> (i) + "Size";

    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            variableName, FortranTypesBuilder::getFourByteInteger (),
            subroutineScope);

    localVariables_ExecutionPlan_OP_MAP_Size.insert (make_pair (i,
        variableDeclaration));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer fields. These fields need to be accessed
   * on the Fortran side, so create local variables that
   * enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> fourByteIntegerVariables;

  fourByteIntegerVariables.push_back (variableName_argsNumber);
  fourByteIntegerVariables.push_back (variableName_indsNumber);
  fourByteIntegerVariables.push_back (variableName_IterationCounter);
  fourByteIntegerVariables.push_back (variableName_blockOffset);
  fourByteIntegerVariables.push_back (variableName_col);
  fourByteIntegerVariables.push_back (variableName_threadSynchRet);
  fourByteIntegerVariables.push_back (variableName_pindSizesSize);
  fourByteIntegerVariables.push_back (variableName_pindOffsSize);
  fourByteIntegerVariables.push_back (variableName_pblkMapSize);
  fourByteIntegerVariables.push_back (variableName_poffsetSize);
  fourByteIntegerVariables.push_back (variableName_pnelemsSize);
  fourByteIntegerVariables.push_back (variableName_pnthrcolSize);
  fourByteIntegerVariables.push_back (variableName_pthrcolSize);

  for (vector <string>::iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getFourByteInteger ()),
            subroutineScope);

    localVariables_Others.insert (make_pair (*it, variableDeclaration));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of integer array fields. These fields need to be
   * accessed on the Fortran side ON THE HOST, so create local
   * variables that enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> integerPointerVariables;

  integerPointerVariables.push_back (variableName_ncolblk);
  integerPointerVariables.push_back (variableName_pnindirect);

  for (vector <string>::iterator it = integerPointerVariables.begin (); it
      != integerPointerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ())), subroutineScope);

    localVariables_Others.insert (make_pair (*it, variableDeclaration));
  }

  /*
   * ======================================================
   * The plan function fills up a 'struct' which has a
   * number of array integer fields. These fields need to be accessed
   * on the Fortran side ON THE DEVICE, so create local variables
   * that enable the data to be transferred accordingly
   * ======================================================
   */

  vector <string> deviceIntegerArrayVariables;

  deviceIntegerArrayVariables.push_back (variableName_pindSizes);
  deviceIntegerArrayVariables.push_back (variableName_pindOffs);
  deviceIntegerArrayVariables.push_back (variableName_pblkMap);
  deviceIntegerArrayVariables.push_back (variableName_poffset);
  deviceIntegerArrayVariables.push_back (variableName_pnelems);
  deviceIntegerArrayVariables.push_back (variableName_pnthrcol);
  deviceIntegerArrayVariables.push_back (variableName_pthrcol);

  for (vector <string>::iterator it = deviceIntegerArrayVariables.begin (); it
      != deviceIntegerArrayVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration =
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            buildPointerType (FortranTypesBuilder::getArray_RankOne (
                FortranTypesBuilder::getFourByteInteger ())), subroutineScope);

    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    localVariables_Others.insert (make_pair (*it, variableDeclaration));
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

HostSubroutineOfIndirectLoop::HostSubroutineOfIndirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    KernelSubroutine & kernelSubroutine, ParallelLoop & parallelLoop,
    SgScopeStatement * moduleScope) :
  HostSubroutine (subroutineName, userDeviceSubroutine, parallelLoop,
      moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of indirect loop", 2);

  createFormalParameters (userDeviceSubroutine, parallelLoop);

  createDataMarshallingLocalVariables (parallelLoop);

  createExecutionPlanLocalVariables (parallelLoop);

  createCUDAKernelVariables ();

  initialiseDataMarshallingLocalVariables (parallelLoop);

  createExecutionPlanStatements (parallelLoop);

  createPlanFunctionCallStatement ();

  createPlanCToForttranPointerConversionStatements ();

  createStatementsToConvertPositionsInExecutionPlanMaps (parallelLoop);

  createKernelCall (kernelSubroutine, parallelLoop);

  copyDataBackFromDeviceAndDeallocate (parallelLoop);
}

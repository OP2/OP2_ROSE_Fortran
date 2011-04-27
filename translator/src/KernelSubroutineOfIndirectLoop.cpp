#include <boost/lexical_cast.hpp>
#include <Debug.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <KernelSubroutineOfIndirectLoop.h>
#include <OP2CommonDefinitions.h>

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

namespace
{
  /*
   * ======================================================
   * Following are names of local shared variables needed
   * in the kernel subroutine of an indirect loop
   * ======================================================
   */

  std::string const variableName_autoshared = "autoshared";
  std::string const variableName_blockID = "blockID";
  std::string const variableName_iterationCounter = "i";
  std::string const variableName_moduloResult = "moduloResult";
  std::string const variableName_nbytes = "nbytes";
  std::string const variableName_nelem = "nelem";
  std::string const variableName_offset_b = "offset_b";
  std::string const variableName_whileLoopBound = "whileLoopBound";
}

void
KernelSubroutineOfIndirectLoop::createThreadZeroStatements (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * The body of the if statement
   * ======================================================
   */

  SgBasicBlock * ifBlock = buildBasicBlock ();

  /*
   * ======================================================
   * Build the if-guard
   * ======================================================
   */

  SgVarRefExp * threadidx_Reference = buildOpaqueVarRefExp (
      variableName_threadidx, subroutineScope);

  SgVarRefExp * x_Reference = buildOpaqueVarRefExp (variableName_x,
      subroutineScope);

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildDotExp (
      threadidx_Reference, x_Reference), buildIntVal (0));

  /*
   * ======================================================
   * 1st statement
   * ======================================================
   */

  SgVarRefExp * blockID_Reference1 = buildVarRefExp (
      localVariables_Others[variableName_blockID]);

  SgVarRefExp * blockOffset_Reference1 = buildVarRefExp (
      formalParameters_PlanVariables[PlanFunctionVariables::blockOffset]);

  SgVarRefExp * pblkMap_Reference1 = buildVarRefExp (
      formalParameters_PlanVariables[PlanFunctionVariables::pblkMap]);

  SgVarRefExp * blockidx_Reference1 = buildOpaqueVarRefExp (
      variableName_blockidx, subroutineScope);

  SgVarRefExp * x_Reference1 = buildOpaqueVarRefExp (variableName_x,
      subroutineScope);

  SgSubtractOp * subtractExpression1 = buildSubtractOp (buildDotExp (
      blockidx_Reference1, x_Reference1), buildIntVal (1));

  SgAddOp * arrayIndexExpression1 = buildAddOp (subtractExpression1,
      blockOffset_Reference1);

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (pblkMap_Reference1,
      arrayIndexExpression1);

  SgStatement * statement1 = buildAssignStatement (blockID_Reference1,
      arrayExpression1);

  ifBlock->append_statement (statement1);

  /*
   * ======================================================
   * 2nd statement
   * ======================================================
   */

  SgVarRefExp * pnelems_Reference2 = buildOpaqueVarRefExp (
      PlanFunctionVariables::pnelems, subroutineScope);

  SgVarRefExp * nelem_Reference2 = buildOpaqueVarRefExp (variableName_nelem,
      subroutineScope);

  SgVarRefExp * blockID_Reference2 = buildVarRefExp (
      localVariables_Others[variableName_blockID]);

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (pnelems_Reference2,
      blockID_Reference2);

  SgStatement * statement2 = buildAssignStatement (nelem_Reference2,
      arrayExpression2);

  ifBlock->append_statement (statement2);

  /*
   * ======================================================
   * 3rd statement
   * ======================================================
   */

  SgVarRefExp * poffset_Reference3 = buildOpaqueVarRefExp (
      PlanFunctionVariables::poffset, subroutineScope);

  SgVarRefExp * offset_b_Reference3 = buildOpaqueVarRefExp (
      variableName_offset_b, subroutineScope);

  SgVarRefExp * blockID_Reference3 = buildVarRefExp (
      localVariables_Others[variableName_blockID]);

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (poffset_Reference3,
      blockID_Reference3);

  SgStatement * statement3 = buildAssignStatement (offset_b_Reference3,
      arrayExpression3);

  ifBlock->append_statement (statement3);

  /*
   * ======================================================
   * 4th statement
   * ======================================================
   */

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      SgVarRefExp * ind_argSize_Reference = buildVarRefExp (
          localVariables_indArgSizes[i]);

      SgVarRefExp * pindSizes_Reference4 = buildVarRefExp (
          formalParameters_PlanVariables[PlanFunctionVariables::pindSizes]);

      SgVarRefExp * blockID_Reference4 = buildVarRefExp (
          localVariables_Others[variableName_blockID]);

      SgAddOp * arrayIndexExpression = buildAddOp (buildIntVal (0),
          buildMultiplyOp (blockID_Reference4, buildIntVal (1)));

      SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (
          pindSizes_Reference4, arrayIndexExpression);

      SgStatement * statement4 = buildAssignStatement (ind_argSize_Reference,
          arrayExpression4);

      ifBlock->append_statement (statement4);
    }
  }

  /*
   * ======================================================
   * 5th statement
   * ======================================================
   */

  SgVarRefExp * nbytes_Reference = buildVarRefExp (
      localVariables_Others[variableName_nbytes]);

  SgStatement * statement5 = buildAssignStatement (nbytes_Reference,
      buildIntVal (0));

  ifBlock->append_statement (statement5);

  /*
   * ======================================================
   * Add the if statement with a NULL else block
   * ======================================================
   */

  SgIfStmt * ifStatement =
      FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  appendStatement (ifStatement, subroutineScope);

  /*
   * ======================================================
   * All threads must synchronize before kernel execution
   * can proceed. Add the statement
   * ======================================================
   */

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranSubroutine ("syncthreads",
          subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  appendStatement (buildExprStatement (subroutineCall), subroutineScope);
}

void
KernelSubroutineOfIndirectLoop::createRemainingLocalVariables ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerVariables;
  fourByteIntegerVariables.push_back (variableName_iterationCounter);
  fourByteIntegerVariables.push_back (variableName_moduloResult);
  fourByteIntegerVariables.push_back (variableName_nbytes);
  fourByteIntegerVariables.push_back (variableName_whileLoopBound);

  for (vector <string>::const_iterator it = fourByteIntegerVariables.begin (); it
      != fourByteIntegerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
        *it, FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

    variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (variableDeclaration, subroutineScope);

    localVariables_Others[*it] = variableDeclaration;
  }
}

void
KernelSubroutineOfIndirectLoop::createSharedLocalVariables (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "ind_arg" + lexical_cast <string> (i)
          + "_size";

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName, FortranTypesBuilder::getFourByteInteger (), NULL,
          subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

      /*
       * Not yet supported by ROSE
       */
      //variableDeclaration->get_declarationModifier ().get_accessModifier ().setShared ();

      appendStatement (variableDeclaration, subroutineScope);

      localVariables_indArgSizes[i] = variableDeclaration;
    }
  }

  vector <string> sharedFourByteIntegerVariables;
  sharedFourByteIntegerVariables.push_back (variableName_blockID);
  sharedFourByteIntegerVariables.push_back (variableName_nelem);
  sharedFourByteIntegerVariables.push_back (variableName_offset_b);

  for (vector <string>::const_iterator it =
      sharedFourByteIntegerVariables.begin (); it
      != sharedFourByteIntegerVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
        *it, FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

    variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    /*
     * Not yet supported by ROSE
     */
    //variableDeclaration->get_declarationModifier ().get_accessModifier ().setShared ();

    appendStatement (variableDeclaration, subroutineScope);

    localVariables_Others[*it] = variableDeclaration;
  }
}

void
KernelSubroutineOfIndirectLoop::createPlanFormalParameters (
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  vector <string> fourByteIntegerArrayPlanVariables;
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pindSizes);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pindOffs);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pblkMap);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::poffset);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pnelems);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pnthrcol);
  fourByteIntegerArrayPlanVariables.push_back (PlanFunctionVariables::pthrcol);

  for (vector <string>::const_iterator it =
      fourByteIntegerArrayPlanVariables.begin (); it
      != fourByteIntegerArrayPlanVariables.end (); ++it)
  {
    SgIntVal * lowerBoundExpression = buildIntVal (0);

    SgVarRefExp * argsSizesReference = buildVarRefExp (
        formalParameter_argsSizes);

    SgVarRefExp * fieldReference = buildVarRefExp (
        deviceDataSizesDeclaration.getPlanVariableSizeFieldDeclaration (*it
            + "Size"));

    SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
        fieldReference);

    SgSubtractOp * upperBoundExpression = buildSubtractOp (
        fieldSelectionExpression, buildIntVal (1));

    SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
        *it, FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
            FortranTypesBuilder::getTwoByteInteger (), lowerBoundExpression,
            upperBoundExpression), NULL, subroutineScope);

    variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
    variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

    appendStatement (variableDeclaration, subroutineScope);

    formalParameters->append_arg (
        *(variableDeclaration->get_variables ().begin ()));

    formalParameters_PlanVariables[*it] = variableDeclaration;
  }

  vector <string> fourByteIntegerPlanVariables;
  fourByteIntegerPlanVariables.push_back (PlanFunctionVariables::blockOffset);

  for (vector <string>::const_iterator it =
      fourByteIntegerPlanVariables.begin (); it
      != fourByteIntegerPlanVariables.end (); ++it)
  {
    SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
        *it, FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

    variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
    variableDeclaration->get_declarationModifier ().get_typeModifier (). setValue ();

    formalParameters_PlanVariables[*it] = variableDeclaration;
  }
}

void
KernelSubroutineOfIndirectLoop::create_OP_DAT_FormalParameters (
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntVal;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters", 2);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      string const variableName = "argument" + lexical_cast <string> (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          formalParameter_argsSizes);

      SgVarRefExp * fieldReference = buildVarRefExp (
          deviceDataSizesDeclaration.get_OP_DAT_SizeFieldDeclaration (i));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              parallelLoop.get_OP_DAT_Type (i), lowerBoundExpression,
              upperBoundExpression), NULL, subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (variableDeclaration, subroutineScope);

      formalParameters->append_arg (
          *(variableDeclaration->get_variables ().begin ()));

      formalParameter_OP_DATs[i] = variableDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false
        && parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pindMaps" + lexical_cast <string> (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          formalParameter_argsSizes);

      SgVarRefExp
          * fieldReference =
              buildVarRefExp (
                  deviceDataSizesDeclaration.get_LocalToGlobalMappingSizeFieldDeclaration (
                      i));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getTwoByteInteger (), lowerBoundExpression,
              upperBoundExpression), NULL, subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (variableDeclaration, subroutineScope);

      formalParameters->append_arg (
          *(variableDeclaration->get_variables ().begin ()));

      formalParameters_LocalToGlobalMapping[i] = variableDeclaration;
    }
  }

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
    {
      string const variableName = "pMaps" + lexical_cast <string> (i);

      SgIntVal * lowerBoundExpression = buildIntVal (0);

      SgVarRefExp * argsSizesReference = buildVarRefExp (
          formalParameter_argsSizes);

      SgVarRefExp
          * fieldReference =
              buildVarRefExp (
                  deviceDataSizesDeclaration.get_GlobalToLocalMappingSizeFieldDeclaration (
                      i));

      SgDotExp * fieldSelectionExpression = buildDotExp (argsSizesReference,
          fieldReference);

      SgSubtractOp * upperBoundExpression = buildSubtractOp (
          fieldSelectionExpression, buildIntVal (1));

      SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
          variableName,
          FortranTypesBuilder::getArray_RankOne_WithLowerAndUpperBounds (
              FortranTypesBuilder::getTwoByteInteger (), lowerBoundExpression,
              upperBoundExpression), NULL, subroutineScope);

      variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
      variableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

      appendStatement (variableDeclaration, subroutineScope);

      formalParameters->append_arg (
          *(variableDeclaration->get_variables ().begin ()));

      formalParameters_GlobalToLocalMapping[i] = variableDeclaration;
    }
  }
}

void
KernelSubroutineOfIndirectLoop::createArgsSizesFormalParameter (
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration)
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating OP_SET size formal parameter",
      2);

  formalParameter_argsSizes = buildVariableDeclaration (
      LoopVariables::argsSizes, deviceDataSizesDeclaration.getType (),
      NULL, subroutineScope);

  formalParameter_argsSizes->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_argsSizes->get_declarationModifier ().get_typeModifier ().setDevice ();

  formalParameters->append_arg (
      *(formalParameter_argsSizes->get_variables ().begin ()));

  appendStatement (formalParameter_argsSizes, subroutineScope);
}

KernelSubroutineOfIndirectLoop::KernelSubroutineOfIndirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    DeviceDataSizesDeclaration & deviceDataSizesDeclaration,
    ParallelLoop & parallelLoop, SgScopeStatement * moduleScope) :
  KernelSubroutine (subroutineName)
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::string;
  using std::vector;

  formalParameters = buildFunctionParameterList ();

  subroutineHeaderStatement = buildProcedureHeaderStatement (
      this->subroutineName.c_str (), buildVoidType (), formalParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (subroutineHeaderStatement, "attributes(global) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createArgsSizesFormalParameter (deviceDataSizesDeclaration);

  create_OP_DAT_FormalParameters (deviceDataSizesDeclaration, parallelLoop);

  createPlanFormalParameters (deviceDataSizesDeclaration, parallelLoop);

  createSharedLocalVariables (parallelLoop);

  createRemainingLocalVariables ();

  createThreadZeroStatements (parallelLoop);
}

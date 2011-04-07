#include <boost/lexical_cast.hpp>
#include <KernelSubroutineOfDirectLoop.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
KernelSubroutineOfDirectLoop::createStatements (
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & op2ParallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildWhileStmt;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  /*
   * ======================================================
   * 'setIter = (threadidx%x - 1) + (blockidx%x -1)  * blockdim%x'
   * ======================================================
   */

  // 'threadidx%x - 1'
  SgExpression * threadIdXMinusOne = buildSubtractOp (buildDotExp (
      variable_Threadidx, variable_X), buildIntVal (1));

  // 'blockidx%x - 1'
  SgExpression * blockIdXMinusOne = buildSubtractOp (buildDotExp (
      variable_Blockidx, variable_X), buildIntVal (1));

  // 'blockdim%x'
  SgExpression * blockDimX = buildDotExp (variable_Blockdim, variable_X);

  // '(blockidx%x -1)  * blockdim%x'
  SgExpression * blockIdXMinusOnePerBlockDimx = buildMultiplyOp (
      blockIdXMinusOne, blockDimX);

  // 'setIter = (threadidx%x - 1) + (blockidx%x -1)  * blockdim%x'
  SgVarRefExp
      * setIterRef =
          buildVarRefExp (
              (*variable_iterationSet->get_variables ().begin ())->get_name ().getString (),
              subroutineScope);

  SgExpression * setIterInitialAssignement = buildAssignOp (setIterRef,
      buildAddOp (threadIdXMinusOne, blockIdXMinusOnePerBlockDimx));

  appendStatement (buildExprStatement (setIterInitialAssignement),
      subroutineScope);

  /*
   * ======================================================
   * Build the do-while loop
   * ======================================================
   */

  SgFunctionRefExp * userKernelRefExp = buildFunctionRefExp (
      userDeviceSubroutine.getSubroutineName (), subroutineScope->get_scope ());

  // 2. build parameters: q(setIter * dim:setIter * dim + dim - 1), qold(setIter * dim:setIter*dim + dim - 1)
  // dim is obtained from the previous parsing
  SgExprListExp * userKernelParams =
      createActualParametersForUserDeviceSubroutine (setIterRef,
          subroutineScope, op2ParallelLoop);

  // 3. build call
  SgFunctionCallExp * userKernelCall = buildFunctionCallExp (userKernelRefExp,
      userKernelParams);

  // 4. build corresponding statement
  SgExprStatement * userKernelCallStmt = buildExprStatement (userKernelCall);

  // build set iteration variable increment: setIter = setIter + blockdim%x * griddim%x
  // (we are only missing griddim%x)
  SgExpression * gridDimXAcc = buildDotExp (variable_GridDim, variable_X);

  // build multiplication: blockdim%x * griddim%x
  SgExpression * blockDimMult = buildMultiplyOp (blockDimX, gridDimXAcc);

  // build sum: setIter + blockdim%x * griddim%x
  SgExpression * sumIterPlusBlockGridX = buildAddOp (setIterRef, blockDimMult);

  // build assignment: setIter = setIter + blockdim%x * griddim%x
  SgExpression * setIterUpdate = buildAssignOp (setIterRef,
      sumIterPlusBlockGridX);

  // 4. build corresponding statement
  SgExprStatement * setIterUpdateStmt = buildExprStatement (setIterUpdate);

  SgBasicBlock * doWhileBlock = buildBasicBlock (userKernelCallStmt,
      setIterUpdateStmt);

  // Build do while statement (do while ( setIter < setsize ))

  // building reference to setsize parameter
  SgVarRefExp
      * setSizeFormalParRef =
          buildVarRefExp (
              (*formalParameter_setSize->get_variables ().begin ())->get_name ().getString (),
              subroutineScope);

  // build do-while guard
  SgExpression * inequalityExp = buildLessThanOp (setIterRef,
      setSizeFormalParRef);

  // build do-while statement
  SgWhileStmt * setWhileStmt = buildWhileStmt (inequalityExp, doWhileBlock);

  // we need to set the end do statement, because the unparse is not able to infer it automatically
  setWhileStmt->set_has_end_statement (true);

  // append do-while statement
  appendStatement (setWhileStmt, subroutineScope);
}

void
KernelSubroutineOfDirectLoop::createLocalVariables ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;

  variable_iterationSet = buildVariableDeclaration ("setIter", buildIntType (),
      NULL, subroutineScope);

  variable_iterationSet->get_declarationModifier ().get_typeModifier ().setDevice ();
  variable_iterationSet->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_iterationSet, subroutineScope);

  /*
   * ======================================================
   * Build opaque variable references needed in the following
   * expressions. These are opaque because the variables are
   * provided as part of the CUDA library and are not seen
   * by ROSE
   * ======================================================
   */
  variable_Threadidx = buildOpaqueVarRefExp ("threadidx", subroutineScope);
  variable_X = buildOpaqueVarRefExp ("x", subroutineScope);
  variable_Blockidx = buildOpaqueVarRefExp ("blockidx", subroutineScope);
  variable_Blockdim = buildOpaqueVarRefExp ("blockdim", subroutineScope);
  variable_GridDim = buildOpaqueVarRefExp ("griddim", subroutineScope);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

KernelSubroutineOfDirectLoop::KernelSubroutineOfDirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    ParallelLoop & op2ParallelLoop, SgScopeStatement * moduleScope) :
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

  createFormalParameters (op2ParallelLoop);

  createLocalVariables ();

  createStatements (userDeviceSubroutine, op2ParallelLoop);
}

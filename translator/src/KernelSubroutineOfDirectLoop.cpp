#include <boost/lexical_cast.hpp>
#include <KernelSubroutineOfDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
KernelSubroutineOfDirectLoop::createUserSubroutineCall (
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & parallelLoop)
{
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", 2);

  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  SgVarRefExp * iterationCounterReference = buildVarRefExp (
      variable_IterationCounter);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      SgExpression * lowerBound = buildMultiplyOp (iterationCounterReference,
          buildIntVal (parallelLoop.get_OP_DAT_Dimension (i)));

      SgExpression * upperBound =
          buildSubtractOp (buildAddOp (lowerBound, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i))), buildIntVal (1));

      SgSubscriptExpression * arraySubscriptExpression =
          new SgSubscriptExpression (ROSEHelper::getFileInfo (), lowerBound,
              upperBound, buildIntVal (1));

      arraySubscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());
      arraySubscriptExpression->setCompilerGenerated ();
      arraySubscriptExpression->setOutputInCodeGeneration ();

      SgExpression * opDatFormalParameterReference = buildVarRefExp (
          formalParameter_OP_DATs[i]);

      SgExpression * parameterExpression = buildPntrArrRefExp (
          opDatFormalParameterReference, arraySubscriptExpression);

      userDeviceSubroutineParameters->append_expression (parameterExpression);
    }
  }

  return buildFunctionCallStmt (userDeviceSubroutine.getSubroutineName (),
      buildVoidType (), userDeviceSubroutineParameters, subroutineScope);
}

void
KernelSubroutineOfDirectLoop::createStatements (
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
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
   * Build opaque variable references needed in the following
   * expressions. These are opaque because the variables are
   * provided as part of the CUDA library and are not seen
   * by ROSE
   * ======================================================
   */

  SgVarRefExp * variable_Threadidx = buildOpaqueVarRefExp (
      variableName_threadidx, subroutineScope);

  SgVarRefExp * variable_X = buildOpaqueVarRefExp (variableName_x,
      subroutineScope);

  SgVarRefExp * variable_Blockidx = buildOpaqueVarRefExp (
      variableName_blockidx, subroutineScope);

  SgVarRefExp * variable_Blockdim = buildOpaqueVarRefExp (
      variableName_blockdim, subroutineScope);

  SgVarRefExp * variable_GridDim = buildOpaqueVarRefExp (variableName_griddim,
      subroutineScope);

  /*
   * ======================================================
   * Initialise the set iteration counter
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (buildDotExp (
      variable_Threadidx, variable_X), buildIntVal (1));

  SgSubtractOp * subtractExpression2 = buildSubtractOp (buildDotExp (
      variable_Blockidx, variable_X), buildIntVal (1));

  SgExpression * blockDimX = buildDotExp (variable_Blockdim, variable_X);

  SgMultiplyOp * multiplyExpression = buildMultiplyOp (subtractExpression1,
      blockDimX);

  SgVarRefExp * iterationCounterReference = buildVarRefExp (
      variable_IterationCounter);

  SgExpression * assignmentExpression = buildAssignOp (
      iterationCounterReference, buildAddOp (subtractExpression2,
          multiplyExpression));

  appendStatement (buildExprStatement (assignmentExpression), subroutineScope);

  /*
   * ======================================================
   * Statement to call user device subroutine
   * ======================================================
   */

  SgStatement * statement1 = createUserSubroutineCall (userDeviceSubroutine,
      parallelLoop);

  /*
   * ======================================================
   * Statement to increment set iteration counter
   * ======================================================
   */

  SgExpression * gridDimXExpression =
      buildDotExp (variable_GridDim, variable_X);

  SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (blockDimX,
      gridDimXExpression);

  SgAddOp * addExpression = buildAddOp (iterationCounterReference,
      multiplyExpression2);

  SgAssignOp * assignmentExpression2 = buildAssignOp (
      iterationCounterReference, addExpression);

  SgExprStatement * statement2 = buildExprStatement (assignmentExpression2);

  /*
   * ======================================================
   * Build the do-while loop
   * ======================================================
   */

  SgBasicBlock * loopBody = buildBasicBlock (statement1, statement2);

  SgVarRefExp * setSizeFormalArgumentReference = buildVarRefExp (
      formalParameter_setSize);

  SgExpression * loopGuard = buildLessThanOp (iterationCounterReference,
      setSizeFormalArgumentReference);

  SgWhileStmt * whileStatement = buildWhileStmt (loopGuard, loopBody);

  whileStatement->set_has_end_statement (true);

  appendStatement (whileStatement, subroutineScope);
}

void
KernelSubroutineOfDirectLoop::createLocalVariables ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  variable_IterationCounter = buildVariableDeclaration ("setIter",
      FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

  variable_IterationCounter->get_declarationModifier ().get_typeModifier ().setDevice ();
  variable_IterationCounter->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_IterationCounter, subroutineScope);
}

void
KernelSubroutineOfDirectLoop::create_OP_DAT_FormalParameters (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildExprListExp;
  using SageInterface::appendStatement;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters", 2);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
      /*
       * ======================================================
       * Obtain the base type of the OP_DAT argument.
       * The base type is almost always an array
       * ======================================================
       */

      SgType * opDatBaseType = parallelLoop.get_OP_DAT_Type (i);

      SgArrayType * isArrayType = isSgArrayType (opDatBaseType);

      ROSE_ASSERT (isArrayType != NULL);

      opDatBaseType = isArrayType->get_base_type ();

      /*
       * ======================================================
       * Build the upper bound of the OP_DAT array which
       * is: (size of OP_SET) * (dimension of OP_DAT) - 1.
       * The term (dimension of OP_DAT) is currently assumed to be
       * a constant
       * ======================================================
       */

      SgIntVal * dimensionOfOP_DAT = buildIntVal (
          parallelLoop.get_OP_DAT_Dimension (i));

      SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
          formalParameter_setSize), dimensionOfOP_DAT);

      SgExpression * minusOneExpression = buildSubtractOp (multiplyExpression,
          buildIntVal (1));

      SgSubscriptExpression * arraySubscriptExpression =
          new SgSubscriptExpression (ROSEHelper::getFileInfo (),
              buildIntVal (0), minusOneExpression, buildIntVal (1));

      arraySubscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());
      arraySubscriptExpression->setCompilerGenerated ();
      arraySubscriptExpression->setOutputInCodeGeneration ();

      /*
       * ======================================================
       * Build array type with the correct subscript
       * ======================================================
       */

      SgArrayType * arrayType = buildArrayType (opDatBaseType,
          arraySubscriptExpression);

      arrayType->set_rank (1);

      arrayType->set_dim_info (buildExprListExp (arraySubscriptExpression));

      /*
       * ======================================================
       * Build the variable declaration
       * ======================================================
       */

      string const & variableName = "argument" + lexical_cast <string> (i);

      SgVariableDeclaration * opDatFormalParameter = buildVariableDeclaration (
          variableName, arrayType, NULL, subroutineScope);

      formalParameters->append_arg (
          *(opDatFormalParameter->get_variables ().begin ()));

      opDatFormalParameter->get_declarationModifier ().get_typeModifier ().setIntent_in ();
      opDatFormalParameter->get_declarationModifier ().get_typeModifier ().setDevice ();
      opDatFormalParameter->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (opDatFormalParameter, subroutineScope);

      formalParameter_OP_DATs[i] = opDatFormalParameter;
    }
  }
}

void
KernelSubroutineOfDirectLoop::createSetSizeFormalParameter ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating OP_SET size formal parameter",
      2);

  formalParameter_setSize = buildVariableDeclaration ("setSize",
      buildIntType (), NULL, subroutineScope);

  formalParameter_setSize->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_setSize->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameter_setSize->get_variables ().begin ()));

  appendStatement (formalParameter_setSize, subroutineScope);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

KernelSubroutineOfDirectLoop::KernelSubroutineOfDirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & ParallelLoop,
    SgScopeStatement * moduleScope) :
  KernelSubroutine (subroutineName)
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

  addTextForUnparser (subroutineHeaderStatement, "attributes(global) ",
      AstUnparseAttribute::e_before);

  appendStatement (subroutineHeaderStatement, moduleScope);

  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();

  createSetSizeFormalParameter ();

  create_OP_DAT_FormalParameters (ParallelLoop);

  createLocalVariables ();

  createStatements (userDeviceSubroutine, ParallelLoop);
}

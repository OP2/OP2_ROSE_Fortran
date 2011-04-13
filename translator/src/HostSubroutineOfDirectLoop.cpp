#include <boost/lexical_cast.hpp>
#include <HostSubroutineOfDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

void
HostSubroutineOfDirectLoop::createKernelCall (
    KernelSubroutine & kernelSubroutine, ParallelLoop & parallelLoop)
{
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating kernel call", 2);

  map <string, OP_DAT_Declaration *>::const_iterator OP_DAT_iterator;

  SgExprListExp * kernelParameters = buildExprListExp ();

  SgExpression * opSetFormalArgumentReference = buildVarRefExp (
      formalParameter_OP_SET);

  SgExpression * sizeFieldExpression = buildDotExp (
      opSetFormalArgumentReference, buildOpaqueVarRefExp ("size",
          subroutineScope));

  kernelParameters->append_expression (sizeFieldExpression);

  for (OP_DAT_iterator = parallelLoop.first_OP_DAT (); OP_DAT_iterator
      != parallelLoop.last_OP_DAT (); ++OP_DAT_iterator)
  {
    string const OP_DATVariableName = OP_DAT_iterator->first;

    SgVarRefExp * opDatDeviceReference = buildVarRefExp (
        localVariables_OP_DAT_VariablesOnDevice[OP_DATVariableName]);

    kernelParameters->append_expression (opDatDeviceReference);
  }

  SgExprStatement * kernelCall = buildFunctionCallStmt (
      kernelSubroutine.getSubroutineName () + "<<<"
          + ROSEHelper::getFirstVariableName (CUDAVariable_blocksPerGrid)
          + ", " + ROSEHelper::getFirstVariableName (
          CUDAVariable_threadsPerBlock) + ", "
          + ROSEHelper::getFirstVariableName (CUDAVariable_sharedMemorySize)
          + ">>>", buildVoidType (), kernelParameters, subroutineScope);

  appendStatement (kernelCall, subroutineScope);
}

void
HostSubroutineOfDirectLoop::createCUDAVariables (ParallelLoop & parallelLoop)
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDoubleType;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionType;
  using SageBuilder::buildFunctionParameterTypeList;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", 2);

  /*
   * ======================================================
   * This function builds the following Fortran code:
   *
   * integer(4) :: bsize          = BSIZE_DEFAULT
   * integer(4) :: reduct_bytes   = 0
   * integer(4) :: reduct_size    = 0
   * integer(4) :: const_bytes    = 0
   * integer(4) :: gsize
   * real(8)    :: reduct_shared
   *
   * gsize         = int ((set%size - 1) / bsize + 1)
   * reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
   * ======================================================
   */

  /*
   * ======================================================
   * Declaration and initialisation of variables and opaque
   * variables
   * ======================================================
   */
  SgVarRefExp * variable_BSIZE_DEFAULT = buildOpaqueVarRefExp ("BSIZE_DEFAULT",
      subroutineScope);

  SgVariableDeclaration * variable_reduct_bytes = buildVariableDeclaration (
      "reduct_bytes", FortranTypesBuilder::getFourByteInteger (),
      buildAssignInitializer (buildIntVal (0), buildIntType ()),
      subroutineScope);

  SgVariableDeclaration * variable_reduct_size = buildVariableDeclaration (
      "reduct_size", FortranTypesBuilder::getFourByteInteger (),
      buildAssignInitializer (buildIntVal (0), buildIntType ()),
      subroutineScope);

  SgVariableDeclaration * variable_const_bytes = buildVariableDeclaration (
      "const_bytes", FortranTypesBuilder::getFourByteInteger (),
      buildAssignInitializer (buildIntVal (0), buildIntType ()),
      subroutineScope);

  variable_reduct_bytes->get_declarationModifier ().get_accessModifier ().setUndefined ();
  variable_reduct_size->get_declarationModifier ().get_accessModifier ().setUndefined ();
  variable_const_bytes->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_reduct_bytes, subroutineScope);
  appendStatement (variable_reduct_size, subroutineScope);
  appendStatement (variable_const_bytes, subroutineScope);

  /*
   * ======================================================
   * gsize = int ((set%size - 1) / bsize + 1)
   * ======================================================
   */

  SgExpression * sizeFieldReference = buildDotExp (buildVarRefExp (
      formalParameter_OP_SET), buildOpaqueVarRefExp ("size", subroutineScope));

  SgExpression * minusOneExpression = buildSubtractOp (sizeFieldReference,
      buildIntVal (1));

  SgExpression * parameter1 = buildAddOp (buildDivideOp (minusOneExpression,
      buildVarRefExp (CUDAVariable_blocksPerGrid)), buildIntVal (1));

  SgFunctionSymbol * intFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("int", subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (parameter1);

  SgFunctionCallExp * intIntrinsicCall = buildFunctionCallExp (
      intFunctionSymbol, actualParameters);

  SgExpression * expression1 = buildAssignOp (buildVarRefExp (
      CUDAVariable_threadsPerBlock), intIntrinsicCall);

  appendStatement (buildExprStatement (expression1), subroutineScope);

  /*
   * ======================================================
   * reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
   * ======================================================
   */

  SgVarRefExp * reduct_sizeReference = buildVarRefExp (variable_reduct_size);

  SgExpression * rhsOfExpression = buildMultiplyOp (reduct_sizeReference,
      buildDivideOp (variable_BSIZE_DEFAULT, buildIntVal (2)));

  SgExpression * expression2 = buildAssignOp (buildVarRefExp (
      CUDAVariable_sharedMemorySize), rhsOfExpression);

  appendStatement (buildExprStatement (expression2), subroutineScope);
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

HostSubroutineOfDirectLoop::HostSubroutineOfDirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    KernelSubroutine & kernelSubroutine, ParallelLoop & parallelLoop,
    SgScopeStatement * moduleScope) :
  HostSubroutine (subroutineName, userDeviceSubroutine, parallelLoop,
      moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", 2);

  createFormalParameters (userDeviceSubroutine, parallelLoop);

  createDataMarshallingLocalVariables (parallelLoop);

  createCUDAKernelVariables ();

  createCUDAVariables (parallelLoop);

  initialiseDataMarshallingLocalVariables (parallelLoop);

  createKernelCall (kernelSubroutine, parallelLoop);

  copyDataBackFromDeviceAndDeallocate (parallelLoop);
}

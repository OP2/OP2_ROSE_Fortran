#include <boost/lexical_cast.hpp>
#include <HostSubroutineOfDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

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

  // 'set%size'
  SgExpression * setPERCENTsize = buildDotExp (buildVarRefExp (
      formalParameter_OP_SET), buildOpaqueVarRefExp ("size", subroutineScope));

  // 'set%size - 1'
  SgExpression * setPERCENTsize_minusOne = buildSubtractOp (setPERCENTsize,
      buildIntVal (1));

  // '(set%size - 1) / bsize  + 1'
  SgExpression * castIntegerParameter = buildAddOp (buildDivideOp (
      setPERCENTsize_minusOne, buildVarRefExp (CUDAVariable_blocksPerGrid)),
      buildIntVal (1));

  /*
   * ======================================================
   * 'int ((set%size - 1) / bsize + 1)'
   *
   * To do this we have to build a function type for the
   * intrinsic 'int'. We do not care about the fact that the
   * function is already defined (also via an intrinsic): we
   * just tell ROSE all the information needed to build a new
   * function
   * ======================================================
   */
  SgFunctionType * functionDoubleToInteger = buildFunctionType (
      buildIntType (), buildFunctionParameterTypeList (buildDoubleType ()));

  SgFunctionRefExp * referenceToIntegerFunction = buildFunctionRefExp ("int",
      functionDoubleToInteger, subroutineScope);

  SgExprListExp * intParams = buildExprListExp (castIntegerParameter);

  SgFunctionCallExp * intIntrinsicCall = buildFunctionCallExp (
      referenceToIntegerFunction->get_symbol (), intParams);

  SgExpression * variable_gsize_assignment = buildAssignOp (buildVarRefExp (
      CUDAVariable_threadsPerBlock), intIntrinsicCall);

  appendStatement (buildExprStatement (variable_gsize_assignment),
      subroutineScope);

  /*
   * ======================================================
   * reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
   * ======================================================
   */

  SgExpression * rhsOfExpression = buildMultiplyOp (buildVarRefExp (
      variable_reduct_size), buildDivideOp (variable_BSIZE_DEFAULT,
      buildIntVal (2)));

  SgExpression * variable_reduct_shared_assignment = buildAssignOp (
      buildVarRefExp (CUDAVariable_sharedMemorySize), rhsOfExpression);

  appendStatement (buildExprStatement (variable_reduct_shared_assignment),
      subroutineScope);
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

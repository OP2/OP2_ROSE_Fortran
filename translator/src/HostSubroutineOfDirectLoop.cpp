#include <boost/lexical_cast.hpp>
#include <HostSubroutineOfDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
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

  SgExprListExp * kernelParameters = buildExprListExp ();

  SgExpression * opSetFormalArgumentReference = buildVarRefExp (
      formalParameter_OP_SET);

  SgExpression * sizeFieldExpression = buildDotExp (
      opSetFormalArgumentReference, buildOpaqueVarRefExp ("size",
          subroutineScope));

  kernelParameters->append_expression (sizeFieldExpression);

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {

      SgVarRefExp * opDatDeviceReference = buildVarRefExp (
          localVariables_OP_DAT_VariablesOnDevice[i]);

      kernelParameters->append_expression (opDatDeviceReference);
    }
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
   * nblocks = int ((set%size - 1) / nthread + 1)
   * ======================================================
   */

  SgVarRefExp * opSetFormalParameterReference = buildVarRefExp (
      formalParameter_OP_SET);

  SgVarRefExp * blocksPerGridReference = buildVarRefExp (
      CUDAVariable_blocksPerGrid);

  SgVarRefExp * threadsPerBlockReference = buildVarRefExp (
      CUDAVariable_threadsPerBlock);

  SgExpression * opSet_SizeField_Reference = buildDotExp (
      opSetFormalParameterReference, buildOpaqueVarRefExp ("size",
          subroutineScope));

  SgExpression * minusOneExpression = buildSubtractOp (
      opSet_SizeField_Reference, buildIntVal (1));

  SgExpression * parameter1 = buildAddOp (buildDivideOp (minusOneExpression,
      threadsPerBlockReference), buildIntVal (1));

  SgFunctionSymbol * intFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("int", subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (parameter1);

  SgFunctionCallExp * intFunctionCall = buildFunctionCallExp (
      intFunctionSymbol, actualParameters);

  SgExpression * assignExpression1 = buildAssignOp (blocksPerGridReference,
      intFunctionCall);

  appendStatement (buildExprStatement (assignExpression1), subroutineScope);

  /*
   * ======================================================
   * reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
   * ======================================================
   */

  SgVarRefExp * sharedMemorySizeReference = buildVarRefExp (
      CUDAVariable_sharedMemorySize);

  SgVarRefExp * reduct_sizeReference = buildVarRefExp (variable_reduct_size);

  SgDivideOp * divideExpression = buildDivideOp (variable_BSIZE_DEFAULT,
      buildIntVal (2));

  SgMultiplyOp * multiplyExpression = buildMultiplyOp (reduct_sizeReference,
      divideExpression);

  SgExpression * assignExpression2 = buildAssignOp (sharedMemorySizeReference,
      multiplyExpression);

  appendStatement (buildExprStatement (assignExpression2), subroutineScope);
}

void
HostSubroutineOfDirectLoop::createDeviceVariablesSizesVariable ( 
  DeviceDataSizesDeclarationDirectLoops & deviceDataSizesDeclarationDirectLoops ) 
{

	Debug::getInstance ()->debugMessage ("Creating device data sizes variable", 2);
	
  /*
   * ======================================================
   * Create the variable which passes the sizes of arguments
   * to the kernel
   * ======================================================
   */
	
  SgVariableDeclaration * variableDeclaration2 =
	FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
		LoopVariables::argsSizes,
		deviceDataSizesDeclarationDirectLoops.getType (),
		subroutineScope);
	
  localVariables_Others[LoopVariables::argsSizes]
		= variableDeclaration2;
}


void
HostSubroutineOfDirectLoop::initialiseDeviceVariablesSizesVariable ( 
	ParallelLoop & parallelLoop )
{
	using std::string;
  using boost::lexical_cast;
	using SageBuilder::buildOpaqueVarRefExp;
	using SageBuilder::buildDotExp;
	using SageBuilder::buildVarRefExp;
	using SageBuilder::buildAssignOp;
	using SageBuilder::buildExprStatement;
	using SageInterface::appendStatement;
	
	/*
   * ======================================================
   * In direct loop sizes are only related to op_dat
	 * variables
   * ======================================================
   */
	
	for (unsigned int i = 1; i
			 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {			
			string const sizeFieldExpression = "parg" + lexical_cast < string > ( i ) + "DatDSize";
			
      SgExpression * sizeVariableField = buildDotExp ( 
				buildVarRefExp ( localVariables_Others[LoopVariables::argsSizes] ),
				buildOpaqueVarRefExp ( sizeFieldExpression, subroutineScope ) );
			

			SgExpression * assignExpression = buildAssignOp ( sizeVariableField,
				buildVarRefExp ( localVariables_OP_DAT_Sizes[i] ) );
			
      appendStatement ( buildExprStatement ( assignExpression ), subroutineScope);
    }
	}
}
																											 


/*
 * ======================================================
 * Public functions
 * ======================================================
 */

HostSubroutineOfDirectLoop::HostSubroutineOfDirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    KernelSubroutine & kernelSubroutine, 
		DeviceDataSizesDeclarationDirectLoops & deviceDataSizesDeclarationDirectLoops,
    ParallelLoop & parallelLoop,
    SgScopeStatement * moduleScope) :
  HostSubroutine (subroutineName, userDeviceSubroutine, parallelLoop,
      moduleScope)
{
  Debug::getInstance ()->debugMessage (
      "Creating host subroutine of direct loop", 2);

  createFormalParameters (userDeviceSubroutine, parallelLoop);

	createDeviceVariablesSizesVariable ( deviceDataSizesDeclarationDirectLoops );
	
  createDataMarshallingLocalVariables (parallelLoop);
	
  createCUDAKernelVariables ();

  createCUDAVariables (parallelLoop);

  initialiseDataMarshallingLocalVariables (parallelLoop);

	initialiseDeviceVariablesSizesVariable ( parallelLoop );
	
  createKernelCall (kernelSubroutine, parallelLoop);

  copyDataBackFromDeviceAndDeallocate (parallelLoop);
}

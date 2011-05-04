#include <boost/lexical_cast.hpp>
#include <HostSubroutineOfDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <OP2CUDAConstants.h>
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

  /*
   * ======================================================
   * argSizes
   * ======================================================
	 */	 		
	kernelParameters->append_expression ( buildVarRefExp ( localVariables_Others[LoopVariables::argsSizes] ) );

  /*
   * ======================================================
   * op_dat content arguments on device
   * ======================================================
	 */
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

  /*
   * ======================================================
   * offsetS
   * ======================================================
	 */	 	
	kernelParameters->append_expression ( buildVarRefExp ( CUDAVariable_offsetS ) );
	
  /*
   * ======================================================
   * set%size
   * ======================================================
	 */	 
  SgExpression * opSetFormalArgumentReference = buildVarRefExp ( formalParameter_OP_SET );
	
  SgExpression * sizeFieldExpression = buildDotExp (
		opSetFormalArgumentReference, buildOpaqueVarRefExp ("size",
																		subroutineScope));
	
  kernelParameters->append_expression (sizeFieldExpression);
	 
	/*
   * ======================================================
   * warpSizeOP2
   * ======================================================
	 */	 	
	kernelParameters->append_expression ( buildVarRefExp ( CUDAVariable_warpSizeOP2 ) );
	
	 
  SgExprStatement * kernelCall = buildFunctionCallStmt (
      kernelSubroutine.getSubroutineName () + "<<<"
          + ROSEHelper::getFirstVariableName (CUDAVariable_blocksPerGrid)
          + ", " + ROSEHelper::getFirstVariableName (
          CUDAVariable_threadsPerBlock) + ", "
          + ROSEHelper::getFirstVariableName (CUDAVariable_sharedMemorySize)
          + ">>>", buildVoidType (), kernelParameters, subroutineScope);

  appendStatement (kernelCall, subroutineScope);
	
	
	/*
   * ======================================================
   * threadSynchRet = cudaThreadSynchronize()
   * ======================================================
	 */	 		
	SgStatement * threadSynchCallStmt = buildThreadSynchroniseFunctionCall ( subroutineScope );
	
  appendStatement ( threadSynchCallStmt, subroutineScope);
	
}

void
HostSubroutineOfDirectLoop::createCUDAVariablesDirectLoops (ParallelLoop & parallelLoop)
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
   * Declaration and initialisation of CUDA variables
	 * specific of direct loops
   * ======================================================
   */
  CUDAVariable_offsetS = buildVariableDeclaration (
      "offsetS", FortranTypesBuilder::getFourByteInteger (),
      buildAssignInitializer (buildIntVal (0), buildIntType ()),
      subroutineScope);

  CUDAVariable_warpSizeOP2 = buildVariableDeclaration (
      "warpSizeOP2", FortranTypesBuilder::getFourByteInteger (),
      buildAssignInitializer (buildIntVal (0), buildIntType ()),
      subroutineScope);

	
  CUDAVariable_threadSynchRet = buildVariableDeclaration (
		"threadSynchRet", FortranTypesBuilder::getFourByteInteger (),
		buildAssignInitializer (buildIntVal (0), buildIntType ()),
		subroutineScope);
	

  CUDAVariable_offsetS->get_declarationModifier ().get_accessModifier ().setUndefined ();
  CUDAVariable_warpSizeOP2->get_declarationModifier ().get_accessModifier ().setUndefined ();
  CUDAVariable_threadSynchRet->get_declarationModifier ().get_accessModifier ().setUndefined ();
	
  Debug::getInstance ()->debugMessage (
		"Before appending the statements", 2);

	
  appendStatement ( CUDAVariable_offsetS, subroutineScope );
  appendStatement ( CUDAVariable_warpSizeOP2, subroutineScope );
  appendStatement ( CUDAVariable_threadSynchRet, subroutineScope );

  Debug::getInstance ()->debugMessage (
		"After appending the statements", 2);


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
			
			string const & variableName = kernelDatArgumentsNames::argNamePrefix + lexical_cast <string> (i);

			string const sizeFieldExpression = variableName + kernelDatArgumentsNames::argNameSizePostfix;
			
      SgExpression * sizeVariableField = buildDotExp ( 
				buildVarRefExp ( localVariables_Others[LoopVariables::argsSizes] ),
				buildOpaqueVarRefExp ( sizeFieldExpression, subroutineScope ) );
			

			SgExpression * assignExpression = buildAssignOp ( sizeVariableField,
				buildVarRefExp ( localVariables_OP_DAT_Sizes[i] ) );
			
      appendStatement ( buildExprStatement ( assignExpression ), subroutineScope );
    }
	}
}
		
void
HostSubroutineOfDirectLoop::initialiseAllCUDAVariables (
	ParallelLoop & parallelLoop )

{
	using SageBuilder::buildOpaqueVarRefExp;
	using SageBuilder::buildVarRefExp;
	using SageBuilder::buildIntVal;
	using SageBuilder::buildExprStatement;
	using SageBuilder::buildAssignOp;
	using SageBuilder::buildMultiplyOp;
	using SageBuilder::buildExprListExp;
	using SageBuilder::buildFunctionCallExp;
	using SageInterface::appendStatement;

	
	SgVarRefExp * variable_OP_WARP_SIZE = buildOpaqueVarRefExp ( "OP_WARP_SIZE",
		subroutineScope );
		
	/*
   * ======================================================
   * initialising fixed value variables nblocks, nthreads
	 * and warpSizeOP2
   * ======================================================
   */

	

	SgExpression * assignExpressionNblocks =
		buildAssignOp ( buildVarRefExp ( CUDAVariable_blocksPerGrid ),
										buildIntVal ( CUDAConstantsDirectLoops::nblocks ) );


	SgExpression * assignExpressionNthreads =
	buildAssignOp ( buildVarRefExp ( CUDAVariable_threadsPerBlock ),
								  buildIntVal ( CUDAConstantsDirectLoops::nthreads ) );
	

	SgExpression * assignExpressionWarpSizeOP2 =
		buildAssignOp ( buildVarRefExp ( CUDAVariable_warpSizeOP2 ),
									  variable_OP_WARP_SIZE );
	

	
	appendStatement ( buildExprStatement ( assignExpressionNblocks ), subroutineScope );
	appendStatement ( buildExprStatement ( assignExpressionNthreads ), subroutineScope );
	appendStatement ( buildExprStatement ( assignExpressionWarpSizeOP2 ), subroutineScope );

	/*
   * ======================================================
   * Computing value for nshared: an input op_dat is copied
	 * to shared memory only if: its dimension is larger than
	 * 1 or it is not encapsulating a global data.
	 * Therefore, the maximum size of shared memory is
	 * equal to maximum size * dimension op_dat copied on
	 * shared memory, multiplied by the number of threads
   * ======================================================
   */
	for (unsigned int i = 1; i
			 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
			int dim = parallelLoop.get_OP_DAT_Dimension ( i );
			if (  dim > 1 &&
					 parallelLoop.get_OP_MAP_Value ( i ) != GLOBAL )
			{
			
				/*
				 * ======================================================
				 * nshared = max ( nshared, size * dim )
				 * ======================================================
				 */
				
			  SgFunctionSymbol * shiftFunctionSymbol =
					FortranTypesBuilder::buildNewFortranFunction ( "max", subroutineScope );
				
				
				/*
				 * ======================================================
				 * retriving size from type of correponding op_dat
				 * declaration
				 * ======================================================
				 */
				
				SgType * opDatBaseType = parallelLoop.get_OP_DAT_Type ( i );
				
				SgArrayType * isArrayType = isSgArrayType ( opDatBaseType );
				
				ROSE_ASSERT ( isArrayType != NULL );
				
				opDatBaseType = isArrayType->get_base_type ();
				
				SgExpression * opDatKindSize = opDatBaseType->get_type_kind ();
				
				/*
				 * ======================================================
				 * if the user hasn't specified a fortran kind, we have
				 * assume standard ones: integer(4) and real(4)
				 * ======================================================
				 */
				
				if ( opDatKindSize == NULL ) 
				{
					
					if ( isSgTypeInt ( opDatBaseType ) != NULL )
						opDatKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_INT );
					
					if ( isSgTypeFloat ( opDatBaseType ) != NULL )
						opDatKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_REAL );
					
					// default case
					if ( opDatKindSize == NULL )
						opDatKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_REAL );
					
				}
				
				SgExpression * secondParameterMaxCall = buildMultiplyOp (
				  buildIntVal ( dim ), opDatKindSize );
				
				SgExprListExp * actualParameters = buildExprListExp ( 
				  buildVarRefExp ( CUDAVariable_sharedMemorySize ), 
					secondParameterMaxCall );
				
				SgFunctionCallExp * maxFunctionCall = buildFunctionCallExp ( shiftFunctionSymbol, 
					actualParameters );
				
				FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
					CUDAVariable_sharedMemorySize, maxFunctionCall, subroutineScope );
			}
		}
	}
	
	/*
	 * ======================================================
	 * nshared = nshared * nthreads 
	 * ======================================================
	 */
	SgExpression * multiplyNsharedPerNthreads = buildMultiplyOp (
    buildVarRefExp ( CUDAVariable_sharedMemorySize ),
		buildVarRefExp ( CUDAVariable_threadsPerBlock ) );
	
	FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
		CUDAVariable_sharedMemorySize, multiplyNsharedPerNthreads, subroutineScope );
	


	/*
	 * ======================================================
	 * 	offsetS = nshared * OP_WARP_SIZE
	 * ======================================================
	 */
	SgExpression * multiplyNsharedPerWarpSize = buildMultiplyOp (
		buildVarRefExp ( CUDAVariable_sharedMemorySize ), variable_OP_WARP_SIZE );
	 
	FortranStatementsAndExpressionsBuilder::appendAssignmentStatement (
	  CUDAVariable_offsetS, multiplyNsharedPerWarpSize, subroutineScope );


	
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

  createCUDAVariablesDirectLoops (parallelLoop);

	initialiseAllCUDAVariables ( parallelLoop );
	
  initialiseDataMarshallingLocalVariables (parallelLoop);

	initialiseDeviceVariablesSizesVariable ( parallelLoop );
	
  createKernelCall (kernelSubroutine, parallelLoop);

  copyDataBackFromDeviceAndDeallocate (parallelLoop);
}

#include <Debug.h>
#include <boost/lexical_cast.hpp>
#include <KernelSubroutine.h>
#include <OP2CommonDefinitions.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>

void
KernelSubroutine::createArgsSizesFormalParameter (
	DeviceDataSizesDeclaration & deviceDataSizesDeclaration )
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;
	
  Debug::getInstance ()->debugMessage ("Creating arg sizes formal parameter",
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


void
KernelSubroutine::createLocalThreadVariables ( ParallelLoop & parallelLoop, 
	SgScopeStatement & subroutineScope, bool isDirectLoop )
{
	using std::string;
  using boost::lexical_cast;
	using SageBuilder::buildIntVal;
	using SageBuilder::buildArrayType;
	using SageBuilder::buildVariableDeclaration;
	using SageBuilder::buildExprListExp;
	using SageInterface::appendStatement;


	Debug::getInstance ()->debugMessage ("Creating local thread variables 1",
																			 2);

	for (unsigned int i = 1; i
			 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
		int dim = parallelLoop.get_OP_DAT_Dimension ( i );

		if ( ( isDirectLoop == true &&
					 dim > 1 &&
					 parallelLoop.get_OP_MAP_Value ( i ) == DIRECT )
				 ||
				 ( parallelLoop.get_OP_MAP_Value ( i ) == GLOBAL &&
					 parallelLoop.get_OP_Access_Value ( i ) != READ_ACCESS ) 
				 ||
				 ( parallelLoop.get_OP_MAP_Value ( i ) == INDIRECT &&
				   parallelLoop.get_OP_Access_Value ( i ) == INC_ACCESS )
			 )
		{
			string const & localThreadVarName = kernelLocalThreadVarsNames::localVarNamePrefix + 
			lexical_cast <string> (i) + kernelLocalThreadVarsNames::localVarNamePostfix;
			
			/*
			 * ======================================================
			 * retriving size from type of correponding op_dat
			 * declaration
			 * ======================================================
			 */
			
			SgType * opDatBaseType = parallelLoop.get_OP_DAT_Type ( i );
			
			SgArrayType * arrayType = isSgArrayType ( opDatBaseType );
			
			ROSE_ASSERT ( arrayType != NULL );
			
			opDatBaseType = arrayType->get_base_type ();
			
			SgArrayType * variableType = FortranTypesBuilder::getArray_RankOne ( 
				opDatBaseType, 0, dim -1 );

			SgVariableDeclaration * variableDeclaration =
				buildVariableDeclaration ( localThreadVarName, variableType, NULL, &subroutineScope );
			
			variableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
			
			appendStatement ( variableDeclaration, &subroutineScope );
			
			localVariables_localThreadVariables[i] = variableDeclaration;
						
		}
	}
}

void
KernelSubroutine::createAutosharedVariable ( 
  ParallelLoop & parallelLoop, SgScopeStatement * scopeStatement )
{

	/*
	 * ======================================================
	 * For now only real(8) or real(4): this is decided
	 * by checking the fortran type kinds of parallel loop
	 * op_dat arguments
	 * ======================================================
	 */

	SgType * autosharedType = NULL;

	for (unsigned int i = 1; i
			 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {
		
			SgType * opDatArrayType = parallelLoop.get_OP_DAT_Type ( i );
			
			SgArrayType * isArrayType = isSgArrayType ( opDatArrayType );
			
			ROSE_ASSERT ( isArrayType != NULL );
			
			SgType * opDatBaseType = isArrayType->get_base_type ();

			SgType * isRealType = isSgTypeFloat ( opDatBaseType );

			/*
			 * ======================================================
			 * We copy the user type with kind (if exists) otherwise
			 * we assume real(4) (CUDA Fortran specific)
			 * ======================================================
			 */
			 
			if ( isRealType != NULL )
				autosharedType = opDatBaseType;
		}
	}
		
	if ( autosharedType != NULL )
	{
	
		/*
		 * ======================================================
		 * At least one of input op_dat variables is a real,
		 * hence we declare the autoshared variable
		 * ======================================================
		 */
		 localVariables_autoshared = FortranStatementsAndExpressionsBuilder::
		   createAndAppendAutosharedVariable ( autosharedType, scopeStatement );
	
	}
}

void
KernelSubroutine::initialiseLocalThreadVariables ( 
  ParallelLoop & parallelLoop, SgScopeStatement * scopeStatement,
	SgVarRefExp * iterationVariableReference )
{
	using SageBuilder::buildOpaqueVarRefExp;
	using SageBuilder::buildPntrArrRefExp;
	using SageBuilder::buildAssignOp;
	using SageBuilder::buildVarRefExp;
	using SageBuilder::buildAssignOp;
	using SageBuilder::buildIntVal;
	using SageBuilder::buildBasicBlock;
	using SageBuilder::buildMultiplyOp;
	using SageBuilder::buildDotExp;
	using SageBuilder::buildExprStatement;
	using SageBuilder::buildAddOp;
	using SageInterface::appendStatement;

	/*
	 * ======================================================
	 * We essentially do a similar thing to the one we make
	 * when we declare local thread variables
	 * ======================================================
	 */
	
	for (unsigned int i = 1; i
			 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
		int dim = parallelLoop.get_OP_DAT_Dimension ( i );
		
		if ( parallelLoop.get_OP_MAP_Value ( i ) == GLOBAL &&
				 parallelLoop.get_OP_Access_Value ( i ) != READ_ACCESS )
		{

			/*
			 * ======================================================
			 * This condition only changes the loop body, not its
			 * lower and upper bounds and stride
			 * ======================================================
			 */
			SgVarRefExp * variable_X = buildOpaqueVarRefExp (variableName_x,
				subroutineScope);
			
			SgVarRefExp * variable_Blockidx = buildOpaqueVarRefExp (
        variableName_blockidx, subroutineScope);
			
			 
			SgBasicBlock * loopBodyBlock;
			SgExpression * accessToIPosition = buildPntrArrRefExp (
			  buildVarRefExp ( localVariables_localThreadVariables[i] ), iterationVariableReference );
			
			if ( parallelLoop.get_OP_Access_Value ( i ) == INC_ACCESS )
			{
				// for (int d=0; d<DIM; d++) ARG_l[d]=ZERO_TYP;
				SgExpression * assignArgToZero = buildAssignOp ( accessToIPosition, buildIntVal ( 0 ) );

				loopBodyBlock =	buildBasicBlock ( buildExprStatement ( assignArgToZero) );
			}
			else
			{
				//for (int d=0; d<DIM; d++) ARG_l[d]=ARG[d+blockIdx.x*DIM];
				SgExpression * blockIdxPerDim = buildMultiplyOp ( 
				  buildDotExp ( variable_Blockidx, variable_X ),
					buildIntVal ( dim ) );

				SgExpression * arrayAccessComplexExpr = buildAddOp ( iterationVariableReference, 
				  blockIdxPerDim );

				SgExpression * complexAccessToArg = buildPntrArrRefExp (
				  buildVarRefExp ( localVariables_localThreadVariables[i] ),
					arrayAccessComplexExpr );

				SgExpression * assignArgToComplexAccess = 
				  buildAssignOp ( accessToIPosition, complexAccessToArg );

				loopBodyBlock =	buildBasicBlock ( buildExprStatement ( assignArgToComplexAccess ) );
			}


			/*
			 * ======================================================
			 * Remember that local thread variables are accessed
			 * in the C-like notation for arrays: 0:N-1
			 * ======================================================
			 */
			
			SgExpression * initializationExpression = buildAssignOp (
				iterationVariableReference, buildIntVal ( 0 ) );

			SgExpression * upperBoundExpression = buildIntVal ( dim - 1 );

			/*
			 * ======================================================
			 * The stride of the loop counter is 1
			 * ======================================================
			 */
			SgExpression * strideExpression = buildIntVal (1);
			

			/*
			 * ======================================================
			 * Add the do-loop statement
			 * ======================================================
			 */
			SgFortranDo * fortranDoStatement =
			FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
				initializationExpression, upperBoundExpression, strideExpression,
				loopBodyBlock );
			
			appendStatement ( fortranDoStatement, scopeStatement );

		}
	}			

}

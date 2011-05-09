#include <ReductionSubroutine.h>
#include <ROSEHelper.h>
#include <Debug.h>
#include <FortranTypesBuilder.h>
#include <OP2CommonDefinitions.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <boost/lexical_cast.hpp>


// TO DO: generate code for OP_MIN and OP_MAX as well

SgStatement *
ReductionSubroutine::createCallToSynchThreads ( )
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildExprStatement;
	
  SgFunctionSymbol * functionSymbol =
	FortranTypesBuilder::buildNewFortranSubroutine ("synchthreads",
																									subroutineScope);
	
  SgExprListExp * actualParameters = buildExprListExp ( );
	
  SgFunctionCallExp * subroutineCall = buildFunctionCallExp (functionSymbol,
																														 actualParameters);
	
  return buildExprStatement (subroutineCall);
}


/*
 * the type of the reduction variable must already include the fact that it is an array
 */

void
ReductionSubroutine::createFormalParameters ( )
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;
	
  Debug::getInstance ()->debugMessage ("Creating reduction procedure formal parameter",
																			 2);
	

  formalParameter_reductionResultDevice = buildVariableDeclaration ( "dat_g",
																																		 reductionVariableType, 
																																		 NULL,
																																		 subroutineScope
																																	 );
									
  formalParameter_reductionResultDevice->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_reductionResultDevice->get_declarationModifier ().get_typeModifier ().setDevice ();
	
  formalParameters->append_arg (
																*(formalParameter_reductionResultDevice->get_variables ().begin ()));
	
  appendStatement (formalParameter_reductionResultDevice, subroutineScope);

	
	/*
	 * ======================================================
	 * \warning: the type of the input data is for now limited
	 * to a scalar value (does not manage reduction on arrays)
	 * ======================================================
	 */
  formalParameter_inputValue = buildVariableDeclaration ( "dat_l",
																												  reductionVariableType->get_base_type(),
																												  NULL, 
																												  subroutineScope 
																												);
	
  formalParameter_inputValue->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_inputValue->get_declarationModifier ().get_typeModifier ().setValue ();
	
  formalParameters->append_arg (
																*(formalParameter_inputValue->get_variables ().begin ()));
	
  appendStatement (formalParameter_inputValue, subroutineScope);
	

	formalParameter_warpSize = buildVariableDeclaration ( "warpsize",
																												buildIntType(),
																												NULL, 
																												subroutineScope 
																											);
	
  formalParameter_warpSize->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_warpSize->get_declarationModifier ().get_typeModifier ().setValue ();
	
  formalParameters->append_arg (
																*(formalParameter_warpSize->get_variables ().begin ()));
	
  appendStatement (formalParameter_warpSize, subroutineScope);
	
	
	formalParameter_sharedMemStartOffset = buildVariableDeclaration ( "autosharedStartOffset",
																																	  buildIntType(),
																																	  NULL, 
																																	  subroutineScope 
																																	);
	
  formalParameter_sharedMemStartOffset->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_sharedMemStartOffset->get_declarationModifier ().get_typeModifier ().setValue ();
	
  formalParameters->append_arg (
																*(formalParameter_sharedMemStartOffset->get_variables ().begin ()));
	
  appendStatement (formalParameter_sharedMemStartOffset, subroutineScope);
	
}


void ReductionSubroutine::createLocalVariables ( )
{
	using SageInterface::appendStatement;
	using SageBuilder::buildVariableDeclaration;
	using SageBuilder::buildIntVal;
	using SageBuilder::buildIntType;	
	using SageBuilder::buildArrayType;
	using SageBuilder::buildOpaqueVarRefExp;

	/*
	 * ======================================================
	 * builds autoshared variable declaration with the same
	 * base type of the reduction variable
	 * ======================================================
	 */

	localVariables_autoshared =  FortranStatementsAndExpressionsBuilder::createAndAppendAutosharedVariable ( 
	  reductionVariableType->get_base_type(), subroutineScope );
		
	localVariables_IterationCounter = buildVariableDeclaration ( "iterCount",
																															 buildIntType (),
																															 NULL,
																															 subroutineScope
																														 );

  localVariables_IterationCounter->get_declarationModifier ().get_accessModifier ().setUndefined ();
	
  appendStatement (localVariables_IterationCounter, subroutineScope);

	localVariables_threadID = buildVariableDeclaration ( "tid",
																											 buildIntType (),
																											 NULL,
																											 subroutineScope
																										 );
	
  localVariables_threadID->get_declarationModifier ().get_accessModifier ().setUndefined ();
	
  appendStatement ( localVariables_threadID, subroutineScope );

	/*
	 * ======================================================
	 * create opaque reference to threadid%x
	 * ======================================================
	 */
	
	variable_Threadidx = buildOpaqueVarRefExp ("threadidx", subroutineScope);	
	variable_Blockdim = buildOpaqueVarRefExp ("blockdim", subroutineScope);
	variable_X = buildOpaqueVarRefExp ("x", subroutineScope);
	
}


void ReductionSubroutine::createStatements ()
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
	using SageBuilder::buildGreaterThanOp;
	using SageBuilder::buildBasicBlock;
	using SageBuilder::buildWhileStmt;
	using SageBuilder::buildEqualityOp;
	using SageBuilder::buildVarRefExp;
	using SageBuilder::buildDivideOp;
	using SageBuilder::buildPntrArrRefExp;
	using SageBuilder::buildVoidType;


	/*
	 * ======================================================
	 * initialise local variables
	 * ======================================================
	 */
	
	SgSubtractOp * subtractExpression1 = buildSubtractOp ( buildDotExp ( variable_Threadidx, 
																																			 variable_X
																																		 ), 
																												 buildIntVal (1)
																											 );

	SgExpression * tidInitialAssignExpression = 
		buildAssignOp ( buildVarRefExp ( localVariables_threadID ),
										subtractExpression1
									);
	
	appendStatement ( buildExprStatement ( tidInitialAssignExpression ), subroutineScope );
	
	SgExpression * blockdimXExpr = buildDotExp ( variable_Blockdim, 
																							 variable_X
																						 );
	
	

//	SgExpression * parameter1 = buildAddOp (buildDivideOp ( minusOneExpression,
//																												  threadsPerBlockReference ), buildIntVal (1));
	
	
  SgFunctionSymbol * shiftFunctionSymbol =
		FortranTypesBuilder::buildNewFortranFunction ("ishft", subroutineScope);
	
  SgExprListExp * actualParameters = buildExprListExp ( blockdimXExpr, buildIntVal ( -1 ) );
	
  SgFunctionCallExp * shiftFunctionCall = buildFunctionCallExp ( shiftFunctionSymbol, 
																																 actualParameters
																															 );
	
  SgExpression * initIterationCounter = buildAssignOp ( buildVarRefExp ( localVariables_IterationCounter ),
																											  shiftFunctionCall
																										  );
	
  appendStatement ( buildExprStatement ( initIterationCounter ), subroutineScope );
	
	
	/*
	 * ======================================================
	 * startoffset is given in terms of bytes. it must be
	 * recomputed in terms of the fortran kind type of the
	 * reduction variable
	 * ======================================================
	 */
	
	SgExpression * autosharedKindSize = reductionVariableType->get_base_type()->get_type_kind ();
	
	/*
	 * ======================================================
	 * if the user hasn't specified a fortran kind, we have
	 * assume standard ones: integer(4) and real(4)
	 * ======================================================
	 */
	
	if ( autosharedKindSize == NULL ) 
	{
		
		if ( isSgTypeInt ( reductionVariableType->get_base_type() ) != NULL )
			autosharedKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_INT );
		
		if ( isSgTypeFloat ( reductionVariableType->get_base_type() ) != NULL )
			autosharedKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_REAL );
		
		// default case
		if ( autosharedKindSize == NULL )
			autosharedKindSize = buildIntVal ( FortranVariableDeafultKinds::DEFAULT_KIND_REAL );
		
	}
	
	SgVarRefExp * autosharedOffsetReference = buildVarRefExp ( formalParameter_sharedMemStartOffset );
	
	SgAssignOp * initStartOffset = buildAssignOp ( autosharedOffsetReference, 
																								buildDivideOp ( autosharedOffsetReference,
																															 autosharedKindSize
																															 )
																								);

	appendStatement ( buildExprStatement ( initStartOffset ), subroutineScope );

	
	/*
	 * ======================================================
	 * synchthreads function call
	 * ======================================================
	 */
	
  appendStatement (	createCallToSynchThreads (), subroutineScope );
	
	/*
	 * ======================================================
	 * builds initial assignment of computed value to shared
	 * memory position
	 * ======================================================
	 */
	
	SgExpression * arrayIndexExpression = buildAddOp ( buildVarRefExp ( formalParameter_sharedMemStartOffset ),
																									   buildVarRefExp ( localVariables_threadID )
																									 );
	
	SgPntrArrRefExp * autosharedSubscriptExpression = buildPntrArrRefExp ( buildVarRefExp ( localVariables_autoshared ),
																															arrayIndexExpression
																														);
	
	
	SgAssignOp * initAutoshared = buildAssignOp ( autosharedSubscriptExpression, 
																								buildVarRefExp ( formalParameter_inputValue )
																							);
	
	appendStatement ( buildExprStatement ( initAutoshared ), subroutineScope );

	SgExpression * mainLoopCondition = buildGreaterThanOp ( buildVarRefExp ( localVariables_IterationCounter ),
																												  buildIntVal ( 0 )
																											  );
	
	SgExpression * ifGuardExpression = buildGreaterThanOp (	buildVarRefExp ( localVariables_IterationCounter ),
																												  buildIntVal ( 0 )
																												);
	
	
	// same expression as above 
	SgPntrArrRefExp * autosharedMyThreadPosition = autosharedSubscriptExpression;
	
	// same expression as above plus the iteration counter
	SgExpression * autosharedOtherThreadPositionExpr = 
		buildAddOp ( arrayIndexExpression,
								 buildVarRefExp ( localVariables_IterationCounter )
							 );
	
	SgPntrArrRefExp * autosharedOtherThreadPosition =
		buildPntrArrRefExp ( buildVarRefExp ( localVariables_autoshared ),
											  autosharedOtherThreadPositionExpr
											);
	
	SgExpression * reductionPartialComputation = buildAssignOp ( autosharedMyThreadPosition, 
																															 buildAddOp ( autosharedMyThreadPosition,
																																						autosharedOtherThreadPosition
																																					)
																														 );
	
	SgBasicBlock * ifBlock = buildBasicBlock ( buildExprStatement ( reductionPartialComputation ) );
	
	SgIfStmt * ifStatement =
		FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse ( ifGuardExpression, 
																																					  ifBlock
																																					);
	
	
	SgExprListExp * actualParametersItCountReassign = buildExprListExp ( buildVarRefExp ( localVariables_IterationCounter ),
																																			 buildIntVal ( -1 )
																																		 );
	
  SgFunctionCallExp * shiftFunctionCallInsideMailLoop = buildFunctionCallExp ( shiftFunctionSymbol, 
																																							 actualParametersItCountReassign
																																						 );
	
  SgExpression * reassignIterationCounter = buildAssignOp ( buildVarRefExp ( localVariables_IterationCounter ),
																													  shiftFunctionCallInsideMailLoop
																											    );
	

	SgStatement * synchthreadsFunctionCallInsideLoop = createCallToSynchThreads ();
	
	//buildFunctionCallStmt ( synchthreadsFunctionSymbol->get_name(),
//																																								buildVoidType(),
//																																								emptyParameters,
//																																								subroutineScope	 
//																																								);	
//	
	
	SgStatement * mainLoopBody = buildBasicBlock ( synchthreadsFunctionCallInsideLoop,
																								 ifStatement,
																								 buildExprStatement ( reassignIterationCounter )
																							 );
	
	SgWhileStmt * reductionImplementation = buildWhileStmt ( buildExprStatement ( mainLoopCondition ), mainLoopBody );
	
	appendStatement ( reductionImplementation, subroutineScope );
	

	/*
	 * ======================================================
	 * Finalisation: synchronisation plus final sum
	 * ======================================================
	 */
	
	SgStatement * synchthreadsFunctionCallFinalisation = createCallToSynchThreads ();

	appendStatement ( synchthreadsFunctionCallFinalisation, subroutineScope );

	SgExpression * ifThreadIdGreaterZero = buildEqualityOp ( buildVarRefExp ( localVariables_threadID ),
																													 buildIntVal ( 0 )
																												 );
	
	
	SgPntrArrRefExp * deviceVariableUniquePosition = buildPntrArrRefExp ( buildVarRefExp ( formalParameter_reductionResultDevice ),
																																			 buildIntVal ( 1 )
																																		 );

	
	
		
	SgPntrArrRefExp * autosharedFinalReductionResultPosition = buildPntrArrRefExp ( buildVarRefExp ( localVariables_autoshared ),
																																								 buildVarRefExp ( formalParameter_sharedMemStartOffset )
																																							 );
	
	
	SgExpression * reductionFinalComputation = buildAssignOp ( deviceVariableUniquePosition, 
																														 buildAddOp ( deviceVariableUniquePosition,
																																					autosharedFinalReductionResultPosition
																																					)
																													 );
	
	
	SgBasicBlock * finalIfBlock = buildBasicBlock ( buildExprStatement ( reductionFinalComputation ) );
	
	SgIfStmt * finalIfStatement =
		FortranStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse ( ifThreadIdGreaterZero, 
																																					  finalIfBlock
																																				  );
	
	appendStatement ( finalIfStatement, subroutineScope );
	
}


/*
 * ======================================================
 * Public functions
 * ======================================================
 */

/*
 *\warning: we assume that the subroutineAndVariableName include both the user subroutine name
 * plus the variable name to which reduction is referred. We suppose that there is only one
 * possible instance of op_gbl on which reduction is performed passed to an op_par_loop
 */ 

ReductionSubroutine::ReductionSubroutine ( std::string const & subroutineAndVariableName,
																					SgScopeStatement * moduleScope,
																					SgArrayType * _reductionVariableType
																					):
Subroutine ( subroutineAndVariableName ), 
reductionVariableType ( _reductionVariableType ) 
{
	using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
	
	
	formalParameters = buildFunctionParameterList ();
	
  subroutineHeaderStatement = 
		buildProcedureHeaderStatement ( this->subroutineName.c_str (), 
																	  buildVoidType (),
																	  formalParameters,
																		SgProcedureHeaderStatement::e_subroutine_subprogram_kind,
																	  moduleScope
																	);
	
  addTextForUnparser ( subroutineHeaderStatement, "attributes(device) ",
											 AstUnparseAttribute::e_before);
	
  appendStatement ( subroutineHeaderStatement, moduleScope );
	
  subroutineScope = subroutineHeaderStatement->get_definition ()->get_body ();
	
  createFormalParameters ( );
	
  createLocalVariables ();
	
  createStatements ( );
	
}



std::map < unsigned int, SgProcedureHeaderStatement *>
ReductionSubroutine::generateReductionSubroutines (
	ParallelLoop & parallelLoop, SgScopeStatement * scopeStatement )
{
	using std::string;
	using boost::lexical_cast;
	
	std::map < unsigned int, SgProcedureHeaderStatement *> reductionSubroutines;

	
	if ( parallelLoop.isReductionRequired () == true )
	{
		for (unsigned int i = 1; i
				 <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
		{
			if ( parallelLoop.isReductionRequiredForSpecificArgument ( i ) == true )
			{
				
				/*
				 * ======================================================
				 * Generates the reduction subroutine name:
				 * <userKernelName> + "_reduction" + "_type"
				 * ======================================================
				 */
								
				SgType * opDatType = parallelLoop.get_OP_DAT_Type ( i );

				SgArrayType * isArrayType = isSgArrayType ( opDatType );	

				ROSE_ASSERT ( isArrayType != NULL );

				SgExpression * opDatKindSize = Subroutine::getFortranKindOfOpDat (
				  isArrayType );

				SgIntVal * isKindIntVal = isSgIntVal ( opDatKindSize );

				ROSE_ASSERT ( isKindIntVal != NULL );

				string typeName;

				if ( isSgTypeInt ( isArrayType->get_base_type() ) != NULL )
					typeName = reductionSubroutineNames::reductionSubroutineTypeInteger;
				else if ( isSgTypeFloat ( isArrayType->get_base_type() ) != NULL ) 
					typeName = reductionSubroutineNames::reductionSubroutineTypeReal;
				else
				Debug::getInstance ()->errorMessage ( 
				 "Error: type for reduction variable is not supported" );


				/*
				 * ======================================================
				 * For now we distinguish between subroutines by
				 * appending also the index of the related op_dat
				 * argument. Eventually, the factorisation will solve
				 * this problem
				 * ======================================================
				 */
				string const & ReductionSubroutineName = 
				kernelDatArgumentsNames::argNamePrefix + 
				lexical_cast < string > ( i ) +
				reductionSubroutineNames::reductionSubroutinePostfix +
				typeName +
				lexical_cast < string > ( isKindIntVal->get_value () );

				ReductionSubroutine * reductSubroutine =
					new ReductionSubroutine ( ReductionSubroutineName,
																		scopeStatement,
																		isArrayType
																	);

				/*
				 * ======================================================
				 * generate one per reduction variable, eventually
				 * we will have to factorise
				 * ======================================================
				 */
				
				reductionSubroutines[i] = reductSubroutine->getSubroutineHeaderStatement ();
			}
		}
	}	
	
	return reductionSubroutines;
}


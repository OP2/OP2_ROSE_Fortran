#include <boost/lexical_cast.hpp>
#include <KernelSubroutineOfDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <ROSEHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

//SgStatement *
//KernelSubroutine::createUserSubroutineCall (
//    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & parallelLoop)
//{
//  using SageBuilder::buildFunctionCallStmt;
//  using SageBuilder::buildVoidType;
//  using SageBuilder::buildIntVal;
//  using SageBuilder::buildMultiplyOp;
//  using SageBuilder::buildAddOp;
//  using SageBuilder::buildSubtractOp;
//  using SageBuilder::buildOpaqueVarRefExp;
//  using SageBuilder::buildDotExp;
//  using SageBuilder::buildVarRefExp;
//  using SageBuilder::buildPntrArrRefExp;
//  using SageBuilder::buildExprListExp;
//  using boost::lexical_cast;
//  using std::string;
//  using std::vector;
//
//  Debug::getInstance ()->debugMessage (
//      "Creating call to user device subroutine", 2);
//
//  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();
//
//  for (unsigned int i = 1; i
//      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
//  {
//    int dim = parallelLoop.get_OP_DAT_Dimension (i);
//
//    SgExpression * parameterExpression = buildIntVal (1);
//
//    if (parallelLoop.get_OP_MAP_Value (i) == GLOBAL)
//    {
//      if (parallelLoop.get_OP_Access_Value (i) == READ_ACCESS)
//      {
//
//        /*
//         * ======================================================
//         * Case of global variable accessed in read mode:
//         * we directly access the device variable, by
//         * passing the kernel the variable name in positions
//         * 0:argSize%<devVarName>-1. The name of the proper field
//         * is obtained by appending "argument", <i>, and "_Size"
//         * ======================================================
//         */
//
//        string const & variableName = kernelDatArgumentsNames::argNamePrefix
//            + lexical_cast <string> (i);
//
//        string const & argSizeName = variableName
//            + kernelDatArgumentsNames::argNameSizePostfix;
//
//        SgExpression * argSizeField = buildDotExp (buildVarRefExp (
//            formalParameter_argsSizes), buildOpaqueVarRefExp (argSizeName,
//            subroutineScope));
//
//        SgExpression * minusOneExpression = buildSubtractOp (argSizeField,
//            buildIntVal (1));
//
//        SgSubscriptExpression * arraySubscriptExpression =
//            new SgSubscriptExpression (ROSEHelper::getFileInfo (), buildIntVal (
//                0), minusOneExpression, buildIntVal (1));
//
//        arraySubscriptExpression->set_endOfConstruct (
//            ROSEHelper::getFileInfo ());
//        arraySubscriptExpression->setCompilerGenerated ();
//        arraySubscriptExpression->setOutputInCodeGeneration ();
//
//        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
//            formalParameter_OP_DATs[i]), arraySubscriptExpression);
//      }
//      else
//      {
//        /*
//         * ======================================================
//         * Case of global variable accessed *not* in read mode:
//         * we access the corresponding local thread variable
//         * ======================================================
//         */
//
//        parameterExpression = buildVarRefExp (
//            localVariables_localThreadVariables[i]);
//      }
//    }
//    else if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT
//        && parallelLoop.get_OP_Access_Value (i) == INC_ACCESS)
//    {
//      parameterExpression = buildVarRefExp (
//          localVariables_localThreadVariables[i]);
//    }
//    else if (parallelLoop.get_OP_MAP_Value (i) == INDIRECT)
//    {
//      Debug::getInstance ()->errorMessage (
//          "Error: the compiler does not support indirect datasets in direct loops accessed in any way different from OP_INC");
//    }
//    else if (parallelLoop.get_OP_MAP_Value (i) == DIRECT)
//    {
//      if (parallelLoop.getNumberOfIndirectDataSets () > 0)
//      {
//
//        SgExpression * deviceVarAccessDirect = buildMultiplyOp (buildAddOp (
//            buildVarRefExp (variable_setElementCounter), buildVarRefExp (
//                variable_offsetInThreadBlock)), buildIntVal (dim));
//
//        SgSubscriptExpression * arraySubscriptExpression =
//            new SgSubscriptExpression (ROSEHelper::getFileInfo (), buildIntVal (
//                0), deviceVarAccessDirect, buildIntVal (1));
//
//        arraySubscriptExpression->set_endOfConstruct (
//            ROSEHelper::getFileInfo ());
//        arraySubscriptExpression->setCompilerGenerated ();
//        arraySubscriptExpression->setOutputInCodeGeneration ();
//
//        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
//            formalParameter_OP_DATs[i]), arraySubscriptExpression);
//
//      }
//      else if (dim == 1)
//      {
//
//        SgExpression * nVarRef = buildVarRefExp (variable_setElementCounter);
//        SgExpression * nPlusDimMinusOneExpr = buildAddOp (nVarRef, buildIntVal (
//            dim - 1));
//
//        SgSubscriptExpression * arraySubscriptExpression =
//            new SgSubscriptExpression (ROSEHelper::getFileInfo (), nVarRef,
//                nPlusDimMinusOneExpr, buildIntVal (1));
//
//        arraySubscriptExpression->set_endOfConstruct (
//            ROSEHelper::getFileInfo ());
//        arraySubscriptExpression->setCompilerGenerated ();
//        arraySubscriptExpression->setOutputInCodeGeneration ();
//
//        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
//            formalParameter_OP_DATs[i]), arraySubscriptExpression);
//
//      }
//      else
//      {
//        parameterExpression = buildVarRefExp (
//            localVariables_localThreadVariables[i]);
//      }
//    }
//
//    /*
//     * ======================================================
//     * Before appending the parameter we must be sure that
//     * it has been created
//     * ======================================================
//     */
//    //		ROSE_ASSERT ( parameterExpression != NULL );
//
//    userDeviceSubroutineParameters->append_expression (parameterExpression);
//  }
//
//  return buildFunctionCallStmt (userDeviceSubroutine.getSubroutineName (),
//      buildVoidType (), userDeviceSubroutineParameters, subroutineScope);
//}

SgBasicBlock *
KernelSubroutineOfDirectLoop::stageInFromDeviceMemoryToLocalThreadVariables (
    ParallelLoop & parallelLoop, SgScopeStatement * scopeStatement)
{
  using SageBuilder::buildBasicBlock_nfi;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;

  /*
   * ======================================================
   * We essentially do a similar thing to the one we make
   * when we declare local thread variables
   * ======================================================
   */

  std::vector <SgStatement *> loopStatements;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    int dim = parallelLoop.get_OP_DAT_Dimension (i);

    if (parallelLoop.get_OP_MAP_Value (i) != GLOBAL
        && parallelLoop.get_OP_Access_Value (i) != WRITE_ACCESS && dim != 1)
    {

      SgVarRefExp * displVarRef = buildVarRefExp (
          variable_displacementInAutoshared);
      SgVarRefExp * tidVarRef = buildVarRefExp (variable_tIdModWarpSize);
      SgVarRefExp * mVarRef = buildVarRefExp (variable_dataPerElementCounter);
      SgVarRefExp * nelemsVarRef = buildVarRefExp (
          variable_numberOfThreadInWarpOrRemainingElems);
      SgVarRefExp * offsetVarRef =
          buildVarRefExp (variable_offsetInThreadBlock);
      SgVarRefExp * autoSharedVarRef = buildVarRefExp (
          localVariables_autoshared);
      ROSE_ASSERT ( displVarRef != NULL );
      ROSE_ASSERT ( tidVarRef != NULL );
      ROSE_ASSERT ( mVarRef != NULL );
      ROSE_ASSERT ( nelemsVarRef != NULL );
      ROSE_ASSERT ( offsetVarRef != NULL );

      /*
       * ======================================================
       * Builds stage in from device memory to shared memory
       * ======================================================
       */

      SgExpression * initLoop = buildAssignOp (mVarRef, buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (dim - 1);

      SgExpression * autosharedAccessFirst = buildAddOp (displVarRef,
          buildAddOp (tidVarRef, buildMultiplyOp (mVarRef, nelemsVarRef)));

      SgExpression * opdatArgAccess = buildAddOp (tidVarRef, buildAddOp (
          buildMultiplyOp (mVarRef, nelemsVarRef), buildMultiplyOp (
              offsetVarRef, buildIntVal (dim))));

      SgExpression * assignAutosharedInit = buildAssignOp (buildPntrArrRefExp (
          autoSharedVarRef, autosharedAccessFirst), buildPntrArrRefExp (
          buildVarRefExp (formalParameter_OP_DATs[i]), opdatArgAccess));

      SgBasicBlock * firstLoopBody = buildBasicBlock (buildExprStatement (
          assignAutosharedInit));

      SgFortranDo * firstLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initLoop, upperBoundExpression, buildIntVal (1), firstLoopBody);

      ROSE_ASSERT ( firstLoopBody->get_parent() != NULL );

      loopStatements.push_back (firstLoopStatement);

      /*
       * ======================================================
       * Builds stage in from shared memory to local thread
       * variables
       * ======================================================
       */

      SgExpression * autoSharedAccessSecond = buildAddOp (displVarRef,
          buildAddOp (mVarRef, buildMultiplyOp (tidVarRef, buildIntVal (dim))));

      SgExpression * assignLocalThreadVarInit = buildAssignOp (
          buildPntrArrRefExp (buildVarRefExp (
              localVariables_localThreadVariables[i]), mVarRef),
          buildPntrArrRefExp (autoSharedVarRef, autoSharedAccessSecond));

      SgBasicBlock * secondLoopBody = buildBasicBlock (buildExprStatement (
          assignLocalThreadVarInit));

      SgFortranDo * secondLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initLoop, upperBoundExpression, buildIntVal (1), secondLoopBody);

      loopStatements.push_back (secondLoopStatement);

    }
  }

  /*
   * ======================================================
   * We cannot rely on the buildBasicBlock_nfi, so
   * we have to add stuff manually in a recursive way
   * over the assignBlock variable
   * ======================================================
   */
  SgBasicBlock * assignBlock = buildBasicBlock ();
  std::vector <SgStatement *>::iterator it;
  for (it = loopStatements.begin (); it != loopStatements.end (); it++)
    assignBlock = buildBasicBlock (assignBlock, (*it));

  return assignBlock;

}

SgBasicBlock *
KernelSubroutineOfDirectLoop::stageOutFromLocalThreadVariablesToDeviceMemory (
    ParallelLoop & parallelLoop, SgScopeStatement * scopeStatement)
{

  using SageBuilder::buildBasicBlock_nfi;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;

  /*
   * ======================================================
   * We essentially do a similar thing to the one we make
   * when we declare local thread variables
   * ======================================================
   */

  std::vector <SgStatement *> loopStatements;

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
		int dim = parallelLoop.get_OP_DAT_Dimension ( i );
		
		if ( parallelLoop.get_OP_MAP_Value ( i ) != GLOBAL &&
				parallelLoop.get_OP_Access_Value ( i ) != WRITE_ACCESS &&
				dim != 1)
		{
			
			SgVarRefExp * displVarRef = buildVarRefExp ( variable_displacementInAutoshared );
			SgVarRefExp * tidVarRef = buildVarRefExp ( variable_tIdModWarpSize );
			SgVarRefExp * mVarRef = buildVarRefExp ( variable_dataPerElementCounter );
			SgVarRefExp * nelemsVarRef = buildVarRefExp ( variable_numberOfThreadInWarpOrRemainingElems );
			SgVarRefExp * offsetVarRef = buildVarRefExp ( variable_offsetInThreadBlock );
			SgVarRefExp * autoSharedVarRef = buildVarRefExp ( localVariables_autoshared );
			ROSE_ASSERT ( displVarRef != NULL );
			ROSE_ASSERT ( tidVarRef != NULL );
			ROSE_ASSERT ( mVarRef != NULL );
			ROSE_ASSERT ( nelemsVarRef != NULL );
			ROSE_ASSERT ( offsetVarRef != NULL );
			
			
			SgExpression * initLoop = buildAssignOp ( mVarRef, buildIntVal ( 0 ) );
			
			SgExpression * upperBoundExpression = buildIntVal ( dim - 1 );
			
			/*
			 * ======================================================
			 * Builds stage out from local thread variables to
			 * shared memory
			 * ======================================================
			 */
			
			SgExpression * autoSharedAccessFirst = buildAddOp ( 
				displVarRef, 
				buildAddOp( mVarRef, 
				buildMultiplyOp ( tidVarRef , buildIntVal ( dim ) ) ) );
			
			SgExpression * assignSharedMemOut = buildAssignOp (
				buildPntrArrRefExp ( autoSharedVarRef,
				  autoSharedAccessFirst ),
				buildPntrArrRefExp ( buildVarRefExp ( localVariables_localThreadVariables[i] ),
				  mVarRef ) );
			
			SgBasicBlock * firstLoopBody = buildBasicBlock (
				buildExprStatement ( assignSharedMemOut ) );
			
			SgFortranDo * firstLoopStatement = 
			FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
				initLoop, upperBoundExpression, buildIntVal ( 1 ),
				firstLoopBody );
			
			loopStatements.push_back ( firstLoopStatement );
			
			/*
			 * ======================================================
			 * Builds stage out from shared memory to device
			 * variables
			 * ======================================================
			 */

			SgExpression * deviceVarAccessSecond = buildAddOp ( tidVarRef, 
			  buildAddOp ( buildMultiplyOp ( mVarRef , nelemsVarRef ), 
					buildMultiplyOp ( offsetVarRef , buildIntVal ( dim ) ) ) );
			
			SgExpression * autosharedAccessSecond = buildAddOp ( displVarRef,
			  buildAddOp ( tidVarRef , buildMultiplyOp ( mVarRef, nelemsVarRef ) ) );
			
			SgExpression * assignDeviceVar = buildAssignOp (
				buildPntrArrRefExp ( buildVarRefExp ( formalParameter_OP_DATs[i] ),
				  deviceVarAccessSecond ), 
				buildPntrArrRefExp ( autoSharedVarRef, 
				  autosharedAccessSecond ) );
			
			SgBasicBlock * secondLoopBody = buildBasicBlock (
				buildExprStatement ( assignDeviceVar ) );
			
			SgFortranDo * secondLoopStatement = 
			FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
				initLoop, upperBoundExpression, buildIntVal ( 1 ),
				secondLoopBody );
			
			ROSE_ASSERT ( secondLoopBody->get_parent() != NULL );
			
			loopStatements.push_back ( secondLoopStatement );
						
		}
	}	
	
  /*
   * ======================================================
   * We cannot rely on the buildBasicBlock_nfi, so
   * we have to add stuff manually in a recursive way
   * over the assignBlock variable
   * ======================================================
   */
  SgBasicBlock * assignBlock = buildBasicBlock ();
  std::vector <SgStatement *>::iterator it;
  for (it = loopStatements.begin (); it != loopStatements.end (); it++)
    assignBlock = buildBasicBlock (assignBlock, (*it));

  return assignBlock;
}

SgBasicBlock *
KernelSubroutineOfDirectLoop::buildMainLoopStatements (
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & parallelLoop,
    SgScopeStatement * scopeStatement)
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;

  /*
   * ======================================================
   * Update offset variable value
   * ======================================================
   */
  SgExpression * initOffsetVariable = buildAssignOp (buildVarRefExp (
      variable_offsetInThreadBlock), buildSubtractOp (buildVarRefExp (
      variable_setElementCounter), buildVarRefExp (variable_tIdModWarpSize)));

  /*
   * ======================================================
   * Update nelems variable value
   * ======================================================
   */
  //nelems = min ( warpSizeOP2, (setSize - offset) )

  SgFunctionSymbol * minFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("min", subroutineScope);

  SgExpression * setSizeMinusOffset = buildSubtractOp (buildVarRefExp (
      formalParameter_setSize), buildVarRefExp (variable_offsetInThreadBlock));

  SgExprListExp * minActualParameters = buildExprListExp (buildVarRefExp (
      formalParameter_warpSizeOP2), setSizeMinusOffset);

  SgFunctionCallExp * minFunctionCall = buildFunctionCallExp (
      minFunctionSymbol, minActualParameters);

  SgAssignOp * assignNelems = buildAssignOp (buildVarRefExp (
      variable_numberOfThreadInWarpOrRemainingElems), minFunctionCall);

  SgBasicBlock * preAssignments =
      stageInFromDeviceMemoryToLocalThreadVariables (parallelLoop,
          scopeStatement);

  SgStatement * userFunctionCall = createUserSubroutineCall (
      userDeviceSubroutine, variable_setElementCounter,
			variable_offsetInThreadBlock,
			parallelLoop);

  SgBasicBlock * postAssignments =
      stageOutFromLocalThreadVariablesToDeviceMemory (parallelLoop,
          scopeStatement);

  SgBasicBlock * mainLoopStmt = buildBasicBlock (buildExprStatement (
      initOffsetVariable), buildExprStatement (assignNelems), preAssignments,
      userFunctionCall, postAssignments);

  ROSE_ASSERT ( preAssignments->get_parent() != NULL );

  return mainLoopStmt;
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
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildDivideOp;
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
   * Initialise the tid variable
   * ======================================================
   */

  SgExpression * threadidxDotX = buildDotExp (variable_Threadidx, variable_X);

  SgExpression * threadidxMinusOne = buildSubtractOp (threadidxMinusOne,
      buildIntVal (1));

  SgFunctionSymbol * modFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("mod", subroutineScope);

  SgExprListExp * modActualParameters = buildExprListExp (threadidxMinusOne,
      buildVarRefExp (formalParameter_warpSizeOP2));

  SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
      modFunctionSymbol, modActualParameters);

  SgAssignOp * assignTid = buildAssignOp (buildVarRefExp (
      variable_tIdModWarpSize), modFunctionCall);

  appendStatement (buildExprStatement (assignTid), subroutineScope);

  /*
   * ======================================================
   * Initialise the argSDisplacement variable
   * ======================================================
   */

  SgExpression * threadidxMinusOneDivWarpSize = buildDivideOp (threadidxMinusOne,
      buildVarRefExp (formalParameter_warpSizeOP2));

  SgExpression * argSDisplacementInitExprWithoutSize = buildMultiplyOp (
      buildVarRefExp (formalParameter_offsetS), threadidxMinusOneDivWarpSize);

  SgExpression * divisionExprForArgSDispl;

  if (compilerExpr_opDatKindSize != NULL)
    divisionExprForArgSDispl = buildDivideOp (
        argSDisplacementInitExprWithoutSize, compilerExpr_opDatKindSize);
  else
    divisionExprForArgSDispl = buildDivideOp (
        argSDisplacementInitExprWithoutSize, buildIntVal (4));

  SgAssignOp * assignArgSDispl = buildAssignOp (buildVarRefExp (
      variable_displacementInAutoshared), divisionExprForArgSDispl);

  appendStatement (buildExprStatement (assignArgSDispl), subroutineScope);

  /*
   * ======================================================
   * Initialise local thread variables (if needed)
   * ======================================================
   */

  initialiseLocalThreadVariables (parallelLoop, subroutineScope,
      buildVarRefExp (variable_setElementCounter));

  /*
   * ======================================================
   * Initialise the set iteration counter
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (buildDotExp (
      variable_Blockidx, variable_X), buildIntVal (1));

  SgSubtractOp * subtractExpression2 = buildSubtractOp (buildDotExp (
      variable_Threadidx, variable_X), buildIntVal (1));

  SgExpression * blockDimX = buildDotExp (variable_Blockdim, variable_X);

  SgMultiplyOp * multiplyExpression = buildMultiplyOp (subtractExpression1,
      blockDimX);

  SgVarRefExp * iterationCounterReference = buildVarRefExp (
      variable_setElementCounter );

//  SgExpression * mainLoopInitExpr = buildAssignOp (
//      iterationCounterReference, buildAddOp (subtractExpression2,
//          multiplyExpression));
//
//	/*
//	 * ======================================================
//	 * Statement to increment set iteration counter
//	 * ======================================================
//	 */
//	
//	SgExpression * gridDimXExpression = buildDotExp (variable_GridDim, variable_X);
//	
//	SgMultiplyOp * incrAssignmentExpression = buildMultiplyOp (blockDimX,
//		gridDimXExpression);
//
//	/*
//	 * ======================================================
//	 * Main loop upper bound expression
//	 * ======================================================
//	 */
//	
//	SgExpression * upperBoundExpression = buildSubtractOp (
//		buildVarRefExp ( formalParameter_setSize ), buildIntVal ( 1 ) );
//
//	/*
//	 * ======================================================
//	 * Build main loop statements
//	 * ======================================================
//	 */
//  SgBasicBlock * loopBody = buildMainLoopStatements ( userDeviceSubroutine, 
//	  parallelLoop, subroutineScope );
//
//
//	/*
//	 * ======================================================
//	 * Build main loop statements
//	 * ======================================================
//	 */
//	
//	SgFortranDo * fortranDoStatement =
//	FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
//		mainLoopInitExpr, upperBoundExpression, incrAssignmentExpression,
//		loopBody );
//	
//	appendStatement ( fortranDoStatement, subroutineScope );
//
//}
//      variable_setElementCounter );

  SgExpression * mainLoopInitExpr = buildAssignOp (iterationCounterReference,
      buildAddOp (subtractExpression2, multiplyExpression));

  /*
   * ======================================================
   * Statement to increment set iteration counter
   * ======================================================
   */

  SgExpression * gridDimXExpression =
      buildDotExp (variable_GridDim, variable_X);

  SgMultiplyOp * incrAssignmentExpression = buildMultiplyOp (blockDimX,
      gridDimXExpression);

  /*
   * ======================================================
   * Main loop upper bound expression
   * ======================================================
   */

  SgExpression * upperBoundExpression = buildSubtractOp (buildVarRefExp (
      formalParameter_setSize), buildIntVal (1));

  /*
   * ======================================================
   * Build main loop statements
   * ======================================================
   */
  SgBasicBlock * loopBody = buildMainLoopStatements (userDeviceSubroutine,
      parallelLoop, subroutineScope);

  /*
   * ======================================================
   * Build main loop statements
   * ======================================================
   */

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          mainLoopInitExpr, upperBoundExpression, incrAssignmentExpression,
          loopBody);

  appendStatement (fortranDoStatement, subroutineScope);

  /*
   * ======================================================
   * Add final support for reduction variables, if needed
   * ======================================================
   */
	createAndAppendReductionSubroutineCall ( parallelLoop, 
	 buildVarRefExp ( variable_setElementCounter ), subroutineScope );
}

void
KernelSubroutineOfDirectLoop::createLocalVariables (ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;

  /*
   * ======================================================
   * Build n local variable to count set elements
   * ======================================================
   */

  variable_setElementCounter = buildVariableDeclaration ("n",
      FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

  variable_setElementCounter->get_declarationModifier ().get_typeModifier ().setDevice ();
  variable_setElementCounter->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_setElementCounter, subroutineScope);

  /*
   * ======================================================
   * Build m local variable to data items per set element
   * ======================================================
   */

  variable_dataPerElementCounter = buildVariableDeclaration ("m",
      FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

  variable_dataPerElementCounter->get_declarationModifier ().get_typeModifier ().setDevice ();
  variable_dataPerElementCounter->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_dataPerElementCounter, subroutineScope);

  /*
   * ======================================================
   * Build tid storing threadid%x value moduled by
   * warpSizeOP2 variable
   * ======================================================
   */

  variable_tIdModWarpSize = buildVariableDeclaration ("tid",
      FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

  variable_tIdModWarpSize->get_declarationModifier ().get_typeModifier ().setDevice ();
  variable_tIdModWarpSize->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_tIdModWarpSize, subroutineScope);

  /*
   * ======================================================
   * Build offset storing n minus tid
   * ======================================================
   */

  variable_offsetInThreadBlock = buildVariableDeclaration ("offset",
      FortranTypesBuilder::getFourByteInteger (), NULL, subroutineScope);

  variable_offsetInThreadBlock->get_declarationModifier ().get_typeModifier ().setDevice ();
  variable_offsetInThreadBlock->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_offsetInThreadBlock, subroutineScope);

  /*
   * ======================================================
   * Build nelems storing the min between the number of 
   * threads in a warp and the remaining set elements
   * to be evaluated
   * ======================================================
   */

  variable_numberOfThreadInWarpOrRemainingElems = buildVariableDeclaration (
      "nelems", FortranTypesBuilder::getFourByteInteger (), NULL,
      subroutineScope);

  variable_numberOfThreadInWarpOrRemainingElems->get_declarationModifier ().get_typeModifier ().setDevice ();
  variable_numberOfThreadInWarpOrRemainingElems->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_numberOfThreadInWarpOrRemainingElems,
      subroutineScope);

  variable_displacementInAutoshared = buildVariableDeclaration (
      "argSDisplacement", FortranTypesBuilder::getFourByteInteger (), NULL,
      subroutineScope);

  variable_displacementInAutoshared->get_declarationModifier ().get_typeModifier ().setDevice ();
  variable_displacementInAutoshared->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_displacementInAutoshared, subroutineScope);

  createLocalThreadVariables (parallelLoop, *subroutineScope, true);

  createAutosharedVariable (parallelLoop, subroutineScope);

}

void
KernelSubroutineOfDirectLoop::create_OP_DAT_FormalParameters (
    ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
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
       * is stored in the argSizes variable
       * ======================================================
       */

      string const & variableName = kernelDatArgumentsNames::argNamePrefix
          + lexical_cast <string> (i);

      string const & argSizeName = variableName
          + kernelDatArgumentsNames::argNameSizePostfix;

      SgExpression * argSizeField = buildDotExp (buildVarRefExp (
          formalParameter_argsSizes), buildOpaqueVarRefExp (argSizeName,
          subroutineScope));

      SgExpression * minusOneExpression = buildSubtractOp (argSizeField,
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

      SgVariableDeclaration * opDatFormalParameter = buildVariableDeclaration (
          variableName, arrayType, NULL, subroutineScope);

      formalParameters->append_arg (
          *(opDatFormalParameter->get_variables ().begin ()));

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

  Debug::getInstance ()->debugMessage ("Creating OP_SET size formal parameter",
      2);

  formalParameter_setSize = buildVariableDeclaration ("setSize",
      buildIntType (), NULL, subroutineScope);

  formalParameter_setSize->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_setSize->get_declarationModifier ().get_typeModifier ().setValue ();

}

void
KernelSubroutineOfDirectLoop::appendSetSizeFormalParameter ()
{
  using SageInterface::appendStatement;

  formalParameters->append_arg (
      *(formalParameter_setSize->get_variables ().begin ()));

  appendStatement (formalParameter_setSize, subroutineScope);

}

void
KernelSubroutineOfDirectLoop::createAndAppendOffsetSFormalParameter ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating offsetS formal parameter", 2);

  formalParameter_offsetS = buildVariableDeclaration ("offsetS",
      buildIntType (), NULL, subroutineScope);

  formalParameter_offsetS->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_offsetS->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameter_offsetS->get_variables ().begin ()));

  appendStatement (formalParameter_offsetS, subroutineScope);
}

void
KernelSubroutineOfDirectLoop::createAndAppendWarpSizeOP2FormalParameter ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating warpSizeOP2 formal parameter",
      2);

  formalParameter_warpSizeOP2 = buildVariableDeclaration ("warpSizeOP2",
      buildIntType (), NULL, subroutineScope);

  formalParameter_warpSizeOP2->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_warpSizeOP2->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameter_warpSizeOP2->get_variables ().begin ()));

  appendStatement (formalParameter_warpSizeOP2, subroutineScope);
}

void
KernelSubroutineOfDirectLoop::detectOPDatsBaseKindType (
    ParallelLoop & parallelLoop)
{
  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.isDuplicate_OP_DAT (i) == false)
    {

      SgType * opDatArrayType = parallelLoop.get_OP_DAT_Type (i);

      SgArrayType * isArrayType = isSgArrayType (opDatArrayType);

      ROSE_ASSERT ( isArrayType != NULL );

      SgType * opDatBaseType = isArrayType->get_base_type ();

      SgType * isRealType = isSgTypeFloat (opDatBaseType);

      /*
       * ======================================================
       * We copy the user type with kind (if exists) otherwise
       * we assume real(4) (CUDA Fortran specific)
       * ======================================================
       */

      if (isRealType != NULL)
        compilerExpr_opDatKindSize = opDatBaseType->get_type_kind ();
    }
  }
}

void
KernelSubroutineOfDirectLoop::createAndAppendFormalParameters (
  DeviceDataSizesDeclarationDirectLoops & DeviceDataSizesDeclarationDirectLoops,
	ParallelLoop & parallelLoop )
{
	createArgsSizesFormalParameter ( DeviceDataSizesDeclarationDirectLoops );

	createSetSizeFormalParameter ();

	create_OP_DAT_FormalParameters ( parallelLoop );

	createAndAppendOffsetSFormalParameter ();

	appendSetSizeFormalParameter ();

	createAndAppendWarpSizeOP2FormalParameter ();

	/*
	 * ======================================================
	 * Add offset parameter in shared memory for reductions
	 * ======================================================
	 */
	
	if ( parallelLoop.isReductionRequired () == true ) {
		createAndAppendSharedMemoryOffesetForReduction ( parallelLoop );
	}
}


/*
 * ======================================================
 * Public functions
 * ======================================================
 */

KernelSubroutineOfDirectLoop::KernelSubroutineOfDirectLoop (
    std::string const & subroutineName,
    UserDeviceSubroutine & userDeviceSubroutine,
    DeviceDataSizesDeclarationDirectLoops & DeviceDataSizesDeclarationDirectLoops,
		std::map < unsigned int, SgProcedureHeaderStatement *> & _reductSubroutines,
		ParallelLoop & parallelLoop,
    SgScopeStatement * moduleScope) :
  KernelSubroutine ( subroutineName, _reductSubroutines ), compilerExpr_opDatKindSize ( NULL )
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

	createAndAppendFormalParameters ( DeviceDataSizesDeclarationDirectLoops,
	   parallelLoop );

  detectOPDatsBaseKindType (parallelLoop);

  createLocalVariables (parallelLoop);

  createStatements (userDeviceSubroutine, parallelLoop);
}

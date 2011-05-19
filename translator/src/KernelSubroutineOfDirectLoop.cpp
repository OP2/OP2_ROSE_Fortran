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
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using boost::lexical_cast;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", 2);

  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    int dim = parallelLoop.get_OP_DAT_Dimension (i);

    SgExpression * parameterExpression = buildIntVal (1);

    if (parallelLoop.get_OP_MAP_Value (i) == GLOBAL)
    {
      if (parallelLoop.get_OP_Access_Value (i) == READ_ACCESS)
      {
        /*
         * ======================================================
         * Case of global variable accessed in read mode:
         * we directly access the device variable, by
         * passing the kernel the variable name in positions
         * 0:argSize%<devVarName>-1
         * ======================================================
         */

        SgExpression
            * argSizeField =
                buildDotExp (
                    buildVarRefExp (
                        formalParameterDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]),
                    buildOpaqueVarRefExp (get_OP_DAT_SizeVariableName (i),
                        subroutineScope));

        SgExpression * minusOneExpression = buildSubtractOp (argSizeField,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (), buildIntVal (
                0), minusOneExpression, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            formalParameterDeclarations[get_OP_DAT_VariableName (i)]),
            arraySubscriptExpression);
      }
      else
      {
        /*
         * ======================================================
         * Case of global variable accessed *not* in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */

        parameterExpression = buildVarRefExp (
            localVariableDeclarations[getLocalThread_OP_DAT_VariableName (i)]);
      }
    }
    else if (parallelLoop.get_OP_MAP_Value (i) == DIRECT)
    {
      if (parallelLoop.getNumberOfIndirectDataSets () > 0)
      {
        SgExpression
            * deviceVarAccessDirectBegin =
                buildMultiplyOp (
                    buildAddOp (
                        buildVarRefExp (
                            localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]),
                        buildVarRefExp (
                            localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock])),
                    buildIntVal (dim));

        SgExpression * deviceVarAccessDirectEnd = buildAddOp (
            deviceVarAccessDirectBegin, buildIntVal (dim));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (),
                deviceVarAccessDirectBegin, deviceVarAccessDirectEnd,
                buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            formalParameterDeclarations[get_OP_DAT_VariableName (i)]),
            arraySubscriptExpression);
      }
      else if (dim == 1)
      {
        SgExpression
            * nVarRef =
                buildVarRefExp (
                    localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]);
        SgExpression * nPlusDimMinusOneExpr = buildAddOp (nVarRef, buildIntVal (
            dim - 1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (ROSEHelper::getFileInfo (), nVarRef,
                nPlusDimMinusOneExpr, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            ROSEHelper::getFileInfo ());
        arraySubscriptExpression->setCompilerGenerated ();
        arraySubscriptExpression->setOutputInCodeGeneration ();

        parameterExpression = buildPntrArrRefExp (buildVarRefExp (
            formalParameterDeclarations[get_OP_DAT_VariableName (i)]),
            arraySubscriptExpression);
      }
      else
      {
        parameterExpression = buildVarRefExp (
            localVariableDeclarations[getLocalThread_OP_DAT_VariableName (i)]);
      }
    }

    userDeviceSubroutineParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userDeviceSubroutine.getSubroutineName (),
      buildVoidType (), userDeviceSubroutineParameters, subroutineScope);
}

SgBasicBlock *
KernelSubroutineOfDirectLoop::stageInFromDeviceMemoryToLocalThreadVariables (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;

  SgBasicBlock * outerBlock = buildBasicBlock ();

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) != GLOBAL
        && parallelLoop.get_OP_Access_Value (i) != WRITE_ACCESS
        && parallelLoop.get_OP_DAT_Dimension (i) != 1)
    {
      SgVarRefExp
          * displVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement]);

      SgVarRefExp
          * tidVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::threadIDModulus]);

      SgVarRefExp
          * mVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter]);

      SgVarRefExp
          * nelemsVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::remainingElements]);

      SgVarRefExp
          * offsetVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]);

      SgVarRefExp
          * autoSharedVarRef =
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

      /*
       * ======================================================
       * Builds stage in from device memory to shared memory
       * ======================================================
       */

      SgExpression * initLoop = buildAssignOp (mVarRef, buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (
          parallelLoop.get_OP_DAT_Dimension (i));

      SgExpression * autosharedAccessFirst = buildAddOp (displVarRef,
          buildAddOp (tidVarRef, buildMultiplyOp (mVarRef, nelemsVarRef)));

      SgExpression * opdatArgAccess = buildAddOp (tidVarRef, buildAddOp (
          buildMultiplyOp (mVarRef, nelemsVarRef),
          buildMultiplyOp (offsetVarRef, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)))));

      SgExpression * assignAutosharedInit = buildAssignOp (buildPntrArrRefExp (
          autoSharedVarRef, autosharedAccessFirst), buildPntrArrRefExp (
          buildVarRefExp (formalParameterDeclarations[get_OP_DAT_VariableName (
              i)]), opdatArgAccess));

      SgBasicBlock * firstLoopBody = buildBasicBlock (buildExprStatement (
          assignAutosharedInit));

      SgFortranDo * firstLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initLoop, upperBoundExpression, buildIntVal (1), firstLoopBody);

      outerBlock->append_statement (firstLoopStatement);

      /*
       * ======================================================
       * Builds stage in from shared memory to local thread
       * variables
       * ======================================================
       */

      SgExpression * autoSharedAccessSecond = buildAddOp (displVarRef,
          buildAddOp (mVarRef, buildMultiplyOp (tidVarRef, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)))));

      SgExpression * assignLocalThreadVarInit = buildAssignOp (
          buildPntrArrRefExp (
              buildVarRefExp (
                  localVariableDeclarations[getLocalThread_OP_DAT_VariableName (
                      i)]), mVarRef), buildPntrArrRefExp (autoSharedVarRef,
              autoSharedAccessSecond));

      SgBasicBlock * secondLoopBody = buildBasicBlock (buildExprStatement (
          assignLocalThreadVarInit));

      SgFortranDo * secondLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initLoop, upperBoundExpression, buildIntVal (1), secondLoopBody);

      outerBlock->append_statement (secondLoopStatement);
    }
  }

  return outerBlock;
}

SgBasicBlock *
KernelSubroutineOfDirectLoop::stageOutFromLocalThreadVariablesToDeviceMemory (
    ParallelLoop & parallelLoop)
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;

  SgBasicBlock * outerBlock = buildBasicBlock ();

  for (unsigned int i = 1; i
      <= parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop.get_OP_MAP_Value (i) != GLOBAL
        && parallelLoop.get_OP_Access_Value (i) != READ_ACCESS
        && parallelLoop.get_OP_DAT_Dimension (i) != 1)
    {
      SgVarRefExp
          * displVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement]);

      SgVarRefExp
          * tidVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::threadIDModulus]);

      SgVarRefExp
          * mVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter]);

      SgVarRefExp
          * nelemsVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::remainingElements]);

      SgVarRefExp
          * offsetVarRef =
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]);

      SgVarRefExp
          * autoSharedVarRef =
              buildVarRefExp (
                  localVariableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

      SgExpression * initLoop = buildAssignOp (mVarRef, buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (
          parallelLoop.get_OP_DAT_Dimension (i) - 1);

      /*
       * ======================================================
       * Builds stage out from local thread variables to
       * shared memory
       * ======================================================
       */

      SgExpression * autoSharedAccessFirst = buildAddOp (displVarRef,
          buildAddOp (mVarRef, buildMultiplyOp (tidVarRef, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)))));

      SgExpression * assignSharedMemOut = buildAssignOp (buildPntrArrRefExp (
          autoSharedVarRef, autoSharedAccessFirst),
          buildPntrArrRefExp (
              buildVarRefExp (
                  localVariableDeclarations[getLocalThread_OP_DAT_VariableName (
                      i)]), mVarRef));

      SgBasicBlock * firstLoopBody = buildBasicBlock (buildExprStatement (
          assignSharedMemOut));

      SgFortranDo * firstLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initLoop, upperBoundExpression, buildIntVal (1), firstLoopBody);

      outerBlock->append_statement (firstLoopStatement);

      /*
       * ======================================================
       * Builds stage out from shared memory to device
       * variables
       * ======================================================
       */

      SgExpression * deviceVarAccessSecond = buildAddOp (tidVarRef, buildAddOp (
          buildMultiplyOp (mVarRef, nelemsVarRef),
          buildMultiplyOp (offsetVarRef, buildIntVal (
              parallelLoop.get_OP_DAT_Dimension (i)))));

      SgExpression * autosharedAccessSecond = buildAddOp (displVarRef,
          buildAddOp (tidVarRef, buildMultiplyOp (mVarRef, nelemsVarRef)));

      SgExpression * assignDeviceVar = buildAssignOp (buildPntrArrRefExp (
          buildVarRefExp (formalParameterDeclarations[get_OP_DAT_VariableName (
              i)]), deviceVarAccessSecond), buildPntrArrRefExp (
          autoSharedVarRef, autosharedAccessSecond));

      SgBasicBlock * secondLoopBody = buildBasicBlock (buildExprStatement (
          assignDeviceVar));

      SgFortranDo * secondLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initLoop, upperBoundExpression, buildIntVal (1), secondLoopBody);

      outerBlock->append_statement (secondLoopStatement);
    }
  }

  return outerBlock;
}

SgBasicBlock *
KernelSubroutineOfDirectLoop::buildMainLoopStatements (
    UserDeviceSubroutine & userDeviceSubroutine, ParallelLoop & parallelLoop)
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
  SgExpression
      * initOffsetVariable =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]),
              buildSubtractOp (
                  buildVarRefExp (
                      localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]),
                  buildVarRefExp (
                      localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::threadIDModulus])));

  /*
   * ======================================================
   * Update nelems variable value
   * ======================================================
   */
  //nelems = min ( warpSizeOP2, (setSize - offset) )

  SgFunctionSymbol * minFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("min", subroutineScope);

  SgExpression
      * setSizeMinusOffset =
          buildSubtractOp (
              buildVarRefExp (
                  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::setSize]),
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]));

  SgExprListExp
      * minActualParameters =
          buildExprListExp (
              buildVarRefExp (
                  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]),
              setSizeMinusOffset);

  SgFunctionCallExp * minFunctionCall = buildFunctionCallExp (
      minFunctionSymbol, minActualParameters);

  SgAssignOp
      * assignNelems =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::remainingElements]),
              minFunctionCall);

  SgBasicBlock * preAssignments =
      stageInFromDeviceMemoryToLocalThreadVariables (parallelLoop);

  SgStatement * userFunctionCall = createUserSubroutineCall (
      userDeviceSubroutine, parallelLoop);

  SgBasicBlock * postAssignments =
      stageOutFromLocalThreadVariablesToDeviceMemory (parallelLoop);

  SgBasicBlock * mainLoopStmt = buildBasicBlock (buildExprStatement (
      initOffsetVariable), buildExprStatement (assignNelems), preAssignments,
      userFunctionCall, postAssignments);

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
      CUDA::Fortran::VariableNames::threadidx, subroutineScope);

  SgVarRefExp * variable_X = buildOpaqueVarRefExp (
      CUDA::Fortran::FieldNames::x, subroutineScope);

  SgVarRefExp * variable_Blockidx = buildOpaqueVarRefExp (
      CUDA::Fortran::VariableNames::blockidx, subroutineScope);

  SgVarRefExp * variable_Blockdim = buildOpaqueVarRefExp (
      CUDA::Fortran::VariableNames::blockdim, subroutineScope);

  SgVarRefExp * variable_GridDim = buildOpaqueVarRefExp (
      CUDA::Fortran::VariableNames::griddim, subroutineScope);

  /*
   * ======================================================
   * Initialise the tid variable
   * ======================================================
   */

  SgExpression * threadidxDotX = buildDotExp (variable_Threadidx, variable_X);

  SgExpression * threadidxMinusOne = buildSubtractOp (threadidxDotX,
      buildIntVal (1));

  SgFunctionSymbol * modFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("mod", subroutineScope);

  SgExprListExp
      * modActualParameters =
          buildExprListExp (
              threadidxMinusOne,
              buildVarRefExp (
                  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]));

  SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
      modFunctionSymbol, modActualParameters);

  SgAssignOp
      * assignTid =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::threadIDModulus]),
              modFunctionCall);

  appendStatement (buildExprStatement (assignTid), subroutineScope);

  /*
   * ======================================================
   * Initialise the argSDisplacement variable
   * ======================================================
   */

  SgExpression
      * threadidxMinusOneDivWarpSize =
          buildDivideOp (
              threadidxMinusOne,
              buildVarRefExp (
                  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]));

  SgExpression
      * argSDisplacementInitExprWithoutSize =
          buildMultiplyOp (
              buildVarRefExp (
                  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize]),
              threadidxMinusOneDivWarpSize);

  SgExpression * divisionExprForArgSDispl;

  if (parallelLoop.getSizeOf_OP_DAT () == 8)
  {
    divisionExprForArgSDispl = buildDivideOp (
        argSDisplacementInitExprWithoutSize, buildIntVal (8));
  }
  else
  {
    divisionExprForArgSDispl = buildDivideOp (
        argSDisplacementInitExprWithoutSize, buildIntVal (4));
  }

  SgAssignOp
      * assignArgSDispl =
          buildAssignOp (
              buildVarRefExp (
                  localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement]),
              divisionExprForArgSDispl);

  appendStatement (buildExprStatement (assignArgSDispl), subroutineScope);

  /*
   * ======================================================
   * Initialise local thread variables (if needed)
   * ======================================================
   */

  initialiseLocalThreadVariables (parallelLoop);

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

  SgVarRefExp
      * iterationCounterReference =
          buildVarRefExp (
              localVariableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]);

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

  SgExpression
      * upperBoundExpression =
          buildSubtractOp (
              buildVarRefExp (
                  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::setSize]),
              buildIntVal (1));

  /*
   * ======================================================
   * Build main loop statements
   * ======================================================
   */
  SgBasicBlock * loopBody = buildMainLoopStatements (userDeviceSubroutine,
      parallelLoop);

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
  createAndAppendReductionSubroutineCall (parallelLoop);
}

void
KernelSubroutineOfDirectLoop::createLocalVariables (ParallelLoop & parallelLoop)
{
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::vector;
  using std::string;

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (
      DirectLoop::Fortran::KernelSubroutine::setElementCounter);

  fourByteIntegers.push_back (
      DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter);

  fourByteIntegers.push_back (
      DirectLoop::Fortran::KernelSubroutine::threadIDModulus);

  fourByteIntegers.push_back (
      DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock);

  fourByteIntegers.push_back (
      DirectLoop::Fortran::KernelSubroutine::remainingElements);

  fourByteIntegers.push_back (
      DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    localVariableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, FortranTypesBuilder::getFourByteInteger (), subroutineScope);
  }
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

      string const & variableName =
          IndirectAndDirectLoop::Fortran::VariablePrefixes::OP_DAT
              + lexical_cast <string> (i);

      string const & variableSizeName = variableName
          + IndirectAndDirectLoop::Fortran::VariableSuffixes::Size;

      SgExpression
          * argSizeField =
              buildDotExp (
                  buildVarRefExp (
                      formalParameterDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]),
                  buildOpaqueVarRefExp (variableSizeName, subroutineScope));

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

      formalParameterDeclarations[get_OP_DAT_VariableName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              get_OP_DAT_VariableName (i), arrayType, subroutineScope);

      formalParameterDeclarations[get_OP_DAT_VariableName (i)]->get_declarationModifier ().get_typeModifier ().setDevice ();

      formalParameters->append_arg (
          *(formalParameterDeclarations[get_OP_DAT_VariableName (i)]->get_variables ().begin ()));
    }
  }
}

void
KernelSubroutineOfDirectLoop::createAndAppendFormalParameters (
    DeviceDataSizesDeclarationDirectLoops & DeviceDataSizesDeclarationDirectLoops,
    ParallelLoop & parallelLoop)
{
  createArgsSizesFormalParameter (DeviceDataSizesDeclarationDirectLoops);

  create_OP_DAT_FormalParameters (parallelLoop);

  /*
   * ======================================================
   * OffsetS formal parameter. This is the offset
   * in the shared memory variable assigned to each
   * thread block (see Mike's Developers Guide, direct
   * loops section)
   * ======================================================
   */

  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize]->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize]->get_variables ().begin ()));

  /*
   * ======================================================
   * Set size formal parameter
   * ======================================================
   */

  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::setSize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          DirectLoop::Fortran::KernelSubroutine::setSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::setSize]->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::setSize]->get_variables ().begin ()));

  /*
   * ======================================================
   * Warp size formal parameter
   * ======================================================
   */

  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          DirectLoop::Fortran::KernelSubroutine::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameterDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]->get_variables ().begin ()));

  if (parallelLoop.isReductionRequired () == true)
  {
    createAndAppendSharedMemoryOffesetForReduction ();
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
    std::map <unsigned int, SgProcedureHeaderStatement *> & _reductSubroutines,
    ParallelLoop & parallelLoop, SgScopeStatement * moduleScope) :
  KernelSubroutine (subroutineName, _reductSubroutines)
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

  createAndAppendFormalParameters (DeviceDataSizesDeclarationDirectLoops,
      parallelLoop);

  createLocalVariables (parallelLoop);

  createLocalThreadVariables (parallelLoop);

  createAutosharedVariable (parallelLoop);

  createStatements (userDeviceSubroutine, parallelLoop);
}

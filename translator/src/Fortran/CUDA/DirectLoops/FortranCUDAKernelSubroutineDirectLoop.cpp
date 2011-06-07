#include <FortranCUDAKernelSubroutineDirectLoop.h>
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
FortranCUDAKernelSubroutineDirectLoop::createUserSubroutineCall ()
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
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", 2);

  SgExprListExp * userDeviceSubroutineParameters = buildExprListExp ();

  for (unsigned int i = 1; i
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    int dim = parallelLoop->get_OP_DAT_Dimension (i);

    SgExpression * parameterExpression = buildIntVal (1);

    if (parallelLoop->get_OP_MAP_Value (i) == GLOBAL)
    {
      if (parallelLoop->get_OP_Access_Value (i) == READ_ACCESS)
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
                        variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]),
                    buildOpaqueVarRefExp (VariableNames::getOpDatSizeName (i),
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
            variableDeclarations[VariableNames::getOpDatName (i)]),
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
            variableDeclarations[VariableNames::getOpDatLocalName (i)]);
      }
    }
    else if (parallelLoop->get_OP_MAP_Value (i) == DIRECT)
    {
      if (parallelLoop->getNumberOfIndirectDataSets () > 0)
      {
        SgExpression
            * deviceVarAccessDirectBegin =
                buildMultiplyOp (
                    buildAddOp (
                        buildVarRefExp (
                            variableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]),
                        buildVarRefExp (
                            variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock])),
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
            variableDeclarations[VariableNames::getOpDatName (i)]),
            arraySubscriptExpression);
      }
      else if (dim == 1)
      {
        SgExpression
            * nVarRef =
                buildVarRefExp (
                    variableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]);
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
            variableDeclarations[VariableNames::getOpDatName (i)]),
            arraySubscriptExpression);
      }
      else
      {
        parameterExpression = buildVarRefExp (
            variableDeclarations[VariableNames::getOpDatLocalName (i)]);
      }
    }

    userDeviceSubroutineParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userSubroutineName, buildVoidType (),
      userDeviceSubroutineParameters, subroutineScope);
}

SgBasicBlock *
FortranCUDAKernelSubroutineDirectLoop::stageInFromDeviceMemoryToLocalThreadVariables ()
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
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) != GLOBAL
        && parallelLoop->get_OP_Access_Value (i) != WRITE_ACCESS
        && parallelLoop->get_OP_DAT_Dimension (i) != 1)
    {
      SgVarRefExp
          * displVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement]);

      SgVarRefExp
          * tidVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::threadIDModulus]);

      SgVarRefExp
          * mVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter]);

      SgVarRefExp
          * nelemsVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::remainingElements]);

      SgVarRefExp
          * offsetVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]);

      SgVarRefExp
          * autoSharedVarRef =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

      /*
       * ======================================================
       * Builds stage in from device memory to shared memory
       * ======================================================
       */

      SgExpression * initLoop = buildAssignOp (mVarRef, buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (
          parallelLoop->get_OP_DAT_Dimension (i));

      SgExpression * autosharedAccessFirst = buildAddOp (displVarRef,
          buildAddOp (tidVarRef, buildMultiplyOp (mVarRef, nelemsVarRef)));

      SgExpression * opdatArgAccess = buildAddOp (tidVarRef, buildAddOp (
          buildMultiplyOp (mVarRef, nelemsVarRef), buildMultiplyOp (
              offsetVarRef,
              buildIntVal (parallelLoop->get_OP_DAT_Dimension (i)))));

      SgExpression * assignAutosharedInit = buildAssignOp (buildPntrArrRefExp (
          autoSharedVarRef, autosharedAccessFirst),
          buildPntrArrRefExp (buildVarRefExp (
              variableDeclarations[VariableNames::getOpDatName (i)]),
              opdatArgAccess));

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
              parallelLoop->get_OP_DAT_Dimension (i)))));

      SgExpression * assignLocalThreadVarInit = buildAssignOp (
          buildPntrArrRefExp (buildVarRefExp (
              variableDeclarations[VariableNames::getOpDatLocalName (i)]),
              mVarRef), buildPntrArrRefExp (autoSharedVarRef,
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
FortranCUDAKernelSubroutineDirectLoop::stageOutFromLocalThreadVariablesToDeviceMemory ()
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
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->get_OP_MAP_Value (i) != GLOBAL
        && parallelLoop->get_OP_Access_Value (i) != READ_ACCESS
        && parallelLoop->get_OP_DAT_Dimension (i) != 1)
    {
      SgVarRefExp
          * displVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement]);

      SgVarRefExp
          * tidVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::threadIDModulus]);

      SgVarRefExp
          * mVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter]);

      SgVarRefExp
          * nelemsVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::remainingElements]);

      SgVarRefExp
          * offsetVarRef =
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]);

      SgVarRefExp
          * autoSharedVarRef =
              buildVarRefExp (
                  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::autoshared]);

      SgExpression * initLoop = buildAssignOp (mVarRef, buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (
          parallelLoop->get_OP_DAT_Dimension (i) - 1);

      /*
       * ======================================================
       * Builds stage out from local thread variables to
       * shared memory
       * ======================================================
       */

      SgExpression * autoSharedAccessFirst = buildAddOp (displVarRef,
          buildAddOp (mVarRef, buildMultiplyOp (tidVarRef, buildIntVal (
              parallelLoop->get_OP_DAT_Dimension (i)))));

      SgExpression * assignSharedMemOut = buildAssignOp (buildPntrArrRefExp (
          autoSharedVarRef, autoSharedAccessFirst), buildPntrArrRefExp (
          buildVarRefExp (
              variableDeclarations[VariableNames::getOpDatLocalName (i)]),
          mVarRef));

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
          buildMultiplyOp (mVarRef, nelemsVarRef), buildMultiplyOp (
              offsetVarRef,
              buildIntVal (parallelLoop->get_OP_DAT_Dimension (i)))));

      SgExpression * autosharedAccessSecond = buildAddOp (displVarRef,
          buildAddOp (tidVarRef, buildMultiplyOp (mVarRef, nelemsVarRef)));

      SgExpression * assignDeviceVar = buildAssignOp (
          buildPntrArrRefExp (buildVarRefExp (
              variableDeclarations[VariableNames::getOpDatName (i)]),
              deviceVarAccessSecond), buildPntrArrRefExp (autoSharedVarRef,
              autosharedAccessSecond));

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
FortranCUDAKernelSubroutineDirectLoop::buildMainLoopStatements ()
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
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]),
              buildSubtractOp (
                  buildVarRefExp (
                      variableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]),
                  buildVarRefExp (
                      variableDeclarations[DirectLoop::Fortran::KernelSubroutine::threadIDModulus])));

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
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::setSize]),
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock]));

  SgExprListExp * minActualParameters = buildExprListExp (buildVarRefExp (
      variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]),
      setSizeMinusOffset);

  SgFunctionCallExp * minFunctionCall = buildFunctionCallExp (
      minFunctionSymbol, minActualParameters);

  SgAssignOp
      * assignNelems =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::remainingElements]),
              minFunctionCall);

  SgBasicBlock * preAssignments =
      stageInFromDeviceMemoryToLocalThreadVariables ();

  SgStatement * userFunctionCall = createUserSubroutineCall ();

  SgBasicBlock * postAssignments =
      stageOutFromLocalThreadVariablesToDeviceMemory ();

  SgBasicBlock * mainLoopStmt = buildBasicBlock (buildExprStatement (
      initOffsetVariable), buildExprStatement (assignNelems), preAssignments,
      userFunctionCall, postAssignments);

  return mainLoopStmt;
}

void
FortranCUDAKernelSubroutineDirectLoop::createStatements ()
{
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
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]));

  SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
      modFunctionSymbol, modActualParameters);

  SgAssignOp
      * assignTid =
          buildAssignOp (
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::threadIDModulus]),
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
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]));

  SgExpression
      * argSDisplacementInitExprWithoutSize =
          buildMultiplyOp (
              buildVarRefExp (
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize]),
              threadidxMinusOneDivWarpSize);

  SgExpression * divisionExprForArgSDispl;

  if (parallelLoop->getSizeOf_OP_DAT () == 8)
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
                  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement]),
              divisionExprForArgSDispl);

  appendStatement (buildExprStatement (assignArgSDispl), subroutineScope);

  /*
   * ======================================================
   * Initialise local thread variables (if needed)
   * ======================================================
   */

  initialiseLocalThreadVariables ();

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
              variableDeclarations[DirectLoop::Fortran::KernelSubroutine::setElementCounter]);

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
      variableDeclarations[DirectLoop::Fortran::KernelSubroutine::setSize]),
      buildIntVal (1));

  /*
   * ======================================================
   * Build main loop statements
   * ======================================================
   */
  SgBasicBlock * loopBody = buildMainLoopStatements ();

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
  createReductionSubroutineCall ();
}

void
FortranCUDAKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
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
    variableDeclarations[*it]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            *it, FortranTypesBuilder::getFourByteInteger (), subroutineScope);
  }

  createLocalThreadDeclarations ();

  createAutosharedDeclaration ();
}

void
FortranCUDAKernelSubroutineDirectLoop::create_OP_DAT_FormalParameters ()
{
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
      <= parallelLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicate_OP_DAT (i) == false)
    {
      /*
       * ======================================================
       * Obtain the base type of the OP_DAT argument.
       * The base type is almost always an array
       * ======================================================
       */

      SgType * opDatBaseType = parallelLoop->get_OP_DAT_Type (i);

      SgArrayType * isArrayType = isSgArrayType (opDatBaseType);

      ROSE_ASSERT (isArrayType != NULL);

      opDatBaseType = isArrayType->get_base_type ();

      /*
       * ======================================================
       * Build the upper bound of the OP_DAT array which
       * is stored in the argSizes variable
       * ======================================================
       */

      string const & variableName = VariableNames::getOpDatSizeName (i);

      SgExpression
          * argSizeField =
              buildDotExp (
                  buildVarRefExp (
                      variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]),
                  buildOpaqueVarRefExp (variableName, subroutineScope));

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

      variableDeclarations[VariableNames::getOpDatName (i)]
          = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              VariableNames::getOpDatName (i), arrayType, subroutineScope,
              formalParameters, 1, DEVICE);
    }
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createlocalVariableDeclarations ()
{
  variableDeclarations[IndirectAndDirectLoop::Fortran::VariableNames::argsSizes]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          IndirectAndDirectLoop::Fortran::VariableNames::argsSizes,
          dataSizesDeclaration->getType (), subroutineScope, formalParameters,
          1, DEVICE);

  create_OP_DAT_FormalParameters ();

  /*
   * ======================================================
   * OffsetS formal parameter. This is the offset
   * in the shared memory variable assigned to each
   * thread block (see Mike's Developers Guide, direct
   * loops section)
   * ======================================================
   */

  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE);

  /*
   * ======================================================
   * Set size formal parameter
   * ======================================================
   */

  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::setSize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::Fortran::KernelSubroutine::setSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE);

  /*
   * ======================================================
   * Warp size formal parameter
   * ======================================================
   */

  variableDeclarations[DirectLoop::Fortran::KernelSubroutine::warpSize]
      = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::Fortran::KernelSubroutine::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE);

  if (parallelLoop->isReductionRequired () == true)
  {
    variableDeclarations[IndirectAndDirectLoop::Fortran::KernelSubroutine::offsetForReduction]
        = FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
            IndirectAndDirectLoop::Fortran::KernelSubroutine::offsetForReduction,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope,
            formalParameters, 1, VALUE);
  }
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

FortranCUDAKernelSubroutineDirectLoop::FortranCUDAKernelSubroutineDirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    ParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration) :
  FortranCUDAKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope)
{
  this->dataSizesDeclaration = dataSizesDeclaration;

  createlocalVariableDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}

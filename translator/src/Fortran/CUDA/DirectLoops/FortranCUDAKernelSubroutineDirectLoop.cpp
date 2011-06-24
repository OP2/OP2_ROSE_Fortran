#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranCUDAReductionSubroutine.h>
#include <ROSEHelper.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
FortranCUDAKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
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

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    int dim = parallelLoop->getOpDatDimension (i);

    SgExpression * parameterExpression = buildIntVal (1);

    if (parallelLoop->getOpMapValue (i) == GLOBAL)
    {
      if (parallelLoop->getOpAccessValue (i) == READ_ACCESS)
      {
        /*
         * ======================================================
         * Case of global variable accessed in read mode:
         * we directly access the device variable, by
         * passing the kernel the variable name in positions
         * 0:argSize%<devVarName>-1
         * ======================================================
         */

        SgExpression * argSizeField = buildDotExp (buildVarRefExp (
            variableDeclarations->get (CommonVariableNames::argsSizes)),
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
            variableDeclarations->get (VariableNames::getOpDatName (i))),
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

        parameterExpression = buildVarRefExp (variableDeclarations->get (
            VariableNames::getOpDatLocalName (i)));
      }
    }
    else if (parallelLoop->getOpMapValue (i) == DIRECT)
    {
      if (parallelLoop->getNumberOfIndirectDataSets () > 0)
      {
        SgExpression
            * deviceVarAccessDirectBegin =
                buildMultiplyOp (
                    buildAddOp (
                        buildVarRefExp (
                            variableDeclarations->get (
                                DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
                        buildVarRefExp (
                            variableDeclarations->get (
                                DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock))),
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
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            arraySubscriptExpression);
      }
      else if (dim == 1)
      {
        SgExpression * nVarRef = buildVarRefExp (variableDeclarations->get (
            DirectLoop::Fortran::KernelSubroutine::setElementCounter));
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
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            arraySubscriptExpression);
      }
      else
      {
        parameterExpression = buildVarRefExp (variableDeclarations->get (
            VariableNames::getOpDatLocalName (i)));
      }
    }

    actualParameters->append_expression (parameterExpression);
  }

  return buildFunctionCallStmt (userSubroutineName, buildVoidType (),
      actualParameters, subroutineScope);
}

SgBasicBlock *
FortranCUDAKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToLocalThreadVariablesStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;
  using SageInterface::appendStatement;

  SgBasicBlock * outerBlock = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) != GLOBAL
        && parallelLoop->getOpAccessValue (i) != WRITE_ACCESS
        && parallelLoop->getOpDatDimension (i) != 1)
    {
      /*
       * ======================================================
       * Builds stage in from device memory to shared memory
       * ======================================================
       */

      SgExpression * initLoop = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (
          parallelLoop->getOpDatDimension (i));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::remainingElements)));

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
          multiplyExpression1);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement)),
          addExpression1);

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::remainingElements)));

      SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock)),
          buildIntVal (parallelLoop->getOpDatDimension (i)));\

      SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
          multiplyExpression3);

      SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
          addExpression3);

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::autoshared)),
          addExpression2);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          addExpression4);

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          arrayExpression1, arrayExpression2);

      SgBasicBlock * firstLoopBody = buildBasicBlock (assignmentStatement1);

      SgFortranDo * firstLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initLoop, upperBoundExpression, buildIntVal (1), firstLoopBody);

      appendStatement (firstLoopStatement, outerBlock);

      /*
       * ======================================================
       * Builds stage in from shared memory to local thread
       * variables
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          multiplyExpression4);

      SgAddOp * addExpression6 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement)),
          addExpression5);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)));

      SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::autoshared)),
          addExpression6);

      SgExprStatement * assignmentStatement2 = buildAssignStatement (
          arrayExpression3, arrayExpression4);

      SgBasicBlock * secondLoopBody = buildBasicBlock (assignmentStatement2);

      SgFortranDo * secondLoopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              initLoop, upperBoundExpression, buildIntVal (1), secondLoopBody);

      appendStatement (secondLoopStatement, outerBlock);
    }
  }

  return outerBlock;
}

SgBasicBlock *
FortranCUDAKernelSubroutineDirectLoop::createStageOutFromLocalThreadVariablesToDeviceMemoryStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildNullExpression;
  using SageInterface::appendStatement;

  SgBasicBlock * outerBlock = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->getOpMapValue (i) != GLOBAL
        && parallelLoop->getOpAccessValue (i) != READ_ACCESS
        && parallelLoop->getOpDatDimension (i) != 1)
    {
      SgExpression * loopInitializationExpression = buildAssignOp (
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          buildIntVal (0));

      SgExpression * upperBoundExpression = buildIntVal (
          parallelLoop->getOpDatDimension (i) - 1);

      /*
       * ======================================================
       * Builds stage out from local thread variables to
       * shared memory
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression1 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          multiplyExpression1);

      SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement)),
          addExpression1);

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)));

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::autoshared)),
          addExpression2);

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          arrayExpression2, arrayExpression1);

      SgBasicBlock * loopBody1 = buildBasicBlock (assignmentStatement1);

      SgFortranDo * loopStatement1 =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopInitializationExpression, upperBoundExpression, buildIntVal (
                  1), loopBody1);

      appendStatement (loopStatement1, outerBlock);

      /*
       * ======================================================
       * Builds stage out from shared memory to device
       * variables
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::remainingElements)));

      SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock)),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
          multiplyExpression3);

      SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
          addExpression3);

      SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::remainingElements)));

      SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
          multiplyExpression4);

      SgAddOp * addExpression6 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement)),
          addExpression5);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          addExpression4);

      SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::autoshared)),
          addExpression6);

      SgExprStatement * assignmentStatement2 = buildAssignStatement (
          arrayExpression3, arrayExpression4);

      SgBasicBlock * loopBody2 = buildBasicBlock (assignmentStatement2);

      SgFortranDo * loopStatement2 =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopInitializationExpression, upperBoundExpression, buildIntVal (
                  1), loopBody2);

      appendStatement (loopStatement2, outerBlock);
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
   * Update offset
   * ======================================================
   */
  SgExpression * initOffsetVariable = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock)),
      buildSubtractOp (buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::threadIDModulus))));

  /*
   * ======================================================
   * Update nelems
   * ======================================================
   */

  SgFunctionSymbol * minFunctionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("min", subroutineScope);

  SgExpression * setSizeMinusOffset = buildSubtractOp (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::setSize)), buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock)));

  SgExprListExp
      * minActualParameters = buildExprListExp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::warpSize)),
          setSizeMinusOffset);

  SgFunctionCallExp * minFunctionCall = buildFunctionCallExp (
      minFunctionSymbol, minActualParameters);

  SgAssignOp * assignNelems = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::remainingElements)),
      minFunctionCall);

  SgBasicBlock * preAssignments =
      createStageInFromDeviceMemoryToLocalThreadVariablesStatements ();

  SgStatement * userFunctionCall = createUserSubroutineCallStatement ();

  SgBasicBlock * postAssignments =
      createStageOutFromLocalThreadVariablesToDeviceMemoryStatements ();

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
      CUDA::Fortran::threadidx, subroutineScope);

  SgVarRefExp * variable_X = buildOpaqueVarRefExp (CUDA::Fortran::x,
      subroutineScope);

  SgVarRefExp * variable_Blockidx = buildOpaqueVarRefExp (
      CUDA::Fortran::blockidx, subroutineScope);

  SgVarRefExp * variable_Blockdim = buildOpaqueVarRefExp (
      CUDA::Fortran::blockdim, subroutineScope);

  SgVarRefExp * variable_GridDim = buildOpaqueVarRefExp (
      CUDA::Fortran::griddim, subroutineScope);

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

  SgExprListExp * modActualParameters = buildExprListExp (threadidxMinusOne,
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpSize)));

  SgFunctionCallExp * modFunctionCall = buildFunctionCallExp (
      modFunctionSymbol, modActualParameters);

  SgAssignOp * assignTid = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
      modFunctionCall);

  appendStatement (buildExprStatement (assignTid), subroutineScope);

  /*
   * ======================================================
   * Initialise the argSDisplacement variable
   * ======================================================
   */

  SgExpression * threadidxMinusOneDivWarpSize = buildDivideOp (
      threadidxMinusOne, buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpSize)));

  SgExpression * argSDisplacementInitExprWithoutSize = buildMultiplyOp (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize)),
      threadidxMinusOneDivWarpSize);

  SgExpression * divisionExprForArgSDispl;

  if (parallelLoop->getSizeOfOpDat () == 8)
  {
    divisionExprForArgSDispl = buildDivideOp (
        argSDisplacementInitExprWithoutSize, buildIntVal (8));
  }
  else
  {
    divisionExprForArgSDispl = buildDivideOp (
        argSDisplacementInitExprWithoutSize, buildIntVal (4));
  }

  SgAssignOp * assignArgSDispl = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement)),
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

  SgVarRefExp * iterationCounterReference = buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::setElementCounter));

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

  SgExpression * upperBoundExpression = buildSubtractOp (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::setSize)), buildIntVal (1));

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

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionSubroutineCall ();
  }
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
    variableDeclarations->add (*it,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (*it,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope));
  }

  createLocalThreadDeclarations ();

  createAutoSharedDeclaration ();

  if (parallelLoop->isReductionRequired () == true)
  {
    variableDeclarations->add (ReductionSubroutine::offsetForReduction,
        FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
            ReductionSubroutine::offsetForReduction,
            FortranTypesBuilder::getFourByteInteger (), subroutineScope, 1,
            VALUE));
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using SageInterface::appendStatement;
  using std::string;

  /*
   * ======================================================
   * Argsizes formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      CommonVariableNames::argsSizes,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          CommonVariableNames::argsSizes, dataSizesDeclaration->getType (),
          subroutineScope, formalParameters, 1, DEVICE));

  /*
   * ======================================================
   * OP_DAT formal parameters
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      /*
       * ======================================================
       * Obtain the base type of the OP_DAT argument
       * ======================================================
       */

      SgType * opDatType = parallelLoop->getOpDatType (i);

      SgArrayType * isArrayType = isSgArrayType (opDatType);

      SgType * opDatBaseType = isArrayType->get_base_type ();

      /*
       * ======================================================
       * Build the upper bound of the OP_DAT array which
       * is stored in the argSizes variable
       * ======================================================
       */
      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::argsSizes)),
          buildVarRefExp (dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getOpDatSizeName (i))));

      SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      SgSubscriptExpression * arraySubscriptExpression =
          new SgSubscriptExpression (ROSEHelper::getFileInfo (),
              buildIntVal (0), subtractExpression, buildIntVal (1));

      arraySubscriptExpression->set_endOfConstruct (ROSEHelper::getFileInfo ());

      /*
       * ======================================================
       * Build array type with the correct subscript
       * ======================================================
       */

      SgArrayType * arrayType = buildArrayType (opDatBaseType,
          arraySubscriptExpression);

      arrayType->set_rank (1);

      arrayType->set_dim_info (buildExprListExp (arraySubscriptExpression));

      string const & variableName = VariableNames::getOpDatName (i);

      variableDeclarations->add (
          variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, arrayType, subroutineScope, formalParameters, 1,
              DEVICE));
    }
  }

  /*
   * ======================================================
   * Warp scratch pad size formal parameter. This is the offset
   * in the shared memory variable assigned to each
   * thread block (see Mike's Developers Guide, direct
   * loops section)
   * ======================================================
   */

  variableDeclarations->add (
      DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Set size formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      DirectLoop::Fortran::KernelSubroutine::setSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::Fortran::KernelSubroutine::setSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Warp size formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      DirectLoop::Fortran::KernelSubroutine::warpSize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::Fortran::KernelSubroutine::warpSize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
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
      parallelLoop, moduleScope), dataSizesDeclaration (dataSizesDeclaration)
{
  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}

#include <FortranCUDAKernelSubroutineDirectLoop.h>
#include <FortranTypesBuilder.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranCUDAReductionSubroutine.h>
#include <RoseHelper.h>
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
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprListExp * actualParameters = buildExprListExp ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    int dim = parallelLoop->getOpDatDimension (i);

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

        SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
            variableDeclarations->get (
                VariableNames::getDataSizesVariableDeclarationName (
                    userSubroutineName))), buildOpaqueVarRefExp (
            VariableNames::getOpDatSizeName (i), subroutineScope));

        SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (), buildIntVal (
                0), subtractExpression, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatName (i))), arraySubscriptExpression);

        actualParameters->append_expression (parameterExpression);
      }
      else
      {
        /*
         * ======================================================
         * Case of global variable accessed NOT in read mode:
         * we access the corresponding local thread variable
         * ======================================================
         */

        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatLocalName (i))));
      }
    }
    else if (parallelLoop->getOpMapValue (i) == DIRECT)
    {
      if (parallelLoop->getNumberOfIndirectOpDats () > 0)
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
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                deviceVarAccessDirectBegin, deviceVarAccessDirectEnd,
                buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatName (i))), arraySubscriptExpression);

        actualParameters->append_expression (parameterExpression);
      }
      else if (dim == 1)
      {
        SgAddOp * addExpression = buildAddOp (buildVarRefExp (
            variableDeclarations->get (
                DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
            buildIntVal (dim - 1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                buildVarRefExp (variableDeclarations->get (
                    DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
                addExpression, buildIntVal (1));

        arraySubscriptExpression->set_endOfConstruct (
            RoseHelper::getFileInfo ());

        SgPntrArrRefExp * parameterExpression = buildPntrArrRefExp (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatName (i))), arraySubscriptExpression);

        actualParameters->append_expression (parameterExpression);
      }
      else
      {
        actualParameters->append_expression (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatLocalName (i))));
      }
    }
    else
    {
      SgIntVal * parameterExpression = buildIntVal (1);

      actualParameters->append_expression (parameterExpression);
    }
  }

  return buildFunctionCallStmt (userSubroutineName, buildVoidType (),
      actualParameters, subroutineScope);
}

SgBasicBlock *
FortranCUDAKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToLocalThreadVariablesStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
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

      SgAssignOp * initialisationExpression = buildAssignOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::dataPerElementCounter)),
          buildIntVal (0));

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

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
          dotExpression);

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
              initialisationExpression, dotExpression, buildIntVal (1),
              firstLoopBody);

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
          dotExpression);

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
              initialisationExpression, dotExpression, buildIntVal (1),
              secondLoopBody);

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
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildSubtractOp;
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

      SgDotExp * dotExpression = buildDotExp (buildVarRefExp (
          variableDeclarations->get (
              VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgExpression * upperBoundExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      /*
       * ======================================================
       * Builds stage out from local thread variables to
       * shared memory
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
          dotExpression);

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
          dotExpression);

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

void
FortranCUDAKernelSubroutineDirectLoop::createExecutionLoopStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageInterface::appendStatement;

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgSubtractOp * subtractExpression1 = buildSubtractOp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::threadIDModulus)));

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock)),
      subtractExpression1);

  appendStatement (assignmentStatement1, loopBody);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("min", subroutineScope);

  SgSubtractOp * subtractExpression2 = buildSubtractOp (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::setSize)), buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::Fortran::KernelSubroutine::offsetInThreadBlock)));

  SgExprListExp * actualParameters = buildExprListExp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpSize)),
      subtractExpression2);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::remainingElements)),
      functionCall);

  appendStatement (assignmentStatement2, loopBody);

  appendStatement (
      createStageInFromDeviceMemoryToLocalThreadVariablesStatements (),
      loopBody);

  appendStatement (createUserSubroutineCallStatement (), loopBody);

  appendStatement (
      createStageOutFromLocalThreadVariablesToDeviceMemoryStatements (),
      loopBody);

  SgDotExp * dotExpression1 = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::blockidx, subroutineScope), buildOpaqueVarRefExp (
      CUDA::Fortran::x, subroutineScope));

  SgSubtractOp * subtractExpression3 = buildSubtractOp (dotExpression1,
      buildIntVal (1));

  SgDotExp * dotExpression2 = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::threadidx, subroutineScope), buildOpaqueVarRefExp (
      CUDA::Fortran::x, subroutineScope));

  SgSubtractOp * subtractExpression4 = buildSubtractOp (dotExpression2,
      buildIntVal (1));

  SgDotExp * dotExpression3 = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::blockdim, subroutineScope), buildOpaqueVarRefExp (
      CUDA::Fortran::x, subroutineScope));

  SgMultiplyOp * multiplyExpression = buildMultiplyOp (subtractExpression3,
      dotExpression3);

  SgAddOp * addExpression =
      buildAddOp (subtractExpression4, multiplyExpression);

  SgExpression * initialisationExpression = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::setElementCounter)),
      addExpression);

  SgDotExp * dotExpression4 = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::griddim, subroutineScope), buildOpaqueVarRefExp (
      CUDA::Fortran::x, subroutineScope));

  SgMultiplyOp * strideExpression = buildMultiplyOp (dotExpression3,
      dotExpression4);

  SgExpression * upperBoundExpression = buildSubtractOp (
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::setSize)), buildIntVal (1));

  SgFortranDo * fortranDoStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initialisationExpression, upperBoundExpression, strideExpression,
          loopBody);

  appendStatement (fortranDoStatement, subroutineScope);
}

void
FortranCUDAKernelSubroutineDirectLoop::createAutoSharedDisplacementInitialisationStatement ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  SgDotExp * dotExpression = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::threadidx, subroutineScope), buildOpaqueVarRefExp (
      CUDA::Fortran::x, subroutineScope));

  SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgDivideOp * divideExpression1 = buildDivideOp (subtractExpression,
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpSize)));

  SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpScratchpadSize)),
      divideExpression1);

  if (parallelLoop->getSizeOfOpDat () == 8)
  {
    SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression,
        buildIntVal (8));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        buildVarRefExp (variableDeclarations->get (
            DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement)),
        divideExpression2);

    appendStatement (assignmentStatement, subroutineScope);
  }
  else
  {
    SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression,
        buildIntVal (4));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        buildVarRefExp (variableDeclarations->get (
            DirectLoop::Fortran::KernelSubroutine::autosharedDisplacement)),
        divideExpression2);

    appendStatement (assignmentStatement, subroutineScope);
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createThreadIDInitialisationStatement ()
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  SgDotExp * dotExpression = buildDotExp (buildOpaqueVarRefExp (
      CUDA::Fortran::threadidx, subroutineScope), buildOpaqueVarRefExp (
      CUDA::Fortran::x, subroutineScope));

  SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
      buildIntVal (1));

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction ("mod", subroutineScope);

  SgExprListExp * actualParameters = buildExprListExp (subtractExpression,
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::warpSize)));

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      actualParameters);

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::Fortran::KernelSubroutine::threadIDModulus)),
      functionCall);

  appendStatement (assignmentStatement, subroutineScope);
}

void
FortranCUDAKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
{
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using std::string;

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
          variableDeclarations->get (
              VariableNames::getDataSizesVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getOpDatSizeName (i))));

      SgSubtractOp * subtractExpression = buildSubtractOp (dotExpression,
          buildIntVal (1));

      SgSubscriptExpression * arraySubscriptExpression =
          new SgSubscriptExpression (RoseHelper::getFileInfo (),
              buildIntVal (0), subtractExpression, buildIntVal (1));

      arraySubscriptExpression->set_endOfConstruct (RoseHelper::getFileInfo ());

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
}

void
FortranCUDAKernelSubroutineDirectLoop::createStatements ()
{
  createThreadIDInitialisationStatement ();

  createAutoSharedDisplacementInitialisationStatement ();

  createInitialiseLocalThreadVariablesStatements ();

  createExecutionLoopStatements ();

  if (parallelLoop->isReductionRequired () == true)
  {
    createReductionLoopStatements ();
  }
}

void
FortranCUDAKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
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
  /*
   * ======================================================
   * OP_DAT dimensions formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      VariableNames::getDimensionsVariableDeclarationName (userSubroutineName),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          VariableNames::getDimensionsVariableDeclarationName (
              userSubroutineName), opDatDimensionsDeclaration->getType (),
          subroutineScope, formalParameters, 1, DEVICE));

  /*
   * ======================================================
   * Argsizes formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      VariableNames::getDataSizesVariableDeclarationName (userSubroutineName),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          VariableNames::getDataSizesVariableDeclarationName (
              userSubroutineName), dataSizesDeclaration->getType (),
          subroutineScope, formalParameters, 1, DEVICE));

  /*
   * ======================================================
   * OP_DAT formal parameters
   * ======================================================
   */

  createOpDatFormalParameterDeclarations ();

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
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranReductionSubroutines * reductionSubroutines,
    FortranCUDADataSizesDeclarationDirectLoop * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  FortranCUDAKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope, reductionSubroutines,
      opDatDimensionsDeclaration), dataSizesDeclaration (dataSizesDeclaration)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Direct, CUDA>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}

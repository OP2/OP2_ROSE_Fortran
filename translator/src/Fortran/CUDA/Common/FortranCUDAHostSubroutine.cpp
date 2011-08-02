#include <Debug.h>
#include <FortranStatementsAndExpressionsBuilder.h>
#include <FortranTypesBuilder.h>
#include <FortranCUDAHostSubroutine.h>
#include <FortranCUDAReductionSubroutine.h>
#include <RoseStatementsAndExpressionsBuilder.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

SgStatement *
FortranCUDAHostSubroutine::createThreadSynchroniseCallStatement ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statement to synchronize CUDA threads", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgFunctionSymbol * functionSymbol =
      FortranTypesBuilder::buildNewFortranFunction (
          CUDA::Fortran::cudaThreadSynchronize, subroutineScope);

  SgFunctionCallExp * functionCall = buildFunctionCallExp (functionSymbol,
      buildExprListExp ());

  SgExprStatement * callStatement = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::threadSynchRet)), functionCall);

  return callStatement;
}

void
FortranCUDAHostSubroutine::createReductionPrologueStatements ()
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildFloatVal;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAddOp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * We need to allocate enough additional shared memory
   * thus we need the highest Fortran kind for OP_DATs
   * of reduction variables
   * ======================================================
   */
  unsigned int maxUsedFortranKind = -1;

  unsigned int dim = -1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      if (parallelLoop->getOpDatDimension (i) > dim)
      {
        dim = parallelLoop->getOpDatDimension (i);
      }

      SgExpression * fortranKind =
          FortranStatementsAndExpressionsBuilder::getFortranKindOfOpDat (
              parallelLoop->getOpDatType (i));

      SgIntVal * kindVal = isSgIntVal (fortranKind);

      ROSE_ASSERT (kindVal != NULL);

      int actualValue = kindVal->get_value ();

      if (maxUsedFortranKind < actualValue)
      {
        maxUsedFortranKind = actualValue;
      }
    }
  }

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::maximumNumberOfThreadBlocks)), buildVarRefExp (
          variableDeclarations->get (CUDA::Fortran::blocksPerGrid)));

  appendStatement (assignmentStatement1, subroutineScope);

  SgMultiplyOp
      * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              ReductionSubroutine::maximumNumberOfThreadBlocks)), buildIntVal (
          dim));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::numberOfThreadItems)), multiplyExpression2);

  appendStatement (assignmentStatement2, subroutineScope);

  SgPntrArrRefExp * arrayIndexExpression5 = buildPntrArrRefExp (buildVarRefExp (
      moduleDeclarations->getReductionArrayHostVariableDeclaration ()),
      buildVarRefExp (variableDeclarations->get (
          CommonVariableNames::iterationCounter1)));

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      arrayIndexExpression5, buildFloatVal (0.0));

  SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement5);

  SgAssignOp * loopInitialization = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (0));

  SgFortranDo * loopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          loopInitialization, buildVarRefExp (variableDeclarations->get (
              ReductionSubroutine::numberOfThreadItems)), buildIntVal (1),
          loopBody);

  appendStatement (loopStatement, subroutineScope);

  SgExprStatement * assignmentStatement6 = buildAssignStatement (
      buildVarRefExp (
          moduleDeclarations->getReductionArrayDeviceVariableDeclaration ()),
      buildVarRefExp (
          moduleDeclarations->getReductionArrayHostVariableDeclaration ()));

  appendStatement (assignmentStatement6, subroutineScope);

  SgExprStatement * assignmentStatement7 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::sharedMemoryStartOffset)), buildVarRefExp (
          variableDeclarations->get (CUDA::Fortran::sharedMemorySize)));

  appendStatement (assignmentStatement7, subroutineScope);

  SgMultiplyOp * multiplyExpression8 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::threadsPerBlock)), buildIntVal (
      maxUsedFortranKind));

  SgExprStatement * assignmentStatement8 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::maximumBytesInSharedMemory)),
      multiplyExpression8);

  appendStatement (assignmentStatement8, subroutineScope);

  SgAddOp * addExpression9 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CUDA::Fortran::sharedMemorySize)),
      buildVarRefExp (variableDeclarations->get (
          ReductionSubroutine::maximumBytesInSharedMemory)));

  SgExprStatement * assignmentStatement9 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get (
          CUDA::Fortran::sharedMemorySize)), addExpression9);

  appendStatement (assignmentStatement9, subroutineScope);
}

void
FortranCUDAHostSubroutine::createReductionEpilogueStatements ()
{
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (
          moduleDeclarations->getReductionArrayHostVariableDeclaration ()),
      buildVarRefExp (
          moduleDeclarations->getReductionArrayDeviceVariableDeclaration ()));

  appendStatement (assignmentStatement1, subroutineScope);

  unsigned int dim = -1;
  unsigned int positionInOPDatsArray = -1;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      dim = parallelLoop->getOpDatDimension (i);
      positionInOPDatsArray = i;
    }
  }

  SgAddOp * addExpression2 = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (1));

  SgPntrArrRefExp * arrayIndexExpression2 = buildPntrArrRefExp (buildVarRefExp (
      variableDeclarations->get (VariableNames::getOpDatDeviceName (
          positionInOPDatsArray))), addExpression2);

  SgMultiplyOp * mutliplyExpression2 = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (dim));

  SgAddOp * addExpression2b = buildAddOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      mutliplyExpression2);

  SgPntrArrRefExp * arrayIndexExpression2b = buildPntrArrRefExp (
      buildVarRefExp (
          moduleDeclarations->getReductionArrayHostVariableDeclaration ()),
      addExpression2b);

  SgExpression * addExpression2c = buildAddOp (arrayIndexExpression2,
      arrayIndexExpression2b);

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      arrayIndexExpression2, addExpression2c);

  SgBasicBlock * innerLoopBody = buildBasicBlock (assignmentStatement2);

  SgExpression * innerLoopInitialization = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter2)),
      buildIntVal (0));

  SgFortranDo * innerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          innerLoopInitialization, buildIntVal (dim - 1), buildIntVal (1),
          innerLoopBody);

  SgBasicBlock * outerLoopBody = buildBasicBlock (innerLoopStatement);

  SgExpression * initOuterLoop = buildAssignOp (buildVarRefExp (
      variableDeclarations->get (CommonVariableNames::iterationCounter1)),
      buildIntVal (0));

  SgFortranDo * outerLoopStatement =
      FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
          initOuterLoop, buildVarRefExp (variableDeclarations->get (
              ReductionSubroutine::numberOfThreadItems)), buildIntVal (1),
          outerLoopBody);

  appendStatement (outerLoopStatement, subroutineScope);
}

void
FortranCUDAHostSubroutine::createReductionLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter1,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (CommonVariableNames::iterationCounter1,
      variableDeclaration1);

  SgVariableDeclaration * variableDeclaration2 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter2,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (CommonVariableNames::iterationCounter2,
      variableDeclaration2);

  SgVariableDeclaration * variableDeclaration3 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          ReductionSubroutine::sharedMemoryStartOffset,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (ReductionSubroutine::sharedMemoryStartOffset,
      variableDeclaration3);

  SgVariableDeclaration * variableDeclaration4 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          ReductionSubroutine::maximumBytesInSharedMemory,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (ReductionSubroutine::maximumBytesInSharedMemory,
      variableDeclaration4);

  SgVariableDeclaration * variableDeclaration5 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          ReductionSubroutine::numberOfThreadItems,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (ReductionSubroutine::numberOfThreadItems,
      variableDeclaration5);

  SgVariableDeclaration * variableDeclaration6 =
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          ReductionSubroutine::maximumNumberOfThreadBlocks,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope);

  variableDeclarations->add (ReductionSubroutine::maximumNumberOfThreadBlocks,
      variableDeclaration6);
}

void
FortranCUDAHostSubroutine::createCUDAKernelEpilogueStatements ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;
  using std::map;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Copy back and de-allocate only used for OP_DAT which
   * are NOT reduction variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getCToFortranVariableName (i))),
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatDeviceName (i))));

      appendStatement (assignmentStatement, subroutineScope);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      SgExprListExp * deallocateParameters = buildExprListExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatDeviceName (i))));

      FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
          deallocateParameters, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createCUDAKernelPrologueStatements ()
{
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntType;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * Allocation and copy in only used for OP_DAT which are
   * NOT reduction variables
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      SgVariableDeclaration * opDatSizeFieldDeclaration =
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getOpDatSizeName (i));

      SgExprListExp * arrayIndexExpression = buildExprListExp (buildVarRefExp (
          opDatSizeFieldDeclaration));

      SgPntrArrRefExp * subscriptExpression = buildPntrArrRefExp (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatDeviceName (i))), arrayIndexExpression);

      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
          buildExprListExp (subscriptExpression), subroutineScope);
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->isReductionRequired (i) == false)
    {
      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatDeviceName (i))), buildVarRefExp (
              variableDeclarations->get (
                  VariableNames::getCToFortranVariableName (i))));

      appendStatement (assignmentStatement, subroutineScope);
    }
  }
}

SgBasicBlock *
FortranCUDAHostSubroutine::createTransferOpDatStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements to transfer OP_DATs onto device",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      /*
       * ======================================================
       * Statement to initialise size of OP_DAT
       * ======================================================
       */
      SgDotExp * dotExpression1 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dim, block));

      if (parallelLoop->getOpMapValue (i) == GLOBAL)
      {
        switch (parallelLoop->getOpAccessValue (i))
        {
          case READ_ACCESS:
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                buildVarRefExp (variableDeclarations->get (
                    VariableNames::getOpDatSizeName (i))), dotExpression1);

            appendStatement (assignmentStatement, block);

            break;
          }

          default:
          {
            break;
          }
        }
      }
      else
      {
        SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            buildOpaqueVarRefExp (CommonVariableNames::set, block));

        SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
            buildOpaqueVarRefExp (CommonVariableNames::size, block));

        SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
            dotExpression3);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatSizeName (i))), multiplyExpression);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      /*
       * ======================================================
       * Statement to convert OP_DAT between C and Fortran
       * pointers
       * ======================================================
       */

      SgDotExp * parameterExpression1A = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dat_d, block));

      SgVarRefExp * parameterExpression2A = buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatDeviceName (i)));

      SgExpression * parameterExpression3A = buildOpaqueVarRefExp ("(/"
          + buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatSizeName (i)))->unparseToString () + "/)",
          block);

      SgStatement * callStatementA =
          SubroutineCalls::createCToFortranPointerCallStatement (
              subroutineScope, parameterExpression1A, parameterExpression2A,
              parameterExpression3A);

      appendStatement (callStatementA, block);

      if (parallelLoop->getOpMapValue (i) == GLOBAL)
      {
        SgDotExp * parameterExpression1B = buildDotExp (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            buildOpaqueVarRefExp (CommonVariableNames::dat, block));

        SgVarRefExp * parameterExpression2B = buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatHostName (i)));

        SgExpression * parameterExpression3B = buildOpaqueVarRefExp ("(/"
            + buildVarRefExp (variableDeclarations->get (
                VariableNames::getOpDatSizeName (i)))->unparseToString ()
            + "/)", block);

        SgStatement * callStatementB =
            SubroutineCalls::createCToFortranPointerCallStatement (
                subroutineScope, parameterExpression1B, parameterExpression2B,
                parameterExpression3B);

        appendStatement (callStatementB, block);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false
        && parallelLoop->getOpMapValue (i) == GLOBAL)
    {
      SgExprStatement * assignmentStatement = buildAssignStatement (
          buildVarRefExp (variableDeclarations->get (
              VariableNames::getOpDatDeviceName (i))), buildVarRefExp (
              variableDeclarations->get (VariableNames::getOpDatHostName (i))));

      appendStatement (assignmentStatement, block);
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAHostSubroutine::createFirstTimeExecutionStatements ()
{
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildBoolValExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildMultiplyOp;
  using SageInterface::appendStatement;

  SgBasicBlock * block = buildBasicBlock ();

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT dimensions",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgVarRefExp * opDatDimensionsReference = buildVarRefExp (
        moduleDeclarations->getDimensionsVariableDeclaration ());

    SgVarRefExp * fieldReference = buildVarRefExp (
        opDatDimensionsDeclaration->getOpDatDimensionField (i));

    SgDotExp * dotExpression1 = buildDotExp (opDatDimensionsReference,
        fieldReference);

    SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
        variableDeclarations->get (VariableNames::getOpDatName (i))),
        buildOpaqueVarRefExp (CommonVariableNames::dim, block));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        dotExpression1, dotExpression2);

    appendStatement (assignmentStatement, block);
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT sizes", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      SgVarRefExp * dataSizesReference = buildVarRefExp (
          moduleDeclarations->getDataSizesVariableDeclaration ());

      SgVarRefExp * fieldReference = buildVarRefExp (
          dataSizesDeclaration->getFieldDeclarations ()->get (
              VariableNames::getOpDatSizeName (i)));

      SgDotExp * dotExpression1 = buildDotExp (dataSizesReference,
          fieldReference);

      SgDotExp * dotExpression2 = buildDotExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatName (i))),
          buildOpaqueVarRefExp (CommonVariableNames::dim, block));

      if (parallelLoop->getOpMapValue (i) == GLOBAL)
      {
        switch (parallelLoop->getOpAccessValue (i))
        {
          case READ_ACCESS:
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                dotExpression1, dotExpression2);

            appendStatement (assignmentStatement, block);

            break;
          }

          default:
          {
            break;
          }
        }
      }
      else
      {
        SgDotExp * dotExpression3 = buildDotExp (buildVarRefExp (
            variableDeclarations->get (VariableNames::getOpDatName (i))),
            buildOpaqueVarRefExp (CommonVariableNames::set, block));

        SgDotExp * dotExpression4 = buildDotExp (dotExpression3,
            buildOpaqueVarRefExp (CommonVariableNames::size, block));

        SgExpression * multiplyExpression = buildMultiplyOp (dotExpression2,
            dotExpression4);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression1, multiplyExpression);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  if (parallelLoop->isReductionRequired ())
  {
    SgPntrArrRefExp * arrayIndexExpression3 = buildPntrArrRefExp (
        buildVarRefExp (
            moduleDeclarations->getReductionArrayHostVariableDeclaration ()),
        buildVarRefExp (variableDeclarations->get (
            ReductionSubroutine::numberOfThreadItems)));

    FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
        buildExprListExp (arrayIndexExpression3), block);

    SgPntrArrRefExp * arrayIndexExpression4 = buildPntrArrRefExp (
        buildVarRefExp (
            moduleDeclarations->getReductionArrayDeviceVariableDeclaration ()),
        buildVarRefExp (variableDeclarations->get (
            ReductionSubroutine::numberOfThreadItems)));

    FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
        buildExprListExp (arrayIndexExpression4), block);
  }

  return block;
}

SgBasicBlock *
FortranCUDAHostSubroutine::createCallToInitialiseConstantsStatements ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildBasicBlock;
  using SageBuilder::buildEqualityOp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildBoolValExp;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statements to call initialise constants subroutine",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  SgBasicBlock * ifBody = buildBasicBlock ();

  SgEqualityOp * ifGuardExpression = buildEqualityOp (buildVarRefExp (
      moduleDeclarations->getInitialiseConstantsBooleanDeclaration ()),
      buildBoolValExp (true));

  SgExprStatement * assignmentStatement = buildAssignStatement (buildVarRefExp (
      moduleDeclarations->getInitialiseConstantsBooleanDeclaration ()),
      buildBoolValExp (false));

  appendStatement (assignmentStatement, ifBody);

  SgExprListExp * actualParameters = buildExprListExp ();

  SgExprStatement * callStatement = buildFunctionCallStmt (
      initialiseConstantsSubroutine->getSubroutineName (), buildVoidType (),
      actualParameters, subroutineScope);

  appendStatement (callStatement, ifBody);

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBody);

  appendStatement (ifStatement, block);

  return block;
}

void
FortranCUDAHostSubroutine::createDataMarshallingLocalVariableDeclarations ()
{
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for data marshalling",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getOpDatSizeName (i);

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, FortranTypesBuilder::getFourByteInteger (),
              subroutineScope));
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      string const & variableName = VariableNames::getOpDatDeviceName (i);

      SgArrayType * isArrayType =
          isSgArrayType (parallelLoop->getOpDatType (i));

      variableDeclarations->add (variableName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              variableName, isArrayType, subroutineScope, 2, DEVICE,
              ALLOCATABLE));

      if (parallelLoop->getOpMapValue (i) == GLOBAL)
      {
        string const & variableName = VariableNames::getOpDatHostName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName,
                buildPointerType (parallelLoop->getOpDatType (i)),
                subroutineScope));
      }
    }
  }
}

void
FortranCUDAHostSubroutine::createCUDAKernelLocalVariableDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating CUDA kernel local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (CUDA::Fortran::blocksPerGrid,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::blocksPerGrid,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (CUDA::Fortran::threadsPerBlock,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::threadsPerBlock,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (CUDA::Fortran::sharedMemorySize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::sharedMemorySize,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (CUDA::Fortran::threadSynchRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::Fortran::threadSynchRet,
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));
}

FortranCUDAHostSubroutine::FortranCUDAHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName,
    FortranParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    FortranInitialiseConstantsSubroutine * initialiseConstantsSubroutine,
    FortranCUDADataSizesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranHostSubroutine (subroutineName, userSubroutineName,
      kernelSubroutineName, parallelLoop, moduleScope),
      initialiseConstantsSubroutine (initialiseConstantsSubroutine),
      dataSizesDeclaration (dataSizesDeclaration), opDatDimensionsDeclaration (
          opDatDimensionsDeclaration), moduleDeclarations (moduleDeclarations)
{
  using SageInterface::addTextForUnparser;

  addTextForUnparser (subroutineHeaderStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);
}

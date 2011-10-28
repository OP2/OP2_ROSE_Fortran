#include <CPPCUDAKernelSubroutineIndirectLoop.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CommonNamespaces.h>
#include <CUDA.h>

SgStatement *
CPPCUDAKernelSubroutineIndirectLoop::createUserSubroutineCallStatement ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating call to user device subroutine", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createIncrementAndWriteAccessEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating increment and write access epilogue statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        if (parallelLoop->isWritten (i) || parallelLoop->isReadAndWritten (i)
            || parallelLoop->isIncremented (i))
        {
          /*
           * ======================================================
           * For loop body
           * ======================================================
           */

          SgBasicBlock * loopBody = buildBasicBlock ();

          SgDivideOp * divideExpression1 = buildDivideOp (
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgPntrArrRefExp * arrayExpression1a = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getIndirectionMapName (i)),
              divideExpression1);

          SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
              arrayExpression1a, buildIntVal (parallelLoop->getOpDatDimension (
                  i)));

          SgModOp * modulusExpression1 = buildModOp (
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression1 = buildAddOp (modulusExpression1,
              multiplyExpression1);

          SgPntrArrRefExp * arrayExpression1b = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getOpDatName (i)), addExpression1);

          SgPntrArrRefExp * arrayExpression1c = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getIndirectionCUDASharedMemoryName (i)),
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1));

          SgExprStatement * assignmentStatement1 = buildAssignStatement (
              arrayExpression1c, arrayExpression1b);

          appendStatement (assignmentStatement1, loopBody);

          /*
           * ======================================================
           * For loop statement
           * ======================================================
           */

          SgExprStatement * initialisationExpression = buildAssignStatement (
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1), CUDA::getThreadId (
                  THREAD_X, subroutineScope));

          SgMultiplyOp * multiplyExpression = buildMultiplyOp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getIndirectionArgumentSizeName (i)),
              buildIntVal (parallelLoop->getOpDatDimension (i)));

          SgLessThanOp * upperBoundExpression = buildLessThanOp (
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1), multiplyExpression);

          SgPlusAssignOp * strideExpression = buildPlusAssignOp (
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1),
              CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

          SgForStatement * forStatement = buildForStatement (
              initialisationExpression, buildExprStatement (
                  upperBoundExpression), strideExpression, loopBody);

          appendStatement (forStatement, block);
        }
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createStageOutFromLocalMemoryToSharedMemoryStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating stage out from local memory to shared memory statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createIncrementAdjustmentStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to adjust incremented OP_DATS",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createInitialiseIncrementAccessStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise scratchpad memory",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);
}

void
CPPCUDAKernelSubroutineIndirectLoop::createExecutionLoopStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating main execution loop statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createInitialiseCUDASharedVariablesStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise shared memory", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * For loop body
         * ======================================================
         */

        SgBasicBlock * loopBody = buildBasicBlock ();

        if (parallelLoop->isIncremented (i))
        {
          SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getIndirectionCUDASharedMemoryName (i)),
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1));

          if (isSgTypeFloat (parallelLoop->getOpDatBaseType (i)))
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                arrayExpression, buildFloatVal (0.0));

            appendStatement (assignmentStatement, loopBody);
          }
          else
          {
            SgExprStatement * assignmentStatement = buildAssignStatement (
                arrayExpression, buildIntVal (0));

            appendStatement (assignmentStatement, loopBody);
          }
        }
        else
        {
          SgDivideOp * divideExpression = buildDivideOp (
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getIndirectionMapName (i)),
              divideExpression);

          SgMultiplyOp * multiplyExpression = buildMultiplyOp (
              arrayExpression1, buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgModOp * modulusExpression = buildModOp (
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1), buildIntVal (
                  parallelLoop->getOpDatDimension (i)));

          SgAddOp * addExpression = buildAddOp (modulusExpression,
              multiplyExpression);

          SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getOpDatName (i)), addExpression);

          SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getIndirectionCUDASharedMemoryName (i)),
              variableDeclarations->getReference (
                  CommonVariableNames::iterationCounter1));

          SgExprStatement * assignmentStatement = buildAssignStatement (
              arrayExpression3, arrayExpression2);

          appendStatement (assignmentStatement, loopBody);
        }

        /*
         * ======================================================
         * For loop statement
         * ======================================================
         */

        SgExprStatement * initialisationExpression = buildAssignStatement (
            variableDeclarations->getReference (
                CommonVariableNames::iterationCounter1), CUDA::getThreadId (
                THREAD_X, subroutineScope));

        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (
                OP2::VariableNames::getIndirectionArgumentSizeName (i)),
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgLessThanOp * upperBoundExpression = buildLessThanOp (
            variableDeclarations->getReference (
                CommonVariableNames::iterationCounter1), multiplyExpression);

        SgPlusAssignOp * strideExpression = buildPlusAssignOp (
            variableDeclarations->getReference (
                CommonVariableNames::iterationCounter1),
            CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

        SgForStatement * forStatement = buildForStatement (
            initialisationExpression,
            buildExprStatement (upperBoundExpression), strideExpression,
            loopBody);

        appendStatement (forStatement, block);
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createSetIndirectionMapPointerStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT shared memory pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
            variableDeclarations->getReference (OP2::VariableNames::blockID),
            buildIntVal (parallelLoop->getOpDatDimension (i)));

        SgAddOp * addExpression1a = buildAddOp (buildIntVal (offset),
            multiplyExpression1);

        SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (
                OP2::VariableNames::PlanFunction::pindOffs), addExpression1a);

        SgAddOp * addExpression1b = buildAddOp (
            variableDeclarations->getReference (
                OP2::VariableNames::getLocalToGlobalMappingName (i)),
            arrayExpression1);

        SgExprStatement
            * assignmentStatement1 = buildAssignStatement (
                variableDeclarations->getReference (
                    OP2::VariableNames::getIndirectionMapName (i)),
                addExpression1b);

        appendStatement (assignmentStatement1, block);

        ++offset;
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createSetOpDatSharedMemoryPointerStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT shared memory pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (OP2::VariableNames::nbytes),
      buildIntVal (0));

  appendStatement (assignmentStatement1, block);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int indirectOpDatCounter = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const autosharedVariableName =
            OP2::VariableNames::getCUDASharedMemoryDeclarationName (
                parallelLoop->getOpDatBaseType (i),
                parallelLoop->getSizeOfOpDat (i));

        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
            variableDeclarations->getReference (autosharedVariableName),
            variableDeclarations->getReference (OP2::VariableNames::nbytes));

        SgAddressOfOp * addressOfExpression2 = buildAddressOfOp (
            arrayExpression2);

        SgExprStatement * assignmentStatement2 = buildAssignStatement (
            variableDeclarations->getReference (
                OP2::VariableNames::getIndirectionCUDASharedMemoryName (i)),
            addressOfExpression2);

        appendStatement (assignmentStatement2, block);

        /*
         * ======================================================
         * New statement
         * ======================================================
         */

        indirectOpDatCounter++;

        if (indirectOpDatCounter
            < parallelLoop->getNumberOfDifferentIndirectOpDats ())
        {
          SgMultiplyOp * multiplyExpression3a = buildMultiplyOp (buildSizeOfOp (
              buildFloatType ()), buildIntVal (parallelLoop->getOpDatDimension (
              i)));

          SgMultiplyOp * multiplyExpression3b = buildMultiplyOp (
              variableDeclarations->getReference (
                  OP2::VariableNames::getIndirectionArgumentSizeName (i)),
              multiplyExpression3a);

          SgFunctionCallExp * functionCallExpression3 =
              OP2::CPPMacroSupport::createRoundUpCallStatement (
                  subroutineScope, multiplyExpression3b);

          SgPlusAssignOp * assignmentStatement3 = buildPlusAssignOp (
              variableDeclarations->getReference (OP2::VariableNames::nbytes),
              functionCallExpression3);

          appendStatement (buildExprStatement (assignmentStatement3), block);
        }
      }
    }
  }

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createIncrementAccessThreadZeroStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating thread zero statements for incremented OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgSubtractOp * subtractExpression1 = buildSubtractOp (
      variableDeclarations->getReference (OP2::VariableNames::nelems),
      buildIntVal (1));

  SgDivideOp * divideExpression1 = buildDivideOp (subtractExpression1,
      CUDA::getThreadBlockDimension (THREAD_X, subroutineScope));

  SgExpression * addExpression1 = buildAddOp (buildIntVal (1),
      divideExpression1);

  SgMultiplyOp * multiplyExpression1 =
      buildMultiplyOp (
          CUDA::getThreadBlockDimension (THREAD_X, subroutineScope),
          addExpression1);

  SgStatement * statement1 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::nelems2, subroutineScope), multiplyExpression1);

  appendStatement (statement1, block);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::pnthrcol,
          subroutineScope), variableDeclarations->getReference (
          OP2::VariableNames::blockID));

  SgStatement * statement2 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::numberOfColours, subroutineScope), arrayExpression2);

  appendStatement (statement2, block);

  return block;
}

SgBasicBlock *
CPPCUDAKernelSubroutineIndirectLoop::createSetNumberOfIndirectElementsPerBlockStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise sizes of indirect OP_DATs",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  unsigned int offset = 0;

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        SgMultiplyOp * multiplyExpression = buildMultiplyOp (
            variableDeclarations->getReference (OP2::VariableNames::blockID),
            buildIntVal (
                parallelLoop->getNumberOfDistinctIndirectOpDatArguments ()));

        SgAddOp * addExpression = buildAddOp (buildIntVal (offset),
            multiplyExpression);

        SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
            variableDeclarations->getReference (
                OP2::VariableNames::PlanFunction::pindSizes), addExpression);

        SgStatement * statement = buildAssignStatement (
            variableDeclarations->getReference (
                OP2::VariableNames::getIndirectionArgumentSizeName (i)),
            arrayExpression);

        appendStatement (statement, block);

        ++offset;
      }
    }
  }

  return block;
}

SgIfStmt *
CPPCUDAKernelSubroutineIndirectLoop::createThreadZeroStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  Debug::getInstance ()->debugMessage ("Creating thread zero statements",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * ifBlock = buildBasicBlock ();

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgAddOp * arrayIndexExpression1 = buildAddOp (CUDA::getBlockId (BLOCK_X,
      subroutineScope), variableDeclarations->getReference (
      OP2::VariableNames::PlanFunction::blockOffset));

  SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
      variableDeclarations->getReference (
          OP2::VariableNames::PlanFunction::pblkMap), arrayIndexExpression1);

  SgStatement * statement1 = buildAssignStatement (
      variableDeclarations->getReference (OP2::VariableNames::blockID),
      arrayExpression1);

  appendStatement (statement1, ifBlock);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::pnelems,
          subroutineScope), variableDeclarations->getReference (
          OP2::VariableNames::blockID));

  SgStatement * statement2 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::nelems, subroutineScope), arrayExpression2);

  appendStatement (statement2, ifBlock);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  if (parallelLoop->hasIncrementedOpDats ())
  {
    appendStatement (createIncrementAccessThreadZeroStatements (), ifBlock);
  }

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (
      buildOpaqueVarRefExp (OP2::VariableNames::PlanFunction::poffset,
          subroutineScope), variableDeclarations->getReference (
          OP2::VariableNames::blockID));

  SgStatement * statement3 = buildAssignStatement (buildOpaqueVarRefExp (
      OP2::VariableNames::sharedMemoryOffset, subroutineScope),
      arrayExpression3);

  appendStatement (statement3, ifBlock);

  appendStatement (createSetNumberOfIndirectElementsPerBlockStatements (),
      ifBlock);

  appendStatement (createSetIndirectionMapPointerStatements (), ifBlock);

  appendStatement (createSetOpDatSharedMemoryPointerStatements (), ifBlock);

  /*
   * ======================================================
   * Add the if statement with a NULL else block
   * ======================================================
   */

  SgEqualityOp * ifGuardExpression = buildEqualityOp (CUDA::getThreadId (
      THREAD_X, subroutineScope), buildIntVal (0));

  SgIfStmt * ifStatement =
      RoseStatementsAndExpressionsBuilder::buildIfStatementWithEmptyElse (
          ifGuardExpression, ifBlock);

  return ifStatement;
}

void
CPPCUDAKernelSubroutineIndirectLoop::createStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;

  appendStatement (createThreadZeroStatements (), subroutineScope);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  appendStatement (createInitialiseCUDASharedVariablesStatements (),
      subroutineScope);

  appendStatement (buildExprStatement (
      CUDA::createDeviceThreadSynchronisationCallStatement (subroutineScope)),
      subroutineScope);

  appendStatement (createIncrementAndWriteAccessEpilogueStatements (),
      subroutineScope);
}

void
CPPCUDAKernelSubroutineIndirectLoop::createIncrementAccessLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for incremented OP_DATS",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration
      * variableDeclaration1 =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              OP2::VariableNames::numberOfColours, buildIntType (),
              subroutineScope);

  variableDeclarations->add (OP2::VariableNames::numberOfColours,
      variableDeclaration1);

  variableDeclaration1->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  SgVariableDeclaration * variableDeclaration2 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::nelems2, buildIntType (), subroutineScope);

  variableDeclaration2->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  variableDeclarations->add (OP2::VariableNames::nelems2, variableDeclaration2);

  variableDeclarations ->add (OP2::VariableNames::colour1,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::colour1, buildIntType (), subroutineScope));

  variableDeclarations ->add (OP2::VariableNames::colour2,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::colour2, buildIntType (), subroutineScope));

  variableDeclarations ->add (CommonVariableNames::iterationCounter2,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter2, buildIntType (),
          subroutineScope));
}

void
CPPCUDAKernelSubroutineIndirectLoop::createExecutionLocalVariableDeclarations ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed to execute kernel",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgVariableDeclaration * variableDeclaration1 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::sharedMemoryOffset, buildIntType (),
          subroutineScope);

  variableDeclaration1->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  variableDeclarations->add (OP2::VariableNames::sharedMemoryOffset,
      variableDeclaration1);

  SgVariableDeclaration * variableDeclaration2 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::nelems, buildIntType (), subroutineScope);

  variableDeclaration2->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  variableDeclarations->add (OP2::VariableNames::nelems, variableDeclaration2);

  SgVariableDeclaration * variableDeclaration3 =
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::nbytes, buildIntType (), subroutineScope);

  variableDeclaration3->get_declarationModifier ().get_storageModifier ().setCudaShared ();

  variableDeclarations ->add (OP2::VariableNames::nbytes, variableDeclaration3);

  variableDeclarations->add (OP2::VariableNames::blockID,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          OP2::VariableNames::blockID, buildIntType (), subroutineScope));

  variableDeclarations ->add (CommonVariableNames::iterationCounter1,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::iterationCounter1, buildIntType (),
          subroutineScope));

  variableDeclarations ->add (CommonVariableNames::upperBound,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CommonVariableNames::upperBound, buildIntType (), subroutineScope));
}

void
CPPCUDAKernelSubroutineIndirectLoop::createLocalVariableDeclarations ()
{
  using namespace SageBuilder;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  createCUDAStageInVariablesVariableDeclarations ();

  createCUDASharedVariableDeclarations ();

  createExecutionLocalVariableDeclarations ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating shared indirection mapping for OP_DAT " + lexical_cast <
                string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName = OP2::VariableNames::getIndirectionMapName (
            i);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (buildIntType ()),
                subroutineScope);

        variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaShared ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating indirection size argument for OP_DAT " + lexical_cast <
                string> (i), Debug::INNER_LOOP_LEVEL, __FILE__, __LINE__);

        string const variableName =
            OP2::VariableNames::getIndirectionArgumentSizeName (i);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildIntType (), subroutineScope);

        variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaShared ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating shared memory pointer for OP_DAT "
                + lexical_cast <string> (i), Debug::INNER_LOOP_LEVEL, __FILE__,
            __LINE__);

        string const variableName =
            OP2::VariableNames::getIndirectionCUDASharedMemoryName (i);

        SgVariableDeclaration * variableDeclaration =
            RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope);

        variableDeclaration->get_declarationModifier ().get_storageModifier ().setCudaShared ();

        variableDeclarations->add (variableName, variableDeclaration);
      }
    }
  }

  if (parallelLoop->hasIncrementedOpDats ())
  {
    createIncrementAccessLocalVariableDeclarations ();
  }
}

void
CPPCUDAKernelSubroutineIndirectLoop::createPlanFormalParameterDeclarations ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating plan formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::pindSizes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::pindSizes, buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::pindOffs,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::pindOffs, buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::pblkMap,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::pblkMap, buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::poffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::poffset, buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::pnelems,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::pnelems, buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::pnthrcol,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::pnthrcol, buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::pthrcol,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::pthrcol, buildPointerType (
              buildIntType ()), subroutineScope, formalParameters));

  variableDeclarations->add (
      OP2::VariableNames::PlanFunction::blockOffset,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          OP2::VariableNames::PlanFunction::blockOffset, buildIntType (),
          subroutineScope, formalParameters));
}

void
CPPCUDAKernelSubroutineIndirectLoop::createOpDatFormalParameterDeclarations ()
{
  using namespace SageBuilder;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating OP_DAT formal parameters",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i))
      {
        string const & variableName =
            OP2::VariableNames::getReductionArrayDeviceName (i);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope, formalParameters));
      }
      else if (parallelLoop->isIndirect (i) || parallelLoop->isDirect (i))
      {
        string const variableName = OP2::VariableNames::getOpDatName (i);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (parallelLoop->getOpDatBaseType (
                    i)), subroutineScope, formalParameters));
      }
      else if (parallelLoop->isRead (i))
      {
        string const & variableName = OP2::VariableNames::getOpDatName (i);

        if (parallelLoop->isPointer (i))
        {
          variableDeclarations->add (
              variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  variableName, buildPointerType (
                      parallelLoop->getOpDatBaseType (i)), subroutineScope,
                  formalParameters));
        }
        else
        {
          variableDeclarations->add (
              variableName,
              RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                  variableName, parallelLoop->getOpDatBaseType (i),
                  subroutineScope, formalParameters));

        }
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isIndirect (i))
      {
        string const & variableName =
            OP2::VariableNames::getLocalToGlobalMappingName (i);

        variableDeclarations->add (
            variableName,
            RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
                variableName, buildPointerType (buildIntType ()),
                subroutineScope, formalParameters));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isIndirect (i))
    {
      string const & variableName =
          OP2::VariableNames::getGlobalToLocalMappingName (i);

      variableDeclarations->add (
          variableName,
          RoseStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, buildPointerType (buildShortType ()),
              subroutineScope, formalParameters));
    }
  }
}

void
CPPCUDAKernelSubroutineIndirectLoop::createFormalParameterDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating formal parameter declarations", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  createOpDatFormalParameterDeclarations ();

  createPlanFormalParameterDeclarations ();
}

CPPCUDAKernelSubroutineIndirectLoop::CPPCUDAKernelSubroutineIndirectLoop (
    SgScopeStatement * moduleScope, CPPCUDAUserSubroutine * userSubroutine,
    CPPParallelLoop * parallelLoop,
    CPPCUDAReductionSubroutines * reductionSubroutines) :
  CPPCUDAKernelSubroutine (moduleScope, userSubroutine, parallelLoop,
      reductionSubroutines)
{
  Debug::getInstance ()->debugMessage (
      "Creating kernel subroutine of indirect loop", Debug::CONSTRUCTOR_LEVEL,
      __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}

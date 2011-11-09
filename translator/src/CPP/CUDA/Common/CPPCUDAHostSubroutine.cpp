#include <CPPCUDAHostSubroutine.h>
#include <CPPCUDAKernelSubroutine.h>
#include <CPPModuleDeclarations.h>
#include <RoseStatementsAndExpressionsBuilder.h>
#include <CUDA.h>
#include <CommonNamespaces.h>
#include <Exceptions.h>

SgForStatement *
CPPCUDAHostSubroutine::createReductionUpdateStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to update reduction variable",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * loopBody1 = buildBasicBlock ();

  if (parallelLoop->isArray (OP_DAT_ArgumentGroup) || parallelLoop->isPointer (
      OP_DAT_ArgumentGroup))
  {
    SgBasicBlock * loopBody2 = buildBasicBlock ();

    SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
        variableDeclarations->getReference (getReductionArrayHostName (
            OP_DAT_ArgumentGroup)), variableDeclarations->getReference (
            getIterationCounterVariableName (2)));

    SgMultiplyOp * multiplyExpression =
        buildMultiplyOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)), buildIntVal (
            parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

    SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (2)), multiplyExpression);

    SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
        variableDeclarations->getReference (getReductionArrayHostName (
            OP_DAT_ArgumentGroup)), addExpression);

    if (parallelLoop->isIncremented (OP_DAT_ArgumentGroup))
    {
      SgPlusAssignOp * plusPlusExpression = buildPlusAssignOp (
          arrayExpression1, arrayExpression2);

      appendStatement (buildExprStatement (plusPlusExpression), loopBody2);
    }
    else if (parallelLoop->isMaximised (OP_DAT_ArgumentGroup))
    {
      SgPointerDerefExp * pointerDerefenceExpression2 = buildPointerDerefExp (
          variableDeclarations->getReference (getReductionArrayHostName (
              OP_DAT_ArgumentGroup)));

      SgFunctionCallExp * maxCallExpression =
          OP2::CPPMacroSupport::createMaxCallStatement (subroutineScope,
              arrayExpression1, arrayExpression2);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression1, maxCallExpression);

      appendStatement (assignmentStatement, loopBody2);
    }
    else if (parallelLoop->isMinimised (OP_DAT_ArgumentGroup))
    {
      SgPointerDerefExp * pointerDerefenceExpression2 = buildPointerDerefExp (
          variableDeclarations->getReference (getReductionArrayHostName (
              OP_DAT_ArgumentGroup)));

      SgFunctionCallExp * minCallExpression =
          OP2::CPPMacroSupport::createMinCallStatement (subroutineScope,
              arrayExpression1, arrayExpression2);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          arrayExpression1, minCallExpression);

      appendStatement (assignmentStatement, loopBody2);
    }

    SgAssignOp * initialisationExpression2 =
        buildAssignOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)), buildIntVal (0));

    SgLessThanOp * upperBoundExpression2 =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)), buildIntVal (
            parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

    SgPlusPlusOp * strideExpression2 =
        buildPlusPlusOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)));

    SgForStatement * forStatement2 = buildForStatement (buildExprStatement (
        initialisationExpression2), buildExprStatement (upperBoundExpression2),
        strideExpression2, loopBody2);

    appendStatement (forStatement2, loopBody1);
  }
  else
  {
    SgPointerDerefExp * pointerDerefenceExpression1 = buildPointerDerefExp (
        variableDeclarations->getReference (getReductionArrayHostName (
            OP_DAT_ArgumentGroup)));

    SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (
        variableDeclarations->getReference (getReductionArrayHostName (
            OP_DAT_ArgumentGroup)), variableDeclarations->getReference (
            getIterationCounterVariableName (1)));

    if (parallelLoop->isIncremented (OP_DAT_ArgumentGroup))
    {
      SgPlusAssignOp * plusPlusExpression = buildPlusAssignOp (
          pointerDerefenceExpression1, arrayExpression);

      appendStatement (buildExprStatement (plusPlusExpression), loopBody1);
    }
    else if (parallelLoop->isMaximised (OP_DAT_ArgumentGroup))
    {
      SgPointerDerefExp * pointerDerefenceExpression2 = buildPointerDerefExp (
          variableDeclarations->getReference (getReductionArrayHostName (
              OP_DAT_ArgumentGroup)));

      SgFunctionCallExp * maxCallExpression =
          OP2::CPPMacroSupport::createMaxCallStatement (subroutineScope,
              pointerDerefenceExpression2, arrayExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          pointerDerefenceExpression1, maxCallExpression);

      appendStatement (assignmentStatement, loopBody1);
    }
    else if (parallelLoop->isMinimised (OP_DAT_ArgumentGroup))
    {
      SgPointerDerefExp * pointerDerefenceExpression2 = buildPointerDerefExp (
          variableDeclarations->getReference (getReductionArrayHostName (
              OP_DAT_ArgumentGroup)));

      SgFunctionCallExp * minCallExpression =
          OP2::CPPMacroSupport::createMinCallStatement (subroutineScope,
              pointerDerefenceExpression2, arrayExpression);

      SgExprStatement * assignmentStatement = buildAssignStatement (
          pointerDerefenceExpression1, minCallExpression);

      appendStatement (assignmentStatement, loopBody1);
    }
  }

  SgAssignOp * initialisationExpression1 = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression1 = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (CUDA::blocksPerGrid));

  SgPlusPlusOp * strideExpression1 = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgForStatement * forStatement1 = buildForStatement (buildExprStatement (
      initialisationExpression1), buildExprStatement (upperBoundExpression1),
      strideExpression1, loopBody1);

  return forStatement1;
}

void
CPPCUDAHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  SgFunctionCallExp
      * moveReductionArraysToHostCall =
          CUDA::CPPRuntimeSupport::getMoveReductionArraysFromDeviceToHostCallStatement (
              subroutineScope, variableDeclarations->getReference (
                  reductionBytes));

  appendStatement (buildExprStatement (moveReductionArraysToHostCall),
      subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      appendStatement (createReductionUpdateStatements (i), subroutineScope);
    }
  }
}

SgForStatement *
CPPCUDAHostSubroutine::createReductionInitialisationStatements (
    unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise reduction variable",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  /*
   * ======================================================
   * For loop statement to initialise reduction array
   * ======================================================
   */

  SgBasicBlock * loopBody1 = buildBasicBlock ();

  SgExpression * rhsExpression;

  if (parallelLoop->isIncremented (OP_DAT_ArgumentGroup))
  {
    if (isSgTypeInt (parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup)))
    {
      rhsExpression = buildIntVal (0);
    }
    else if (isSgTypeFloat (parallelLoop->getOpDatBaseType (
        OP_DAT_ArgumentGroup)))
    {
      rhsExpression = buildFloatVal (0);
    }
    else if (isSgTypeDouble (parallelLoop->getOpDatBaseType (
        OP_DAT_ArgumentGroup)))
    {
      rhsExpression = buildDoubleVal (0);
    }
    else
    {
      throw Exceptions::ParallelLoop::UnsupportedBaseTypeException (
          "Reduction type not supported");
    }
  }
  else
  {
    rhsExpression = variableDeclarations->getReference (getOpDatName (
        OP_DAT_ArgumentGroup));
  }

  if (parallelLoop->isArray (OP_DAT_ArgumentGroup) || parallelLoop->isPointer (
      OP_DAT_ArgumentGroup))
  {
    SgBasicBlock * loopBody2 = buildBasicBlock ();

    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (dataOnHost,
        subroutineScope));

    SgCastExp * castExpression = buildCastExp (dotExpression, buildPointerType (
        parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup)));

    SgMultiplyOp * multiplyExpression =
        buildMultiplyOp (variableDeclarations->getReference (
            getIterationCounterVariableName (1)), buildIntVal (
            parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

    SgAddOp * addExpression = buildAddOp (variableDeclarations->getReference (
        getIterationCounterVariableName (2)), multiplyExpression);

    SgPntrArrRefExp * arrayExpression = buildPntrArrRefExp (castExpression,
        addExpression);

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayExpression, rhsExpression);

    appendStatement (assignmentStatement, loopBody2);

    SgAssignOp * initialisationExpression2 =
        buildAssignOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)), buildIntVal (0));

    SgLessThanOp * upperBoundExpression2 =
        buildLessThanOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)), buildIntVal (
            parallelLoop->getOpDatDimension (OP_DAT_ArgumentGroup)));

    SgPlusPlusOp * strideExpression2 =
        buildPlusPlusOp (variableDeclarations->getReference (
            getIterationCounterVariableName (2)));

    SgForStatement * forStatement2 = buildForStatement (buildExprStatement (
        initialisationExpression2), buildExprStatement (upperBoundExpression2),
        strideExpression2, loopBody2);

    appendStatement (forStatement2, loopBody1);
  }
  else
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (dataOnHost,
        subroutineScope));

    SgCastExp * castExpression = buildCastExp (dotExpression, buildPointerType (
        parallelLoop->getOpDatBaseType (OP_DAT_ArgumentGroup)));

    SgPntrArrRefExp * arrayExpression =
        buildPntrArrRefExp (castExpression, variableDeclarations->getReference (
            getIterationCounterVariableName (1)));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        arrayExpression, rhsExpression);

    appendStatement (assignmentStatement, loopBody1);
  }

  SgAssignOp * initialisationExpression1 = buildAssignOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      buildIntVal (0));

  SgLessThanOp * upperBoundExpression1 = buildLessThanOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)),
      variableDeclarations->getReference (CUDA::blocksPerGrid));

  SgPlusPlusOp * strideExpression1 = buildPlusPlusOp (
      variableDeclarations->getReference (getIterationCounterVariableName (1)));

  SgForStatement * forStatement1 = buildForStatement (buildExprStatement (
      initialisationExpression1), buildExprStatement (upperBoundExpression1),
      strideExpression1, loopBody1);

  return forStatement1;
}

void
CPPCUDAHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      variableDeclarations->getReference (reductionBytes), buildIntVal (0));

  appendStatement (assignmentStatement1, subroutineScope);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      variableDeclarations->getReference (reductionSharedMemorySize),
      buildIntVal (0));

  appendStatement (assignmentStatement2, subroutineScope);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      SgSizeOfOp * sizeOfExpression = buildSizeOfOp (
          parallelLoop->getOpDatBaseType (i));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (CUDA::blocksPerGrid),
          sizeOfExpression);

      SgFunctionCallExp * roundUpCallExpression;

      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            multiplyExpression1, buildIntVal (parallelLoop->getOpDatDimension (
                i)));

        roundUpCallExpression
            = OP2::CPPMacroSupport::createRoundUpCallStatement (
                subroutineScope, multiplyExpression2);
      }
      else
      {
        roundUpCallExpression
            = OP2::CPPMacroSupport::createRoundUpCallStatement (
                subroutineScope, multiplyExpression1);
      }

      SgPlusAssignOp * assignmentStatement3 = buildPlusAssignOp (
          variableDeclarations->getReference (reductionBytes),
          roundUpCallExpression);

      appendStatement (buildExprStatement (assignmentStatement3),
          subroutineScope);

      SgFunctionCallExp * maxCallExpression =
          OP2::CPPMacroSupport::createMaxCallStatement (subroutineScope,
              variableDeclarations->getReference (reductionSharedMemorySize),
              sizeOfExpression);

      SgExprStatement * assignmentStatement4 = buildAssignStatement (
          variableDeclarations->getReference (reductionSharedMemorySize),
          maxCallExpression);

      appendStatement (assignmentStatement4, subroutineScope);
    }
  }

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgFunctionCallExp * reallocateReductionArraysExpression =
      CUDA::CPPRuntimeSupport::getReallocateReductionArraysCallStatement (
          subroutineScope, variableDeclarations->getReference (reductionBytes));

  appendStatement (buildExprStatement (reallocateReductionArraysExpression),
      subroutineScope);

  /*
   * ======================================================
   * New statement
   * ======================================================
   */

  SgExprStatement * assignmentStatement5 = buildAssignStatement (
      variableDeclarations->getReference (reductionBytes), buildIntVal (0));

  appendStatement (assignmentStatement5, subroutineScope);

  /*
   * ======================================================
   * New statements
   * ======================================================
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      SgAddOp
          * addExpression1 =
              buildAddOp (
                  CUDA::CPPRuntimeSupport::getPointerToMemoryAllocatedForHostReductionArray (
                      subroutineScope), variableDeclarations->getReference (
                      reductionBytes));

      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dataOnHost, subroutineScope));

      SgExprStatement * assignmentStatement4 = buildAssignStatement (
          dotExpression1, addExpression1);

      appendStatement (assignmentStatement4, subroutineScope);

      SgAddOp
          * addExpression2 =
              buildAddOp (
                  CUDA::CPPRuntimeSupport::getPointerToMemoryAllocatedForDeviceReductionArray (
                      subroutineScope), variableDeclarations->getReference (
                      reductionBytes));

      SgDotExp * dotExpression2 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dataOnDevice, subroutineScope));

      SgExprStatement * assignmentStatement5 = buildAssignStatement (
          dotExpression2, addExpression2);

      appendStatement (assignmentStatement5, subroutineScope);

      appendStatement (createReductionInitialisationStatements (i),
          subroutineScope);

      /*
       * ======================================================
       * New statement
       * ======================================================
       */

      SgSizeOfOp * sizeOfExpression = buildSizeOfOp (
          parallelLoop->getOpDatBaseType (i));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (CUDA::blocksPerGrid),
          sizeOfExpression);

      SgFunctionCallExp * roundUpCallExpression;

      if (parallelLoop->isArray (i) || parallelLoop->isPointer (i))
      {
        SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
            multiplyExpression1, buildIntVal (parallelLoop->getOpDatDimension (
                i)));

        roundUpCallExpression
            = OP2::CPPMacroSupport::createRoundUpCallStatement (
                subroutineScope, multiplyExpression2);
      }
      else
      {
        roundUpCallExpression
            = OP2::CPPMacroSupport::createRoundUpCallStatement (
                subroutineScope, multiplyExpression1);
      }

      SgPlusAssignOp * assignmentStatement6 = buildPlusAssignOp (
          variableDeclarations->getReference (reductionBytes),
          roundUpCallExpression);

      appendStatement (buildExprStatement (assignmentStatement6),
          subroutineScope);
    }
  }

  SgFunctionCallExp
      * moveReductionArraysToDeviceCall =
          CUDA::CPPRuntimeSupport::getMoveReductionArraysFromHostToDeviceCallStatement (
              subroutineScope, variableDeclarations->getReference (
                  reductionBytes));

  appendStatement (buildExprStatement (moveReductionArraysToDeviceCall),
      subroutineScope);
}

void
CPPCUDAHostSubroutine::createReductionDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2::VariableNames;
  using namespace CommonVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (
      getIterationCounterVariableName (1),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1), buildIntType (), subroutineScope));

  variableDeclarations->add (
      getIterationCounterVariableName (2),
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2), buildIntType (), subroutineScope));

  variableDeclarations->add (reductionBytes,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          reductionBytes, buildIntType (), subroutineScope));

  variableDeclarations->add (reductionSharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          reductionSharedMemorySize, buildIntType (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      string const reductionArrayHostName = getReductionArrayHostName (i);

      Debug::getInstance ()->debugMessage ("Creating host reduction pointer '"
          + reductionArrayHostName + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgVariableDeclaration * reductionArrayHost =
          RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayHostName, buildPointerType (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope);

      variableDeclarations->add (reductionArrayHostName, reductionArrayHost);
    }
  }
}

void
CPPCUDAHostSubroutine::createCUDAConfigurationLaunchDeclarations ()
{
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration launch local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (CUDA::blocksPerGrid,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::blocksPerGrid, buildIntType (), subroutineScope));

  variableDeclarations->add (CUDA::threadsPerBlock,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::threadsPerBlock, buildIntType (), subroutineScope));

  variableDeclarations->add (CUDA::sharedMemorySize,
      RoseStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::sharedMemorySize, buildIntType (), subroutineScope));
}

CPPCUDAHostSubroutine::CPPCUDAHostSubroutine (SgScopeStatement * moduleScope,
    CPPCUDAKernelSubroutine * calleeSubroutine, CPPParallelLoop * parallelLoop,
    CPPModuleDeclarations * moduleDeclarations) :
  CPPHostSubroutine (moduleScope, calleeSubroutine, parallelLoop),
      moduleDeclarations (moduleDeclarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
}

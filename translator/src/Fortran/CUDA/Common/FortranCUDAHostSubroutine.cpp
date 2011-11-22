#include "FortranCUDAHostSubroutine.h"
#include "FortranKernelSubroutine.h"
#include "FortranParallelLoop.h"
#include "FortranCUDAOpDatCardinalitiesDeclaration.h"
#include "FortranCUDAModuleDeclarations.h"
#include "FortranOpDatDimensionsDeclaration.h"
#include "FortranStatementsAndExpressionsBuilder.h"
#include "FortranTypesBuilder.h"
#include "RoseStatementsAndExpressionsBuilder.h"
#include "Debug.h"
#include "CompilerGeneratedNames.h"
#include "OP2.h"
#include "CUDA.h"

void
FortranCUDAHostSubroutine::createReductionPrologueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating reduction prologue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          variableDeclarations->getReference (CUDA::blocksPerGrid),
          buildIntVal (parallelLoop->getOpDatDimension (i)));

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          variableDeclarations->getReference (getReductionCardinalityName (i)),
          multiplyExpression1);

      appendStatement (assignmentStatement1, subroutineScope);

      SgSubtractOp * upperBoundExpression1 = buildSubtractOp (
          variableDeclarations->getReference (getReductionCardinalityName (i)),
          buildIntVal (1));

      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          buildIntVal (0), upperBoundExpression1, subroutineScope);

      FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
          variableDeclarations->getReference (getReductionArrayDeviceName (i)),
          buildIntVal (0), upperBoundExpression1, subroutineScope);

      SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
          variableDeclarations->getReference (getReductionArrayHostName (i)),
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)));

      SgExprStatement * assignmentStatement2;

      if (isSgTypeInt (parallelLoop->getOpDatBaseType (i)))
      {
        assignmentStatement2 = buildAssignStatement (arrayIndexExpression1,
            buildIntVal (0));
      }
      else
      {
        assignmentStatement2 = buildAssignStatement (arrayIndexExpression1,
            buildFloatVal (0));
      }

      SgBasicBlock * loopBody = buildBasicBlock (assignmentStatement2);

      SgAssignOp * loopInitialiserExpression = buildAssignOp (
          variableDeclarations->getReference (getIterationCounterVariableName (
              1)), buildIntVal (0));

      SgSubtractOp * upperBoundExpression2 = buildSubtractOp (
          variableDeclarations->getReference (getReductionCardinalityName (i)),
          buildIntVal (1));

      SgFortranDo * loopStatement =
          FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
              loopInitialiserExpression, upperBoundExpression2,
              buildIntVal (1), loopBody);

      appendStatement (loopStatement, subroutineScope);

      SgExprStatement * assignmentStatement3 = buildAssignStatement (
          variableDeclarations->getReference (getReductionArrayDeviceName (i)),
          variableDeclarations->getReference (getReductionArrayHostName (i)));

      appendStatement (assignmentStatement3, subroutineScope);
    }
  }
}

void
FortranCUDAHostSubroutine::createReductionEpilogueStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating reduction epilogue statements", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i) == true)
    {
      if (parallelLoop->getOpDatDimension (i) == 1)
      {
        Debug::getInstance ()->debugMessage (
            "Creating statements for OP_DAT argument '"
                + lexical_cast <string> (i) + "'", Debug::FUNCTION_LEVEL,
            __FILE__, __LINE__);

        /*
         * ======================================================
         * Assign device reduction array to host reduction array
         * ======================================================
         */

        SgExprStatement * assignmentStatement1 =
            buildAssignStatement (variableDeclarations->getReference (
                getReductionArrayHostName (i)),
                variableDeclarations->getReference (
                    getReductionArrayDeviceName (i)));

        appendStatement (assignmentStatement1, subroutineScope);

        /*
         * ======================================================
         * Iterate over all elements in the reduction host array
         * and compute either the sum, maximum, or minimum
         * ======================================================
         */

        SgPntrArrRefExp * arrayIndexExpression1 = buildPntrArrRefExp (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)));

        SgExpression * reductionComputationExpression;

        if (parallelLoop->isIncremented (i))
        {
          reductionComputationExpression = buildAddOp (arrayIndexExpression1,
              variableDeclarations->getReference (getOpDatHostName (i)));
        }
        else if (parallelLoop->isMaximised (i))
        {
          SgFunctionSymbol * maxFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("max",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, variableDeclarations->getReference (
                  getOpDatHostName (i)));

          reductionComputationExpression = buildFunctionCallExp (
              maxFunctionSymbol, actualParameters);
        }
        else if (parallelLoop->isMinimised (i))
        {
          SgFunctionSymbol * maxFunctionSymbol =
              FortranTypesBuilder::buildNewFortranFunction ("min",
                  subroutineScope);

          SgExprListExp * actualParameters = buildExprListExp (
              arrayIndexExpression1, variableDeclarations->getReference (
                  getOpDatHostName (i)));

          reductionComputationExpression = buildFunctionCallExp (
              maxFunctionSymbol, actualParameters);
        }

        ROSE_ASSERT (reductionComputationExpression != NULL);

        SgExprStatement * assignmentStatement3 = buildAssignStatement (
            variableDeclarations->getReference (getOpDatHostName (i)),
            reductionComputationExpression);

        SgBasicBlock * loopBody = buildBasicBlock ();

        appendStatement (assignmentStatement3, loopBody);

        SgAssignOp * loopInitialiserExpression = buildAssignOp (
            variableDeclarations->getReference (
                getIterationCounterVariableName (1)), buildIntVal (0));

        SgSubtractOp * upperBoundExpression =
            buildSubtractOp (variableDeclarations->getReference (
                getReductionCardinalityName (i)), buildIntVal (1));

        SgFortranDo * loopStatement =
            FortranStatementsAndExpressionsBuilder::buildFortranDoStatement (
                loopInitialiserExpression, upperBoundExpression,
                buildIntVal (1), loopBody);

        appendStatement (loopStatement, subroutineScope);

        FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
            variableDeclarations->getReference (getReductionArrayHostName (i)),
            subroutineScope);

        FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
            variableDeclarations->getReference (getReductionArrayDeviceName (i)),
            subroutineScope);
      }
    }
  }
}

void
FortranCUDAHostSubroutine::createReductionDeclarations ()
{
  using namespace OP2VariableNames;
  using namespace ReductionVariableNames;
  using namespace LoopVariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations needed for reduction",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (getIterationCounterVariableName (1),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (1),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  variableDeclarations->add (getIterationCounterVariableName (2),
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          getIterationCounterVariableName (2),
          FortranTypesBuilder::getFourByteInteger (), subroutineScope));

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isReductionRequired (i))
    {
      string const reductionArrayHostName = getReductionArrayHostName (i);

      Debug::getInstance ()->debugMessage ("Creating host reduction array '"
          + reductionArrayHostName + "'", Debug::HIGHEST_DEBUG_LEVEL, __FILE__,
          __LINE__);

      SgVariableDeclaration * reductionArrayHost =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayHostName, FortranTypesBuilder::getArray_RankOne (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope, 1,
              ALLOCATABLE);

      variableDeclarations->add (reductionArrayHostName, reductionArrayHost);

      string const reductionArrayDeviceName = getReductionArrayDeviceName (i);

      Debug::getInstance ()->debugMessage ("Creating device reduction array '"
          + reductionArrayDeviceName + "'", Debug::HIGHEST_DEBUG_LEVEL,
          __FILE__, __LINE__);

      SgVariableDeclaration * reductionArrayDevice =
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionArrayDeviceName, FortranTypesBuilder::getArray_RankOne (
                  parallelLoop->getOpDatBaseType (i)), subroutineScope, 2,
              ALLOCATABLE, CUDA_DEVICE);

      variableDeclarations->add (reductionArrayDeviceName, reductionArrayDevice);

      string const & reductionCardinalityName = getReductionCardinalityName (i);

      variableDeclarations->add (reductionCardinalityName,
          FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
              reductionCardinalityName,
              FortranTypesBuilder::getFourByteInteger (), subroutineScope));
    }
  }
}

SgExpression *
FortranCUDAHostSubroutine::createRHSOfInitialiseOpDatCardinalityStatement (
    SgScopeStatement * scope, unsigned int OP_DAT_ArgumentGroup)
{
  using namespace SageBuilder;
  using namespace OP2::RunTimeVariableNames;
  using namespace OP2VariableNames;

  if (parallelLoop->isDirect (OP_DAT_ArgumentGroup)
      || parallelLoop->isIndirect (OP_DAT_ArgumentGroup))
  {
    SgDotExp * dotExpression1 =
        buildDotExp (variableDeclarations->getReference (getOpDatName (
            OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (dimension, scope));

    SgDotExp * dotExpression2 =
        buildDotExp (variableDeclarations->getReference (getOpDatName (
            OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (set, scope));

    SgDotExp * dotExpression3 = buildDotExp (dotExpression2,
        buildOpaqueVarRefExp (size, scope));

    SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
        dotExpression3);

    return multiplyExpression;
  }
  else if (parallelLoop->isReductionRequired (OP_DAT_ArgumentGroup))
  {
    if (parallelLoop->isArray (OP_DAT_ArgumentGroup))
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (
              OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (dimension, scope));

      SgDotExp * dotExpression2 = buildDotExp (
          variableDeclarations->getReference (getOpSetName ()),
          buildOpaqueVarRefExp (size, scope));

      SgExpression * multiplyExpression = buildMultiplyOp (dotExpression1,
          dotExpression2);

      return multiplyExpression;
    }
    else
    {
      SgDotExp * dotExpression = buildDotExp (
          variableDeclarations->getReference (getOpSetName ()),
          buildOpaqueVarRefExp (size, scope));

      return dotExpression;
    }
  }
  else if (parallelLoop->isArray (OP_DAT_ArgumentGroup)
      && parallelLoop->isRead (OP_DAT_ArgumentGroup))
  {
    SgDotExp * dotExpression = buildDotExp (variableDeclarations->getReference (
        getOpDatName (OP_DAT_ArgumentGroup)), buildOpaqueVarRefExp (dimension,
        scope));

    return dotExpression;
  }
}

SgBasicBlock *
FortranCUDAHostSubroutine::createDeallocateStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to deallocate global arrays which are read",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobal (i) && parallelLoop->isArray (i)
          && parallelLoop->isRead (i))
      {
        FortranStatementsAndExpressionsBuilder::appendDeallocateStatement (
            variableDeclarations->getReference (getOpDatDeviceName (i)), block);
      }
    }
  }

  return block;
}

SgBasicBlock *
FortranCUDAHostSubroutine::createTransferOpDatStatements ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using namespace OP2::RunTimeVariableNames;

  Debug::getInstance ()->debugMessage (
      "Creating statements to transfer OP_DATs onto device",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  SgBasicBlock * block = buildBasicBlock ();

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT dimensions",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
    {
      SgDotExp * dotExpression1 = buildDotExp (
          variableDeclarations->getReference (opDatDimensions),
          dimensionsDeclaration->getOpDatDimensionField (i));

      SgDotExp * dotExpression2 = buildDotExp (
          variableDeclarations->getReference (getOpDatName (i)),
          buildOpaqueVarRefExp (dimension, block));

      SgExprStatement * assignmentStatement = buildAssignStatement (
          dotExpression1, dotExpression2);

      appendStatement (assignmentStatement, block);
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT cardinalities (on device)",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        SgDotExp * dotExpression = buildDotExp (
            variableDeclarations->getReference (opDatCardinalities),
            cardinalitiesDeclaration->getFieldDeclarations ()->getReference (
                getOpDatCardinalityName (i)));

        SgExpression * rhsOfAssigment =
            createRHSOfInitialiseOpDatCardinalityStatement (block, i);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            dotExpression, rhsOfAssigment);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to initialise OP_DAT cardinalities (on host)",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        SgExpression * rhsOfAssigment =
            createRHSOfInitialiseOpDatCardinalityStatement (block, i);

        SgExprStatement * assignmentStatement = buildAssignStatement (
            variableDeclarations->getReference (getOpDatCardinalityName (i)),
            rhsOfAssigment);

        appendStatement (assignmentStatement, block);
      }
    }
  }

  Debug::getInstance ()->debugMessage (
      "Creating statements to convert OP_DATs between C and Fortran pointers",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
      {
        /*
         * ======================================================
         * Statement to convert OP_DAT between C and Fortran
         * pointers
         * ======================================================
         */

        Debug::getInstance ()->debugMessage (
            "Direct/Indirect/Global non-scalar conversion",
            Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

        SgDotExp * parameterExpression1A = buildDotExp (
            variableDeclarations->getReference (getOpDatName (i)),
            buildOpaqueVarRefExp (dataOnDevice, block));

        SgVarRefExp * parameterExpression2A =
            variableDeclarations->getReference (getOpDatDeviceName (i));

        SgAggregateInitializer
            * parameterExpression3A =
                FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                    variableDeclarations->getReference (
                        getOpDatCardinalityName (i)));

        SgStatement
            * callStatementA =
                FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                    subroutineScope, parameterExpression1A,
                    parameterExpression2A, parameterExpression3A);

        appendStatement (callStatementA, block);

      }
      else if (parallelLoop->isReductionRequired (i))
      {
        if (parallelLoop->isArray (i))
        {

        }
        else
        {
          Debug::getInstance ()->debugMessage ("Global scalar conversion",
              Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

          SgDotExp * parameterExpression1A = buildDotExp (
              variableDeclarations->getReference (getOpDatName (i)),
              buildOpaqueVarRefExp (dataOnHost, block));

          SgVarRefExp * parameterExpression1B =
              variableDeclarations->getReference (getOpDatHostName (i));

          SgStatement
              * callStatementA =
                  FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                      subroutineScope, parameterExpression1A,
                      parameterExpression1B);

          appendStatement (callStatementA, block);
        }
      }
      else
      {
        if (parallelLoop->isRead (i))
        {
          if (parallelLoop->isArray (i))
          {
            Debug::getInstance ()->debugMessage ("Global array conversion",
                Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

            SgDotExp * parameterExpression1A = buildDotExp (
                variableDeclarations->getReference (getOpDatName (i)),
                buildOpaqueVarRefExp (dataOnHost, block));

            SgVarRefExp * parameterExpression1B =
                variableDeclarations->getReference (getOpDatHostName (i));

            SgAggregateInitializer * parameterExpression1C =
                FortranStatementsAndExpressionsBuilder::buildShapeExpression (
                    variableDeclarations->getReference (
                        getOpDatCardinalityName (i)));

            SgStatement
                * callStatementA =
                    FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                        subroutineScope, parameterExpression1A,
                        parameterExpression1B, parameterExpression1C);

            appendStatement (callStatementA, block);

            SgVarRefExp * arrayExpression = variableDeclarations->getReference (
                getOpDatDeviceName (i));

            SgIntVal * lowerBound = buildIntVal (0);

            SgVarRefExp * upperBound = variableDeclarations->getReference (
                getOpDatCardinalityName (i));

            FortranStatementsAndExpressionsBuilder::appendAllocateStatement (
                arrayExpression, buildIntVal (1), upperBound, block);

            SgExprStatement * assignmentStatement1 = buildAssignStatement (
                variableDeclarations->getReference (getOpDatDeviceName (i)),
                variableDeclarations->getReference (getOpDatHostName (i)));

            appendStatement (assignmentStatement1, block);
          }
          else
          {
            Debug::getInstance ()->debugMessage ("Global scalar conversion",
                Debug::HIGHEST_DEBUG_LEVEL, __FILE__, __LINE__);

            SgDotExp * parameterExpression1A = buildDotExp (
                variableDeclarations->getReference (getOpDatName (i)),
                buildOpaqueVarRefExp (dataOnHost, block));

            SgVarRefExp * parameterExpression1B =
                variableDeclarations->getReference (getOpDatHostName (i));

            SgStatement
                * callStatementA =
                    FortranStatementsAndExpressionsBuilder::createCToFortranPointerCallStatement (
                        subroutineScope, parameterExpression1A,
                        parameterExpression1B);

            appendStatement (callStatementA, block);
          }
        }
      }
    }
  }

  return block;
}

void
FortranCUDAHostSubroutine::createDataMarshallingDeclarations ()
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace OP2VariableNames;
  using boost::lexical_cast;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Creating local variable declarations for data marshalling",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      FortranParallelLoop * fortranParallelLoop =
          static_cast <FortranParallelLoop *> (parallelLoop);

      if (fortranParallelLoop->isCardinalityDeclarationNeeded (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating OP_DAT size variable for OP_DAT "
                + lexical_cast <string> (i), Debug::HIGHEST_DEBUG_LEVEL,
            __FILE__, __LINE__);

        string const & variableName = getOpDatCardinalityName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getFourByteInteger (),
                subroutineScope));
      }
    }
  }

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isDirect (i) || parallelLoop->isIndirect (i))
      {
        Debug::getInstance ()->debugMessage (
            "Creating device array for OP_DAT " + lexical_cast <string> (i),
            Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

        string const & variableName = getOpDatDeviceName (i);

        variableDeclarations->add (variableName,
            FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                variableName, FortranTypesBuilder::getArray_RankOne (
                    parallelLoop->getOpDatBaseType (i)), subroutineScope, 2,
                CUDA_DEVICE, ALLOCATABLE));
      }
      else if (parallelLoop->isReductionRequired (i))
      {
        if (parallelLoop->isArray (i))
        {
          Debug::getInstance ()->debugMessage (
              "Creating device array for array REDUCTION " + lexical_cast <
                  string> (i), Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

          string const & variableName = getOpDatDeviceName (i);

          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, FortranTypesBuilder::getArray_RankOne (
                      parallelLoop->getOpDatBaseType (i)), subroutineScope, 2,
                  CUDA_DEVICE, ALLOCATABLE));
        }
        else
        {
          Debug::getInstance ()->debugMessage (
              "Creating host scalar pointer for scalar REDUCTION "
                  + lexical_cast <string> (i), Debug::FUNCTION_LEVEL, __FILE__,
              __LINE__);

          string const & variableName = getOpDatHostName (i);

          variableDeclarations->add (
              variableName,
              FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                  variableName, buildPointerType (
                      parallelLoop->getOpDatBaseType (i)), subroutineScope));
        }
      }
      else
      {
        if (parallelLoop->isRead (i))
        {
          if (parallelLoop->isArray (i))
          {
            Debug::getInstance ()->debugMessage (
                "Creating host and device arrays for OP_GBL read "
                    + lexical_cast <string> (i), Debug::FUNCTION_LEVEL,
                __FILE__, __LINE__);

            string const & variableNameOnDevice = getOpDatDeviceName (i);

            Debug::getInstance ()->debugMessage (
                "Creating device array with name " + variableNameOnDevice,
                Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

            variableDeclarations->add (
                variableNameOnDevice,
                FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                    variableNameOnDevice,
                    FortranTypesBuilder::getArray_RankOne (
                        parallelLoop->getOpDatBaseType (i)), subroutineScope,
                    2, CUDA_DEVICE, ALLOCATABLE));

            string const & variableNameOnHost = getOpDatHostName (i);

            Debug::getInstance ()->debugMessage (
                "Creating host array with name " + variableNameOnHost,
                Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

            variableDeclarations->add (
                variableNameOnHost,
                FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                    variableNameOnHost, buildPointerType (
                        FortranTypesBuilder::getArray_RankOne (
                            parallelLoop->getOpDatBaseType (i))),
                    subroutineScope));
          }
          else
          {
            string const & variableName = getOpDatHostName (i);

            variableDeclarations->add (
                variableName,
                FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
                    variableName, buildPointerType (
                        parallelLoop->getOpDatBaseType (i)), subroutineScope));
          }
        }
      }
    }
  }
}

void
FortranCUDAHostSubroutine::createCUDAConfigurationLaunchDeclarations ()
{
  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration launch local variable declarations",
      Debug::FUNCTION_LEVEL, __FILE__, __LINE__);

  variableDeclarations->add (CUDA::blocksPerGrid,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::blocksPerGrid, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::threadsPerBlock,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::threadsPerBlock, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::sharedMemorySize,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::sharedMemorySize, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));

  variableDeclarations->add (CUDA::threadSynchRet,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          CUDA::threadSynchRet, FortranTypesBuilder::getFourByteInteger (),
          subroutineScope));
}

void
FortranCUDAHostSubroutine::createOpDatCardinalitiesDeclaration ()
{
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT cardinalities declaration ", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (opDatCardinalities,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          opDatCardinalities, cardinalitiesDeclaration->getType (),
          subroutineScope, 1, CUDA_DEVICE));
}

void
FortranCUDAHostSubroutine::createOpDatDimensionsDeclaration ()
{
  using namespace OP2VariableNames;

  Debug::getInstance ()->debugMessage (
      "Generating OP_DAT dimensions declaration", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  variableDeclarations->add (opDatDimensions,
      FortranStatementsAndExpressionsBuilder::appendVariableDeclaration (
          opDatDimensions, dimensionsDeclaration->getType (), subroutineScope,
          1, CUDA_DEVICE));
}

FortranCUDAHostSubroutine::FortranCUDAHostSubroutine (
    SgScopeStatement * moduleScope, FortranKernelSubroutine * kernelSubroutine,
    FortranParallelLoop * parallelLoop,
    FortranCUDAOpDatCardinalitiesDeclaration * dataSizesDeclaration,
    FortranOpDatDimensionsDeclaration * opDatDimensionsDeclaration,
    FortranCUDAModuleDeclarations * moduleDeclarations) :
  FortranHostSubroutine (moduleScope, kernelSubroutine, parallelLoop),
      cardinalitiesDeclaration (dataSizesDeclaration), dimensionsDeclaration (
          opDatDimensionsDeclaration), moduleDeclarations (moduleDeclarations)
{
  subroutineHeaderStatement->get_functionModifier ().setCudaHost ();
}

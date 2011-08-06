#include <CPPOpenCLKernelSubroutineDirectLoop.h>
//#include <CPPTypesBuilder.h>
//#include <CPPStatementsAndExpressionsBuilder.h>
//#include <CPPOpenCLReductionSubroutine.h>
#include <RoseHelper.h>
#include <Debug.h>
using namespace SageBuilder; //TODO: remove
/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgStatement *
CPPOpenCLKernelSubroutineDirectLoop::createUserSubroutineCallStatement ()
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
        //FIXME


        SgDotExp * dotExpression = buildDotExp (
            buildVarRefExp (
              variableDeclarations->get ( VariableNames::getDataSizesVariableDeclarationName (userSubroutineName) ) ), 
            buildOpaqueVarRefExp (
              VariableNames::getOpDatSizeName (i), 
              subroutineScope ) );

        SgSubtractOp * subtractExpression = buildSubtractOp (
            dotExpression,
            buildIntVal (1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (
                RoseHelper::getFileInfo (), 
                buildIntVal (0), 
                subtractExpression, 
                buildIntVal (1));

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

        actualParameters->append_expression (
            buildVarRefExp ( variableDeclarations->get (VariableNames::getOpDatLocalName (i))));
      }
    }
    else if (parallelLoop->getOpMapValue (i) == DIRECT)
    {
      if (parallelLoop->getNumberOfIndirectOpDats () > 0)
      {
        SgExpression * deviceVarAccessDirectBegin = 
            buildMultiplyOp (
                    buildAddOp (
                        buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::setElementCounter)),
                        buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock))),
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
                DirectLoop::CPP::KernelSubroutine::setElementCounter)),
            buildIntVal (dim - 1));

        SgSubscriptExpression * arraySubscriptExpression =
            new SgSubscriptExpression (RoseHelper::getFileInfo (),
                buildVarRefExp (variableDeclarations->get (
                    DirectLoop::CPP::KernelSubroutine::setElementCounter)),
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
  
  //needed to have the constants accessible in the user kernel
  // TODO: declare constants struct
  actualParameters->append_expression( 
      buildVarRefExp ( variableDeclarations->get ( OpenCL::CPP::constants ) ) );

  return buildFunctionCallStmt (
      userSubroutineName, 
      buildVoidType (),
      actualParameters, 
      subroutineScope);
}

SgBasicBlock *
CPPOpenCLKernelSubroutineDirectLoop::createStageInFromDeviceMemoryToLocalThreadVariablesStatements ()
{
  //TODO: implement this first!
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
  using SageBuilder::buildForStatement;
  using SageBuilder::buildLessThanOp;
  using SageBuilder::buildPlusPlusOp;
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

      SgDotExp * dotExpression = buildDotExp ( //FIXME
          buildVarRefExp ( variableDeclarations->get ( VariableNames::getDimensionsVariableDeclarationName ( userSubroutineName))), 
          buildVarRefExp ( opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      SgMultiplyOp * multiplyExpression1 = buildMultiplyOp (
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::remainingElements)));

      SgAddOp * addExpression1 = buildAddOp (
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::threadIDModulus)),
          multiplyExpression1);

      SgAddOp * addExpression2 = buildAddOp (
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::autosharedDisplacement)),
          addExpression1);

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::remainingElements)));

      SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock)),
          dotExpression);

      SgAddOp * addExpression3 = buildAddOp (
          multiplyExpression2,
          multiplyExpression3);

      SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
          variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::threadIDModulus)),
          addExpression3);

      SgPntrArrRefExp * arrayExpression1 = buildPntrArrRefExp (
          buildVarRefExp ( variableDeclarations->get (CommonVariableNames::autoshared)),
          addExpression2);

      SgPntrArrRefExp * arrayExpression2 = buildPntrArrRefExp (
          buildVarRefExp ( variableDeclarations->get (VariableNames::getOpDatName (i))),
          addExpression4);
      
      /* ======================================================
       * autoshared[tid + m * nelems] = autoshared[tid + m * nelems + offset * XX]
       * ======================================================
       */
      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          arrayExpression1, 
          arrayExpression2);
      
      SgAssignOp * initialisationExpression = buildAssignOp (
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          buildIntVal (0) );
      
      SgExpression * testExpression = buildLessThanOp(
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          dotExpression );
      
      SgExpression * incrementExpression = buildPlusPlusOp(
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          SgUnaryOp::postfix);
      
      SgBasicBlock * firstLoopBody = buildBasicBlock (assignmentStatement1);
     
      SgForStatement * firstLoopStatement = buildForStatement(
          initialisationExpression,
          testExpression,
          incrementExpression,
          firstLoopBody );

      /* SgCPPDo * firstLoopStatement =
          CPPStatementsAndExpressionsBuilder::buildCPPDoStatement (
              initialisationExpression, dotExpression, buildIntVal (1),
              firstLoopBody); */

      appendStatement (firstLoopStatement, outerBlock);

      /*
       * ======================================================
       * Builds stage in from shared memory to local thread
       * variables
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::threadIDModulus)),
          dotExpression);

      SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          multiplyExpression4);

      SgAddOp * addExpression6 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::autosharedDisplacement)),
          addExpression5);

      SgPntrArrRefExp * arrayExpression3 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (VariableNames::getOpDatLocalName (i))),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)));

      SgPntrArrRefExp * arrayExpression4 = buildPntrArrRefExp (buildVarRefExp (
          variableDeclarations->get (CommonVariableNames::autoshared)),
          addExpression6);

      SgExprStatement * assignmentStatement2 = buildAssignStatement (
          arrayExpression3, arrayExpression4);

      SgBasicBlock * secondLoopBody = buildBasicBlock (assignmentStatement2);
      
      SgForStatement * secondLoopStatement = buildForStatement(
          initialisationExpression,
          testExpression,
          incrementExpression,
          secondLoopBody );

      /*SgCPPDo * secondLoopStatement =
          CPPStatementsAndExpressionsBuilder::buildCPPDoStatement (
              initialisationExpression, dotExpression, buildIntVal (1),
              secondLoopBody); */

      appendStatement (secondLoopStatement, outerBlock);
    }
  }

  return outerBlock;
}

SgBasicBlock *
CPPOpenCLKernelSubroutineDirectLoop::createStageOutFromLocalThreadVariablesToDeviceMemoryStatements ()
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
      SgDotExp * dotExpression = buildDotExp (buildVarRefExp ( //FIXME: set_size (arg)
          variableDeclarations->get (
              VariableNames::getDimensionsVariableDeclarationName (
                  userSubroutineName))), buildVarRefExp (
          opDatDimensionsDeclaration->getOpDatDimensionField (i)));

      /*
       * ======================================================
       * Builds stage out from local thread variables to
       * shared memory
       * ======================================================
       */

      SgExprStatement * assignmentStatement1 = buildAssignStatement (
          buildPntrArrRefExp (
              buildVarRefExp (variableDeclarations->get (CommonVariableNames::autoshared)),
              buildAddOp ( //FIXME: need autoshared_displaced1
                  buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::autosharedDisplacement)),
                  buildAddOp (
                      buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
                      buildMultiplyOp (
                          buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::threadIDModulus)),
                          dotExpression)))),
          buildPntrArrRefExp (
              buildVarRefExp ( variableDeclarations->get (VariableNames::getOpDatLocalName (i))),
              buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter))));

      SgBasicBlock * loopBody1 = buildBasicBlock (assignmentStatement1);

      SgAssignOp * initialisationExpression = buildAssignOp (
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          buildIntVal (0) );
      
      SgExpression * testExpression = buildLessThanOp(
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          dotExpression );
      
      SgExpression * incrementExpression = buildPlusPlusOp(
          buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          SgUnaryOp::postfix);
     
      SgForStatement * loopStatement1 = buildForStatement(
          initialisationExpression,
          testExpression,
          incrementExpression,
          loopBody1 );
      
      /* SgCPPDo * loopStatement1 =
          CPPStatementsAndExpressionsBuilder::buildCPPDoStatement (
              loopInitializationExpression, upperBoundExpression, buildIntVal (
                  1), loopBody1); */

      appendStatement (loopStatement1, outerBlock);

      /*
       * ======================================================
       * Builds stage out from shared memory to device
       * variables
       * ======================================================
       */

      SgMultiplyOp * multiplyExpression2 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::remainingElements)));

      SgMultiplyOp * multiplyExpression3 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock)),
          dotExpression);

      SgAddOp * addExpression3 = buildAddOp (multiplyExpression2,
          multiplyExpression3);

      SgAddOp * addExpression4 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::threadIDModulus)),
          addExpression3);

      SgMultiplyOp * multiplyExpression4 = buildMultiplyOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::dataPerElementCounter)),
          buildVarRefExp (variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::remainingElements)));

      SgAddOp * addExpression5 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::threadIDModulus)),
          multiplyExpression4);

      SgAddOp * addExpression6 = buildAddOp (buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::autosharedDisplacement)),
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

      SgForStatement * loopStatement2 = buildForStatement(
          initialisationExpression,
          testExpression,
          incrementExpression,
          loopBody2 );
      
      /*
      SgCPPDo * loopStatement2 =
          CPPStatementsAndExpressionsBuilder::buildCPPDoStatement (
              loopInitializationExpression, upperBoundExpression, buildIntVal (
                  1), loopBody2); */

      appendStatement (loopStatement2, outerBlock);
    }
  }

  return outerBlock;
}

void
CPPOpenCLKernelSubroutineDirectLoop::createExecutionLoopStatements ()
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
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;

  SgBasicBlock * loopBody = buildBasicBlock ();

  SgExprStatement * assignmentStatement1 = buildAssignStatement (
      buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock)),
      buildSubtractOp (
            buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::setElementCounter)),
            buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::threadIDModulus))));

  appendStatement (assignmentStatement1, loopBody);


  SgFunctionCallExp * functionCall = buildFunctionCallExp (
      CPPTypesBuilder::buildNewCPPFunction ("MIN", subroutineScope),
      buildExprListExp (
          buildVarRefExp ( variableDeclarations->get (DirectLoop::CPP::KernelSubroutine::warpSize)),
          buildSubtractOp (
              buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::setSize)), 
              buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock)))));

  SgExprStatement * assignmentStatement2 = buildAssignStatement (
      buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::remainingElements) ),
      functionCall);

  appendStatement (assignmentStatement2, loopBody);

  appendStatement (
      createStageInFromDeviceMemoryToLocalThreadVariablesStatements (),
      loopBody);

  appendStatement (createUserSubroutineCallStatement (), 
      loopBody);

  appendStatement (
      createStageOutFromLocalThreadVariablesToDeviceMemoryStatements (),
      loopBody);

  SgExpression * initialisationExpression = buildAssignOp (
      buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::setElementCounter)),
      buildFunctionCallExp(
          OpenCL::CPP::getGlobalId,
          buildIntType(),
          buildExprListExp(
              buildIntVal(0))));
  
  SgExpression * testExpression = buildLessThanOp(
      buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::setElementCounter)),
      buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::setSize) ) );
  
  SgExpression * incrementExpression = buildPlusAssignOp(
      buildVarRefExp ( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::setElementCounter)),
      buildFunctionCallExp(
          OpenCL::CPP::getGlobalSize,
          buildIntType(),
          buildExprListExp(
              buildIntVal(0))));
  
  SgForStatement * forStatement = buildForStatement(
      initialisationExpression,
      testExpression,
      incrementExpression,
      loopBody );

  appendStatement (forStatement, subroutineScope);
}

void
CPPOpenCLKernelSubroutineDirectLoop::createAutoSharedDisplacementInitialisationStatement ()
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

  SgDivideOp * divideExpression1 = buildDivideOp (
      buildFunctionCallExp(
           OpenCL::CPP::getLocalId,
           buildIntType(),
           buildExprListExp(
               buildIntVal(0))),
      buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::warpSize)));

  SgMultiplyOp * multiplyExpression = buildMultiplyOp (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::CPP::KernelSubroutine::warpScratchpadSize)),
      divideExpression1);

  if (parallelLoop->getSizeOfOpDat () == 8)
  {
    SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression,
        buildIntVal (8));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        buildVarRefExp (variableDeclarations->get (
            DirectLoop::CPP::KernelSubroutine::autosharedDisplacement)),
        divideExpression2);

    appendStatement (assignmentStatement, subroutineScope);
  }
  else
  {
    SgDivideOp * divideExpression2 = buildDivideOp (multiplyExpression,
        buildIntVal (4));

    SgExprStatement * assignmentStatement = buildAssignStatement (
        buildVarRefExp (variableDeclarations->get (
            DirectLoop::CPP::KernelSubroutine::autosharedDisplacement)),
        divideExpression2);

    appendStatement (assignmentStatement, subroutineScope);
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createThreadIDInitialisationStatement ()
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
  
  /*
   * ======================================================
   * tid = get_local_id(0) % OP_WARP_SIZE
   * ======================================================
   */ 

  SgExprStatement * assignmentStatement = buildAssignStatement (
      buildVarRefExp( variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::threadIDModulus ) ),
      buildModOp(
          buildFunctionCallExp(
              OpenCL::CPP::getGlobalSize,
              buildIntType(),
              buildExprListExp(
                  buildIntVal(0))),
          buildVarRefExp (variableDeclarations->get ( DirectLoop::CPP::KernelSubroutine::warpSize) ) ) );

  appendStatement (assignmentStatement, subroutineScope);
}

void
CPPOpenCLKernelSubroutineDirectLoop::createOpDatFormalParameterDeclarations ()
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
          CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
              variableName, arrayType, subroutineScope, formalParameters, 1,
              DEVICE));
    }
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createStatements ()
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
CPPOpenCLKernelSubroutineDirectLoop::createLocalVariableDeclarations ()
{
  using std::vector;
  using std::string;

  vector <string> fourByteIntegers;

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::setElementCounter);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::dataPerElementCounter);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::threadIDModulus);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::remainingElements);

  fourByteIntegers.push_back (
      DirectLoop::CPP::KernelSubroutine::autosharedDisplacement);

  for (vector <string>::iterator it = fourByteIntegers.begin (); it
      != fourByteIntegers.end (); ++it)
  {
    variableDeclarations->add (*it,
        buildVariableDeclaration(
            *it,
            buildIntType(),
            NULL,
            subroutineScope ) );
  }

  createLocalThreadDeclarations ();

  createAutoSharedDeclaration ();

  if (parallelLoop->isReductionRequired () == true)
  {
    variableDeclarations->add (ReductionSubroutine::offsetForReduction,
        CPPStatementsAndExpressionsBuilder::appendVariableDeclaration (
            ReductionSubroutine::offsetForReduction,
            CPPTypesBuilder::getFourByteInteger (), subroutineScope, 1,
            VALUE));
  }
}

void
CPPOpenCLKernelSubroutineDirectLoop::createFormalParameterDeclarations ()
{
  /*
   * ======================================================
   * OP_DAT dimensions formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      VariableNames::getDimensionsVariableDeclarationName (userSubroutineName),
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
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
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
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
      DirectLoop::CPP::KernelSubroutine::warpScratchpadSize,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::CPP::KernelSubroutine::warpScratchpadSize,
          CPPTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Set size formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      DirectLoop::CPP::KernelSubroutine::setSize,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::CPP::KernelSubroutine::setSize,
          CPPTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));

  /*
   * ======================================================
   * Warp size formal parameter
   * ======================================================
   */

  variableDeclarations->add (
      DirectLoop::CPP::KernelSubroutine::warpSize,
      CPPStatementsAndExpressionsBuilder::appendVariableDeclarationAsFormalParameter (
          DirectLoop::CPP::KernelSubroutine::warpSize,
          CPPTypesBuilder::getFourByteInteger (), subroutineScope,
          formalParameters, 1, VALUE));
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

CPPOpenCLKernelSubroutineDirectLoop::CPPOpenCLKernelSubroutineDirectLoop (
    std::string const & subroutineName, std::string const & userSubroutineName,
    CPPParallelLoop * parallelLoop, SgScopeStatement * moduleScope,
    CPPOpenCLDataSizesDeclarationDirectLoop * dataSizesDeclaration,
    CPPOpDatDimensionsDeclaration * opDatDimensionsDeclaration) :
  CPPOpenCLKernelSubroutine (subroutineName, userSubroutineName,
      parallelLoop, moduleScope, opDatDimensionsDeclaration),
      dataSizesDeclaration (dataSizesDeclaration)
{
  Debug::getInstance ()->debugMessage ("<Kernel, Direct, OpenCL>",
      Debug::CONSTRUCTOR_LEVEL, __FILE__, __LINE__);

  createFormalParameterDeclarations ();

  createLocalVariableDeclarations ();

  createStatements ();
}

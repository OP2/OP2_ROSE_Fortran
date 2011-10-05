#include <CPPOpenCLHostSubroutine.h>
#include <CPPOpenCLReductionSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */

void
CPPOpenCLHostSubroutine::createReductionPrologueStatements ()
{
  using SageInterface::appendStatement;

  SgExpression * b_ref = buildVarRefExp (variableDeclarations->get (
      CommonVariableNames::iterationCounter1));
  SgExpression * d_ref = buildVarRefExp (variableDeclarations->get (
      CommonVariableNames::iterationCounter2));
  SgExpression * reduct_size_ref = buildVarRefExp (variableDeclarations->get (
      ReductionSubroutine::reductionArraySize));
  SgExpression * reduct_bytes_ref = buildVarRefExp (variableDeclarations->get (
      ReductionSubroutine::reductionArrayBytes));
  //  SgExpression * nblocks_ref = buildVarRefExp ( variableDeclarations->get (OpenCL::CPP::blocksPerGrid) );
  SgExpression * maxblocks_ref = buildVarRefExp (variableDeclarations->get (
      ReductionSubroutine::maximumNumberOfThreadBlocks));
  SgExpression * OP_reduct_h_ref = buildVarRefExp (variableDeclarations->get (
      ReductionSubroutine::reductionH));

  SgExprStatement * tempStatement = NULL;

  /*
   * ======================================================
   * reduct_bytes = 0
   * ======================================================
   */
  tempStatement = buildAssignStatement (reduct_bytes_ref, buildIntVal (0));

  appendStatement (tempStatement, subroutineScope);

  /*
   * ======================================================
   * reduct_size = 0
   * ======================================================
   */
  tempStatement = buildAssignStatement (reduct_size_ref, buildIntVal (0));

  appendStatement (tempStatement, subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argNh_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatHostName (i))); //TODO: check
    SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (i));

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobal (i) && parallelLoop->isRead (i) == false)
      {
        /*
         * ======================================================
         * reduct_bytes += ROUND_UP( maxblocks*dimN*sizeof(TYP) )
         * ======================================================
         */
        tempStatement = buildExprStatement (buildPlusAssignOp (
            reduct_bytes_ref, buildFunctionCallExp ("ROUND_UP",
                buildIntType (), buildExprListExp (buildMultiplyOp (
                    maxblocks_ref, buildMultiplyOp (dimN_val, buildSizeOfOp (
                        parallelLoop->getOpDatType (i))))))));

        appendStatement (tempStatement, subroutineScope);

        /*
         * ======================================================
         * reduct_size = MAX(reduct_size, sizeof(TYP) )
         * ======================================================
         */
        tempStatement = buildAssignStatement (reduct_size_ref,
            buildFunctionCallExp ("MAX", buildIntType (),
                buildExprListExp (reduct_size_ref, buildSizeOfOp (
                    parallelLoop->getOpDatType (i)))));

        appendStatement (tempStatement, subroutineScope);
      }
    }
  }

  /*
   * ======================================================
   * reallocReductArrays( reduct_bytes )
   * ======================================================
   */
  tempStatement = buildFunctionCallStmt ("reallocReductArrays",
      buildVoidType (), buildExprListExp (reduct_bytes_ref));

  appendStatement (tempStatement, subroutineScope);

  /*
   * ======================================================
   * reduct_bytes = 0
   * ======================================================
   */
  tempStatement = buildAssignStatement (reduct_bytes_ref, buildIntVal (0));

  appendStatement (tempStatement, subroutineScope);

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    SgExpression * argNh_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatHostName (i))); //TODO: check
    SgExpression * argN_ref = buildVarRefExp (variableDeclarations->get (
        OP2::VariableNames::getOpDatName (i)));
    SgExpression * dimN_val = buildIntVal (parallelLoop->getOpDatDimension (i));

    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isGlobal (i) && parallelLoop->isRead (i) == false)
      {
        /*
         * ======================================================
         * argN.data = OP_reduct_h + reduct_bytes
         * ======================================================
         */
        tempStatement = buildAssignStatement (buildDotExp (argN_ref,
            buildOpaqueVarRefExp ("data")), buildAddOp (OP_reduct_h_ref,
            reduct_bytes_ref));

        appendStatement (tempStatement, subroutineScope);

        /*
         * ======================================================
         * argN.data_d = OP_reduct_h + reduct_bytes
         * ======================================================
         */
        tempStatement = buildAssignStatement (buildDotExp (argN_ref,
            buildOpaqueVarRefExp ("data_d")), buildAddOp (OP_reduct_h_ref,
            reduct_bytes_ref));

        appendStatement (tempStatement, subroutineScope);

        /* 
         * ======================================================
         * BEGIN for ( b=0; b<maxblocks; b++ ) 
         * ======================================================
         */

        SgStatement * initialisationExpression1 = buildExprStatement (
            buildAssignOp (b_ref, buildIntVal (0)));

        SgStatement * testExpression1 = buildExprStatement (buildLessThanOp (
            b_ref, maxblocks_ref));

        SgExpression * incrementExpression1 = buildPlusPlusOp (b_ref);

        SgBasicBlock * loopBody1 = buildBasicBlock ();

        SgForStatement * forStatement1 = buildForStatement (
            initialisationExpression1, testExpression1, incrementExpression1,
            loopBody1);

        appendStatement (forStatement1, subroutineScope);

        /* 
         * ======================================================
         * BEGIN for ( d=0; d<dimN; d++ ) 
         * ======================================================
         */

        SgStatement * initialisationExpression2 = buildExprStatement (
            buildAssignOp (d_ref, buildIntVal (0)));

        SgStatement * testExpression2 = buildExprStatement (buildLessThanOp (
            d_ref, dimN_val));

        SgExpression * incrementExpression2 = buildPlusPlusOp (d_ref);

        SgBasicBlock * loopBody2 = buildBasicBlock ();

        SgForStatement * forStatement2 = buildForStatement (
            initialisationExpression2, testExpression2, incrementExpression2,
            loopBody2);

        appendStatement (forStatement2, loopBody1);

        if (parallelLoop->isIncremented (i))
        {
          /*
           * ======================================================
           * ((TYP *)argN.data)[d+b*dimN] = 0
           * ======================================================
           */
          tempStatement = buildAssignStatement (buildPntrArrRefExp (
              buildCastExp (buildDotExp (argN_ref,
                  buildOpaqueVarRefExp ("data")), buildPointerType (
                  parallelLoop->getOpDatType (i))), buildAddOp (d_ref,
                  buildMultiplyOp (b_ref, dimN_val))), buildIntVal (0));

        }
        else
        {
          /*
           * ======================================================
           * ((TYP *)argN.data)[d+b*dimN] = argNh[d]
           * ======================================================
           */
          tempStatement = buildAssignStatement (buildPntrArrRefExp (
              buildCastExp (buildDotExp (argN_ref,
                  buildOpaqueVarRefExp ("data")), buildPointerType (
                  parallelLoop->getOpDatType (i))), buildAddOp (d_ref,
                  buildMultiplyOp (b_ref, dimN_val))), buildPntrArrRefExp (
              argNh_ref, //XXX ???
              d_ref));

        }
        appendStatement (tempStatement, loopBody2);

      }
      /* 
       * ======================================================
       * END for ( d=0; d<dimN; d++ ) 
       * ======================================================
       */

      /* 
       * ======================================================
       * END for ( b=0; b<maxblocks; b++ ) 
       * ======================================================
       */
    }
  }

}

SgStatement *
CPPOpenCLHostSubroutine::createGetKernelStatement (std::string & kernelName)
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildStringVal;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildOpaqueType;
  using SageBuilder::buildVariableDeclaration;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildStringVal (kernelName));

  SgExpression * getKernel = buildFunctionCallExp (OpenCL::CPP::getKernel,
      buildVoidType (), //FIXME
      actualParameters, subroutineScope);

  SgAssignInitializer * assignInitializer = buildAssignInitializer (getKernel,
      buildOpaqueType (OpenCL::CPP::kernelType, subroutineScope));

  SgVariableDeclaration * variableDeclaration = buildVariableDeclaration (
      OpenCL::CPP::kernel, buildOpaqueType (OpenCL::CPP::kernelType,
          subroutineScope), assignInitializer, subroutineScope);

  variableDeclarations->add (OpenCL::CPP::kernel, variableDeclaration);

  return variableDeclaration;

}

SgStatement *
CPPOpenCLHostSubroutine::createKernelArgumentStatement (
    SgExpression * argumentNumber, SgExpression * size, SgExpression * data,
    SgScopeStatement * scope)
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildBitOrOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildIntType;
  using SageBuilder::buildExprListExp;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::kernel)));
  actualParameters->append_expression (argumentNumber);
  actualParameters->append_expression (size);
  actualParameters->append_expression (data);

  SgFunctionCallExp * setKernelArgument = buildFunctionCallExp (
      OpenCL::CPP::setKernelArg, buildIntType (), actualParameters, scope);

  SgExpression * bitOrExpression = buildBitOrOp (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::errVar)), setKernelArgument);

  SgStatement * assignment = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::errVar)), bitOrExpression);

  return assignment;
}

SgStatement *
CPPOpenCLHostSubroutine::createKernelCallBlock (std::string & kernelName,
    std::vector <std::pair <SgExpression *, SgExpression *> > & argList,
    SgScopeStatement * scope)
{
  using SageBuilder::buildPlusPlusOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;

  SgBasicBlock * toReturn = buildBasicBlock ();

  appendStatement (createGetKernelStatement (kernelName), toReturn);

  buildAssignStatement (buildVarRefExp (variableDeclarations->get (
      OpenCL::CPP::argumentCounterVariable)), buildIntVal (0));

  for (int i = 0; i < argList.size (); ++i)
  {
    createKernelArgumentStatement (buildPlusPlusOp (buildVarRefExp (
        variableDeclarations->get (OpenCL::CPP::argumentCounterVariable))),
        argList[i].first, argList[i].second, scope);
  }

  createKernelCallStatement ();

  createFinishStatement ();

  //FIXME
  return NULL;
}

SgStatement *
CPPOpenCLHostSubroutine::createKernelCallStatement ()
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildBitOrOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildNullExpression;
  using SageBuilder::buildAddressOfOp;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionCallStmt;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::commanQueue)));
  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::kernel)));
  actualParameters->append_expression (buildIntVal (1));
  actualParameters->append_expression (buildNullExpression ());
  actualParameters->append_expression (buildAddressOfOp (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::totalThreads))));
  actualParameters->append_expression (buildAddressOfOp (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::threadsPerBlock))));
  actualParameters->append_expression (buildIntVal (0));
  actualParameters->append_expression (buildNullExpression ());
  actualParameters->append_expression (buildNullExpression ());

  SgExpression * enqueueKernel = buildFunctionCallExp (
      OpenCL::CPP::enqueueKernel, buildVoidType (), //FIXME
      actualParameters, subroutineScope);

  SgStatement * assignment = buildAssignStatement (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::errVar)), enqueueKernel);

  return assignment;

}

SgStatement *
CPPOpenCLHostSubroutine::createErrorCheckStatement (std::string & message)
{
  //TODO: implement me! (maybe)
}

SgStatement *
CPPOpenCLHostSubroutine::createFinishStatement ()
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::commanQueue)));

  SgStatement * enqueueKernel = buildFunctionCallStmt (
      OpenCL::CPP::enqueueKernel, buildVoidType (), //FIXME
      actualParameters, subroutineScope);

  return enqueueKernel;

}

SgStatement *
CPPOpenCLHostSubroutine::createKernelFunctionCallStatement ()
{ //XXX: scope?
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildOpaqueType;
  using SageBuilder::buildSizeOfOp;
  using SageBuilder::buildNullExpression;
  using SageBuilder::buildIntType;
  using SageBuilder::buildAddressOfOp;
  using SageBuilder::buildBasicBlock;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating statement to call OpenCL kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

  std::vector <std::pair <SgExpression *, SgExpression *> > kernelArguments;

  SgStatement * toReturn = buildBasicBlock ();

  /*SgExprListExp * actualParameters = buildExprListExp ();

   actualParameters->append_expression (buildVarRefExp (
   moduleDeclarations->getDimensionsVariableDeclaration ()));

   actualParameters->append_expression (buildVarRefExp (
   moduleDeclarations->getDataSizesVariableDeclaration ()));
   */

  for (unsigned int i = 1; i <= parallelLoop->getNumberOfOpDatArgumentGroups (); ++i)
  {
    if (parallelLoop->isDuplicateOpDat (i) == false)
    {
      if (parallelLoop->isReductionRequired (i) == false)
      {
        kernelArguments.push_back (std::make_pair (buildSizeOfOp (
            buildOpaqueType (OpenCL::CPP::pointerType, NULL )), //FIXME
            buildAddressOfOp (buildVarRefExp (variableDeclarations->get (
                OP2::VariableNames::getOpDatDeviceName (i))))));
      }
      else
      {
        kernelArguments.push_back (
            std::make_pair (
                buildSizeOfOp (buildOpaqueType (OpenCL::CPP::pointerType, NULL )), //FIXME
                buildAddressOfOp (
                    buildVarRefExp (
                        moduleDeclarations->getReductionArrayDeviceVariableDeclaration ()))));
      }
    }
  }

  kernelArguments.push_back (std::make_pair (buildSizeOfOp (buildIntType ()), //FIXME
      buildVarRefExp (variableDeclarations->get (
          DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock)))); //FIXME move in OP2::VariableNames

  kernelArguments.push_back (std::make_pair (buildSizeOfOp (buildIntType ()), //FIXME
      buildDotExp (buildVarRefExp (variableDeclarations->get (
          OP2::VariableNames::getOpSetName ())), buildOpaqueVarRefExp (
          OP2::VariableNames::size, subroutineScope)))); //FIXME move in OP2::VariableNames


  //TODO: what about this?
  /*

   if (parallelLoop->isReductionRequired () == true)
   {
   actualParameters->append_expression (
   buildVarRefExp (variableDeclarations->get (
   ReductionSubroutine::sharedMemoryStartOffset)));
   } */

  /*
   * Pass the size of the shared memory as a parameter
   */
  kernelArguments.push_back (std::make_pair (buildVarRefExp (
      variableDeclarations->get (OpenCL::CPP::sharedMemorySize)), //FIXME
      buildNullExpression ())); //FIXME move in OP2::VariableNames

  toReturn = createKernelCallBlock (kernelSubroutineName, kernelArguments,
      subroutineScope);

  return toReturn;
}

CPPOpenCLHostSubroutine::CPPOpenCLHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, CPPParallelLoop * parallelLoop,
    SgScopeStatement * moduleScope) :
  CPPHostSubroutine (subroutineName, userSubroutineName, kernelSubroutineName,
      parallelLoop, moduleScope)
{
}

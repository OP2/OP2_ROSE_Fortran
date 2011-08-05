#include <CPPOpenCLHostSubroutine.h>

/*
 * ======================================================
 * Protected functions
 * ======================================================
 */


SgStatement *
CPPOpenCLHostSubroutine::createGetKernelStatement (
    std::string & kernelName )
{
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildStringVal;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildOpaqueType;
  using SageBuilder::buildVariableDeclaration;

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression( buildStringVal( kernelName ) );


  SgExpression * getKernel = buildFunctionCallStmt (
      OpenCL::CPP::getKernel,
      buildVoidType(), //FIXME
      actualParameters,
      subroutineScope );

  SgAssignInitializer * assignInitializer = buildAssignInitializer(
      getKernel,
      buildOpaqueType( OpenCL::CPP::kernelType ) );

  SgVariableDeclaration variableDeclaration = buildVariableDeclaration(
      OpenCL::CPP::kernel,
      buildOpaqueType( OpenCL::CPP::kernelType ),
      assignInitializer,
      subroutineScope );

  variableDeclarations->add( OpenCL::CPP::kernel, variableDeclaration );

  return variableDeclaration;

}

SgStatement *
CPPOpenCLHostSubroutine::createKernelArgumentStatement (
    SgExpression * argumentNumber,
    SgExpression * size,
    SgExpression * data,
    SgScopeStatement * scope)
{
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildAssignStatement;
  using SageBuilder::buildBitOrOp;
  using SageBuilder::buildVarRefExp;
  
  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression( buildVarRefExp( variableDeclarations->get( OpenCL::CPP::kernel ) ) );
  actualParameters->append_expression( argumentNumber );
  actualParameters->append_expression( size );
  actualParameters->append_expression( data );


  SgFunctionCallExp * setKernelArgument = buildFunctionCallExp(
      OpenCL::CPP::setKernelArg,
      actualParameters,
      scope );



  SgExpression * bitOrExpression = buildBitOrOp(
      buildVarRefExp( variableDeclarations->get( OpenCL::CPP::errVar ) ),
      setKernelArgument );

  SgStatement * assignment = buildAssignStatement(
      buildVarRefExp( variableDeclarations->get( OpenCL::CPP::errVar ) ),
      bitOrExpression );

  return assigment;
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

  SgExprListExp * actualParameters = buildExprListExp ();

  actualParameters->append_expression( buildVarRefExp( variableDeclarations->get( OpenCL::CPP::commanQueue ) ) );
  actualParameters->append_expression( buildVarRefExp( variableDeclarations->get( OpenCL::CPP::kernel ) ) );
  actualParameters->append_expression( buildIntVal( 1 ) );
  actualParameters->append_expression( buildNullExpression() );
  actualParameters->append_expression( buildAddressOfOp(
      buildVarRefExp( variableDeclarations->get( OpenCL::CPP::totalThreads ) ) ) );
  actualParameters->append_expression( buildAddressOfOp(
      buildVarRefExp( variableDeclarations->get( OpenCL::CPP::threadsPerBlock ) ) ) );
  actualParameters->append_expression( buildIntVal( 0 ) );
  actualParameters->append_expression( buildNullExpression() );
  actualParameters->append_expression( buildNullExpression() );



  SgExpression * enqueueKernel = buildFunctionCallStmt (
      OpenCL::CPP::enqueueKernel,
      buildVoidType (), //FIXME
      actualParameters,
      subroutineScope);

  SgStatement * assignment = buildAssignStatement(
      buildVarRefExp( variableDeclarations->get( OpenCL::CPP::errVar ) ),
      enqueueKernel );

  return assigment;
}

SgStatement *
CPPOpenCLHostSubroutine::createErrorCheckStatement (
    std::string & message)
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

  actualParameters->append_expression( buildVarRefExp( variableDeclarations->get( OpenCL::CPP::commanQueue ) ) );


  SgStatement * enqueueKernel = buildFunctionCallStmt (
      OpenCL::CPP::enqueueKernel,
      buildVoidType(), //FIXME
      actualParameters,
      subroutineScope );


}

SgStatement *
CPPOpenCLHostSubroutine::createKernelFunctionCallStatement ()
{
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
  using SageInterface::appendStatement;


  Debug::getInstance ()->debugMessage (
      "Creating statement to call OpenCL kernel", Debug::FUNCTION_LEVEL,
      __FILE__, __LINE__);

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
        SgSizeOfOp * size = buildSizeOfOp( buildOpaqueType( "cl_mem", NULL ) ); //FIXME
        SgAddressOfOp * data = buildAddressOfOp(
            variableDeclarations->get ( VariableNames::getOpDatDeviceName (i) ) );
        createKernelFunctionCallStatement( NULL, size, data );

      }
      else
      {
        //actualParameters->append_expression (buildVarRefExp (
        //    moduleDeclarations->getReductionArrayDeviceVariableDeclaration ()));
        SgSizeOfOp * size = buildSizeOfOp( buildOpaqueType( "cl_mem", NULL ) ); //FIXME
        SgAddressOfOp * data = buildAddressOfOp(
            moduleDeclarations->getReductionArrayDeviceVariableDeclaration () );
        createKernelFunctionCallStatement( NULL, size, data );
      }
    }
  }

  SgSizeOfOp * size = buildSizeOfOp( buildIntType() );
  SgAddressOfOp * data = buildAddressOfOp(
      buildVarRefExp (
          variableDeclarations->get (
              DirectLoop::CPP::KernelSubroutine::offsetInThreadBlock ) ) ); //FIXME move in VariableNames
  createKernelFunctionCallStatement( NULL, size, data );


  SgExpression * setSize = buildDotExp (
      buildVarRefExp ( variableDeclarations->get ( VariableNames::getOpSetName () ) ),
      buildOpaqueVarRefExp ( CommonVariableNames::size, subroutineScope ) );

  SgSizeOfOp * size = buildSizeOfOp( buildIntType() );
  SgAddressOfOp * data = buildAddressOfOp( setSize ); //FIXME move in VariableNames
  createKernelFunctionCallStatement( NULL, size, data );

  //TODO: what about this?
/*
  actualParameters->append_expression (buildVarRefExp (
      variableDeclarations->get (
          DirectLoop::CPP::KernelSubroutine::warpSize)));
          */

  if (parallelLoop->isReductionRequired () == true)
  {
    actualParameters->append_expression (
        buildVarRefExp (variableDeclarations->get (
            ReductionSubroutine::sharedMemoryStartOffset)));
  }

  /*
   * Pass the size of the shared memory as a parameter
   */
  SgVarRefExp * shared = buildVarRefExp( variableDeclarations->get(OpenCL::CPP::sharedMemorySize) );
  createKernelFunctionCallStatement( NULL, size, buildNullExpression() );


  SgExprStatement * callStatement = createKernelCallStatement();

  return callStatement;
}

CPPOpenCLHostSubroutine::CPPOpenCLHostSubroutine (
    std::string const & subroutineName, std::string const & userSubroutineName,
    std::string const & kernelSubroutineName, SgScopeStatement * moduleScope,
    CPPParallelLoop * parallelLoop) :
  CPPHostSubroutine (subroutineName, userSubroutineName, kernelSubroutineName,
      moduleScope, parallelLoop)
{
}

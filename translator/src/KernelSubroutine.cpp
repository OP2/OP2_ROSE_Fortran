#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <FortranTypesBuilder.h>
#include <ROSEHelper.h>
#include <KernelSubroutine.h>
#include <Debug.h>

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgExprListExp *
KernelSubroutine::createActualParametersForUserDeviceSubroutine (
    SgVarRefExp * iterSetVarRef, SgScopeStatement * subroutineScope,
    ParallelLoop & op2ParallelLoop)
{
  using SageBuilder::buildIntVal;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildExprListExp;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Obtaining kernel parameters", 2);

  SgInitializedNamePtrList mainKernelArgs = formalParameters->get_args ();

  vector <SgExpression *> vectParsExps;

  /*
   * ======================================================
   * Iterate over all the formal parameters except the first,
   * because it is always the size of the set
   * ======================================================
   */
  SgInitializedNamePtrList::iterator inPrIterator = mainKernelArgs.begin () + 1;
  inPrIterator++;

  for (int i = 0; inPrIterator != mainKernelArgs.end (); ++i, ++inPrIterator)
  {
    // generating subscript: setIter * 4:setIter * 4 + 4 - 1
    //(for now we know that the set is the same for all arguments = args[2].size

    // setIter * 4
    SgIntVal * opDatDim = buildIntVal (4);

    SgExpression * lowerBound = buildMultiplyOp (iterSetVarRef, opDatDim);

    // setIter * 4 + 4
    SgExpression * setIterMulDimPlusDim = buildAddOp (lowerBound, opDatDim);

    // setIter * 4 + 4 - 1
    SgExpression * upperBound = buildSubtractOp (setIterMulDimPlusDim,
        buildIntVal (1));

    //setIter * 4:setIter * 4 + 4 - 1
    SgSubscriptExpression * arraySubScript = new SgSubscriptExpression (
        ROSEHelper::getFileInfo (), lowerBound, upperBound, buildIntVal (1));

    arraySubScript->set_endOfConstruct (ROSEHelper::getFileInfo ());
    arraySubScript->setCompilerGenerated ();
    arraySubScript->setOutputInCodeGeneration ();

    SgExpression * formalParRef = buildVarRefExp ((*inPrIterator)->get_name (),
        subroutineScope);

    SgExpression * parAccess =
        buildPntrArrRefExp (formalParRef, arraySubScript);

    vectParsExps.push_back (parAccess);
  }

  SgExprListExp * userKernelParams = buildExprListExp (vectParsExps);
  userKernelParams->set_parent (subroutineScope);
  userKernelParams->setCompilerGenerated ();
  userKernelParams->setOutputInCodeGeneration ();

  return userKernelParams;
}

void
KernelSubroutine::setUp_OP_DAT_ArgumentTypes (
    std::vector <SgType *> & opDatArgumentTypes,
    SgVariableDeclaration * setSizeFormalParameter, ParallelLoop & parallelLoop)
{
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildExprListExp;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage ("Looking up types of arguments", 2);

  map <string, OP_DAT_Declaration *>::const_iterator OP_DAT_iterator;

  // build: user type arrays with bounds 0:op_set%size * op_dat%dim - 1
  for (OP_DAT_iterator = parallelLoop.first_OP_DAT (); OP_DAT_iterator
      != parallelLoop.last_OP_DAT (); ++OP_DAT_iterator)
  {
    string const & opDatVariableName = OP_DAT_iterator->first;
    OP_DAT_Declaration * opDatDeclaration = OP_DAT_iterator->second;

    for (unsigned int i = 0; i < parallelLoop.getNumberOfOP_DATOccurrences (
        opDatVariableName); ++i)
    {
      // 'setsize * op_dat%dim'
      // Currently 'op_dat%dim' is a constant
      SgExpression * setSizeBy_OP_DAT_Dimension = buildMultiplyOp (
          buildVarRefExp (setSizeFormalParameter), buildIntVal (
              opDatDeclaration->getDimension ()));

      // 'op_set%size * op_dat%dim -1'
      SgExpression * upperBound = buildSubtractOp (setSizeBy_OP_DAT_Dimension,
          buildIntVal (1));

      // '0: op_set%size * op_dat%dim - 1'
      SgSubscriptExpression * arraySubscript = new SgSubscriptExpression (
          ROSEHelper::getFileInfo (), buildIntVal (0), upperBound, buildIntVal (
              1));

      arraySubscript->set_endOfConstruct (ROSEHelper::getFileInfo ());
      arraySubscript->setCompilerGenerated ();
      arraySubscript->setOutputInCodeGeneration ();

      /*
       * ======================================================
       * Obtain the primitive type of the OP_DAT argument
       * ======================================================
       */
      SgType * opDatBaseType = opDatDeclaration->getActualType ();

      SgArrayType * isArrayType = isSgArrayType (opDatBaseType);
      if (isArrayType != NULL)
      {
        /*
         * If the base type is an array (as is always or nearly the case), we need to
         * modify the array subscript. If we copy the array type, we won't get the attributes, hence we copy
         * only the base type and we re-create the array type
         */
        opDatBaseType = isArrayType->get_base_type ();
      }

      /*
       * ======================================================
       * Build array type with the correct subscript
       * ======================================================
       */
      SgArrayType * arrayType = buildArrayType (opDatBaseType, arraySubscript);
      arrayType->set_rank (1);
      arrayType->set_dim_info (buildExprListExp (arraySubscript));

      opDatArgumentTypes.push_back (arrayType);
    }
  }
}

void
KernelSubroutine::createFormalParameters (ParallelLoop & parallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;
  using std::string;
  using std::vector;

  formalParameter_setSize = buildVariableDeclaration ("setSize",
      buildIntType (), NULL, subroutineScope);

  formalParameter_setSize->get_declarationModifier ().get_accessModifier ().setUndefined ();
  formalParameter_setSize->get_declarationModifier ().get_typeModifier ().setValue ();

  formalParameters->append_arg (
      *(formalParameter_setSize->get_variables ().begin ()));

  appendStatement (formalParameter_setSize, subroutineScope);

  vector <SgType *> argTypes;

  setUp_OP_DAT_ArgumentTypes (argTypes, formalParameter_setSize, parallelLoop);

  // Create formal parameters (we have a OP_DAT argument starting from 3 and then every 4 arguments
  // (4 is stored inside a macro (ARG_LINE_LENGTH) copied inside variable argLineLength in constructor
  // retriving op_dat original type from argument: must generate real(8), dimension(0:setsize * 4 -1)
  // where 4 is the carinality of each input op_dat element associate to each set element (can be found
  // in the op_decl_dat call)

  for (unsigned int i = 0; i
      < parallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); i++)
  {
    string const argName = "arg" + lexical_cast <string> (i);

    SgVariableDeclaration * argFormalParameter = buildVariableDeclaration (
        argName, argTypes[i], NULL, subroutineScope);

    formalParameters->append_arg (
        *(argFormalParameter->get_variables ().begin ()));

    argFormalParameter->get_declarationModifier ().get_typeModifier ().setIntent_in ();
    argFormalParameter->get_declarationModifier ().get_typeModifier ().setDevice ();
    argFormalParameter->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (argFormalParameter, subroutineScope);
  }
}

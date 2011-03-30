#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "Debug.h"
#include "CreateKernels.h"
#include "FortranTypeDeclarations.h"

/*
 * ======================================================
 * Private functions
 * ======================================================
 */

SgClassDeclaration *
CreateKernels::buildNewTypeDeclaration (std::string const & typeName,
    SgScopeStatement * scope, bool fortranModule)
{
  SgClassDefinition * classDefinition = new SgClassDefinition (fileInfo);

  SgClassDeclaration * nonDefiningClassDeclaration = new SgClassDeclaration (
      fileInfo, typeName, SgClassDeclaration::e_class, NULL, NULL);

  SgClassDeclaration * classDeclaration;

  if (fortranModule)
  {
    classDeclaration = new SgModuleStatement (fileInfo, typeName,
        SgClassDeclaration::e_class, NULL, classDefinition);
  }
  else
  {
    classDeclaration = new SgClassDeclaration (fileInfo, typeName,
        SgClassDeclaration::e_class, NULL, classDefinition);
  }

  classDefinition->set_endOfConstruct (fileInfo);
  classDefinition->set_declaration (classDeclaration);

  nonDefiningClassDeclaration->set_endOfConstruct (fileInfo);
  nonDefiningClassDeclaration->set_definingDeclaration (classDeclaration);
  nonDefiningClassDeclaration->set_firstNondefiningDeclaration (
      nonDefiningClassDeclaration);
  nonDefiningClassDeclaration->set_scope (scope);
  nonDefiningClassDeclaration->set_parent (scope);
  nonDefiningClassDeclaration->set_type (SgClassType::createType (
      nonDefiningClassDeclaration));
  nonDefiningClassDeclaration->setForward ();

  classDeclaration->set_endOfConstruct (fileInfo);
  classDeclaration->set_definingDeclaration (classDeclaration);
  classDeclaration->set_firstNondefiningDeclaration (
      nonDefiningClassDeclaration);
  classDeclaration->set_scope (scope);
  classDeclaration->set_parent (scope);
  classDeclaration->set_type (nonDefiningClassDeclaration->get_type ());

  SgClassSymbol * classSymbol = new SgClassSymbol (classDeclaration);
  scope->insert_symbol (classDeclaration->get_name (), classSymbol);

  return classDeclaration;
}

void
CreateKernels::createHostSubroutineDeclarationsForPlanFunction (
    SgScopeStatement * subroutineScope, OP2ParallelLoop & op2ParallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildPointerType;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::vector;
  using std::map;
  using std::string;

  vector <string> planArrayVariables;
  planArrayVariables.push_back ("opDats");
  planArrayVariables.push_back ("opIndirections");
  planArrayVariables.push_back ("opMaps");
  planArrayVariables.push_back ("opAccesses");
  planArrayVariables.push_back ("inds");

  for (vector <string>::iterator it = planArrayVariables.begin (); it
      != planArrayVariables.end (); ++it)
  {
    SgVariableDeclaration * intArrayDeclaration = buildVariableDeclaration (
        *it, FortranTypeDeclarations::getFourByteIntegerArray_RankOne (1,
            op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups ()), NULL,
        subroutineScope);

    intArrayDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (intArrayDeclaration, subroutineScope);
  }

  vector <string> integerVariables;
  integerVariables.push_back ("argsNumber");
  integerVariables.push_back ("indsNumber");
  integerVariables.push_back ("iter");
  integerVariables.push_back ("blockOffset");
  integerVariables.push_back ("col");
  integerVariables.push_back ("nblocks");
  integerVariables.push_back ("nthread");
  integerVariables.push_back ("nshared");
  integerVariables.push_back ("i");
  integerVariables.push_back ("m");
  integerVariables.push_back ("threadSynchRet");

  for (vector <string>::iterator it = integerVariables.begin (); it
      != integerVariables.end (); ++it)
  {
    SgVariableDeclaration * intDeclaration = buildVariableDeclaration (*it,
        FortranTypeDeclarations::getFourByteInteger (), NULL, subroutineScope);

    intDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (intDeclaration, subroutineScope);
  }

  SgClassType * c_ptrType = buildNewTypeDeclaration ("c_ptr", subroutineScope,
      false)->get_type ();
  SgVariableDeclaration * planRetDeclaration = buildVariableDeclaration (
      "planRet", c_ptrType, NULL, subroutineScope);
  planRetDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
  appendStatement (planRetDeclaration, subroutineScope);

  SgClassType * opPlanType = buildNewTypeDeclaration ("op_plan",
      subroutineScope, false)->get_type ();
  SgVariableDeclaration * opPlanDeclaration = buildVariableDeclaration (
      "actualPlan", opPlanType, NULL, subroutineScope);
  opPlanDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
  appendStatement (opPlanDeclaration, subroutineScope);

  vector <string> integerPointerVariables;
  integerPointerVariables.push_back ("ncolblk");
  integerPointerVariables.push_back ("pnindirect");

  for (vector <string>::iterator it = integerPointerVariables.begin (); it
      != integerPointerVariables.end (); ++it)
  {
    SgVariableDeclaration * intPointerDeclaration = buildVariableDeclaration (
        *it, buildPointerType (
            FortranTypeDeclarations::getFourByteIntegerArray_RankOne ()), NULL,
        subroutineScope);

    intPointerDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (intPointerDeclaration, subroutineScope);
  }

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "pMaps" + lexical_cast <string> (i);

    SgVariableDeclaration * variable_pMaps_n = buildVariableDeclaration (
        variableName,
        FortranTypeDeclarations::getTwoByteIntegerArray_RankOne (), NULL,
        subroutineScope);

    variable_pMaps_n->get_declarationModifier ().get_accessModifier ().setUndefined ();
    variable_pMaps_n->get_declarationModifier ().get_typeModifier ().setDevice ();
    variable_pMaps_n->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    appendStatement (variable_pMaps_n, subroutineScope);
  }

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "pMaps" + lexical_cast <string> (i) + "Size";

    SgVariableDeclaration * variable_pMaps_n_Size = buildVariableDeclaration (
        variableName, FortranTypeDeclarations::getFourByteInteger (), NULL,
        subroutineScope);

    variable_pMaps_n_Size->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (variable_pMaps_n_Size, subroutineScope);
  }

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "pMaps" + lexical_cast <string> (i) + "Size";

    SgVariableDeclaration * variable_pMaps_n_Size = buildVariableDeclaration (
        variableName, FortranTypeDeclarations::getFourByteInteger (), NULL,
        subroutineScope);

    variable_pMaps_n_Size->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (variable_pMaps_n_Size, subroutineScope);
  }

  int i = 0;
  std::map <string, OP_DAT_Declaration *>::const_iterator OP_DAT_iterator;

  for (OP_DAT_iterator = op2ParallelLoop.first_OP_DAT (); OP_DAT_iterator
      != op2ParallelLoop.last_OP_DAT (); ++i, ++OP_DAT_iterator)
  {
    string const variableName = "pArg" + lexical_cast <string> (i) + "DatD";

    SgVariableDeclaration * variable_pArg_n_DatD = buildVariableDeclaration (
        variableName, OP_DAT_iterator->second->getActualType (), NULL,
        subroutineScope);

    variable_pArg_n_DatD->get_declarationModifier ().get_accessModifier ().setUndefined ();
    variable_pArg_n_DatD->get_declarationModifier ().get_typeModifier ().setDevice ();
    variable_pArg_n_DatD->get_declarationModifier ().get_typeModifier ().setAllocatable ();

    appendStatement (variable_pArg_n_DatD, subroutineScope);
  }

  i = 0;
  for (OP_DAT_iterator = op2ParallelLoop.first_OP_DAT (); OP_DAT_iterator
      != op2ParallelLoop.last_OP_DAT (); ++i, ++OP_DAT_iterator)
  {
    string const variableName = "pArg" + lexical_cast <string> (i) + "DatDSize";

    SgVariableDeclaration * variable_pArg_n_DatDSize =
        buildVariableDeclaration (variableName,
            FortranTypeDeclarations::getFourByteInteger (), NULL,
            subroutineScope);

    variable_pArg_n_DatDSize->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (variable_pArg_n_DatDSize, subroutineScope);
  }
}

void
CreateKernels::fix_OP_PAR_LOOP_Calls (SgScopeStatement * scope,
    OP2ParallelLoop & op2ParallelLoop, SgFunctionCallExp * functionCallExp,
    SgProcedureHeaderStatement * hostSubroutine)
{
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildStringVal;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::getScope;
  using SageInterface::getEnclosingFileNode;
  using SageInterface::insertStatementAfter;
  using SageInterface::getPreviousStatement;
  using SageInterface::findLastDeclarationStatement;

  Debug::getInstance ()->debugMessage ("Patching call to OP_PAR_LOOP", 2);

  /*
   * ======================================================
   * We first need to add a 'use <NEWLY_CREATED_CUDA_MODULE>'
   * statement to the Fortran module where the OP_PAR_LOOP
   * call takes place
   * ======================================================
   */

  /*
   * ======================================================
   * Recursively go back in the scopes until we can find a
   * declaration statement
   * ======================================================
   */
  SgStatement * lastDeclarationStatement = findLastDeclarationStatement (scope);
  SgScopeStatement * parent = scope;
  while (lastDeclarationStatement == NULL)
  {
    parent = (SgScopeStatement *) parent->get_parent ();
    lastDeclarationStatement = findLastDeclarationStatement (parent);
  }

  if (lastDeclarationStatement == NULL)
  {
    Debug::getInstance ()->errorMessage (
        "Could not find declaration statements");
  }

  /*
   * ======================================================
   * Now find the last 'use' statement
   * ======================================================
   */
  SgStatement * previousStatement = lastDeclarationStatement;
  SgUseStatement * lastUseStatement;
  do
  {
    previousStatement = getPreviousStatement (previousStatement);
    lastUseStatement = isSgUseStatement (previousStatement);
  }
  while (lastUseStatement == NULL);

  if (lastUseStatement == NULL)
  {
    Debug::getInstance ()->errorMessage ("Could not find last 'use' statement");
  }

  /*
   * ======================================================
   * Add a new 'use' statement
   * ======================================================
   */
  SgUseStatement* newUseStatement = new SgUseStatement (getEnclosingFileNode (
      lastUseStatement)->get_file_info (),
      op2ParallelLoop.getCUDAModuleName (), false);

  insertStatementAfter (lastUseStatement, newUseStatement);

  /*
   * ======================================================
   * Build a string variable which contains the name of the
   * kernel. This variable is passed to the host code in
   * setting up and tearing down the relevant device code
   * ======================================================
   */

  /*
   * ======================================================
   * The character array contains exactly the number of
   * characters as the kernel name
   * ======================================================
   */
  SgTypeString * characterArray = FortranTypeDeclarations::getString (
      op2ParallelLoop.getUserDeviceSubroutine ().getSubroutineName ().size ());

  SgAssignInitializer * initializer = buildAssignInitializer (buildStringVal (
      op2ParallelLoop.getUserDeviceSubroutine ().getSubroutineName ()),
      characterArray);

  SgVariableDeclaration * kernelStringVariable =
      buildVariableDeclaration (
          op2ParallelLoop.getUserDeviceSubroutine ().getSubroutineName ()
              + "_name", characterArray, initializer, getScope (
              lastDeclarationStatement));

  insertStatementAfter (lastDeclarationStatement, kernelStringVariable);

  /*
   * ======================================================
   * Modify the call to OP_PAR_LOOP with a call to the newly
   * built CUDA host subroutine
   * ======================================================
   */

  /*
   * ======================================================
   * Get a reference to the created CUDA host subroutine
   * ======================================================
   */
  SgFunctionRefExp * hostSubroutineReference = buildFunctionRefExp (
      hostSubroutine);
  functionCallExp->set_function (hostSubroutineReference);

  /*
   * ======================================================
   * Modify the first parameter from a kernel reference to
   * a kernel name
   * ======================================================
   */
  SgExpressionPtrList & arguments =
      functionCallExp->get_args ()->get_expressions ();

  arguments.erase (arguments.begin ());

  arguments.insert (arguments.begin (), buildVarRefExp (kernelStringVariable));

  /*
   * ======================================================
   * Set where the function call is invoked as a transformation
   * in the unparser
   * ======================================================
   */
  SgLocatedNode * functionCallLocation = isSgLocatedNode (functionCallExp);
  functionCallLocation->get_file_info ()->setTransformation ();
}

SgExprListExp *
CreateKernels::createUserDeviceFunctionParameters (
    SgFunctionParameterList * mainKernelParameters,
    SgVarRefExp * iterSetVarRef, SgScopeStatement * subroutineScope,
    OP2ParallelLoop & op2ParallelLoop)
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

  SgInitializedNamePtrList mainKernelArgs = mainKernelParameters->get_args ();

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
        fileInfo, lowerBound, upperBound, buildIntVal (1));

    arraySubScript->set_endOfConstruct (fileInfo);
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
CreateKernels::createHostSubroutineStatements (SgScopeStatement * scope,
    OP2ParallelLoop & op2ParallelLoop)
{
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildPntrArrRefExp;
  using SageBuilder::buildIntType;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildFunctionType;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildFunctionCallStmt;
  using SageBuilder::buildFunctionParameterTypeList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage ("Adding statements to host subroutine",
      2);

  /*
   * ======================================================
   * This function builds the following Fortran code:
   *
   * data0Size = arg0%dim * arg0%set%size
   * data1Size = argN-1%dim * argN-1%set%size
   *
   * allocate(userArgument0(data0Size))
   * allocate(userArgumentN-1(dataN-1Size))
   *
   * CALL c_f_pointer(arg0%dat,c2fPtr0,(/data0Size/))
   * CALL c_f_pointer(arg1%dat,c2fPtrN-1,(/dataN-1Size/))
   *
   * userArgument0 = c2fPtr0
   * userArgumentN-1 = c2fPtrN-1
   *
   * 'call KERNEL_NAME<<<gsize,bsize,reduct_shared>>> (set%size, userArgument0, ..., userArgumentN-1)'
   *
   * c2fPtr0 = userArgument0
   * c2fPtrN-1 = userArgumentN-1
   *
   * deallocate(userArgument0)
   * deallocate(userArgumentN-1)
   * ======================================================
   */

  HostSubroutineOfDirectLoop
      & hostSubroutine =
          dynamic_cast <HostSubroutineOfDirectLoop &> (op2ParallelLoop.getHostSubroutine ());

  vector <SgVariableDeclaration *>::const_iterator OP_DAT_Iterator;
  vector <SgVariableDeclaration *>::const_iterator dataSize_Iterator;
  vector <SgVariableDeclaration *>::const_iterator cToFortranPointer_Iterator;
  vector <SgVariableDeclaration *>::const_iterator
      kernelSubroutineArgument_Iterator;

  /*
   * ======================================================
   * 'data0Size = (arg0%dim * (arg0%set)%size)'
   * ...
   * 'data1Size = (argN-1%dim * (argN-1%set)%size)'
   * ======================================================
   */

  SgStatement * lastAppendedStatement = NULL;

  OP_DAT_Iterator = hostSubroutine.first_OP_DAT_Argument ();
  dataSize_Iterator = hostSubroutine.first_SizeOf_OP_DAT_Variable ();

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i, ++OP_DAT_Iterator, ++dataSize_Iterator)
  {
    SgVarRefExp * opDatArgumentReference = buildVarRefExp (*OP_DAT_Iterator);
    SgVarRefExp * dataISizeArgumentReference = buildVarRefExp (
        *dataSize_Iterator);

    // 'argI%set'
    SgExpression * argISetField = buildDotExp (opDatArgumentReference,
        buildOpaqueVarRefExp ("set", scope));

    // 'argI%dim'
    SgExpression * argIDimField = buildDotExp (opDatArgumentReference,
        buildOpaqueVarRefExp ("dim", scope));

    // '(argI%set)%size'
    SgExpression * argISetSizeField = buildDotExp (argISetField,
        buildOpaqueVarRefExp ("size", scope));

    // 'arg0%dim * (argI%set)%size'
    SgExpression * multiplyExpression = buildMultiplyOp (argIDimField,
        argISetSizeField);

    // 'data0Size = argI%dim * (argI%set)%size'
    SgExpression * assignmentExpression = buildAssignOp (
        dataISizeArgumentReference, multiplyExpression);

    // finally append the statement
    SgStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);

    appendStatement (assignmentStatement, scope);

    if (i == op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups () - 1)
    {
      lastAppendedStatement = assignmentStatement;
    }
  }

  ROSE_ASSERT (lastAppendedStatement != NULL);

  /*
   * ======================================================
   *  'allocate (userArgument0 (data0Size))'
   *  ...
   *  'allocate (userArgumentN-1 (dataN-1Size))'
   * ======================================================
   */

  Debug::getInstance ()->debugMessage (
      "Adding 'allocate' function calls for device variables", 5);

  dataSize_Iterator = hostSubroutine.first_SizeOf_OP_DAT_Variable ();
  kernelSubroutineArgument_Iterator
      = hostSubroutine.first_KernelSubroutineActualParameter ();

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i, ++dataSize_Iterator, ++kernelSubroutineArgument_Iterator)
  {
    // 'userArgumentI (dataISize)'
    SgVarRefExp * argumentIArgumentReference = buildVarRefExp (
        *kernelSubroutineArgument_Iterator);
    SgVarRefExp * dataISizeArgumentReference = buildVarRefExp (
        *dataSize_Iterator);

    SgExprListExp * arrayExpression = buildExprListExp (buildPntrArrRefExp (
        argumentIArgumentReference, dataISizeArgumentReference));

    /*
     * ======================================================
     * TODO: Unfortunately, we cannot use CALL with a Fortran
     * function. Instead we have to use 'addTextForUnaparser'
     * for now
     * ======================================================
     */
    string const allocateStatement = "allocate ("
        + arrayExpression->unparseToString () + ")\n";

    addTextForUnparser (lastAppendedStatement, allocateStatement,
        AstUnparseAttribute::e_after);
  }

  /*
   * ======================================================
   * 'call c_f_pointer (opDat0%dat, c2fPtr0, (/data0Size/))'
   * ...
   * 'call c_f_pointer (opDatN-1%dat, c2fPtrN-1, (/dataN-1Size/))'
   * ======================================================
   */

  cToFortranPointer_Iterator = hostSubroutine.first_CToFortranPointer ();
  OP_DAT_Iterator = hostSubroutine.first_OP_DAT_Argument ();
  dataSize_Iterator = hostSubroutine.first_SizeOf_OP_DAT_Variable ();

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i, ++cToFortranPointer_Iterator, ++OP_DAT_Iterator, ++dataSize_Iterator)
  {
    // 'opDatI%dat'
    SgVarRefExp * opDatArgumentReference = buildVarRefExp (*OP_DAT_Iterator);

    SgExpression * opDatIDatField = buildDotExp (opDatArgumentReference,
        buildOpaqueVarRefExp ("dat", scope));

    // transform the expression c2fPtrI to a variable reference
    SgVarRefExp * c2fPtrIVarRef = buildVarRefExp (*cToFortranPointer_Iterator);

    SgInitializedNamePtrList & variables =
        (*dataSize_Iterator)->get_variables ();

    SgExpression * shapeExpr = buildOpaqueVarRefExp ("(/"
        + variables.front ()->get_name ().getString () + "/)", scope);

    SgExprListExp * c2fFunActualParameters = buildExprListExp (opDatIDatField,
        c2fPtrIVarRef, shapeExpr);

    // TODO: transform types to the correct ones!
    SgFunctionParameterTypeList * c2fFunInputTypes =
        buildFunctionParameterTypeList (buildIntType (), buildIntType (),
            buildIntType ());

    SgFunctionType * c2fFunType = buildFunctionType (buildVoidType (),
        c2fFunInputTypes);
    SgFunctionRefExp * c2fFunRef = buildFunctionRefExp ("c_f_pointer",
        c2fFunType, scope);

    SgFunctionSymbol * c2fFunSymbol = c2fFunRef->get_symbol_i ();
    SgFunctionCallExp * c2fFunCall = buildFunctionCallExp (c2fFunSymbol,
        c2fFunActualParameters);

    SgStatement * c2fFunCallStmt = buildExprStatement (c2fFunCall);

    appendStatement (c2fFunCallStmt, scope);
  }

  /*
   * ======================================================
   * 'userArgument0 = c2fPtr0'
   * ...
   * 'userArgumentN-1 = c2fPtrN-1'
   * ======================================================
   */

  kernelSubroutineArgument_Iterator
      = hostSubroutine.first_KernelSubroutineActualParameter ();
  cToFortranPointer_Iterator = hostSubroutine.first_CToFortranPointer ();

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i, ++kernelSubroutineArgument_Iterator, ++cToFortranPointer_Iterator)
  {
    SgExpression * assignmentExpression = buildAssignOp (buildVarRefExp (
        *kernelSubroutineArgument_Iterator), buildVarRefExp (
        *cToFortranPointer_Iterator));

    appendStatement (buildExprStatement (assignmentExpression), scope);
  }

  /*
   * ======================================================
   * 'call KERNEL_NAME<<<gsize,bsize,reduct_shared>>> (set%size, userArgument0, ..., userArgumentN-1)'
   * ======================================================
   */

  SgExprListExp * kernelParameters = buildExprListExp ();

  SgExpression * iterationSetVarRef = buildVarRefExp (
      hostSubroutine.get_OP_SET_Argument ());
  SgExpression * setFieldSizeExpression = buildDotExp (iterationSetVarRef,
      buildOpaqueVarRefExp ("size", scope));
  kernelParameters->append_expression (setFieldSizeExpression);

  kernelSubroutineArgument_Iterator
      = hostSubroutine.first_KernelSubroutineActualParameter ();
  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i, ++kernelSubroutineArgument_Iterator)
  {
    kernelParameters->append_expression (buildVarRefExp (
        *kernelSubroutineArgument_Iterator));
  }

  SgExprStatement * kernelCall = buildFunctionCallStmt (
      op2ParallelLoop.getKernelSubroutine ().getSubroutineName ()
          + "<<<gsize,bsize,reduct_shared>>>", buildVoidType (),
      kernelParameters, scope);

  appendStatement (kernelCall, scope);

  /*
   * ======================================================
   * 'c2fPtr0 = userArgument0'
   * ...
   * 'c2fPtrN-1 = userArgumentN-1'
   * ======================================================
   */

  kernelSubroutineArgument_Iterator
      = hostSubroutine.first_KernelSubroutineActualParameter ();
  cToFortranPointer_Iterator = hostSubroutine.first_CToFortranPointer ();

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i, ++kernelSubroutineArgument_Iterator, ++cToFortranPointer_Iterator)
  {
    SgExpression * assignmentExpression = buildAssignOp (buildVarRefExp (
        *cToFortranPointer_Iterator), buildVarRefExp (
        *kernelSubroutineArgument_Iterator));

    SgExprStatement * assignmentStatement = buildExprStatement (
        assignmentExpression);
    appendStatement (assignmentStatement, scope);

    lastAppendedStatement = assignmentStatement;
  }

  ROSE_ASSERT (lastAppendedStatement != NULL);

  /*
   * ======================================================
   *  'deallocate (userArgument0)'
   *  ...
   *  'deallocate (userArgumentN-1)'
   * ======================================================
   */

  Debug::getInstance ()->debugMessage (
      "Adding 'deallocate' function calls for device variables", 5);

  kernelSubroutineArgument_Iterator
      = hostSubroutine.first_KernelSubroutineActualParameter ();

  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); ++i, ++kernelSubroutineArgument_Iterator)
  {
    /*
     * TODO: Unfortunately, we cannot use CALL with a Fortran function
     * Instead we have to use 'addTextForUnaparser' for now
     */
    string const deallocateStatement = "deallocate (" + buildVarRefExp (
        *kernelSubroutineArgument_Iterator)->unparseToString () + ")\n";

    addTextForUnparser (lastAppendedStatement, deallocateStatement,
        AstUnparseAttribute::e_after);
  }
}

void
CreateKernels::createHostSubroutineCUDAVariables (SgScopeStatement * scope,
    OP2ParallelLoop & op2ParallelLoop)
{
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildAssignInitializer;
  using SageBuilder::buildIntVal;
  using SageBuilder::buildIntType;
  using SageBuilder::buildDotExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildAddOp;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildSubtractOp;
  using SageBuilder::buildDoubleType;
  using SageBuilder::buildMultiplyOp;
  using SageBuilder::buildAssignOp;
  using SageBuilder::buildDivideOp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildExprStatement;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildFunctionType;
  using SageBuilder::buildFunctionParameterTypeList;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", 2);

  /*
   * ======================================================
   * This function builds the following Fortran code:
   *
   * integer(4) :: bsize          = BSIZE_DEFAULT
   * integer(4) :: reduct_bytes   = 0
   * integer(4) :: reduct_size    = 0
   * integer(4) :: const_bytes    = 0
   * integer(4) :: gsize
   * real(8)    :: reduct_shared
   *
   * gsize         = int ((set%size - 1) / bsize + 1)
   * reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
   * ======================================================
   */

  /*
   * ======================================================
   * Declaration and initialisation of variables and opaque
   * variables
   * ======================================================
   */
  SgVarRefExp * variable_BSIZE_DEFAULT = buildOpaqueVarRefExp ("BSIZE_DEFAULT",
      scope);

  SgVariableDeclaration * variable_bsize = buildVariableDeclaration ("bsize",
      FortranTypeDeclarations::getFourByteInteger (), buildAssignInitializer (
          variable_BSIZE_DEFAULT, buildIntType ()), scope);

  SgVariableDeclaration * variable_gsize = buildVariableDeclaration ("gsize",
      FortranTypeDeclarations::getFourByteInteger (), NULL, scope);

  SgVariableDeclaration * variable_reduct_bytes = buildVariableDeclaration (
      "reduct_bytes", FortranTypeDeclarations::getFourByteInteger (),
      buildAssignInitializer (buildIntVal (0), buildIntType ()), scope);

  SgVariableDeclaration * variable_reduct_size = buildVariableDeclaration (
      "reduct_size", FortranTypeDeclarations::getFourByteInteger (),
      buildAssignInitializer (buildIntVal (0), buildIntType ()), scope);

  SgVariableDeclaration * variable_reduct_shared = buildVariableDeclaration (
      "reduct_shared", FortranTypeDeclarations::getDoublePrecisionFloat (),
      NULL, scope);

  SgVariableDeclaration * variable_const_bytes = buildVariableDeclaration (
      "const_bytes", FortranTypeDeclarations::getFourByteInteger (),
      buildAssignInitializer (buildIntVal (0), buildIntType ()), scope);

  variable_bsize->get_declarationModifier ().get_accessModifier ().setUndefined ();
  variable_gsize->get_declarationModifier ().get_accessModifier ().setUndefined ();
  variable_reduct_bytes->get_declarationModifier ().get_accessModifier ().setUndefined ();
  variable_reduct_size->get_declarationModifier ().get_accessModifier ().setUndefined ();
  variable_reduct_shared->get_declarationModifier ().get_accessModifier ().setUndefined ();
  variable_const_bytes->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (variable_bsize, scope);
  appendStatement (variable_gsize, scope);
  appendStatement (variable_reduct_bytes, scope);
  appendStatement (variable_reduct_size, scope);
  appendStatement (variable_reduct_shared, scope);
  appendStatement (variable_const_bytes, scope);

  /*
   * ======================================================
   * gsize = int ((set%size - 1) / bsize + 1)
   * ======================================================
   */

  // 'set%size'
  SgExpression * setPERCENTsize = buildDotExp (buildVarRefExp (
      op2ParallelLoop.getHostSubroutine ().get_OP_SET_Argument ()),
      buildOpaqueVarRefExp ("size", scope));

  // 'set%size - 1'
  SgExpression * setPERCENTsize_minusOne = buildSubtractOp (setPERCENTsize,
      buildIntVal (1));

  // '(set%size - 1) / bsize  + 1'
  SgExpression * castIntegerParameter = buildAddOp (buildDivideOp (
      setPERCENTsize_minusOne, buildVarRefExp (variable_bsize)),
      buildIntVal (1));

  /*
   * ======================================================
   * 'int ((set%size - 1) / bsize + 1)'
   *
   * To do this we have to build a function type for the
   * intrinsic 'int'. We do not care about the fact that the
   * function is already defined (also via an intrinsic): we
   * just tell ROSE all the information needed to build a new
   * function
   * ======================================================
   */
  SgFunctionType * functionDoubleToInteger = buildFunctionType (
      buildIntType (), buildFunctionParameterTypeList (buildDoubleType ()));

  SgFunctionRefExp * referenceToIntegerFunction = buildFunctionRefExp ("int",
      functionDoubleToInteger, scope);

  SgExprListExp * intParams = buildExprListExp (castIntegerParameter);

  SgFunctionCallExp * intIntrinsicCall = buildFunctionCallExp (
      referenceToIntegerFunction->get_symbol_i (), intParams);

  // 'gsize = int ((set%size - 1) / bsize + 1)'
  SgExpression * variable_gsize_assignment = buildAssignOp (buildVarRefExp (
      variable_gsize), intIntrinsicCall);

  appendStatement (buildExprStatement (variable_gsize_assignment), scope);

  /*
   * ======================================================
   * reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
   * ======================================================
   */

  // 'reduct_size * (BSIZE_DEFAULT / 2)'
  SgExpression * rhsOfExpression = buildMultiplyOp (buildVarRefExp (
      variable_reduct_size), buildDivideOp (variable_BSIZE_DEFAULT,
      buildIntVal (2)));

  // 'reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)'
  SgExpression * variable_reduct_shared_assignment = buildAssignOp (
      buildVarRefExp (variable_reduct_shared), rhsOfExpression);

  appendStatement (buildExprStatement (variable_reduct_shared_assignment),
      scope);
}

void
CreateKernels::setUp_OP_DAT_ArgumentTypes (
    std::vector <SgType *> & opDatArgumentTypes,
    SgVariableDeclaration * setSizeFormalParameter,
    OP2ParallelLoop & op2ParallelLoop)
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
  for (OP_DAT_iterator = op2ParallelLoop.first_OP_DAT (); OP_DAT_iterator
      != op2ParallelLoop.last_OP_DAT (); ++OP_DAT_iterator)
  {
    string const & opDatVariableName = OP_DAT_iterator->first;
    OP_DAT_Declaration * opDatDeclaration = OP_DAT_iterator->second;

    for (unsigned int i = 0; i < op2ParallelLoop.get_OP_DAT_Occurrences (
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
          buildIntVal (0), upperBound, buildIntVal (1));

      arraySubscript->set_startOfConstruct (fileInfo);
      arraySubscript->set_endOfConstruct (fileInfo);
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
CreateKernels::createMainKernelDeviceSubroutine_ForIndirectLoop (
    SgScopeStatement * moduleScope, OP2ParallelLoop & op2ParallelLoop)
{
  Debug::getInstance ()->debugMessage (
      "Building main kernel routine for indirect loop", 2);
}

void
CreateKernels::createMainKernelDeviceSubroutine_ForDirectLoop (
    SgScopeStatement * moduleScope, OP2ParallelLoop & op2ParallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildOpaqueVarRefExp;
  using SageBuilder::buildVarRefExp;
  using SageBuilder::buildFunctionCallExp;
  using SageBuilder::buildFunctionRefExp;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildIntType;
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
  using SageBuilder::buildFunctionParameterList;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Building main kernel routine for direct loop", 2);

  /*
   * ======================================================
   * Create the new subroutine, its formal parameters, and
   * its local variables
   * ======================================================
   */

  SgFunctionParameterList * newSubroutineParameters =
      buildFunctionParameterList ();

  SgProcedureHeaderStatement * newSubroutine = buildProcedureHeaderStatement (
      op2ParallelLoop.getKernelSubroutine ().getSubroutineName ().c_str (),
      buildVoidType (), newSubroutineParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (newSubroutine, "attributes(global) ",
      AstUnparseAttribute::e_before);

  appendStatement (newSubroutine, moduleScope);

  SgScopeStatement * newSubroutineScope =
      newSubroutine->get_definition ()->get_body ();

  /*
   * ======================================================
   * Formal parameters
   * ======================================================
   */
  string const setSizeName = "setSize";
  SgVariableDeclaration * setSizeFormalParameter = buildVariableDeclaration (
      setSizeName, buildIntType (), NULL, newSubroutineScope);

  setSizeFormalParameter->get_declarationModifier ().get_accessModifier ().setUndefined ();
  setSizeFormalParameter->get_declarationModifier ().get_typeModifier ().setValue ();

  newSubroutineParameters->append_arg (
      *(setSizeFormalParameter->get_variables ().begin ()));

  appendStatement (setSizeFormalParameter, newSubroutineScope);

  // b. Building remaining formal parameters types from program
  vector <SgType *> argTypes;
  setUp_OP_DAT_ArgumentTypes (argTypes, setSizeFormalParameter, op2ParallelLoop);

  // b. create formal parameters (we have a OP_DAT argument starting from 3 and then every 4 arguments
  // (4 is stored inside a macro (ARG_LINE_LENGTH) copied inside variable argLineLength in constructor
  // retriving op_dat original type from argument: must generate real(8), dimension(0:setsize * 4 -1)
  // where 4 is the carinality of each input op_dat element associate to each set element (can be found
  // in the op_decl_dat call)
  for (unsigned int i = 0; i
      < op2ParallelLoop.getNumberOf_OP_DAT_ArgumentGroups (); i++)
  {
    /*
     * ======================================================
     * Create 'argi' formal parameter
     * ======================================================
     */
    string const argName = "arg" + lexical_cast <string> (i);

    SgVariableDeclaration * argFormalParameter = buildVariableDeclaration (
        argName, argTypes[i], NULL, newSubroutineScope);

    newSubroutineParameters->append_arg (
        *(argFormalParameter->get_variables ().begin ()));

    argFormalParameter->get_declarationModifier ().get_typeModifier ().setIntent_in ();
    argFormalParameter->get_declarationModifier ().get_typeModifier ().setDevice ();
    argFormalParameter->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (argFormalParameter, newSubroutineScope);
  }

  /*
   * ======================================================
   * Local variables
   * ======================================================
   */
  string const iterSetName = "setIter";

  SgVariableDeclaration * iterSetVariable = buildVariableDeclaration (
      *(new SgName (iterSetName)), buildIntType (), NULL, newSubroutineScope);

  iterSetVariable->get_declarationModifier ().get_typeModifier ().setDevice ();
  iterSetVariable->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (iterSetVariable, newSubroutineScope);

  /*
   * ======================================================
   * Build opaque variable references needed in the following
   * expressions. These are opaque because the variables are
   * provided as part of the CUDA library and are not seen
   * by ROSE
   * ======================================================
   */
  SgVarRefExp * variableThreadidx = buildOpaqueVarRefExp ("threadidx",
      newSubroutineScope);
  SgVarRefExp * variableX = buildOpaqueVarRefExp ("x", newSubroutineScope);
  SgVarRefExp * variableBlockidx = buildOpaqueVarRefExp ("blockidx",
      newSubroutineScope);
  SgVarRefExp * variableBlockdim = buildOpaqueVarRefExp ("blockdim",
      newSubroutineScope);
  SgVarRefExp * variableGridDim = buildOpaqueVarRefExp ("griddim",
      newSubroutineScope);

  /*
   * ======================================================
   * 'setIter = (threadidx%x - 1) + (blockidx%x -1)  * blockdim%x'
   * ======================================================
   */

  // 'threadidx%x - 1'
  SgExpression * threadIdXMinusOne = buildSubtractOp (buildDotExp (
      variableThreadidx, variableX), buildIntVal (1));

  // 'blockidx%x - 1'
  SgExpression * blockIdXMinusOne = buildSubtractOp (buildDotExp (
      variableBlockidx, variableX), buildIntVal (1));

  // 'blockdim%x'
  SgExpression * blockDimX = buildDotExp (variableBlockdim, variableX);

  // '(blockidx%x -1)  * blockdim%x'
  SgExpression * blockIdXMinusOnePerBlockDimx = buildMultiplyOp (
      blockIdXMinusOne, blockDimX);

  // 'setIter = (threadidx%x - 1) + (blockidx%x -1)  * blockdim%x'
  SgVarRefExp * setIterRef = buildVarRefExp (iterSetName, newSubroutineScope);
  SgExpression * setIterInitialAssignement = buildAssignOp (setIterRef,
      buildAddOp (threadIdXMinusOne, blockIdXMinusOnePerBlockDimx));

  appendStatement (buildExprStatement (setIterInitialAssignement),
      newSubroutineScope);

  /*
   * ======================================================
   * Build the do-while loop
   * ======================================================
   */

  /*
   * ======================================================
   *  Build a call to the user function
   * ======================================================
   */
  SgFunctionRefExp * userKernelRefExp = buildFunctionRefExp (
      op2ParallelLoop.getUserDeviceSubroutine ().getSubroutineName (),
      newSubroutineScope->get_scope ());

  // 2. build parameters: q(setIter * dim:setIter * dim + dim - 1), qold(setIter * dim:setIter*dim + dim - 1)
  // dim is obtained from the previous parsing
  SgExprListExp * userKernelParams = createUserDeviceFunctionParameters (
      newSubroutineParameters, setIterRef, newSubroutineScope, op2ParallelLoop);

  // 3. build call
  SgFunctionCallExp * userKernelCall = buildFunctionCallExp (userKernelRefExp,
      userKernelParams);

  // 4. build corresponding statement
  SgExprStatement * userKernelCallStmt = buildExprStatement (userKernelCall);

  // build set iteration variable increment: setIter = setIter + blockdim%x * griddim%x
  // (we are only missing griddim%x)
  SgExpression * gridDimXAcc = buildDotExp (variableGridDim, variableX);

  // build multiplication: blockdim%x * griddim%x
  SgExpression * blockDimMult = buildMultiplyOp (blockDimX, gridDimXAcc);

  // build sum: setIter + blockdim%x * griddim%x
  SgExpression * sumIterPlusBlockGridX = buildAddOp (setIterRef, blockDimMult);

  // build assignment: setIter = setIter + blockdim%x * griddim%x
  SgExpression * setIterUpdate = buildAssignOp (setIterRef,
      sumIterPlusBlockGridX);

  // 4. build corresponding statement
  SgExprStatement * setIterUpdateStmt = buildExprStatement (setIterUpdate);

  SgBasicBlock * doWhileBlock = buildBasicBlock (userKernelCallStmt,
      setIterUpdateStmt);

  // Build do while statement (do while ( setIter < setsize ))

  // building reference to setsize parameter
  SgVarRefExp * setSizeFormalParRef = buildVarRefExp (setSizeName,
      newSubroutineScope);

  // build do-while guard
  SgExpression * inequalityExp = buildLessThanOp (setIterRef,
      setSizeFormalParRef);

  // build do-while statement
  SgWhileStmt * setWhileStmt = buildWhileStmt (inequalityExp, doWhileBlock);

  // we need to set the end do statement, because the unparse is not able to infer it automatically
  setWhileStmt->set_has_end_statement (true);

  // append do-while statement
  appendStatement (setWhileStmt, newSubroutineScope);
}

void
CreateKernels::createHostSubroutineLocals (SgScopeStatement* subroutineScope,
    OP2ParallelLoop & op2ParallelLoop)
{
  using boost::lexical_cast;
  using SageBuilder::buildIntType;
  using SageBuilder::buildPointerType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;
  using std::map;

  Debug::getInstance ()->debugMessage ("Creating local variables", 2);

  /*
   * ======================================================
   * This function builds the following Fortran code:
   *
   * INTEGER :: data0Size
   * INTEGER :: dataN-1Size
   *
   * real(8), dimension(:), allocatable, device :: argument0
   * real(8), dimension(:), allocatable, device :: argumentN-1
   *
   * real(8), dimension(:), pointer :: c2fPtr0
   * real(8), dimension(:), pointer :: c2fPtrN-1
   * ======================================================
   */

  map <string, OP_DAT_Declaration *>::const_iterator OP_DAT_iterator;

  HostSubroutineOfDirectLoop
      & hostSubroutine =
          dynamic_cast <HostSubroutineOfDirectLoop &> (op2ParallelLoop.getHostSubroutine ());

  /*
   * ======================================================
   * INTEGER :: data0Size
   * ...
   * INTEGER :: dataN-1Size
   * ======================================================
   */
  int i = 0;

  for (OP_DAT_iterator = op2ParallelLoop.first_OP_DAT (); OP_DAT_iterator
      != op2ParallelLoop.last_OP_DAT (); ++OP_DAT_iterator)
  {
    for (unsigned int count = 0; count
        < op2ParallelLoop.get_OP_DAT_Occurrences (OP_DAT_iterator->first); ++count)
    {
      string const variableName = "data" + lexical_cast <string> (i) + "Size";

      SgVariableDeclaration * variable_dataNSize = buildVariableDeclaration (
          variableName, buildIntType (), NULL, subroutineScope);

      variable_dataNSize->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variable_dataNSize, subroutineScope);

      hostSubroutine.setSizeOf_OP_DAT_Variable (variable_dataNSize);

      ++i;
    }
  }

  /*
   * ======================================================
   * real(8), dimension(:), allocatable, device :: userArgument0
   * ...
   * real(8), dimension(:), allocatable, device :: userArgumentN-1
   * ======================================================
   */
  i = 0;

  for (OP_DAT_iterator = op2ParallelLoop.first_OP_DAT (); OP_DAT_iterator
      != op2ParallelLoop.last_OP_DAT (); ++OP_DAT_iterator)
  {
    OP_DAT_Declaration * opDatDeclaration = OP_DAT_iterator->second;

    for (unsigned int count = 0; count
        < op2ParallelLoop.get_OP_DAT_Occurrences (OP_DAT_iterator->first); ++count)
    {
      string const variableName = "userArgument" + lexical_cast <string> (i);

      SgVariableDeclaration * user_argument_n = buildVariableDeclaration (
          variableName, opDatDeclaration->getActualType (), NULL,
          subroutineScope);

      user_argument_n->get_declarationModifier ().get_typeModifier ().setDevice ();
      user_argument_n->get_declarationModifier ().get_typeModifier ().setAllocatable ();
      user_argument_n->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (user_argument_n, subroutineScope);

      hostSubroutine.setKernelSubroutineActualParameter (user_argument_n);

      ++i;
    }
  }

  /*
   * ======================================================
   * real(8), dimension(:), pointer :: c2fPtr0
   * ...
   * real(8), dimension(:), pointer :: c2fPtrN-1
   * ======================================================
   */
  i = 0;

  for (OP_DAT_iterator = op2ParallelLoop.first_OP_DAT (); OP_DAT_iterator
      != op2ParallelLoop.last_OP_DAT (); ++OP_DAT_iterator)
  {
    OP_DAT_Declaration * opDatDeclaration = OP_DAT_iterator->second;

    for (unsigned int count = 0; count
        < op2ParallelLoop.get_OP_DAT_Occurrences (OP_DAT_iterator->first); ++count)
    {
      string const variableName = "c2fPtr" + lexical_cast <string> (i);

      SgVariableDeclaration * variable_c2fptr_n = buildVariableDeclaration (
          variableName, buildPointerType (opDatDeclaration->getActualType ()),
          NULL, subroutineScope);

      variable_c2fptr_n->get_declarationModifier ().get_accessModifier ().setUndefined ();

      appendStatement (variable_c2fptr_n, subroutineScope);

      hostSubroutine.setCToFortranPointer (variable_c2fptr_n);

      ++i;
    }
  }

  /*
   * ======================================================
   * Add CUDA configuration parameters
   * ======================================================
   */
  createHostSubroutineCUDAVariables (subroutineScope, op2ParallelLoop);
}

void
CreateKernels::createHostSubroutineFormalParamaters (
    SgScopeStatement * subroutineScope,
    SgFunctionParameterList * hostParameters, OP2ParallelLoop & op2ParallelLoop)
{
  using boost::iequals;
  using boost::lexical_cast;
  using SageBuilder::buildCharType;
  using SageBuilder::buildExprListExp;
  using SageBuilder::buildArrayType;
  using SageBuilder::buildVariableDeclaration;
  using SageBuilder::buildIntType;
  using SageInterface::appendStatement;
  using std::vector;
  using std::string;

  /*
   * ======================================================
   * This variable is the integer suffix appended to formal
   * parameters in an OP_DAT batch of arguments
   * ======================================================
   */
  int variableNameSuffix = -1;

  for (vector <SgExpression *>::const_iterator it =
      op2ParallelLoop.getActualArguments ().begin (); it
      != op2ParallelLoop.getActualArguments ().end (); ++it)
  {
    Debug::getInstance ()->debugMessage ("Argument type: "
        + (*it)->class_name (), 8);

    switch ((*it)->variantT ())
    {
      case V_SgFunctionRefExp:
      {
        /*
         * ======================================================
         * Found the kernel name argument
         * ======================================================
         */

        string const variableName = "subroutineName";

        /*
         * Build asterisk shape expression indicating that the size of the
         * array is decided at run time
         */
        SgAsteriskShapeExp * asterisk = new SgAsteriskShapeExp (fileInfo);
        asterisk->set_endOfConstruct (fileInfo);

        /*
         * Build an array of characters
         */
        SgArrayType * charArray = buildArrayType (buildCharType (), asterisk);

        /*
         * The dimension of the array is the asterisk
         */
        SgExprListExp * dimensionExprList = buildExprListExp (asterisk);
        charArray->set_dim_info (dimensionExprList);
        charArray->set_rank (1);

        SgVariableDeclaration * charArrayDeclaration =
            buildVariableDeclaration (variableName, charArray, NULL,
                subroutineScope);

        hostParameters->append_arg (
            *(charArrayDeclaration->get_variables ().begin ()));

        charArrayDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
        charArrayDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

        appendStatement (charArrayDeclaration, subroutineScope);

        break;
      }

      case V_SgVarRefExp:
      {
        SgVarRefExp * variableReference = isSgVarRefExp (*it);

        switch (variableReference->get_type ()->variantT ())
        {
          case V_SgClassType:
          {
            SgClassType* classReference = isSgClassType (
                variableReference->get_type ());
            string const className = classReference->get_name ().getString ();

            if (iequals (className, OP2::OP_SET_NAME))
            {
              /*
               * ======================================================
               * Found an OP_SET argument
               * ======================================================
               */
              string const variableName = "iterationSet";

              SgVariableDeclaration * opSetDeclaration =
                  buildVariableDeclaration (variableName, classReference, NULL,
                      subroutineScope);

              opSetDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
              opSetDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              hostParameters->append_arg (
                  *(opSetDeclaration->get_variables ().begin ()));

              appendStatement (opSetDeclaration, subroutineScope);

              op2ParallelLoop.getHostSubroutine ().set_OP_SET_Argument (
                  opSetDeclaration);
            }

            else if (iequals (className, OP2::OP_MAP_NAME))
            {
              /*
               * ======================================================
               * Found an OP_MAP argument
               * ======================================================
               */
              string const variableName = OP2::OP_MAP_VariableNamePrefix
                  + lexical_cast <string> (variableNameSuffix);

              SgVariableDeclaration * opMapDeclaration =
                  buildVariableDeclaration (variableName, classReference, NULL,
                      subroutineScope);

              opMapDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
              opMapDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              hostParameters->append_arg (
                  *(opMapDeclaration->get_variables ().begin ()));

              appendStatement (opMapDeclaration, subroutineScope);

              op2ParallelLoop.getHostSubroutine ().set_OP_MAP_Argument (
                  opMapDeclaration);
            }

            else if (iequals (className, OP2::OP_DAT_NAME))
            {
              /*
               * ======================================================
               * Found an OP_DAT argument
               * ======================================================
               */

              /*
               * ======================================================
               * A new batch of OP_DAT arguments has been discovered
               * Therefore, increment the variable name suffix
               * ======================================================
               */
              variableNameSuffix++;

              string const variableName = OP2::OP_DAT_VariableNamePrefix
                  + lexical_cast <string> (variableNameSuffix);

              SgVariableDeclaration * opDatDeclaration =
                  buildVariableDeclaration (variableName, classReference, NULL,
                      subroutineScope);

              opDatDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
              opDatDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              hostParameters->append_arg (
                  *(opDatDeclaration->get_variables ().begin ()));

              appendStatement (opDatDeclaration, subroutineScope);

              op2ParallelLoop.getHostSubroutine ().set_OP_DAT_Argument (
                  opDatDeclaration);
            }
            else
            {
              Debug::getInstance ()->errorMessage ("Unrecognised class: "
                  + className);
            }

            break;
          }

          case V_SgTypeInt:
          {
            /*
             * ======================================================
             * Found an OP_ACCESS argument
             * ======================================================
             */

            string const variableName = OP2::OP_ACCESS_VariableNamePrefix
                + lexical_cast <string> (variableNameSuffix);

            SgVariableDeclaration * opAccessDeclaration =
                buildVariableDeclaration (variableName, buildIntType (), NULL,
                    subroutineScope);

            opAccessDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
            opAccessDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

            hostParameters->append_arg (
                *(opAccessDeclaration->get_variables ().begin ()));

            appendStatement (opAccessDeclaration, subroutineScope);

            op2ParallelLoop.getHostSubroutine ().set_OP_ACESS_Argument (
                opAccessDeclaration);

            break;
          }

          default:
          {
            break;
          }
        }

        break;
      }

      case V_SgMinusOp:
      {
        /*
         * ======================================================
         * Found an indirection argument
         * ======================================================
         */

        string const variableName = OP2::OP_INDIRECTION_VariableNamePrefix
            + lexical_cast <string> (variableNameSuffix);

        SgVariableDeclaration * opIndirectionDeclaration =
            buildVariableDeclaration (variableName, buildIntType (), NULL,
                subroutineScope);

        opIndirectionDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
        opIndirectionDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

        hostParameters->append_arg (
            *(opIndirectionDeclaration->get_variables ().begin ()));

        appendStatement (opIndirectionDeclaration, subroutineScope);

        op2ParallelLoop.getHostSubroutine ().set_OP_INDIRECTION_Argument (
            opIndirectionDeclaration);

        break;
      }

      default:
      {
        break;
      }
    }
  }
}

SgProcedureHeaderStatement *
CreateKernels::createHostSubroutine (SgScopeStatement * moduleScope,
    OP2ParallelLoop & op2ParallelLoop)
{
  using SageBuilder::buildFunctionParameterList;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildProcedureHeaderStatement;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;

  Debug::getInstance ()->debugMessage (
      "Creating host subroutine for direct loop", 2);

  /*
   * ======================================================
   * Create the host subroutine
   * ======================================================
   */
  SgFunctionParameterList * hostParameters = buildFunctionParameterList ();

  SgProcedureHeaderStatement
      * subroutineStatement =
          buildProcedureHeaderStatement (
              op2ParallelLoop.getHostSubroutine ().getSubroutineName ().c_str (),
              buildVoidType (), hostParameters,
              SgProcedureHeaderStatement::e_subroutine_subprogram_kind,
              moduleScope);

  appendStatement (subroutineStatement, moduleScope);

  addTextForUnparser (subroutineStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  /*
   * ======================================================
   * Add formal parameters
   * ======================================================
   */
  createHostSubroutineFormalParamaters (
      subroutineStatement->get_definition ()->get_body (), hostParameters,
      op2ParallelLoop);

  return subroutineStatement;
}

SgScopeStatement *
CreateKernels::createCUDAModule (SgSourceFile & sourceFile,
    OP2ParallelLoop & op2ParallelLoop)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating direct loop CUDA module", 2);

  SgGlobal * globalScope = sourceFile.get_globalScope ();

  /*
   * ======================================================
   * Create the module statement
   * ======================================================
   */
  SgModuleStatement * cudaModule =
      dynamic_cast <SgModuleStatement *> (buildNewTypeDeclaration (
          op2ParallelLoop.getCUDAModuleName (), globalScope, true));

  cudaModule->get_definition ()->setCaseInsensitive (true);

  appendStatement (cudaModule, globalScope);

  /*
   * ======================================================
   * Add the 'use' statements of this module
   * ======================================================
   */
  vector <string> libs;
  libs.push_back ("ISO_C_BINDING");
  libs.push_back ("OP2_C");
  libs.push_back ("cudafor");

  for (vector <string>::const_iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        sourceFile.get_file_info (), (*it), false);
    useStatement->set_definingDeclaration (cudaModule);
    appendStatement (useStatement, cudaModule->get_definition ());
  }

  /*
   * ======================================================
   * Add the 'contains' statement
   * ======================================================
   */
  SgContainsStatement * containsStatement = new SgContainsStatement (
      sourceFile.get_file_info ());
  containsStatement->set_parent (cudaModule->get_definition ());
  containsStatement->set_definingDeclaration (containsStatement);

  SgDeclarationStatementPtrList & statementList =
      cudaModule->get_definition ()->getDeclarationList ();
  statementList.push_back (containsStatement);

  return containsStatement->get_scope ();
}

SgSourceFile &
CreateKernels::createSourceFile (OP2ParallelLoop & op2ParallelLoop)
{
  using std::string;
  using SageBuilder::buildFile;

  /*
   * ======================================================
   * To create a new file (to which the AST is later unparsed),
   * the API expects the name of an existing file and the
   * name of the output file. There is no input file corresponding
   * to our output file, therefore we first create a dummy
   * Fortran file
   * ======================================================
   */
  string const inputFileName = "BLANK_" + op2ParallelLoop.getCUDAModuleName ()
      + ".F95";

  FILE * inputFile = fopen (inputFileName.c_str (), "w+");
  if (inputFile != NULL)
  {
    Debug::getInstance ()->debugMessage ("Creating dummy source file '"
        + inputFileName + "'", 2);
    fclose (inputFile);
  }
  else
  {
    Debug::getInstance ()->errorMessage (
        "Could not create dummy Fortran file '" + inputFileName + "'");
  }

  /*
   * ======================================================
   * Now generate the CUDA file
   * ======================================================
   */
  string const outputFileName = op2ParallelLoop.getCUDAModuleName () + ".CUF";

  Debug::getInstance ()->debugMessage ("Generating CUDA file '"
      + outputFileName + "'", 2);

  SgSourceFile * sourceFile = isSgSourceFile (buildFile (inputFileName,
      outputFileName, NULL));

  /*
   * ======================================================
   * Later unparse according to the Fortran 95 standard
   * ======================================================
   */
  sourceFile->set_F95_only (true);

  /*
   * ======================================================
   * No implicit symbols shall be allowed in the generated
   * Fortran file
   * ======================================================
   */
  sourceFile->set_fortran_implicit_none (true);

  sourceFile->set_outputFormat (SgFile::e_free_form_output_format);

  /*
   * ======================================================
   * Store the file so it can be unparsed after AST
   * construction
   * ======================================================
   */
  generatedFiles.push_back (sourceFile);

  return *sourceFile;
}

/*
 * ======================================================
 * Public functions
 * ======================================================
 */

void
CreateKernels::visit (SgNode * node)
{
  using boost::lexical_cast;
  using boost::starts_with;
  using std::string;
  using std::pair;
  using std::make_pair;

  switch (node->variantT ())
  {
    case V_SgFunctionCallExp:
    {
      /*
       * ======================================================
       * Function call found in AST
       * ======================================================
       */
      SgFunctionCallExp* functionCallExp = isSgFunctionCallExp (node);

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

      /*
       * ======================================================
       * The prefix of all parallel loop calls in Fortran contains
       * 'OP_PAR_LOOP'. The suffix of the call, however, changes
       * depending on the number of expected parameters. Therefore,
       * any match of this prefix indicates a call of interest
       * to the translator
       * ======================================================
       */
      if (starts_with (calleeName, OP2::OP_PAR_LOOP_PREFIX))
      {
        /*
         * ======================================================
         * The first argument to an 'OP_PAR_LOOP' call should be
         * a reference to the kernel function. Cast it and proceed,
         * otherwise throw an exception
         * ======================================================
         */
        SgExpressionPtrList & actualArguments =
            functionCallExp->get_args ()->get_expressions ();

        SgFunctionRefExp * functionRefExp = isSgFunctionRefExp (
            actualArguments.front ());
        try
        {
          if (functionRefExp != NULL)
          {
            string const
                kernelHostName =
                    functionRefExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

            Debug::getInstance ()->debugMessage ("Found '" + calleeName
                + "' with host kernel '" + kernelHostName + "'", 1);

            if (op2ParallelLoops.find (kernelHostName)
                == op2ParallelLoops.end ())
            {
              /*
               * ======================================================
               * If this kernel has not been previously encountered then build
               * the CUDA-Fortran file and modify the calls in the original
               * Fortran source code
               * ======================================================
               */

              OP2ParallelLoop * op2ParallelLoop = new OP2ParallelLoop (
                  kernelHostName, actualArguments, declarations);

              op2ParallelLoops.insert (make_pair (kernelHostName,
                  op2ParallelLoop));

              /*
               * ======================================================
               * Generate an additional source file for this OP_PAR_LOOP
               * ======================================================
               */
              SgSourceFile & sourceFile = createSourceFile (*op2ParallelLoop);

              /*
               * ======================================================
               * Create the CUDA module
               * ======================================================
               */
              SgScopeStatement * moduleScope = createCUDAModule (sourceFile,
                  *op2ParallelLoop);

              /*
               * ======================================================
               * Generate and modify user kernel so that it can run on
               * the device
               * ======================================================
               */

              UserDeviceSubroutine & userDeviceSubroutine =
                  op2ParallelLoop->getUserDeviceSubroutine ();

              userDeviceSubroutine.copyAndModifySubroutine (moduleScope,
                  op2ParallelLoop->getUserHostSubroutine (),
                  *declarations);

              if (op2ParallelLoop->isDirectLoop ())
              {
                /*
                 * ======================================================
                 * Direct loop
                 * ======================================================
                 */

                createMainKernelDeviceSubroutine_ForDirectLoop (moduleScope,
                    *op2ParallelLoop);

                SgProcedureHeaderStatement * hostSubroutine =
                    createHostSubroutine (moduleScope, *op2ParallelLoop);

                SgScopeStatement * subroutineScope =
                    hostSubroutine->get_definition ()->get_body ();

                /*
                 * ======================================================
                 * Add local variables
                 * ======================================================
                 */
                createHostSubroutineLocals (subroutineScope, *op2ParallelLoop);

                /*
                 * ======================================================
                 * Add main statements, which includes allocation/deallocation
                 * of memory on the device, and the kernel call
                 * ======================================================
                 */
                createHostSubroutineStatements (subroutineScope,
                    *op2ParallelLoop);

                /*
                 * ======================================================
                 * Get the scope of the AST node representing the entire
                 * call statement
                 * ======================================================
                 */
                SgScopeStatement * scope = isSgExprStatement (
                    node->get_parent ())->get_scope ();

                ROSE_ASSERT (scope != NULL);

                fix_OP_PAR_LOOP_Calls (scope, *op2ParallelLoop,
                    functionCallExp, hostSubroutine);
              }
              else
              {
                /*
                 * ======================================================
                 * Indirect loop
                 * ======================================================
                 */
                createMainKernelDeviceSubroutine_ForIndirectLoop (moduleScope,
                    *op2ParallelLoop);

                SgProcedureHeaderStatement * hostSubroutine =
                    createHostSubroutine (moduleScope, *op2ParallelLoop);

                SgScopeStatement * subroutineScope =
                    hostSubroutine->get_definition ()->get_body ();

                createHostSubroutineDeclarationsForPlanFunction (
                    subroutineScope, *op2ParallelLoop);
              }
            }
          }
          else
          {
            throw actualArguments.front ();
          }
        }
        catch (SgNode * exceptionNode)
        {
          Debug::getInstance ()->errorMessage (
              "First argument to 'OP_PAR_LOOP' is not a function. The argument has type '"
                  + exceptionNode->class_name () + "'");
        }
      }

      break;
    }

    case V_SgSourceFile:
    {
      SgSourceFile * sourceFile = isSgSourceFile (node);

      Debug::getInstance ()->debugMessage ("Found file "
          + sourceFile->getFileName (), 8);
      break;
    }

    default:
    {
      break;
    }
  }
}

void
CreateKernels::unparse ()
{
  using std::vector;

  Debug::getInstance ()->verboseMessage ("Generating files");

  for (vector <SgSourceFile *>::const_iterator it = generatedFiles.begin (); it
      != generatedFiles.end (); ++it)
  {
    Debug::getInstance ()->debugMessage ("Unparsing to '"
        + (*it)->getFileName () + "'", 1);

    /*
     * ======================================================
     * Unparse the created files after checking consistency
     * of ASTs
     * ======================================================
     */
    SgProject * project = (*it)->get_project ();

    if (project == NULL)
    {
      std::cout << "Project null\n";
    }

    AstTests::runAllTests (project);

    project->unparse ();
  }
}

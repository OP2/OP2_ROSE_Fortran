#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "OP2CommonDefinitions.h"
#include "Debug.h"
#include "CreateKernels.h"
#include "OpParLoop.h"

/*
 * ====================================================================================================
 * Functions with internal linkage
 * ====================================================================================================
 */

bool
checkFormalParameter (SgVariableDeclaration * variableDeclaration,
    SgFunctionParameterList * parameterList)
{
  using std::string;

  string const
      variableName =
          variableDeclaration->get_definition ()->get_vardefn ()->get_name ().getString ();

  for (SgInitializedNamePtrList::iterator it =
      parameterList->get_args ().begin (); it
      != parameterList->get_args ().end (); ++it)
  {
    string const parameterName = (*it)->get_name ().getString ();

    if (variableName.compare (parameterName) == 0)
    {
      return true;
    }
  }

  return false;
}

/*
 * ====================================================================================================
 * Private functions
 * ====================================================================================================
 */

void
CreateKernels::fix_OP_PAR_LOOP_Calls (SgFunctionCallExp * functionCallExp,
    SgProcedureHeaderStatement * hostSubroutine, SgScopeStatement * scope,
    std::string createdCUDAModuleName)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  Debug::getInstance ()->debugMessage ("Fixing parallel loop invocations", 2);

  SgLocatedNode * functionCallNode = isSgLocatedNode (functionCallExp);

  ROSE_ASSERT (functionCallNode != NULL);

  /*
   * Set this node as a transformation in the unparser
   */
  functionCallNode->get_file_info ()->setTransformation ();

  // get the last declaration statement
  SgStatement * lastDeclStmt = findLastDeclarationStatement (scope);

  SgScopeStatement * parent = scope;

  // recursively go back in the scopes until we can find a declaration statement
  // we assume that there must be at least one declaration statement (TODO: fix this)
  while (lastDeclStmt == NULL)
  {
    parent = (SgScopeStatement *) parent->get_parent ();
    if (parent == NULL)
    {
      Debug::getInstance ()->debugMessage (
          "Error: cannot locate a place to declare the kernel name while transforming: '"
              + opParLoop->getKernelHostName () + "'", 1);
      exit (1);
    }

    lastDeclStmt = findLastDeclarationStatement (parent);
  }

  SgStatement * lookForUse = lastDeclStmt;
  SgUseStatement * gotUseStatement;

  do
  {
    lookForUse = getPreviousStatement (lookForUse);
    gotUseStatement = isSgUseStatement (lookForUse);
  }
  while (gotUseStatement == NULL);

  SgFile * enclosingFile = getEnclosingFileNode (gotUseStatement);

  SgUseStatement* useStatement1 = new SgUseStatement (
      enclosingFile->get_file_info (), createdCUDAModuleName, false);

  SgModuleStatement * gotUseModule = gotUseStatement->get_module ();
  useStatement1->set_definingDeclaration (gotUseModule);

  ROSE_ASSERT ( useStatement1 != NULL );
  ROSE_ASSERT ( gotUseStatement != NULL );

  insertStatement (gotUseStatement, useStatement1);

  /*
   * Build the type, which is a character array
   */
  SgExpression * index = buildIntVal (opParLoop->getKernelHostName ().size ());

  // not used for now
  //    std::string cCharName = "c_char";
  //    SgModifierType * cCharType = buildFortranKindType ( buildCharType (), buildWcharVal_nfi ( 0, cCharName ) );


  SgArrayType* charArray = buildArrayType (buildCharType (), index);

  /*
   * The dimension of the array is the asterisk
   */
  SgExprListExp* dimensionExprList = buildExprListExp (index);
  charArray->set_dim_info (dimensionExprList);

  /*
   * The character array only has one dimension
   */
  charArray->set_rank (1);

  SgAssignInitializer * kernelNameInit = buildAssignInitializer (
      buildStringVal (opParLoop->getKernelHostName ()), charArray);

  /*
   * Create the declaration and append it
   */
  string const METANAME = "Name";
  SgVariableDeclaration * kernelMetaName = buildVariableDeclaration (
      opParLoop->getKernelHostName () + METANAME, charArray, kernelNameInit,
      getScope (lastDeclStmt));

  insertStatementAfter (lastDeclStmt, kernelMetaName);

  // modifying the call op_par_loop_* with a call to the newly built CUDA host subroutine
  SgFunctionRefExp * cudaOpParLoopRef = buildFunctionRefExp (hostSubroutine);
  functionCallExp->set_function (cudaOpParLoopRef);

  // modifiying the first parameter from kernel reference to kernel name

  // obtaining the arguments of the call
  SgExprListExp * args = functionCallExp->get_args ();
  ROSE_ASSERT ( args != NULL );

  SgExpressionPtrList & exprs = args->get_expressions ();

  // remove the first parameter corresponding to the kernel reference
  exprs.erase (exprs.begin ());

  // insert a reference to the variable containing the kernel name
  exprs.insert (exprs.begin (), buildVarRefExp (kernelMetaName));
}

void
CreateKernels::lookupArgumentsTypes (std::vector <SgType *> * argTypes,
    SgExpressionPtrList& args, SgVariableDeclaration * setSizeFormalPar)
{
  using namespace std;
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage ("Looking up types of arguments", 2);

  // build: user type arrays with bounds 0:op_set%size * op_dat%dim - 1
  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); i++)
  {
    // build setsize * op_dat%dim (for now I fixed the dimension)
    // access to actualArgumentsDimValues to get the op_dat dimension

    int datDimension = opParLoop->get_OP_DAT_Dimension (i);

    SgExpression * multiplySubscr = buildMultiplyOp (buildVarRefExp (
        setSizeFormalPar), buildIntVal (datDimension));

    ROSE_ASSERT (multiplySubscr != NULL);

    // building (op_set%size * op_dat%dim)  -1
    SgExpression * upperBound = buildSubtractOp (multiplySubscr,
        buildIntVal (1));

    ROSE_ASSERT (upperBound != NULL);

    // We need a file_info because we need to tell rose that the subscript expression
    // must be generated in the output file. If we use the same file_info of the
    // source file we end up with an empty output file
    Sg_File_Info * newFileInfo = new Sg_File_Info ();

    // 0: op_set%size * op_dat%dim - 1
    //(for now we know that the set is the same for all arguments = args[2].size
    SgSubscriptExpression * arraySubScript = new SgSubscriptExpression (
        newFileInfo, buildIntVal (0), upperBound, buildIntVal (1));

    // as we are not using a builder from the SageBuilder interface we need to set things properly..
    arraySubScript->set_endOfConstruct (newFileInfo);
    arraySubScript->setCompilerGenerated ();
    arraySubScript->setOutputInCodeGeneration ();

    // the correct type can be retrieved from the stored information
    SgType * baseType = opParLoop->get_OP_DAT_ActualType (i);

    SgArrayType * guessArrayType = NULL;
    if ((guessArrayType = isSgArrayType (baseType)) != NULL)
    {
      // if the user-defined type is already an array (as it is always or nearly the case), we need to modify
      // the array subscript. If we copy the array type, we won't get the attributes, hence we copy
      // only the base type and we re-create the array type
      baseType = guessArrayType->get_base_type ();
    }

    // build array type (real(8) for now) with the correct subscript
    SgArrayType * arrayType = buildArrayType (baseType, arraySubScript);

    SgExprListExp * dimensionExprList = buildExprListExp (arraySubScript);

    arrayType->set_dim_info (dimensionExprList);
    arrayType->set_rank (1);

    argTypes->push_back (arrayType);
  }
}

SgExprListExp *
CreateKernels::buildUserKernelParams (
    SgFunctionParameterList * mainKernelParameters,
    SgVarRefExp * iterSetVarRef, SgScopeStatement * subroutineScope)
{
  using namespace std;
  using namespace SageBuilder;

  Debug::getInstance ()->debugMessage ("Obtaining kernel parameters", 2);

  // obtain argument list from input parameters list
  SgInitializedNamePtrList mainKernelArgs = mainKernelParameters->get_args ();

  // the size of user kernel call is equal to the number of parameters to the main user kernel function
  // minus 1 (the first argument is the setsize)
  vector <SgExpression *> vectParsExps;// = new vector < SgExpression * > ( mainKernelArgs.size() -1 );

  SgInitializedNamePtrList::iterator inPrIterator = mainKernelArgs.begin ();
  // the first argument is always setsize, so we discard it
  inPrIterator++;

  // we visit each argument and build a corresponding varRefExpr
  for (int i = 0; inPrIterator != mainKernelArgs.end (); i++, inPrIterator++)
  {
    // generating: q(setIter * 4:setIter * 4 + 4 - 1)

    // Generating subscript

    // generating subscript: setIter * 4:setIter * 4 + 4 - 1
    //(for now we know that the set is the same for all arguments = args[2].size

    // setIter * 4
    SgIntVal * opDatDim = buildIntVal (opParLoop->get_OP_DAT_Dimension (i));

    SgExpression * lowerBound = buildMultiplyOp (iterSetVarRef, opDatDim);

    // setIter * 4 + 4
    SgExpression * setIterMulDimPlusDim = buildAddOp (lowerBound, opDatDim);

    // setIter * 4 + 4 - 1
    SgExpression * upperBound = buildSubtractOp (setIterMulDimPlusDim,
        buildIntVal (1));

    //setIter * 4:setIter * 4 + 4 - 1 (we need file info stuff because we are not using SageBuilder,
    // as there is not such a building function)
    Sg_File_Info * newFileInfo = new Sg_File_Info ();

    SgSubscriptExpression * arraySubScript = new SgSubscriptExpression (
        newFileInfo, lowerBound, upperBound, buildIntVal (1));

    // as we are not using a builder from the SageBuilder interface we need to set things properly..
    arraySubScript->set_endOfConstruct (newFileInfo);
    arraySubScript->setCompilerGenerated ();
    arraySubScript->setOutputInCodeGeneration ();

    SgExpression * formalParRef = buildVarRefExp ((*inPrIterator)->get_name (),
        subroutineScope);

    SgExpression * parAccess =
        buildPntrArrRefExp (formalParRef, arraySubScript);

    vectParsExps.push_back (parAccess);
  }

  // do not know why this is not done inside the build function below
  Sg_File_Info * userKernelParamsFileInfo = new Sg_File_Info ();

  SgExprListExp * userKernelParams = buildExprListExp_nfi (vectParsExps);
  userKernelParams->set_parent (subroutineScope);
  userKernelParams->set_startOfConstruct (userKernelParamsFileInfo);
  userKernelParams->set_endOfConstruct (userKernelParamsFileInfo);
  userKernelParams->set_file_info (userKernelParamsFileInfo);
  userKernelParams->setCompilerGenerated ();
  userKernelParams->setOutputInCodeGeneration ();

  return userKernelParams;
}

void
CreateKernels::createHostSubroutineStatements (SgScopeStatement * scope,
    SgExpressionPtrList& args)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  Debug::getInstance ()->debugMessage ("Adding statements to host subroutine",
      2);

  /*
   * This function builds the following Fortran code:
   *
   * data0Size = arg0%dim * arg0%set%size
   * allocate(c2fPtr0(data0Size))
   * data1Size = arg1%dim * arg1%set%size
   * allocate(c2fPtr1(data1Size))
   * CALL c_f_pointer(arg0%dat,c2fPtr0,(/data0Size/))
   * CALL c_f_pointer(arg1%dat,c2fPtr1,(/data1Size/))
   * c2fPtr0 = c2fPtr0
   * c2fPtr1 = c2fPtr1
   * CALL op_cuda_save_soln<<<gsize,bsize,reduct_shared>>>(iterationSet%size,c2fPtr0,c2fPtr1)
   * c2fPtr0 = c2fPtr0
   * c2fPtr1 = c2fPtr1
   */

  // build:             data0Size = ( arg0%dim * (arg0%set)%size)
  //                                    data1Size = ( arg1%dim * (arg1%set)%size)

  // iterator over op_dat formal parameters
  vector <SgVariableDeclaration *>::const_iterator argIt =
      opParLoop->get_OP_DAT_Arguments ();

  // iterator over local variables
  vector <SgVariableDeclaration *>::const_iterator localVarIt =
      opParLoop->get_Host_Subroutine_Variables ();

  SgStatement * lastAppendedStatement = NULL;
  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); i++, argIt++, localVarIt++)
  {
    // transform the expression to a variable reference
    SgExpression * opDatArgRef = buildVarRefExp (*argIt);

    // build: argI%set
    SgVarRefExp * setField = buildOpaqueVarRefExp ("set", scope);
    SgExpression * argISetField = buildDotExp (opDatArgRef, setField);

    // build: (argI%set)%size
    SgVarRefExp * sizeField = buildOpaqueVarRefExp ("size", scope);
    SgExpression * argISetSizeField = buildDotExp (argISetField, sizeField);

    // build: argI%dim
    SgVarRefExp * dimField = buildOpaqueVarRefExp ("dim", scope);
    SgExpression * argIDimField = buildDotExp (opDatArgRef, dimField);

    // build: ( arg0%dim * (arg0%set)%size)
    SgExpression * multiplExpr = buildMultiplyOp (argIDimField,
        argISetSizeField);

    // get variable reference to dataISize local variable
    //          ROSE_ASSERT ( **localVarIt != NULL );
    SgVarRefExp * dataISizeRef = buildVarRefExp (*localVarIt);

    // build: data0Size = ( arg0%dim * (arg0%set)%size)
    SgExpression * assignDataISize = buildAssignOp (dataISizeRef, multiplExpr);

    // finally append the statement
    SgStatement * assignDataISizeStmt = buildExprStatement (assignDataISize);
    appendStatement (assignDataISizeStmt, scope);

    if (i == opParLoop->getNumberOf_OP_DAT_ArgumentGroups () - 1) // last iteration
      lastAppendedStatement = assignDataISizeStmt;
  }

  // the first set of opParLoop->getNumberOf_OP_DAT_ArgumentGroups() variables in declaredC2FortranVariables
  // is the one of the argumentI variables, which is the ones that we are going to use now
  vector <SgVariableDeclaration *>::const_iterator c2fVarIt =
      opParLoop->get_C_To_Fortran_Variables ();

  // build:     allocate ( argument0 ( data0Size ) )
  //                                    allocate ( argumentN-11 ( dataN-11Size ) )

  // we need the dataISize variables again
  localVarIt = opParLoop->get_Host_Subroutine_Variables ();

  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); i++, c2fVarIt++, localVarIt++)
  {
    // build function type for allocate subroutine
    string const allocateName = "allocate";

    // build: argumentI ( dataISize )
    // We have to use a trick: we treat argumentI as an array (as it is) and we dereference it with dataISize

    // transform the expression argumentI to a variable reference
    SgVarRefExp * argumentIVarRef = buildVarRefExp (*c2fVarIt);
    SgVarRefExp * dataISizeVarRef = buildVarRefExp (*localVarIt);

    SgExpression * fakeArrayAccess = buildPntrArrRefExp (argumentIVarRef,
        dataISizeVarRef);

    SgExprListExp * fakeArrayAccessList = buildExprListExp (fakeArrayAccess);

    // building array type with : shape specifier
    SgColonShapeExp * colonExp = new SgColonShapeExp ();
    Sg_File_Info* colonFileInfo = new Sg_File_Info ();
    colonExp->set_startOfConstruct (colonFileInfo);
    colonExp->set_endOfConstruct (colonFileInfo);

    // for now, the type is fixed to real(8) = double precision, but we have to get it
    // from the declaration op_decl_dat -> declaration of input data

    // building real(kind=8): NOT WORKING FOR NOW (it seems that real(8) and dimension(:) do
    // not like each other in the unparser
    //          SgModifierType * realEight = buildFortranKindType ( buildFloatType(), buildIntVal ( 8 ) );

    // Unfortunately, we cannot use CALL with a Fortran function: we have to use addTextForUnaparser for now
    string const allocateStringfied = "allocate";
    string const allocateParamsStringfied =
        fakeArrayAccessList->unparseToString ();
    string const allocateStmtStringfied = allocateStringfied + "("
        + allocateParamsStringfied + ")\n";

    addTextForUnparser (lastAppendedStatement, allocateStmtStringfied,
        AstUnparseAttribute::e_after);
  }

  // build:     call c_f_pointer ( arg0%dat, c2fPtr0, (/data0Size/) )
  //                                    call c_f_pointer ( arg1%dat, c2fPtr1, (/data1Size/) )

  // iterator over op_dat formal parameters
  argIt = opParLoop->get_OP_DAT_Arguments ();

  // we need the dataISize variables again
  localVarIt = opParLoop->get_Host_Subroutine_Variables ();

  string const c2fFunName = "c_f_pointer";

  // The good news is that the c2fVarIt should point exactly to the first c2fPtrI
  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); i++, c2fVarIt++, argIt++, localVarIt++)
  {
    // build: argI%dat

    // transform the expression to a variable reference
    SgVarRefExp * opDatArgRef = buildVarRefExp (*argIt);

    // argI%dat
    SgVarRefExp * datField = buildOpaqueVarRefExp ("dat", scope);
    SgExpression * argIDatField = buildDotExp (opDatArgRef, datField);

    // transform the expression c2fPtrI to a variable reference
    SgVarRefExp * c2fPtrIVarRef = buildVarRefExp (*c2fVarIt);

    // there is not another known way to build (/size/)
    SgInitializedNamePtrList singleVarName = (*localVarIt)->get_variables ();
    SgInitializedNamePtrList::iterator singleVarNameIt = singleVarName.begin ();

    string const sizeShapeName = "(/"
        + (*singleVarNameIt)->get_name ().getString () + "/)";
    SgExpression * shapeExpr = buildOpaqueVarRefExp (sizeShapeName, scope);

    SgExprListExp * c2fFunActualParameters = buildExprListExp (argIDatField,
        c2fPtrIVarRef, shapeExpr);

    // TODO: transform types to the correct ones!
    SgFunctionParameterTypeList * c2fFunInputTypes =
        buildFunctionParameterTypeList (buildIntType (), buildIntType (),
            buildIntType ());

    SgFunctionType * c2fFunType = buildFunctionType (buildVoidType (),
        c2fFunInputTypes);
    SgFunctionRefExp * c2fFunRef = buildFunctionRefExp (*(new SgName (
        c2fFunName)), c2fFunType, scope);
    // build function call
    SgFunctionSymbol * c2fFunSymbol = c2fFunRef->get_symbol_i ();
    SgFunctionCallExp * c2fFunCall = buildFunctionCallExp (c2fFunSymbol,
        c2fFunActualParameters);

    SgStatement * c2fFunCallStmt = buildExprStatement (c2fFunCall);

    appendStatement (c2fFunCallStmt, scope);
  }

  // build:     argument0 = c2fPtr0
  //                            argument1 = c2fPtr1
  // As both variable declarations are stored in the same vector, we need two iteratos

  // first come the argumentI variables
  vector <SgVariableDeclaration *>::const_iterator argumentIVarIt =
      opParLoop->get_C_To_Fortran_Variables ();

  // then, after <opParLoop->getNumberOf_OP_DAT_ArgumentGroups()> argumentI variables, we have the c2fPtrI variables
  vector <SgVariableDeclaration *>::const_iterator c2fPtrVarIt =
      opParLoop->get_C_To_Fortran_Variables ();
  c2fPtrVarIt += opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); // exploiting pointer arithmetics!


  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); i++, argumentIVarIt++, c2fPtrVarIt++)
  {
    SgVarRefExp * argumentIRef = buildVarRefExp (*argumentIVarIt);
    SgVarRefExp * c2FPtrIRef = buildVarRefExp (*c2fPtrVarIt);

    SgExpression * assignC2FPtrIToArgumentI = buildAssignOp (argumentIRef,
        c2FPtrIRef);
    SgStatement * assignC2FPtrIToArgumentIStmt = buildExprStatement (
        assignC2FPtrIToArgumentI);

    appendStatement (assignC2FPtrIToArgumentIStmt, scope);
  }

  // build: call op_cuda_save_soln<<<gsize,bsize,reduct_shared>>> ( set%size, argument0, argument1  )

  // build: argument0, argument1, set%size
  argumentIVarIt = opParLoop->get_C_To_Fortran_Variables ();
  SgExprListExp * kPars = buildExprListExp ();

  // build: set%size
  SgExpression * iterationSetVarRef = buildVarRefExp (
      opParLoop->get_OP_SET_Argument ());
  SgVarRefExp * setSizeField = buildOpaqueVarRefExp ("size", scope);
  SgExpression * setFieldSizeExpr = buildDotExp (iterationSetVarRef,
      setSizeField);
  kPars->append_expression (setFieldSizeExpr);

  // build: argumentI
  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); i++, argumentIVarIt++)
  {
    SgVarRefExp * argumentIRef = buildVarRefExp (*argumentIVarIt);
    kPars->append_expression (argumentIRef);
  }

  SgExprStatement * kCall = buildFunctionCallStmt ("op_cuda_"
      + opParLoop->getKernelHostName () + "<<<gsize,bsize,reduct_shared>>>",
      buildVoidType (), kPars, scope);

  appendStatement (kCall, scope);

  // building copy back
  argumentIVarIt = opParLoop->get_C_To_Fortran_Variables ();
  c2fPtrVarIt = opParLoop->get_C_To_Fortran_Variables ();
  c2fPtrVarIt += opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); // exploiting pointer arithmetics!

  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); i++, argumentIVarIt++, c2fPtrVarIt++)
  {
    SgVarRefExp * argumentIRef = buildVarRefExp (*argumentIVarIt);
    SgVarRefExp * c2FPtrIRef = buildVarRefExp (*c2fPtrVarIt);

    SgExpression * assignArgumentIToC2FPtrI = buildAssignOp (c2FPtrIRef,
        argumentIRef);
    SgStatement * assignArgumentIToC2FPtrIStmt = buildExprStatement (
        assignArgumentIToC2FPtrI);

    appendStatement (assignArgumentIToC2FPtrIStmt, scope);
  }
}

void
CreateKernels::createHostSubroutineCUDAVariables (SgScopeStatement * scope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;

  Debug::getInstance ()->debugMessage (
      "Creating CUDA configuration parameters", 2);

  /*
   * This function builds the following Fortran code:
   *
   * integer(4) :: bsize = BSIZE_DEFAULT
   * integer(4) :: gsize
   * integer(4) :: reduct_bytes = 0
   * integer(4) :: reduct_size = 0
   * real(8)    :: reduct_shared
   * integer(4) :: const_bytes = 0
   * gsize         = int ((set%size - 1) / bsize + 1)
   * reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)
   */

  /*
   * ====================================================================================================
   * Declaration and initialisation of variables and opaque variables
   * ====================================================================================================
   */
  SgModifierType * intFour = buildFortranKindType (buildIntType (),
      buildIntVal (4));

  SgModifierType * realEight = buildFortranKindType (buildFloatType (),
      buildIntVal (8));

  SgVarRefExp * variable_BSIZE_DEFAULT = buildOpaqueVarRefExp ("BSIZE_DEFAULT",
      scope);

  SgVariableDeclaration * variable_bsize = buildVariableDeclaration ("bsize",
      intFour,
      buildAssignInitializer (variable_BSIZE_DEFAULT, buildIntType ()), scope);

  SgVariableDeclaration * variable_gsize = buildVariableDeclaration ("gsize",
      intFour, NULL, scope);

  SgVariableDeclaration * variable_reduct_bytes = buildVariableDeclaration (
      "reduct_bytes", intFour, buildAssignInitializer (buildIntVal (0),
          buildIntType ()), scope);

  SgVariableDeclaration * variable_reduct_size = buildVariableDeclaration (
      "reduct_size", intFour, buildAssignInitializer (buildIntVal (0),
          buildIntType ()), scope);

  SgVariableDeclaration * variable_reduct_shared = buildVariableDeclaration (
      "reduct_shared", realEight, NULL, scope);

  SgVariableDeclaration * variable_const_bytes = buildVariableDeclaration (
      "const_bytes", intFour, buildAssignInitializer (buildIntVal (0),
          buildIntType ()), scope);

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
   * ====================================================================================================
   * Create statement 'gsize = int ((set%size - 1) / bsize + 1)'
   * ====================================================================================================
   */

  // 'set%size'
  SgExpression * setPERCENTsize = buildDotExp (buildVarRefExp (
      opParLoop->get_OP_SET_Argument ()), buildOpaqueVarRefExp ("size", scope));

  // 'set%size - 1'
  SgExpression * setPERCENTsize_minusOne = buildSubtractOp (setPERCENTsize,
      buildIntVal (1));

  // '(set%size - 1) / bsize  + 1'
  SgExpression * castIntegerParameter = buildAddOp (buildDivideOp (
      setPERCENTsize_minusOne, buildVarRefExp (variable_bsize)),
      buildIntVal (1));

  /*
   * 'int ((set%size - 1) / bsize + 1)'
   *
   * To do this we have to build a function type for the intrinsic 'int'
   * We do not care about the fact that the function is already defined (also via an intrinsic):
   * we just tell ROSE all the information needed to build a new function
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
   * ====================================================================================================
   * Create statement 'reduct_shared = reduct_size * (BSIZE_DEFAULT / 2)'
   * ====================================================================================================
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
CreateKernels::createHostSubroutineLocals (SgScopeStatement* subroutineScope,
    SgExpressionPtrList& args)
{
  using boost::lexical_cast;
  using SageBuilder::buildIntType;
  using SageBuilder::buildPointerType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using std::string;

  Debug::getInstance ()->debugMessage ("Creating local variables", 2);

  /*
   * This function builds the following Fortran code:
   *
   * INTEGER :: data0Size
   * INTEGER :: dataNSize
   * real(8), dimension(:), allocatable, device :: argument0
   * real(8), dimension(:), allocatable, device :: argumentN-1
   * real(8), dimension(:), pointer :: c2fPtr0
   * real(8), dimension(:), pointer :: c2fPtrN-1
   */

  /*
   * ====================================================================================================
   * INTEGER :: data0Size
   * INTEGER :: dataNSize
   * ====================================================================================================
   */
  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "data" + lexical_cast <string> (i) + "Size";

    SgVariableDeclaration * variable_dataNSize = buildVariableDeclaration (
        variableName, buildIntType (), NULL, subroutineScope);

    variable_dataNSize->get_declarationModifier ().get_accessModifier ().setUndefined ();

    opParLoop->set_Host_Subroutine_Variable (variable_dataNSize);

    appendStatement (variable_dataNSize, subroutineScope);
  }

  /*
   * ====================================================================================================
   * real(8), dimension(:), allocatable, device :: argument0
   * real(8), dimension(:), allocatable, device :: argumentN-1
   * ====================================================================================================
   */
  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "c2fPtr" + lexical_cast <string> (i);

    SgVariableDeclaration * variable_argument_n = buildVariableDeclaration (
        variableName, opParLoop->get_OP_DAT_ActualType (i), NULL,
        subroutineScope);

    variable_argument_n->get_declarationModifier ().get_typeModifier ().setDevice ();
    variable_argument_n->get_declarationModifier ().get_typeModifier ().setAllocatable ();
    variable_argument_n->get_declarationModifier ().get_accessModifier ().setUndefined ();

    opParLoop->set_C_To_Fortran_Variable (variable_argument_n);

    appendStatement (variable_argument_n, subroutineScope);
  }

  /*
   * ====================================================================================================
   * real(8), dimension(:), pointer :: c2fPtr0
   * real(8), dimension(:), pointer :: c2fPtrN-1
   * ====================================================================================================
   */
  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); ++i)
  {
    string const variableName = "c2fPtr" + lexical_cast <string> (i);

    SgVariableDeclaration * variable_c2fptr_n = buildVariableDeclaration (
        variableName, buildPointerType (opParLoop->get_OP_DAT_ActualType (i)),
        NULL, subroutineScope);

    variable_c2fptr_n->get_declarationModifier ().get_accessModifier ().setUndefined ();

    opParLoop->set_C_To_Fortran_Variable (variable_c2fptr_n);

    appendStatement (variable_c2fptr_n, subroutineScope);
  }

  /*
   * ====================================================================================================
   * Add CUDA configuration parameters
   * ====================================================================================================
   */
  createHostSubroutineCUDAVariables (subroutineScope);
}

void
CreateKernels::createHostSubroutineFormalParamaters (
    SgScopeStatement * subroutineScope, SgExpressionPtrList & args,
    SgFunctionParameterList * hostParameters)
{
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
   * ====================================================================================================
   * These are the prefixes used by Mike Giles for variables of a particular OP2 data type
   * ====================================================================================================
   */
  string const indirectionPrefix = "idx";
  string const OP_ACCESS_Prefix = "access";
  string const OP_DAT_Prefix = "arg";
  string const OP_MAP_Prefix = "ptr";

  int suffix = -1;

  for (vector <SgExpression *>::iterator it = args.begin (); it != args.end (); ++it)
  {
    SgExpression* argument = *it;

    Debug::getInstance ()->debugMessage ("Argument type: "
        + argument->class_name (), 8);

    switch (argument->variantT ())
    {
      case V_SgFunctionRefExp:
      {
        /*
         * ====================================================================================================
         * Found the kernel name argument
         * ====================================================================================================
         */

        string const variableName = "subroutineName";

        /*
         * Build asterisk shape expression indicating that the size of the
         * array is decided at run time
         */
        SgAsteriskShapeExp* asterisk = new SgAsteriskShapeExp ();
        Sg_File_Info* asteriskFileInfo = new Sg_File_Info ();
        asterisk->set_startOfConstruct (asteriskFileInfo);
        asterisk->set_endOfConstruct (asteriskFileInfo);

        /*
         * Build the type, which is a character array
         */
        SgArrayType* charArray = buildArrayType (buildCharType (), asterisk);

        /*
         * The dimension of the array is the asterisk
         */
        SgExprListExp* dimensionExprList = buildExprListExp (asterisk);
        charArray->set_dim_info (dimensionExprList);

        /*
         * The character array only has one dimension
         */
        charArray->set_rank (1);

        SgVariableDeclaration* charArrayDeclaration = buildVariableDeclaration (
            variableName, charArray, NULL, subroutineScope);

        hostParameters->append_arg (
            *(charArrayDeclaration->get_variables ().begin ()));

        charArrayDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
        charArrayDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

        appendStatement (charArrayDeclaration, subroutineScope);

        break;
      }

      case V_SgVarRefExp:
      {
        SgVarRefExp* variableReference = isSgVarRefExp (argument);

        switch (variableReference->get_type ()->variantT ())
        {
          case V_SgClassType:
          {
            SgClassType* classReference = isSgClassType (
                variableReference->get_type ());
            string const className = classReference->get_name ().getString ();

            if (strcmp (className.c_str (), OP2::OP_SET_NAME.c_str ()) == 0)
            {
              /*
               * ====================================================================================================
               * Found an OP_SET argument
               * ====================================================================================================
               */
              SgVariableDeclaration* opSetDeclaration =
                  buildVariableDeclaration ("iterationSet", classReference,
                      NULL, subroutineScope);

              opSetDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
              opSetDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              hostParameters->append_arg (
                  *(opSetDeclaration->get_variables ().begin ()));

              appendStatement (opSetDeclaration, subroutineScope);

              opParLoop->set_OP_SET_Argument (opSetDeclaration);
            }

            else if (strcmp (className.c_str (), OP2::OP_MAP_NAME.c_str ())
                == 0)
            {
              /*
               * ====================================================================================================
               * Found an OP_MAP argument
               * ====================================================================================================
               */
              string const variableName = OP_MAP_Prefix
                  + lexical_cast <string> (suffix);

              SgVariableDeclaration* opMapDeclaration =
                  buildVariableDeclaration (variableName, classReference, NULL,
                      subroutineScope);

              opMapDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              hostParameters->append_arg (
                  *(opMapDeclaration->get_variables ().begin ()));

              appendStatement (opMapDeclaration, subroutineScope);

              opParLoop->set_OP_MAP_Argument (opMapDeclaration);
            }

            else if (strcmp (className.c_str (), OP2::OP_DAT_NAME.c_str ())
                == 0)
            {
              /*
               * ====================================================================================================
               * Found an OP_DAT argument
               * ====================================================================================================
               */
              suffix++;

              string const variableName = OP_DAT_Prefix
                  + lexical_cast <string> (suffix);

              SgVariableDeclaration * opDatDeclaration =
                  buildVariableDeclaration (variableName, classReference, NULL,
                      subroutineScope);

              opDatDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

              hostParameters->append_arg (
                  *(opDatDeclaration->get_variables ().begin ()));

              appendStatement (opDatDeclaration, subroutineScope);

              opParLoop->set_OP_DAT_Argument (opDatDeclaration);
            }
            else
            {
              Debug::getInstance ()->debugMessage ("Unrecognised class: "
                  + className, 1);
              exit (1);
            }

            break;
          }

          case V_SgTypeInt:
          {
            /*
             * ====================================================================================================
             * Found an OP_ACCESS argument
             * ====================================================================================================
             */

            string const variableName = OP_ACCESS_Prefix
                + lexical_cast <string> (suffix);

            SgVariableDeclaration* opAccessDeclaration =
                buildVariableDeclaration (variableName, buildIntType (), NULL,
                    subroutineScope);

            opAccessDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
            opAccessDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

            hostParameters->append_arg (
                *(opAccessDeclaration->get_variables ().begin ()));

            appendStatement (opAccessDeclaration, subroutineScope);

            opParLoop->set_OP_ACESS_Argument (opAccessDeclaration);

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
         * ====================================================================================================
         * Found an indirection argument
         * ====================================================================================================
         */

        string const variableName = indirectionPrefix + lexical_cast <string> (
            suffix);

        SgVariableDeclaration* opIndirectionDeclaration =
            buildVariableDeclaration (variableName, buildIntType (), NULL,
                subroutineScope);

        opIndirectionDeclaration->get_declarationModifier ().get_typeModifier ().setIntent_in ();
        opIndirectionDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();

        hostParameters->append_arg (
            *(opIndirectionDeclaration->get_variables ().begin ()));

        appendStatement (opIndirectionDeclaration, subroutineScope);

        opParLoop->set_OP_INDIRECTION_Argument (opIndirectionDeclaration);

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
CreateKernels::createHostSubroutine (SgExpressionPtrList & args,
    SgSourceFile & sourceFile, SgScopeStatement * moduleScope)
{
  using namespace SageBuilder;
  using namespace SageInterface;
  using namespace std;
  using namespace OP2;

  Debug::getInstance ()->debugMessage ("Creating host subroutine", 2);

  /*
   * ====================================================================================================
   * Create the host subroutine
   * ====================================================================================================
   */
  string const hostSubroutineName = OP_PAR_LOOP_PREFIX + "_"
      + opParLoop->getKernelHostName ().c_str ();

  SgFunctionParameterList * hostParameters = buildFunctionParameterList ();

  SgProcedureHeaderStatement
      * subroutineStatement =
          buildProcedureHeaderStatement (hostSubroutineName.c_str (),
              buildVoidType (), hostParameters,
              SgProcedureHeaderStatement::e_subroutine_subprogram_kind,
              moduleScope);

  appendStatement (subroutineStatement, moduleScope);

  addTextForUnparser (subroutineStatement, "attributes(host) ",
      AstUnparseAttribute::e_before);

  SgBasicBlock * subroutineScope =
      subroutineStatement->get_definition ()->get_body ();

  /*
   * ====================================================================================================
   * Add formal parameters
   * ====================================================================================================
   */
  createHostSubroutineFormalParamaters (subroutineScope, args, hostParameters);

  /*
   * ====================================================================================================
   * Add local variables
   * ====================================================================================================
   */
  createHostSubroutineLocals (subroutineScope, args);

  /*
   * ====================================================================================================
   * Add main statements, which includes allocation of memory on the device and kernel calls
   * ====================================================================================================
   */
  createHostSubroutineStatements (subroutineScope, args);

  return subroutineStatement;
}

void
CreateKernels::createMainKernelSubroutine (SgScopeStatement * moduleScope,
    SgExpressionPtrList & args, Sg_File_Info & fileInfo)
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

  Debug::getInstance ()->debugMessage ("Building main kernel routine", 2);

  /*
   * ====================================================================================================
   * Create the new subroutine, its formal parameters, and its local variables
   * ====================================================================================================
   */

  string const newSubroutineName = "op_cuda_" + opParLoop->getKernelHostName ();

  SgFunctionParameterList * newSubroutineParameters =
      buildFunctionParameterList ();

  SgProcedureHeaderStatement * newSubroutine = buildProcedureHeaderStatement (
      newSubroutineName.c_str (), buildVoidType (), newSubroutineParameters,
      SgProcedureHeaderStatement::e_subroutine_subprogram_kind, moduleScope);

  addTextForUnparser (newSubroutine, "attributes(global) ",
      AstUnparseAttribute::e_before);

  appendStatement (newSubroutine, moduleScope);

  SgScopeStatement * newSubroutineScope =
      newSubroutine->get_definition ()->get_body ();

  /*
   * ====================================================================================================
   * Formal parameters
   * ====================================================================================================
   */
  string const setSizeName = "setsize";
  SgVariableDeclaration * setSizeFormalParameter = buildVariableDeclaration (
      setSizeName, buildIntType (), NULL, newSubroutineScope);

  setSizeFormalParameter->get_declarationModifier ().get_accessModifier ().setUndefined ();
  setSizeFormalParameter->get_declarationModifier ().get_typeModifier ().setValue ();

  newSubroutineParameters->append_arg (
      *(setSizeFormalParameter->get_variables ().begin ()));

  appendStatement (setSizeFormalParameter, newSubroutineScope);

  // b. Building remaining formal parameters types from program
  vector <SgType *> * argTypes = new vector <SgType *> ;

  lookupArgumentsTypes (argTypes, args, setSizeFormalParameter);

  // check that the size of argTypes is exactly the expected number of arguments to avoid wrong accesses to argTypes below
  ROSE_ASSERT ( argTypes->size() == opParLoop->getNumberOf_OP_DAT_ArgumentGroups() );

  // b. create formal parameters (we have a OP_DAT argument starting from 3 and then every 4 arguments
  // (4 is stored inside a macro (ARG_LINE_LENGTH) copied inside variable argLineLength in constructor
  // retriving op_dat original type from argument: must generate real(8), dimension(0:setsize * 4 -1)
  // where 4 is the carinality of each input op_dat element associate to each set element (can be found
  // in the op_decl_dat call)
  for (unsigned int i = 0; i < opParLoop->getNumberOf_OP_DAT_ArgumentGroups (); i++)
  {
    /*
     * Create 'arg_i' formal parameter
     */
    string const argName = "arg_" + lexical_cast <string> (i);

    SgVariableDeclaration * argFormalParameter = buildVariableDeclaration (
        argName, (*argTypes)[i], NULL, newSubroutineScope);

    newSubroutineParameters->append_arg (
        *(argFormalParameter->get_variables ().begin ()));

    argFormalParameter->get_declarationModifier ().get_typeModifier ().setIntent_in ();
    argFormalParameter->get_declarationModifier ().get_typeModifier ().setDevice ();
    argFormalParameter->get_declarationModifier ().get_accessModifier ().setUndefined ();

    appendStatement (argFormalParameter, newSubroutineScope);
  }

  /*
   * ====================================================================================================
   * Local variables
   * ====================================================================================================
   */
  string const iterSetName = "setIter";

  SgVariableDeclaration * iterSetVariable = buildVariableDeclaration (
      *(new SgName (iterSetName)), buildIntType (), NULL, newSubroutineScope);

  iterSetVariable->get_declarationModifier ().get_typeModifier ().setDevice ();
  iterSetVariable->get_declarationModifier ().get_accessModifier ().setUndefined ();

  appendStatement (iterSetVariable, newSubroutineScope);

  /*
   * ====================================================================================================
   * Build opaque variable references needed in the following expressions. These are opaque because
   * the variables are provided as part of the CUDA library and are not seen by ROSE
   * ====================================================================================================
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
   * ====================================================================================================
   * Initialise set iteration variable to 'setIter = (threadidx%x - 1) + (blockidx%x -1)  * blockdim%x'
   * ====================================================================================================
   */

  /*
   * Sub-expression 'threadidx%x - 1'
   */
  SgExpression * threadIdXMinusOne = buildSubtractOp (buildDotExp (
      variableThreadidx, variableX), buildIntVal (1));

  /*
   * Sub-expression 'blockidx%x - 1'
   */
  SgExpression * blockIdXMinusOne = buildSubtractOp (buildDotExp (
      variableBlockidx, variableX), buildIntVal (1));

  /*
   * Sub-expression 'blockdim%x'
   */
  SgExpression * blockDimX = buildDotExp (variableBlockdim, variableX);

  /*
   * Sub-expression '(blockidx%x -1)  * blockdim%x'
   */
  SgExpression * blockIdXMinusOnePerBlockDimx = buildMultiplyOp (
      blockIdXMinusOne, blockDimX);

  /*
   * Assign the value of the expression '(threadidx%x - 1) + (blockidx%x -1)  * blockdim%x'
   * to 'setIter'
   */
  SgVarRefExp * setIterRef = buildVarRefExp (iterSetName, newSubroutineScope);
  SgExpression * setIterInitialAssignement = buildAssignOp (setIterRef,
      buildAddOp (threadIdXMinusOne, blockIdXMinusOnePerBlockDimx));

  /*
   * Add as an assignment statement
   */
  appendStatement (buildExprStatement (setIterInitialAssignement),
      newSubroutineScope);

  /*
   * ====================================================================================================
   * Build the do-while loop
   * ====================================================================================================
   */

  /*
   *  Build a call to the user kernel subroutine
   */
  SgFunctionRefExp * userKernelRefExp = buildFunctionRefExp (
      opParLoop->getKernelDeviceName (), newSubroutineScope->get_scope ());

  // 2. build parameters: q(setIter * dim:setIter * dim + dim - 1), qold(setIter * dim:setIter*dim + dim - 1)
  // dim is obtained from the previous parsing
  SgExprListExp * userKernelParams = buildUserKernelParams (
      newSubroutineParameters, setIterRef, newSubroutineScope);

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
CreateKernels::createCUDAKernel (SgScopeStatement * moduleScope)
{
  using SageBuilder::buildProcedureHeaderStatement;
  using SageBuilder::buildFunctionParameterList;
  using SageBuilder::buildVoidType;
  using SageBuilder::buildVariableDeclaration;
  using SageInterface::appendStatement;
  using SageInterface::addTextForUnparser;
  using std::vector;
  using std::string;

  Debug::getInstance ()->debugMessage (
      "Modifying and outputting original kernel to CUDA file", 2);

  SgProcedureHeaderStatement * newSubroutine;
  SgFunctionParameterList * newParameters;

  SgProcedureHeaderStatement * originalSubroutine = NULL;
  SgFunctionParameterList * originalParameters = NULL;

  /*
   * ====================================================================================================
   * Find the original subroutine (otherwise the definition would be empty)
   * ====================================================================================================
   */
  for (vector <SgProcedureHeaderStatement *>::iterator it =
      inputSubroutines.begin (); it != inputSubroutines.end (); ++it)
  {
    SgProcedureHeaderStatement * subroutine = *it;

    if (opParLoop->getKernelHostName ().compare (
        subroutine->get_name ().getString ()) == 0)
    {
      /*
       * Grab the subroutine and its parameters
       */
      originalSubroutine = subroutine;
      originalParameters = subroutine->get_parameterList ();
      break;
    }
  }
  ROSE_ASSERT (originalSubroutine != NULL);

  /*
   * ====================================================================================================
   * Create the new subroutine
   * ====================================================================================================
   */
  newParameters = buildFunctionParameterList ();

  newSubroutine = buildProcedureHeaderStatement (
      opParLoop->getKernelDeviceName ().c_str (), buildVoidType (),
      newParameters, SgProcedureHeaderStatement::e_subroutine_subprogram_kind,
      moduleScope);

  appendStatement (newSubroutine, moduleScope);

  /*
   * Currently have to add the 'attribute' statement by hand as there is no grammar
   * support
   */
  addTextForUnparser (newSubroutine, "attributes(device) ",
      AstUnparseAttribute::e_before);

  /*
   * ====================================================================================================
   * Visit all the statements in the original subroutine and append them to new subroutine while making
   * necessary changes.
   * TODO: This may not work for nested bodies. The alternative is to use a copy routine, but currently
   * not working either
   * ====================================================================================================
   */
  SgScopeStatement * newSubroutineScope =
      newSubroutine->get_definition ()->get_body ();
  Rose_STL_Container<SgStatement *> originalStatements = originalSubroutine->get_definition()->get_body()->get_statements();
  for (Rose_STL_Container<SgStatement *>::iterator it = originalStatements.begin(); it != originalStatements.end(); ++it)
  {
    /*
     * Copy the statement if:
     * 1) It is NOT a variable declaration
     * 2) It is a variable declaration BUT it is not a formal parameter declaration
     */
    SgVariableDeclaration * isVariableDeclaration = isSgVariableDeclaration (*it);

    if (isVariableDeclaration == NULL)
    {
      /*
       * Append the statement to the new subroutine
       */
      appendStatement (*it, newSubroutineScope);

      if ( isSgImplicitStatement (*it) != NULL )
      {
        /*
         * Append the variable declarations immediately after the 'IMPLICIT NONE' statement
         */

        for (SgInitializedNamePtrList::iterator paramIt = originalParameters->get_args ().begin (); paramIt != originalParameters->get_args ().end (); ++paramIt)
        {
          /*
           * Build a new variable declaration using the properties of the original declaration
           */
          SgVariableDeclaration * newVariableDeclaration = buildVariableDeclaration (
              (*paramIt)->get_name ().getString (), (*paramIt)->get_typeptr (), NULL,
              newSubroutineScope);

          /*
           * Set the Fortran attributes of the declared variables
           */
          newVariableDeclaration->get_declarationModifier ().get_accessModifier ().setUndefined ();
          newVariableDeclaration->get_declarationModifier ().get_typeModifier ().setDevice ();

          newParameters->append_arg (*(newVariableDeclaration->get_variables ().begin ()));

          /*
           * Append the variable declaration to the body of the new subroutine
           */
          appendStatement (newVariableDeclaration, newSubroutineScope);
        }
      }
    }
    else if (!checkFormalParameter (isVariableDeclaration, newParameters))
    {
      /*
       * Append the statement to the new subroutine
       */
      appendStatement (*it, newSubroutineScope);
    }
  }

  /*
   * ====================================================================================================
   * The following class visits every created node for the subroutine and ensures that it is generated
   * during unparsing
   * ====================================================================================================
   */
  class GenerateAllNodesInUnparser: public AstSimpleProcessing
  {
    virtual void
    visit (SgNode * node)
    {
      SgLocatedNode * locatedNode = isSgLocatedNode (node);
      if (locatedNode != NULL)
      {
        locatedNode->setOutputInCodeGeneration ();
      }
    }
  };
  (new GenerateAllNodesInUnparser ())->traverse(newSubroutine, preorder);
}

SgModuleStatement *
CreateKernels::buildClassDeclarationAndDefinition (std::string moduleName,
    SgScopeStatement * fileScope)
{
  Debug::getInstance ()->debugMessage ("Building module", 2);

  /*
   * ====================================================================================================
   * This function builds a class declaration and definition
   * (both the defining and nondefining declarations as required)
   * ====================================================================================================
   */

  // Build a file info object marked as a transformation
  Sg_File_Info* fileInfo =
      Sg_File_Info::generateDefaultFileInfoForCompilerGeneratedNode ();
  fileInfo->setOutputInCodeGeneration ();

  /*
   * This is the class definition (the fileInfo specifies the position of the opening brace).
   * Must also set the end of construct explicitly otherwise ROSE complains (where this is not a
   * transformation, it is the location of the closing brace)
   */
  SgClassDefinition * classDefinition = new SgClassDefinition (fileInfo);
  classDefinition->set_endOfConstruct (fileInfo);

  // This is the defining declaration for the class (with a reference to the class definition)
  SgModuleStatement * classDeclaration = new SgModuleStatement (fileInfo,
      moduleName.c_str (), SgClassDeclaration::e_struct, NULL, classDefinition);

  // Set the defining declaration in the defining declaration!
  classDeclaration->set_definingDeclaration (classDeclaration);

  // Set the non defining declaration in the defining declaration (both are required)
  SgClassDeclaration * nondefiningClassDeclaration = new SgClassDeclaration (
      fileInfo, moduleName.c_str (), SgClassDeclaration::e_struct, NULL, NULL);
  nondefiningClassDeclaration->set_type (SgClassType::createType (
      nondefiningClassDeclaration));

  // Set the internal reference to the non-defining declaration
  classDeclaration->set_firstNondefiningDeclaration (
      nondefiningClassDeclaration);
  classDeclaration->set_type (nondefiningClassDeclaration->get_type ());

  // Set the defining and non-defining declarations in the non-defining class declaration!
  nondefiningClassDeclaration->set_firstNondefiningDeclaration (
      nondefiningClassDeclaration);
  nondefiningClassDeclaration->set_definingDeclaration (classDeclaration);

  // Set the nondefining declaration as a forward declaration!
  nondefiningClassDeclaration->setForward ();

  // Don't forget the set the declaration in the definition (IR node constructors are side-effect free!)!
  classDefinition->set_declaration (classDeclaration);

  // set the scope explicitly (name qualification tricks can imply it is not always the parent IR node!)
  classDeclaration->set_scope (fileScope);
  nondefiningClassDeclaration->set_scope (fileScope);

  /*
   * Add function symbol to global scope
   */
  fileScope->insert_symbol (classDeclaration->get_name (), new SgClassSymbol (
      classDeclaration));

  /*
   * Error checking
   */
  ROSE_ASSERT (classDeclaration->get_definingDeclaration() != NULL);
  ROSE_ASSERT (classDeclaration->get_firstNondefiningDeclaration() != NULL);
  ROSE_ASSERT (classDeclaration->get_definition() != NULL);
  ROSE_ASSERT (fileScope->lookup_class_symbol(classDeclaration->get_name()) != NULL);

  return classDeclaration;
}

SgScopeStatement *
CreateKernels::createDirectLoopCUDAModule (SgSourceFile & sourceFile,
    std::string const moduleName)
{
  using std::string;
  using std::vector;
  using SageInterface::appendStatement;

  Debug::getInstance ()->debugMessage ("Creating direct loop CUDA module", 2);

  SgGlobal * globalScope = sourceFile.get_globalScope ();

  /*
   * Create the module statement
   */
  SgModuleStatement * cudaModule = buildClassDeclarationAndDefinition (
      moduleName, globalScope);

  cudaModule->get_definition ()->setCaseInsensitive (true);

  appendStatement (cudaModule, globalScope);

  /*
   * Add the 'use' statements of this module
   */
  vector <string> libs;
  libs.push_back ("ISO_C_BINDING");
  libs.push_back ("OP2_C");
  libs.push_back ("cudafor");

  for (vector <string>::iterator it = libs.begin (); it != libs.end (); ++it)
  {
    SgUseStatement* useStatement = new SgUseStatement (
        sourceFile.get_file_info (), (*it), false);
    useStatement->set_definingDeclaration (cudaModule);
    appendStatement (useStatement, cudaModule->get_definition ());
  }

  /*
   * Add the 'contains' statement
   */
  SgContainsStatement * containsStatement = new SgContainsStatement (
      sourceFile.get_file_info ());
  containsStatement->set_parent (cudaModule->get_definition ());
  containsStatement->set_definingDeclaration (containsStatement);

  vector <SgDeclarationStatement *> & statementList =
      cudaModule->get_definition ()->getDeclarationList ();
  statementList.push_back (containsStatement);

  return containsStatement->get_scope ();
}

SgSourceFile*
CreateKernels::createSourceFile ()
{
  using std::string;
  using SageBuilder::buildFile;

  Debug::getInstance ()->debugMessage ("Creating source file", 2);

  /*
   * To create a new file (to which the AST is later unparsed), the API expects
   * the name of an existing file and the name of the output file. There is no
   * input file corresponding to our output file, therefore we first create a
   * dummy Fortran file
   */
  string const FORTRAN_FILE_SUFFIX = ".F95";
  string const inputFileName = "BLANK" + FORTRAN_FILE_SUFFIX;

  FILE* inputFile = fopen (inputFileName.c_str (), "w+");
  if (inputFile != NULL)
  {
    fclose (inputFile);
  }
  else
  {
    Debug::getInstance ()->debugMessage (
        "Could not create dummy Fortran file '" + inputFileName + "'", 1);
    exit (1);
  }

  /*
   * Now generate the CUDA file
   */
  string const CUDA_FILE_NAME_SUFFIX = "_cuda_module.CUF";
  string const outputFileName = opParLoop->getKernelHostName ()
      + CUDA_FILE_NAME_SUFFIX;

  Debug::getInstance ()->debugMessage ("Generating CUDA file '"
      + outputFileName + "'", 1);

  SgSourceFile* sourceFile = isSgSourceFile (buildFile (inputFileName,
      outputFileName, NULL));

  /*
   * Later unparse according to the Fortran 95 standard
   */
  sourceFile->set_F95_only (true);

  /*
   * No implicit symbols shall be allowed in the generated Fortran file
   */
  sourceFile->set_fortran_implicit_none (true);

  sourceFile->set_outputFormat (SgFile::e_free_form_output_format);

  /*
   * Store the file so it can be unparsed after AST construction
   */
  kernelOutputFiles.push_back (sourceFile);

  return sourceFile;
}

void
CreateKernels::retrieveArgumentsTypes (SgExpressionPtrList & args)
{
  using boost::lexical_cast;
  using std::string;
  using std::vector;

  Debug::getInstance ()->debugMessage (
      "Retrieving actual types of OP_DAT arguments", 2);

  /*
   * Scan OP_DAT arguments and get their actual base types.
   * Note that the first OP_DAT argument starts at BASE_OPDAT_ARG_INDEX.
   * Then we have an OP_DAT argument every NUMBER_OF_OP_DAT_ARGUMENTS
   */
  for (vector <SgExpression *>::iterator it = args.begin ()
      + OP2::BASE_OP_DAT_ARG_INDEX; it != args.end (); it
      += OP2::NUMBER_OF_OP_DAT_ARGUMENTS)
  {
    if ((*it)->variantT () == V_SgVarRefExp)
    {
      /*
       * The argument of the OP_PAR_LOOP is a variable reference (expression)
       */
      SgVarRefExp * variableReference = isSgVarRefExp (*it);

      if (variableReference->get_type ()->variantT () == V_SgClassType)
      {
        SgClassType* classReference = isSgClassType (
            variableReference->get_type ());

        string const className = classReference->get_name ().getString ();

        if (className.compare (OP2::OP_DAT_NAME) == 0)
        {
          /*
           * Found an OP_DAT variable, so retrieve the type of the corresponding input
           * data from the its name
           */
          SgExpressionPtrList declarationArgs =
              opDeclaredVariables->getDeclaredOpDatArgs (
                  variableReference->get_symbol ()->get_name ().getString ());

          SgVarRefExp * opDatInputArg = isSgVarRefExp (
              declarationArgs[OP2::OP_DECL_DAT_INPUT_REF]);

          SgType * opDatInputType = opDatInputArg->get_type ();

          SgIntVal * opDatDimension = isSgIntVal (
              declarationArgs[OP2::OP_DECL_DAT_DIMENSION_VAL]);

          ROSE_ASSERT (opDatInputType != NULL);
          ROSE_ASSERT (opDatDimension != NULL);

          opParLoop->set_OP_DAT_ActualType (opDatInputType);
          opParLoop->set_OP_DAT_Dimension (opDatDimension->get_value ());

          Debug::getInstance ()->debugMessage ("The OP_DAT variable '"
              + variableReference->get_symbol ()->get_name ().getString ()
              + "' has type " + opDatInputType->class_name ()
              + " and dimension " + lexical_cast <string> (
              opDatDimension->get_value ()), 6);
        }
      }
    }
  }
}

/*
 * ====================================================================================================
 * Public functions
 * ====================================================================================================
 */

/*
 * Over-riding implementation of the AST vertex traversal function
 */
void
CreateKernels::visit (SgNode * node)
{
  using boost::lexical_cast;
  using boost::starts_with;
  using std::string;
  using OP2::OP_PAR_LOOP_PREFIX;

  switch (node->variantT ())
  {
    case V_SgProcedureHeaderStatement:
    {
      /*
       * Remember all subroutine definitions so as to later recover
       * actual kernel definitions when copying them into the new files
       */

      SgProcedureHeaderStatement * procedureHeaderStatement =
          isSgProcedureHeaderStatement (node);
      inputSubroutines.push_back (procedureHeaderStatement);

      Debug::getInstance ()->debugMessage ("Found procedure header statement '"
          + procedureHeaderStatement->get_name ().getString () + "'", 8);

      break;
    }

    case V_SgFunctionCallExp:
    {
      /*
       * Function call found in the AST
       */
      SgFunctionCallExp* functionCallExp = isSgFunctionCallExp (node);

      string const
          calleeName =
              functionCallExp->getAssociatedFunctionDeclaration ()->get_name ().getString ();

      /*
       * The prefix of all parallel loop calls in Fortran contains 'op_par_loop'.
       * The suffix of the call, however, changes depending on the number of expected
       * parameters. Therefore, any match of this prefix indicates a call of interest
       * to the translator
       */
      if (starts_with (calleeName, OP_PAR_LOOP_PREFIX))
      {
        /*
         * Retrieve the arguments of the call
         */
        SgExpressionPtrList& args =
            functionCallExp->get_args ()->get_expressions ();

        Debug::getInstance ()->debugMessage ("Found call to '" + calleeName
            + "'", 2);

        /*
         * TODO: Remove this comparison to handle arbitrary OP_PAR_LOOPs
         * An 'op_par_loop_2' always has 10 arguments
         */
        if (args.size () == 10)
        {
          /*
           * The first argument to an 'op_par_loop' call should be a reference to
           * the kernel function. Cast it and proceed, otherwise throw an exception
           */
          SgFunctionRefExp * functionRefExp =
              isSgFunctionRefExp (args.front ());
          try
          {
            if (functionRefExp != NULL)
            {
              opParLoop
                  = new OpParLoop (
                      functionRefExp->getAssociatedFunctionDeclaration ()->get_name ().getString (),
                      (args.size () - 2) / 4);

              Debug::getInstance ()->debugMessage ("Found kernel '"
                  + opParLoop->getKernelHostName () + "'", 2);

              /*
               * Retrieve arguments data types
               */
              retrieveArgumentsTypes (args);

              /*
               * Generate the CUDA file for this kernel
               */
              SgSourceFile * sourceFile = createSourceFile ();

              // TODO: substitute with a check of the input arguments to understand if it
              // is a direct or indirect loop
              if (true)
              {
                /*
                 * The name of the generated CUDA-Fortran module
                 */
                string const CUDAModuleName = OP_PAR_LOOP_PREFIX + "_"
                    + opParLoop->getKernelHostName () + "_module";

                /*
                 * Create the CUDA module
                 */
                SgScopeStatement * moduleScope = createDirectLoopCUDAModule (
                    *sourceFile, CUDAModuleName);

                /*
                 * Generate and modify user kernel so that it can run on the device
                 */
                createCUDAKernel (moduleScope);

                createMainKernelSubroutine (moduleScope, args,
                    *(sourceFile)->get_file_info ());

                SgProcedureHeaderStatement * hostSubroutine =
                    createHostSubroutine (args, *sourceFile, moduleScope);

                SgExprStatement * exprStat = isSgExprStatement (
                    node->get_parent ());

                fix_OP_PAR_LOOP_Calls (functionCallExp, hostSubroutine,
                    exprStat->get_scope (), CUDAModuleName);
              }
            }
            else
            {
              throw args.front ();
            }
          }
          catch (SgNode *exceptionNode)
          {
            Debug::getInstance ()->debugMessage (
                "First argument to 'OP_PAR_LOOP' is not a kernel. The argument has type '"
                    + exceptionNode->class_name () + "'", 1);
            exit (1);
          }
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

  Debug::getInstance ()->verboseMessage ("Generating CUDA files");

  for (vector <SgSourceFile *>::const_iterator it = kernelOutputFiles.begin (); it
      != kernelOutputFiles.end (); ++it)
  {
    Debug::getInstance ()->debugMessage ("Unparsing AST to "
        + (*it)->getFileName (), 1);

    /*
     * Get the source project associated with this file
     */
    SgProject* project = (*it)->get_project ();

    /*
     * Ensure the AST is consistent before unparsing
     */
    AstTests::runAllTests (project);

    /*
     * Generate the file
     */
    project->unparse ();
  }
}

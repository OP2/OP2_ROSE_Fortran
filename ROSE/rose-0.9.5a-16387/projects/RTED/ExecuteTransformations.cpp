#include <rose.h>

// DQ (2/9/2010): Testing use of ROE to compile ROSE.
#ifndef USE_ROSE

#include <string>
#include <set>

#include <boost/foreach.hpp>
#include "RtedSymbols.h"
#include "DataStructures.h"
#include "RtedTransformation.h"
#include "RtedVisit.h"


// ---------------------------------------
// Perform all transformations...
//
// Do insertions LIFO, so, e.g. if we want to add stmt1; stmt2; after stmt0
// add stmt2 first, then add stmt1
// ---------------------------------------
void RtedTransformation::executeTransformations()
{
  BOOST_FOREACH( ReturnInfo ret, returnstmt )
  {
    changeReturnStmt(ret);
  }

  // bracket the bodies of constructors with enter/exit.  This is easier than
  // bracketing the variable declarations, and isn't harmful because the
  // return type is fixed.  However, it would not be wrong to simply bracket
  // the variable declaration, e.g.
  //    MyClassWithConstructor a;
  //
  //  transformed to:
  //    enterScope("constructor");
  //    MyClassWithConstructor a;
  //    exitScope("constructor");
  //
  BOOST_FOREACH( SgFunctionDefinition* fndef, function_definitions ) {
    // tps : 09/14/2009 : handle templates
    SgTemplateInstantiationFunctionDecl* istemplate =
      isSgTemplateInstantiationFunctionDecl(fndef->get_parent());
    if (RTEDDEBUG) std::cerr <<" ^^^^ Found definition : " << fndef->get_declaration()->get_name().str() <<       "  is template: " << istemplate << std::endl;
    if (istemplate) {
      SgGlobal* gl = isSgGlobal(istemplate->get_parent());
      ROSE_ASSERT(gl);
      const SgNodePtrList&          nodes2 = NodeQuery::querySubTree(istemplate, V_SgLocatedNode);
      SgNodePtrList::const_iterator nodesIT2 = nodes2.begin();
      for (; nodesIT2 != nodes2.end(); nodesIT2++) {
        SgLocatedNode* node = isSgLocatedNode(*nodesIT2);
        ROSE_ASSERT(node);
        Sg_File_Info* file_info = node->get_file_info();
        file_info->setOutputInCodeGeneration();
      }
      // insert after template declaration
      // find last template declaration, which should be part of SgGlobal
      const SgNodePtrList&          nodes3 = NodeQuery::querySubTree(gl, V_SgTemplateInstantiationFunctionDecl);
      SgNodePtrList::const_iterator nodesIT3 = nodes3.begin();
      SgTemplateInstantiationFunctionDecl* templ = NULL;
      for (; nodesIT3 != nodes3.end(); nodesIT3++) {
        SgTemplateInstantiationFunctionDecl* temp = isSgTemplateInstantiationFunctionDecl(*nodesIT3);
        std::string temp_str = temp->get_qualified_name().str();
        std::string template_str = istemplate->get_qualified_name().str();
        if (temp!=istemplate)
          if (temp_str.compare(template_str)==0)
            templ =temp;
        //std::cerr << " Found templ : " << temp->get_qualified_name().str() << ":"<<temp<<
        // "  istemplate : " << istemplate->get_qualified_name().str() << ":"<<istemplate<<
        // "         " << (temp_str.compare(template_str)==0) << std::endl;
      }
      ROSE_ASSERT(templ);
      SageInterface::removeStatement(istemplate);
      SageInterface::insertStatementAfter(templ,istemplate);
    }
  }

  // add calls to register pointer change after pointer arithmetic
  BOOST_FOREACH( SgExpression* op, pointer_movements ) {
    ROSE_ASSERT( op );
    insert_pointer_change( op );
  }

  // This must occur before variable
  // initialization, so that assignments of function return values happen before exitScope is called.
  std::for_each( callsites.begin(),
                 callsites.end(),
                 std::bind1st(std::mem_fun(&RtedTransformation::transformCallSites), this)
               );

  InitializedVarMap::const_iterator it5 = variableIsInitialized.begin();
  for (; it5 != variableIsInitialized.end(); it5++) {
    SgVarRefExp*                   varref = it5->first;
    InitializedVarMap::mapped_type p = it5->second;

    insertInitializeVariable(p.first, varref, p.second);
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in create_array_define_varRef_multiArray  : "  << create_array_define_varRef_multiArray.size() << std::endl;
  std::map<SgVarRefExp*, RtedArray>::const_iterator itm =  create_array_define_varRef_multiArray.begin();
  for (; itm != create_array_define_varRef_multiArray.end(); ++itm) {
    insertArrayCreateCall(itm->second, itm->first);
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in variable_access_varref  : " << variable_access_varref.size() << std::endl;
  BOOST_FOREACH( SgVarRefExp* vr, variable_access_varref ) {
    ROSE_ASSERT(vr);
    insertAccessVariable(vr,NULL);
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in variable_access_pointer  : " << variable_access_pointerderef.size() << std::endl;
  std::map<SgPointerDerefExp*,SgVarRefExp*>::const_iterator itAccess2 = variable_access_pointerderef.begin();
  for (; itAccess2 != variable_access_pointerderef.end(); ++itAccess2) {
    insertAccessVariable(itAccess2->second, itAccess2->first);
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in variable_access_arrowexp  : " << variable_access_arrowexp.size() << std::endl;
  std::map<SgArrowExp*, SgVarRefExp*>::const_iterator itAccessArr = variable_access_arrowexp.begin();
  for (; itAccessArr != variable_access_arrowexp.end(); ++itAccessArr) {
    insertAccessVariable(itAccessArr->second, itAccessArr->first);
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in variable_access_arrowthisexp  : " << variable_access_arrowthisexp.size() << std::endl;
  std::map<SgExpression*, SgThisExp*>::const_iterator itAccessArr2 =  variable_access_arrowthisexp.begin();
  for (; itAccessArr2 != variable_access_arrowthisexp.end(); ++itAccessArr2) {
    SgExpression* pd = itAccessArr2->first;
    SgThisExp*    in = itAccessArr2->second;

    insertCheckIfThisNull(in);
    insertAccessVariable(in, pd);
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in create_array_define_varRef_multiArray_stack  : "  << create_array_define_varRef_multiArray_stack.size() << std::endl;
  std::map<SgInitializedName*, RtedArray>::const_iterator itv = create_array_define_varRef_multiArray_stack.begin();
  for (; itv != create_array_define_varRef_multiArray_stack.end(); ++itv) {
    insertArrayCreateCall(itv->second);
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in variable_declarations  : " << variable_declarations.size() << std::endl;
  BOOST_FOREACH( SgInitializedName* initName, variable_declarations )
    insertVariableCreateCall(initName);

  // make sure register types wind up before variable & array create, so that
  // types are always available
  if (RTEDDEBUG) std::cerr << "\n # Elements in class_definitions  : " << class_definitions.size() << std::endl;
  std::map<SgClassDefinition*,RtedClassDefinition*>::const_iterator refIt = class_definitions.begin();
  for (; refIt != class_definitions.end(); ++refIt) {
    RtedClassDefinition* rtedClass = refIt->second;

    insertRegisterTypeCall(rtedClass);
    insertCreateObjectCall( rtedClass );
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in create_array_access_call  : " << create_array_access_call.size() << std::endl;
  std::map<SgPntrArrRefExp*, RtedArray>::const_iterator ita = create_array_access_call.begin();
  for (; ita != create_array_access_call.end(); ++ita) {
    insertArrayAccessCall(ita->first, ita->second);
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in function_call_missing_def  : " << function_call_missing_def.size() << std::endl;
  BOOST_FOREACH( SgFunctionCallExp* fncall, function_call_missing_def )
    insertAssertFunctionSignature( fncall );

  if (RTEDDEBUG) std::cerr << "\n # Elements in function_definitions  : " << function_definitions.size() << std::endl;
  BOOST_FOREACH( SgFunctionDefinition* fndef, function_definitions) {
    insertVariableCreateInitForParams( fndef );
    insertConfirmFunctionSignature( fndef );
  }

  if (RTEDDEBUG) std::cerr << "\n # Elements in funccall_call : " << function_call.size() << std::endl;
  BOOST_FOREACH( RtedArguments& funcs, function_call) {
    if (isStringModifyingFunctionCall(funcs.f_name) ) {
      insertFuncCall(funcs);
    } else if (isFileIOFunctionCall(funcs.f_name)) {
      insertIOFuncCall(funcs);
    }
  }

  BOOST_FOREACH( Deallocations::value_type releaseOp, frees)
  {
    insertFreeCall( releaseOp.first, releaseOp.second );
  }

  BOOST_FOREACH( SgFunctionCallExp* fcallexp, reallocs )
  {
    insertReallocateCall( fcallexp );
  }

  std::for_each( upcBlockingOps.begin(),
                 upcBlockingOps.end(),
                 std::bind1st(std::mem_fun(&RtedTransformation::transformUpcBlockingOps), this)
               );

  std::for_each( sharedptr_derefs.begin(),
                 sharedptr_derefs.end(),
                 std::bind1st(std::mem_fun(&RtedTransformation::transformPtrDerefs), this)
               );

  // bracket scope statements with calls to enterScope and exitScope.
  if (RTEDDEBUG) std::cerr << "\n # Elements in scopes  : " << scopes.size() << std::endl;
  BOOST_FOREACH( ScopeContainer::value_type pseudoblock, scopes ) {
    // bracket all scopes except constructors with enter/exit
    bracketWithScopeEnterExit( pseudoblock, pseudoblock->get_endOfConstruct() );
  }

  if (RTEDDEBUG)  std::cerr << "Inserting main close call" << std::endl;

  insertMainCloseCall();
}


#endif

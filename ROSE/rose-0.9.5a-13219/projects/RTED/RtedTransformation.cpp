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

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;



/* -----------------------------------------------------------
 * Run frontend and return project
 * -----------------------------------------------------------*/
SgProject*
RtedTransformation::parse(int argc, char** argv) {
   SgProject* project = frontend(argc, argv);
   ROSE_ASSERT(project);
   return project;
}

/* -----------------------------------------------------------
 * Load all function symbols that are needed for transformations
 * Basically we need to know which functions to insert
 * -----------------------------------------------------------*/
void RtedTransformation::loadFunctionSymbols(SgProject* project) {
   // traverse the AST and find locations that need to be transformed
   symbols->traverse(project, preorder);

   ROSE_ASSERT(symbols->roseCreateHeap);
   ROSE_ASSERT(symbols->roseAccessHeap);
   ROSE_ASSERT(symbols->roseFunctionCall);
   ROSE_ASSERT(symbols->roseAssertFunctionSignature);
   ROSE_ASSERT(symbols->roseConfirmFunctionSignature);
   ROSE_ASSERT(symbols->roseFreeMemory);
   ROSE_ASSERT(symbols->roseReallocateMemory);
   ROSE_ASSERT(symbols->roseConvertIntToString);
   ROSE_ASSERT(symbols->roseCheckpoint);
   ROSE_ASSERT(symbols->roseCreateVariable);
   ROSE_ASSERT(symbols->roseCreateObject);
   ROSE_ASSERT(symbols->roseInitVariable);
   ROSE_ASSERT(symbols->roseMovePointer);
   ROSE_ASSERT(symbols->roseAccessVariable);
   ROSE_ASSERT(symbols->roseEnterScope);
   ROSE_ASSERT(symbols->roseExitScope);
   ROSE_ASSERT(symbols->roseIOFunctionCall);
   ROSE_ASSERT(symbols->roseRegisterTypeCall);
   ROSE_ASSERT(symbols->size_t_member);
   ROSE_ASSERT(symbols->roseCheckIfThisNULL);
}

/* -----------------------------------------------------------
 * Perform all transformations needed
 * -----------------------------------------------------------*/
void RtedTransformation::transform(SgProject* project, set<string> &rtedfiles) {
   if (RTEDDEBUG())   cout << "Running Transformation..." << endl;
   globalScope = getFirstGlobalScope(isSgProject(project));
   ROSE_ASSERT( project);

   this -> rtedfiles = &rtedfiles;
   loadFunctionSymbols(project);

   VariableTraversal varTraversal(this);
   InheritedAttribute inheritedAttribute(false,false,false,false,false,false);
   //   InheritedAttribute inheritedAttribute(bools);
   // Call the traversal starting at the project (root) node of the AST
   varTraversal.traverseInputFiles(project,inheritedAttribute);


   // tps: Traverse all classes that appear in header files and create copy in source file within a namespace.
   // We need to know the sizeOf classes. To do so we need to modify the class but do not want to do this in the header file right now.
   vector<SgClassDeclaration*> traverseClasses;
   insertNamespaceIntoSourceFile(project,traverseClasses);
   // traverse all header files and collect information
   vector<SgClassDeclaration*>::const_iterator travClassIt = traverseClasses.begin();
   for (;travClassIt!=traverseClasses.end();++travClassIt) {
      // traverse the new classes with RTED namespace
      traverse(*travClassIt,preorder);
   }

   executeTransformations();
}


/* -----------------------------------------------------------
 * Collects information needed for transformations
 * -----------------------------------------------------------*/

void RtedTransformation::visit(SgNode* n) {
   if (isSgScopeStatement(n)) {
#if 1
      // if, while, do, etc., where we need to check for locals going out of scope
      visit_isSgScopeStatement(n);
      // *********************** DETECT structs and class definitions ***************
      if (isSgClassDefinition(n)) {
         // call to a specific function that needs to be checked
         //cerr << " +++++++++++++++++++++ FOUND Class Def!! ++++++++++++++++ " << endl;
         visit_isClassDefinition(isSgClassDefinition(n));
      }
#endif
   }

}

#endif

// vim:et sta ts=2 sw=2:

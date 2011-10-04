/*
 * RtedTransf_variable.cpp
 *
 *  Created on: Jul 7, 2009
 *      Author: panas2
 */

#include <rose.h>

// DQ (2/9/2010): Testing use of ROE to compile ROSE.
#ifndef USE_ROSE

#include <string>
#include <boost/foreach.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "RtedSymbols.h"
#include "DataStructures.h"
#include "RtedTransformation.h"

#include "sageGeneric.hpp"

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;

// ------------------------ VARIABLE SPECIFIC CODE --------------------------

bool isFileIOVariable(SgType* type)
{
        SgType*     under = skip_Typedefs(type);
        std::string name = under->unparseToString();

        bool res =  boost::starts_with( name, std::string("class ::std::basic_fstream") );

        // std::cerr << "@@@ IOVar?" << name << " " << res << std::endl;
        return res;
}


void RtedTransformation::insertCreateObjectCall(RtedClassDefinition* rcdef)
{
        SgClassDefinition*                 cdef = rcdef -> classDef;
        SgMemberFunctionDeclarationPtrList constructors;

        appendConstructors(cdef, constructors);

        BOOST_FOREACH( SgMemberFunctionDeclaration* constructor, constructors )
        {
            // FIXME 2: Probably the thing to do in this case is simply to bail and
            // trust that the constructor will get transformed when its definition
            // is processed
            SgFunctionDefinition* def = constructor -> get_definition();


            ROSE_ASSERT( def );

            SgBasicBlock* body = def -> get_body();

            // We need the symbol to build a this expression
            SgSymbolTable* sym_tab
            = cdef -> get_declaration() -> get_scope() -> get_symbol_table();

            SgClassSymbol* csym
            = isSgClassSymbol(
                            sym_tab -> find_class(
                                            cdef -> get_declaration() -> get_name() ));
            ROSE_ASSERT( csym );

            SgType* type = cdef -> get_declaration() -> get_type();

            // build arguments to roseCreateObject
            SgExprListExp* arg_list = buildExprListExp();

            appendExpression( arg_list, ctorTypeDesc(mkTypeInformation(type, true, false)) );

            appendAddressAndSize( arg_list,
                                  Whole,
                                  buildPointerDerefExp( buildThisExp( csym )), // we want &(*this), sizeof(*this)
                                  type,
                                  NULL
                                );

            appendFileInfo( arg_list, body );

            // create the function call and prepend it to the constructor's body
            ROSE_ASSERT( symbols.roseCreateObject );
            SgExprStatement* fncall =
            buildExprStatement(
                            buildFunctionCallExp(
                                            buildFunctionRefExp( symbols.roseCreateObject ),
                                            arg_list ));
            attachComment( fncall, "", PreprocessingInfo::before );
            attachComment(
                            fncall,
                            "RS: Create Variable, parameters: "
                            "type, basetype, indirection_level, "
                            "address, size, filename, line, linetransformed",
                            PreprocessingInfo::before );

            ROSE_ASSERT( fncall );
            body -> prepend_statement( fncall );
        }
}


/// \brief              adjusts scopes for some variable declaration
/// \param stmt         current statement, might be adjusted
/// \param scope        current statement, might be adjusted
/// \param was_compgen  this output variable is set to true
///                     when stmt could not be adjusted b/c it was compiler generated.
/// \param mainfirst    the first statement in main (typically RtedTransformation::mainfirst)
///
/// \todo
/// \note  I refactored a number of code repetitions into this functions.
///        A was_compgen typically leads into a ROSE_ASSERT, but not always.
///        Possibly, the difference behavior might just reflect clones
///        that have run apart...
static
void adjustStmtAndScopeIfNeeded(SgStatement*& stmt, SgScopeStatement*& scope, bool& was_compgen, SgStatement* mainfirst)
{
    // what if there is an array creation within a ClassDefinition
    if (isSgClassDefinition(scope)) {
            // new stmt = the classdef scope
            SgClassDeclaration* decl = isSgClassDeclaration(scope->get_parent());
            ROSE_ASSERT(decl);

            stmt = isSgVariableDeclaration(decl->get_parent());
            if (stmt)
            {
              scope = scope->get_scope();
              // We want to insert the stmt before this classdefinition, if its still in a valid block
              cerr << " ....... Found ClassDefinition Scope. New Scope is : "
                            << scope->class_name() << "  stmt:" << stmt->class_name()
                            << endl;
            }
            else
            {
              was_compgen = true;
              cerr << " Error . stmt is unknown : "
                   << decl->get_parent()->class_name() << endl;

              ROSE_ASSERT( decl->get_file_info()->isCompilerGenerated() );
            }
    }
    // what if there is an array creation in a global scope
    else if (isSgGlobal(scope))
    {
            cerr << "RuntimeInstrumentation :: WARNING - Scope not handled!!! : "
                 << " : " << scope->class_name()
                 << endl;

            // We need to add this new statement to the beginning of main
            // get the first statement in main as stmt
            stmt = mainfirst;
            scope = stmt->get_scope();
    }
}

static
void adjustStmtAndScopeIfNeeded_NoCompilerGen(SgStatement*& stmt, SgScopeStatement*& scope, SgStatement* mainfirst)
{
  bool compgen = false;

  adjustStmtAndScopeIfNeeded(stmt, scope, compgen, mainfirst);
  ROSE_ASSERT(!compgen);
}



void RtedTransformation::insertVariableCreateCall(SgInitializedName* initName)
{
        ROSE_ASSERT(initName);

        SgStatement*      stmt = getSurroundingStatement(*initName);

        // extern variables are not handled; they will be handled in the
        // defining translation unit.
        // \pp \todo shall we control if we have the defining translation unit?
        if (isGlobalExternVariable(stmt)) return;

        SgScopeStatement* scope = stmt->get_scope();
        ROSE_ASSERT(scope);

        adjustStmtAndScopeIfNeeded_NoCompilerGen(stmt, scope, mainFirst);

        // for( int i =0;
        // ForStmt .. ForInitStmt .. <stmt>
        // upc_forall (int i = 9;
        // ...
        if (isSgForInitStatement(stmt -> get_parent())) {
                // we have to handle for statements separately, because of parsing
                // issues introduced by variable declarations in the for loop's
                // init statement
                SgFunctionCallExp* buildVar = buildVariableCreateCallExpr(initName);
                ROSE_ASSERT(buildVar != NULL);

                SgStatement* const for_loop = GeneralizdFor::is( stmt -> get_parent() -> get_parent() );
                ROSE_ASSERT(for_loop != NULL);

                prependPseudoForInitializerExpression(buildVar, for_loop);
        } else if (isSgIfStmt(scope)) {
                SgExprStatement* exprStmt = buildVariableCreateCallStmt(initName);
                ROSE_ASSERT(exprStmt);

                SgStatement* trueb = isSgIfStmt(scope)->get_true_body();
                SgStatement* falseb = isSgIfStmt(scope)->get_false_body();
                bool partOfTrue = traverseAllChildrenAndFind(initName, trueb);
                bool partOfFalse = traverseAllChildrenAndFind(initName, falseb);
                bool partOfCondition = (!partOfTrue && !partOfFalse);

                if (trueb && (partOfTrue || partOfCondition)) {
                        if (!isSgBasicBlock(trueb)) {
                                removeStatement(trueb);
                                SgBasicBlock* bb = buildBasicBlock();
                                bb->set_parent(isSgIfStmt(scope));
                                isSgIfStmt(scope)->set_true_body(bb);
                                bb->prepend_statement(trueb);
                                trueb = bb;
                        }
                        prependStatement(exprStmt, isSgScopeStatement(trueb));
                }
                if (falseb && (partOfFalse || partOfCondition)) {
                        if (!isSgBasicBlock(falseb)) {
                                removeStatement(falseb);
                                SgBasicBlock* bb = buildBasicBlock();
                                bb->set_parent(isSgIfStmt(scope));
                                isSgIfStmt(scope)->set_false_body(bb);
                                bb->prepend_statement(falseb);
                                falseb = bb;
                        }
                        prependStatement(exprStmt, isSgScopeStatement(falseb));
                } else if (partOfCondition) {
                        // create false statement, this is sometimes needed
                        SgBasicBlock* bb = buildBasicBlock();
                        bb->set_parent(isSgIfStmt(scope));
                        isSgIfStmt(scope)->set_false_body(bb);
                        falseb = bb;
                        prependStatement(exprStmt, isSgScopeStatement(falseb));
                }
        } else if (isNormalScope(scope)) {
                // insert new stmt (exprStmt) after (old) stmt
                SgExprStatement* exprStmt = buildVariableCreateCallStmt(initName);

                ROSE_ASSERT(exprStmt);
                //~ cerr << "++++++++++++ stmt :" << stmt << " mainFirst:"
                                //~ << mainFirst << "   initName->get_scope():"
                                //~ << initName->get_scope()
                                //~ << "   mainFirst->get_scope():"
                                //~ << mainFirst->get_scope() << endl;
                // FIXME 2: stmt == mainFirst is probably wrong for cases where the
                // statment we want to instrument really is the first one in main (and not
                // merely one in the global scope)
                if (stmt == mainFirst && initName->get_scope() != mainFirst->get_scope()) {
                        insertStatementAfter(globalsInitLoc, exprStmt);
                        globalsInitLoc = exprStmt;
                        // mainBody -> prepend_statement(exprStmt);
                        // cerr << "+++++++ insert Before... " << endl;
                } else {
                        // insert new stmt (exprStmt) after (old) stmt
                        insertStatementAfter(stmt, exprStmt);
                        cerr << "+++++++ insert After... " << endl;
                }
        }
        else
        {
                cerr
                                << "RuntimeInstrumentation :: WARNING - unhandled (unexpected) scope!!! : "
                                << initName->get_mangled_name().str() << " : " << scope->class_name() << endl;

                 // crash on unexpected scopes
                 ROSE_ASSERT(isSgNamespaceDefinitionStatement(scope));
        }
}


// convenience function
SgFunctionCallExp*
RtedTransformation::buildVariableCreateCallExpr(SgInitializedName* initName, bool forceinit) {

        SgInitializer* initializer = initName->get_initializer();

        // FIXME 2: We don't handle initializer clauses in constructors.
        // E.g.
        // class A {
        //  int x, y;
        //  A : x( 1 ), y( 200 ) { }
        // };
        //
        /* For non objects, any initializer is enough for the variable to be
         * fully initialized, e.g.
         *      int y = 200;    // safe to read y
         * However, all objects will have some initialzer, even if it's nothing more
         * than a constructor initializer.  Which members are initialized is up to
         * the constructor.
         */
        bool initb = (initializer && !(isSgConstructorInitializer(initializer)))
                        || forceinit;

        string debug_name = initName -> get_name();

        // \pp \todo genVarRef seems to leak
        return buildVariableCreateCallExpr(genVarRef(initName), debug_name, initb);
}


SgFunctionCallExp*
RtedTransformation::buildVariableCreateCallExpr(SgVarRefExp* var_ref, const string& debug_name, bool initb)
{
        ROSE_ASSERT( var_ref );

        SgType*            varType = var_ref -> get_type();

        // variables are handled by buildArrayCreateCall
        ROSE_ASSERT ( "SgArrayType" != skip_ModifierType(varType)->class_name() );

        SgInitializedName* initName = var_ref -> get_symbol() -> get_declaration();

        // build the function call : runtimeSystem-->createArray(params); ---------------------------
        SgExprListExp*     arg_list = buildExprListExp();


        appendExpression(arg_list, ctorTypeDesc(mkTypeInformation(varType, true, false)) );

        SgScopeStatement*  scope = get_scope(initName);
        ROSE_ASSERT( scope );

        appendAddressAndSize(arg_list, Whole, scope, var_ref, NULL);

        const bool     var_init = initb && !isFileIOVariable(varType);
        AllocKind      allocKind = varAllocKind(*initName);
        SgExpression*  callName = buildStringVal(debug_name);
        SgExpression*  callNameExp = buildStringVal(debug_name);

        appendBool      (arg_list, var_init);
        appendAllocKind (arg_list, allocKind);

        appendExpression(arg_list, callName);
        appendExpression(arg_list, callNameExp);
        appendClassName (arg_list, varType);
        appendFileInfo  (arg_list, scope, var_ref->get_file_info());

        ROSE_ASSERT(symbols.roseCreateVariable);
        SgFunctionRefExp* memRef_r = buildFunctionRefExp(symbols.roseCreateVariable);

        return buildFunctionCallExp(memRef_r, arg_list);
}

// convenience function
SgExprStatement*
RtedTransformation::buildVariableCreateCallStmt(SgInitializedName* initName, bool isParam)
{
        SgFunctionCallExp* fn_call = buildVariableCreateCallExpr(initName, isParam);
        ROSE_ASSERT(fn_call);

        SgExprStatement*   exprStmt = buildExprStatement(fn_call);
        string             comment = "RS : Create Variable, parameters : (name, mangl_name, type, basetype, address, sizeof, initialized, fileOpen, classname, filename, linenr, linenrTransformed)";

        attachComment(exprStmt, "", PreprocessingInfo::before);
        attachComment(exprStmt, comment, PreprocessingInfo::before);

        return exprStmt;
}


SgExpression*
RtedTransformation::buildVariableInitCallExpr( SgInitializedName* initName,
                                               SgVarRefExp* varRefE,
                                               SgStatement* stmt,
                                               AllocKind allockind
                                             )
{
        ROSE_ASSERT(initName && varRefE && stmt);

        // with
        //    arr[ ix ] = value;
        // we want the type of (arr[ ix ]), not arr, as that is the type being
        // written
        // tps (09/14/2009): We need to get all dereferences on the way up
        SgExpression* const     exp = getExprBelowAssignment(varRefE);
        ROSE_ASSERT(exp);
        cerr << " getExprBelowAssignment : " << exp->class_name() << "   "
             << stmt->unparseToString() << endl;

        // build the function call : runtimeSystem-->createArray(params); ---------------------------
        SgExprListExp* const    arg_list = buildExprListExp();
        SgScopeStatement* const scope = get_scope(initName);

        // with
        //    int* p;
        // this is a pointer change
        //    p = (int*) malloc(sizeof(int));
        // but this is not
        //    *p = 10;
        const int               is_pointer_change = (  isSgExprStatement(stmt)
                                                    && isSgPointerType(exp->get_type())
                                                    && ((allockind & akUndefined) == akUndefined)
                                                    );

        // \pp The test for akUndefined was moved from RuntmieSystem.cpp
        //     Not sure why we need it.

        appendExpression(arg_list, ctorTypeDesc(mkTypeInformation(exp -> get_type(), false, false)) );
        appendAddressAndSize(arg_list, Whole, scope, exp, NULL);
        appendExpression(arg_list, buildIntVal(is_pointer_change));
        appendClassName(arg_list, exp -> get_type());
        appendFileInfo(arg_list, stmt);

        ROSE_ASSERT(symbols.roseInitVariable);
        std::string             symbolName2 = symbols.roseInitVariable->get_name().str();
        SgFunctionRefExp*       memRef_r = buildFunctionRefExp(symbols.roseInitVariable);
        SgExpression*           result = buildFunctionCallExp(memRef_r, arg_list);

        return result;
}

void RtedTransformation::insertInitializeVariable(SgInitializedName* initName, SgVarRefExp* varRefE, AllocKind allocKind)
{
        ROSE_ASSERT(initName && varRefE);
        // assert(varRefE->get_parent()) fails b/c varRefE might not be
        //   a proper AST element (constructed at RtedTransf_variable.cpp:763)

        SgStatement* stmt = NULL;
        if (varRefE->get_parent()) // we created a verRef for AssignInitializers which do not have a parent
                stmt = getSurroundingStatement(*varRefE);
        else
                stmt = getSurroundingStatement(*initName);

        ROSE_ASSERT(stmt);

        SgScopeStatement* scope = stmt->get_scope();
        bool              compgen = false;

        ROSE_ASSERT(scope);
        adjustStmtAndScopeIfNeeded(stmt, scope, compgen, mainFirst);

        if (compgen) return;

        string name = initName->get_mangled_name().str();
        cerr << "          ... running insertInitializeVariable :  " << name
                        << "   scope: " << scope->class_name() << endl;

        if (isSgForInitStatement(stmt -> get_parent())) {
                // we have to handle for statements separately, because of parsing
                // issues introduced by variable declarations in the for loop's
                // init statement
                SgExpression*      funcCallExp_vec = buildVariableInitCallExpr(initName, varRefE, stmt, allocKind);
                SgStatement* const for_stmt = GeneralizdFor::is(stmt -> get_parent() -> get_parent());

                ROSE_ASSERT(for_stmt != NULL);
                prependPseudoForInitializerExpression(funcCallExp_vec, for_stmt);
        } else if (isSgIfStmt(scope)) {
                SgExpression*      funcCallExp = buildVariableInitCallExpr(initName, varRefE, stmt, allocKind);
                SgExprStatement*   exprStmt = buildExprStatement(funcCallExp);
                cerr << "If Statment : inserting initvar" << endl;
                ROSE_ASSERT(exprStmt);

                SgIfStmt*          ifStmt = isSgIfStmt(scope);
                SgStatement*       trueb = ifStmt->get_true_body();
                SgStatement*       falseb = ifStmt->get_false_body();
                // find out if the varRefE is part of the true, false or
                // the condition
                const bool         partOfTrue = traverseAllChildrenAndFind(varRefE, trueb);
                const bool         partOfFalse = traverseAllChildrenAndFind(varRefE, falseb);
                const bool         partOfCondition = (!partOfTrue && !partOfFalse);

                cerr << "@@@@ If cond : partOfTrue: " << partOfTrue
                                << "   partOfFalse:" << partOfFalse
                                << "  partOfCondition:" << partOfCondition << endl;

                // \pp \todo simplify the branches below by using the
                //           SageInterface::ensureBasicBlockAs* function family.
                if (trueb && (partOfTrue || partOfCondition)) {
                        if (!isSgBasicBlock(trueb)) {
                                removeStatement(trueb);
                                SgBasicBlock* bb = buildBasicBlock();
                                bb->set_parent(ifStmt);
                                ifStmt->set_true_body(bb);
                                bb->prepend_statement(trueb);
                                trueb = bb;
                        }
                        prependStatement(exprStmt, isSgScopeStatement(trueb));
                }
                if (falseb && (partOfFalse || partOfCondition)) {
                        if (!isSgBasicBlock(falseb)) {
                                removeStatement(falseb);
                                SgBasicBlock* bb = buildBasicBlock();
                                bb->set_parent(ifStmt);
                                ifStmt->set_false_body(bb);
                                bb->prepend_statement(falseb);
                                falseb = bb;
                        }
                        prependStatement(exprStmt, isSgScopeStatement(falseb));
                } else if (partOfCondition) {
                        // create false statement, this is sometimes needed
                        SgBasicBlock* bb = buildBasicBlock();
                        bb->set_parent(ifStmt);
                        ifStmt->set_false_body(bb);
                        falseb = bb;
                        prependStatement(exprStmt, isSgScopeStatement(falseb));
                }
        }
        else if (isNormalScope(scope))
        {
                SgExpression*    funcCallExp = buildVariableInitCallExpr(initName, varRefE, stmt, allocKind);
                SgExprStatement* exprStmt = buildExprStatement(funcCallExp);
                string           empty_comment;
                string           comment = "RS : Init Variable, parameters : (tpye, basetype, class_name, address, size, ismalloc, is_pointer_change, filename, line, linenrTransformed, error line)";

                attachComment(exprStmt, empty_comment, PreprocessingInfo::before);
                attachComment(exprStmt, comment, PreprocessingInfo::before);

                // insert new stmt (exprStmt) before (old) stmt
                insertStatementAfter(stmt, exprStmt);
        } // basic block
        else
        {
                cerr
                                << " -----------> RuntimeInstrumentation :: unhandled (unexpected) block! : "
                                << name << " : " << scope->class_name() << "  - "
                                << stmt->unparseToString() << endl;

                // crash for unexpected scopes
                ROSE_ASSERT(isSgNamespaceDefinitionStatement(scope));
        }
}

void RtedTransformation::insertCheckIfThisNull(SgThisExp* texp)
{
        ROSE_ASSERT(texp);

        cerr << "access to this : " << texp->unparseToString() << endl;

        SgStatement*      stmt = getSurroundingStatement(*texp);
        SgScopeStatement* scope = stmt->get_scope();
        ROSE_ASSERT(scope);

        adjustStmtAndScopeIfNeeded_NoCompilerGen(stmt, scope, mainFirst);

        if (isNormalScope(scope)) {
                // build the function call : runtimeSystem-->createArray(params); ---------------------------
                SgExprListExp* arg_list = buildExprListExp();

                appendExpression(arg_list, buildThisExp(texp->get_class_symbol()));
                appendFileInfo(arg_list, stmt);

                insertCheck( ilBefore,
                             stmt,
                             symbols.roseCheckIfThisNULL,
                             arg_list,
                             "RS : roseCheckIfThisNULL, parameters : (ThisExp, filename, line, line transformed, error Str)"
                           );

        } // basic block
        else
        {
                cerr
                                << " -----------> RuntimeInstrumentation :: unhandled (unexpected) block! : "
                                << " : " << scope->class_name() << "  - "
                                << stmt->unparseToString() << endl;
                ROSE_ASSERT(isSgNamespaceDefinitionStatement(scope));
        }
}

void RtedTransformation::insertAccessVariable(SgThisExp* varRefE, SgExpression* derefExp)
{
        ROSE_ASSERT(varRefE);

        SgStatement*        stmt = getSurroundingStatement(*varRefE);
        SgClassDeclaration* decl = varRefE->get_class_symbol()->get_declaration();
        ROSE_ASSERT(decl);
        SgScopeStatement*   scope = decl->get_scope();
        insertAccessVariable(scope, derefExp, stmt, varRefE);
}

void RtedTransformation::insertAccessVariable(SgVarRefExp* varRefE, SgExpression* derefExp)
{
        ROSE_ASSERT(varRefE);

        SgStatement* stmt = getSurroundingStatement(*varRefE);
        // make sure there is no extern in front of stmt
        SgInitializedName* initName = varRefE->get_symbol()->get_declaration();
        SgScopeStatement* initNamescope = initName->get_scope();

        SgDotExp* parent_dot = isSgDotExp(varRefE -> get_parent());
        if (parent_dot && parent_dot -> get_lhs_operand() == varRefE) {
                //      x = s.y
                // does not need a var ref to y, only to s
                return;
        }
        insertAccessVariable(initNamescope, derefExp, stmt, varRefE);
}

struct ReadCanceler
{
  int rwmask;

  explicit
  ReadCanceler(int read_write_flag)
  : rwmask(read_write_flag)
  {}

  void cancel_read() { rwmask = 0; }

  void handle(const SgNode&) { ROSE_ASSERT(false); }
  void handle(const SgExpression&) {} // default case, no changes

  void handle(const SgAddOp&) { cancel_read(); }
  void handle(const SgSubtractOp&) { cancel_read(); }

  operator int() { return rwmask; }
};

void RtedTransformation::insertAccessVariable( SgScopeStatement* initscope,
                                               SgExpression* derefExp,
                                               SgStatement* stmt,
                                               SgExpression* varRefE
                                             )
{
    ROSE_ASSERT(stmt && varRefE);

    if (stmt->get_file_info()->isCompilerGenerated())
      return;

    SgVarRefExp* asVarRef = isSgVarRefExp(varRefE);
    if (asVarRef && !isInInstrumentedFile(asVarRef->get_symbol()->get_declaration()))
       return;

    if (isFileIOVariable(varRefE->get_type()))
      return;

    SgScopeStatement* scope = stmt->get_scope();
    ROSE_ASSERT(scope);

    cerr << "          ... running insertAccessVariable :  " //<< name
         << "   scope: " << scope->class_name() << endl;

    adjustStmtAndScopeIfNeeded_NoCompilerGen(stmt, scope, mainFirst);

    if (isNormalScope(scope))
    {
            // build the function call : runtimeSystem-->createArray(params); ---------------------------

            int           read_write_mask = Read;
            SgExpression* accessed_exp = varRefE;
            SgExpression* write_location_exp = 0;
            if (derefExp)
            {
                    SgPointerDerefExp* deref_op = isSgPointerDerefExp(derefExp);
                    SgArrowExp*        arrow_op = isSgArrowExp(derefExp);
                    ROSE_ASSERT( deref_op || arrow_op );

                    if (arrow_op)
                    {
                            // with
                            //    p -> b = 2
                            // we need to be able to read
                            //    *p
                            if (isUsedAsLvalue(arrow_op))
                            {
                                    bool isReadOnly =
                                                    isthereAnotherDerefOpBetweenCurrentAndAssign(
                                                                    derefExp);
                                    if (isSgThisExp(arrow_op->get_lhs_operand()))
                                    {
                                       return;
                                    }

                                    accessed_exp = arrow_op->get_lhs_operand();
                                    if (!isReadOnly)
                                    {
                                       read_write_mask |= Write;
                                       write_location_exp = arrow_op;
                                    }
                            }
                            else
                            {
                                    // not a l-value
                                    if (isSgMemberFunctionType(arrow_op -> get_rhs_operand() -> get_type()))
                                            // for member function invocations, we just want to
                                            // check that the pointer is good
                                            accessed_exp = arrow_op -> get_lhs_operand();
                                    else
                                            // normally we'll be reading the member itself
                                            accessed_exp = arrow_op;
                            }
                    }
                    else
                    {
                            // consider
                            //    int *p;
                            //    *p = 24601;
                            //  It is necessary that &p, sizeof(p) is readable, but not
                            //  &(*p), sizeof(*p).
                            if (isUsedAsLvalue(derefExp))
                            {
                                    bool isReadOnly =
                                                    isthereAnotherDerefOpBetweenCurrentAndAssign(
                                                                    derefExp);
                                    accessed_exp = deref_op -> get_operand();

                                    // \pp added this extra case (to make the code uglier)
                                    //     the case is needed to handle upc related tests
                                    //     e.g.: *(arr + idx) = 7
                                    //     here we do not want to test whether
                                    //     arr is readable or not, just test
                                    //     the writeop
                                    //     not sure, if this disables
                                    //     some existing RTED tests
                                    if (!isReadOnly)
                                    {
                                      read_write_mask = sg::dispatch(ReadCanceler(read_write_mask), accessed_exp);
                                      write_location_exp = deref_op;
                                      read_write_mask |= Write;
                                    }
                            }
                            else
                            {
                                    accessed_exp = deref_op;
                            }
                    }
            }

            SgExprListExp* arg_list = buildExprListExp();

            // \pp clear addresses if not needed
            if ((read_write_mask & Read) != Read) accessed_exp = 0;
            if ((read_write_mask & Write) != Write) write_location_exp = 0;

            appendAddressAndSize(arg_list, Elem, initscope, accessed_exp, NULL);
            appendAddressAndSize(arg_list, Elem, initscope, write_location_exp, NULL);
            appendExpression(arg_list, buildIntVal(read_write_mask));
            appendFileInfo(arg_list, stmt);

            insertCheck( ilBefore,
                         stmt,
                         symbols.roseAccessVariable,
                         arg_list,
                         "RS : Access Variable, parameters : (address_r, sizeof(type)_r, address_w, sizeof(type)_w, r/w, filename, line, line transformed, error Str)"
                       );
    } // basic block
    else
    {
            cerr            << " -----------> RuntimeInstrumentation :: unexpected (unhandled) scope! : "
                            << " : " << scope->class_name() << "  - "
                            << stmt->unparseToString() << endl;
            ROSE_ASSERT(isSgNamespaceDefinitionStatement(scope));
    }
}


struct AllocInfo
{
  SgType*   newtype;
  AllocKind allocKind;

  explicit
  AllocInfo(AllocKind default_kind)
  : newtype(NULL), allocKind(default_kind)
  {}

  void handle(SgNode&) { ROSE_ASSERT(false); }

  void handle(SgNewExp& n)
  {
    newtype = n.get_type();
    allocKind = cxxHeapAllocKind(newtype);
  }

  void handle(SgExpression& n) {}
};


void RtedTransformation::visit_isAssignInitializer(SgAssignInitializer* const assign)
{
        ROSE_ASSERT(assign);

        // \pp why is assign->get_parent not enough?
        std::cerr << assign->unparseToString() << std::endl;
        SgInitializedName*   initName = sg::ancestor<SgInitializedName>(assign);

        // \pp assign initializers can also occur as arguments to function calls
        //     e.g., foo(23); // void foo(const double&);
        if (!initName) return;

        // ---------------------------------------------
        // we now know that this variable must be initialized
        // if we have not set this variable to be initialized yet,
        // we do so
        cerr << ">>>>>>> Setting this var to be assign initialized : "
                        << initName->unparseToString() << "  and assignInit: "
                        << assign->unparseToString() << endl;
        SgStatement*         stmt = getSurroundingStatement(*initName);
        SgScopeStatement*    scope = stmt->get_scope();
        ROSE_ASSERT(scope);
        //      SgType* type = initName->get_type();

        // dont do this if the variable is global or an enum constant
        // \pp why not for global variables
        if (!isSgGlobal(initName->get_scope()) && !isSgEnumDeclaration(initName->get_parent()))
        {
            SgVarRefExp* const varRef = buildVarRefExp(initName, scope);

            // \pp \todo do we need the following line?
            varRef->get_file_info()->unsetOutputInCodeGeneration();

            const AllocInfo    allocInfo = sg::dispatch(AllocInfo(akStack), assign->get_operand());

            cerr << "Adding variable init : " << varRef->unparseToString() << endl;
            variableIsInitialized[varRef] = InitializedVarMap::mapped_type(initName, allocInfo.allocKind);

            // tps (09/15/2009): The following code handles AssignInitializers for SgNewExp
            // e.g. int *p = new int;
            // \pp \todo the following block could be pushed into AllocInfo
            if (allocInfo.newtype)
            {
                // TODO 2: This handles new assign initializers, but not malloc assign
                //          initializers.  Consider, e.g:
                //
                //          int* x = (int*) malloc( sizeof( int ));
                SgExpression* sizeExp = buildSizeOfOp(allocInfo.newtype);
                RtedArray     arr(initName, getSurroundingStatement(*initName), allocInfo.allocKind, sizeExp);

                variablesUsedForArray.push_back(varRef);
                create_array_define_varRef_multiArray[varRef] = arr;

                cerr << ">> Setting this var to be initialized : " << initName->unparseToString() << endl;
            }
        }
}


//
// Functions added to treat UPC-forall and C/C++ for loops
//   somewhat uniformly
//

namespace GeneralizdFor
{
  SgStatement* is(SgNode* astNode)
  {
    SgStatement* res = NULL;

    switch (astNode->variantT())
    {
      case V_SgForStatement:
      case V_SgUpcForAllStatement:
        res = isSgStatement(astNode);
        break;
      default: ;
    }

    return res;
  }

  SgForInitStatement* initializer(SgStatement* forloop)
  {
    SgForInitStatement* res = NULL;

    switch (forloop->variantT())
    {
      case V_SgForStatement:
        res = isSgForStatement(forloop)->get_for_init_stmt();
        break;
      case V_SgUpcForAllStatement:
        res = isSgUpcForAllStatement(forloop)->get_for_init_stmt();
        break;
      default: ROSE_ASSERT(false);
    }

    ROSE_ASSERT(res != NULL);
    return res;
  }

  SgStatement* test(SgStatement* forloop)
  {
    SgStatement* res = NULL;

    switch (forloop->variantT())
    {
      case V_SgForStatement:
        res = isSgForStatement(forloop)->get_test();
        break;
      case V_SgUpcForAllStatement:
        res = isSgUpcForAllStatement(forloop)->get_test();
        break;
      default: ROSE_ASSERT(false);
    }

    ROSE_ASSERT(res != NULL);
    return res;
  }
}

#endif

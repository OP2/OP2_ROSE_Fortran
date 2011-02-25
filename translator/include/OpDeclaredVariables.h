/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class stores all OP2 declared variables in the context of a OP2 program
 * 
 * This class extends AstSimpleProcessing since that class provides
 * utilities to traverse the Abstract Syntax Tree (AST).
 *
 */

#ifndef OP_DECLARED_VARIABLES_H
#define OP_DECLARED_VARIABLES_H

#include <rose.h>

using namespace std;

class OpDeclaredVariables: public AstSimpleProcessing
{
  private:
    /*
     * The project which the source-to-source translator operates upon
     */
    SgProject * project;

    /*
     * Actual arguments passed to every declaration of OP_SET in the program
     */
    map <SgName, SgExpressionPtrList> declaredOpSets;

    /*
     * Actual arguments passed to every declaration of OP_MAP in the program
     */
    map <SgName, SgExpressionPtrList> declaredOpMaps;
    /*
     * Actual arguments passed to every declaration of OP_DAT in the program
     */
    map <SgName, SgExpressionPtrList> declaredOpDats;

  public:

    OpDeclaredVariables (SgProject * project)
    {
      this->project = project;
    }

    map <SgName, SgExpressionPtrList>
    getDeclaredOpSets ()
    {
      return declaredOpSets;
    }

    map <SgName, SgExpressionPtrList>
    getDeclaredOpMaps ()
    {
      return declaredOpMaps;
    }

    map <SgName, SgExpressionPtrList>
    getDeclaredOpDats ()
    {
      return declaredOpDats;
    }

    SgExpressionPtrList
    getDeclaredOpSetArgs (SgName * opSetName)
    {
      return declaredOpSets[*opSetName];
    }

    SgExpressionPtrList
    getDeclaredOpMapArgs (SgName * opMapName)
    {
      return declaredOpMaps[*opMapName];
    }

    SgExpressionPtrList
    getDeclaredOpDatArgs (SgName * opDatName)
    {
      return declaredOpDats[*opDatName];
    }

    /*
     * Visit the program and fill up the private variables above
     */
    void
    visit (SgNode * currentNode);

};

#endif

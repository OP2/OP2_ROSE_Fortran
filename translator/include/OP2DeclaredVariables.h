/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class stores all OP2 declared variables in the context of a OP2 program
 * 
 * This class extends AstSimpleProcessing since that class provides
 * utilities to traverse the Abstract Syntax Tree (AST).
 *
 */

#ifndef OP2_DECLARED_VARIABLES_H
#define OP2_DECLARED_VARIABLES_H

#include <rose.h>

class OP2DeclaredVariables: public AstSimpleProcessing
{
  private:

    /*
     * The project which the source-to-source translator operates upon
     */
    SgProject * project;

    /*
     * Actual arguments passed to every OP_DECL_SET in the program
     */
    std::map <std::string, SgExpressionPtrList> OP_SET_Declarations;

    /*
     * Actual arguments passed to every OP_DECL_MAP in the program
     */
    std::map <std::string, SgExpressionPtrList> OP_MAP_Declarations;

    /*
     * Actual arguments passed to every OP_DECL_DAT in the program
     */
    std::map <std::string, SgExpressionPtrList> OP_DAT_Declarations;

    /*
     * Actual arguments passed to every OP_DECL_GBL in the program
     */
    std::map <std::string, SgExpressionPtrList> OP_GBL_Declarations;

  public:

    OP2DeclaredVariables (SgProject * project)
    {
      this->project = project;
    }

    SgExpressionPtrList
    get_OP_DECL_SET_Arguments (std::string opSETName)
    {
      return OP_SET_Declarations[opSETName];
    }

    SgExpressionPtrList
    get_OP_DECL_MAP_Arguments (std::string opMAPName)
    {
      return OP_MAP_Declarations[opMAPName];
    }

    SgExpressionPtrList
    get_OP_DECL_DAT_Arguments (std::string opDATName)
    {
      return OP_DAT_Declarations[opDATName];
    }

    SgExpressionPtrList
    get_OP_DECL_GBL_Arguments (std::string opGBLName)
    {
      return OP_DAT_Declarations[opGBLName];
    }

    /*
     * Traverses the supplied Fortran files to discover
     * OP_DECL_SET, OP_DECL_MAP, OP_DECL_DAT function calls
     * and store their actual arguments
     */
    void
    visit (SgNode * currentNode);
};

#endif

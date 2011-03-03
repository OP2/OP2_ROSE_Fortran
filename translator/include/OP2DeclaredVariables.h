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
    std::map <std::string, SgExpressionPtrList> OP_SET_Declarations;

    /*
     * Actual arguments passed to every declaration of OP_MAP in the program
     */
    std::map <std::string, SgExpressionPtrList> OP_MAP_Declarations;

    /*
     * Actual arguments passed to every declaration of OP_DAT in the program
     */
    std::map <std::string, SgExpressionPtrList> OP_DAT_Declarations;

  public:

    OpDeclaredVariables (SgProject * project)
    {
      this->project = project;
    }

    std::map <std::string, SgExpressionPtrList>
    getDeclaredOpSets ()
    {
      return OP_SET_Declarations;
    }

    std::map <std::string, SgExpressionPtrList>
    getDeclaredOpMaps ()
    {
      return OP_MAP_Declarations;
    }

    std::map <std::string, SgExpressionPtrList>
    getDeclaredOpDats ()
    {
      return OP_DAT_Declarations;
    }

    SgExpressionPtrList
    getDeclaredOpSetArgs (std::string opSetName)
    {
      return OP_SET_Declarations[opSetName];
    }

    SgExpressionPtrList
    getDeclaredOpMapArgs (std::string opMapName)
    {
      return OP_MAP_Declarations[opMapName];
    }

    SgExpressionPtrList
    getDeclaredOpDatArgs (std::string opDatName)
    {
      return OP_DAT_Declarations[opDatName];
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

/*
 * Written by Carlo Bertolli and Adam Betts
 *
 * This class stores all OP2 declared variables in the context of a OP2 program
 * 
 * This class extends AstSimpleProcessing since that class provides
 * utilities to traverse the Abstract Syntax Tree (AST).
 */

#ifndef OP2_DECLARED_VARIABLES_H
#define OP2_DECLARED_VARIABLES_H

#include <rose.h>

#include "OP2Variables.h"

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
    std::vector <OP_SET_Declaration *> OP_SET_Declarations;

    /*
     * Actual arguments passed to every OP_DECL_MAP in the program
     */
    std::vector <OP_MAP_Declaration *> OP_MAP_Declarations;

    /*
     * Actual arguments passed to every OP_DECL_DAT in the program
     */
    std::vector <OP_DAT_Declaration *> OP_DAT_Declarations;

    /*
     * Actual arguments passed to every OP_DECL_GBL in the program
     */
    std::vector <OP_GBL_Declaration *> OP_GBL_Declarations;

  public:

    OP2DeclaredVariables (SgProject * project)
    {
      this->project = project;
    }

    OP_SET_Declaration &
    get_OP_SET_Declaration (std::string const & opSETName)
        throw (std::string const &);

    OP_MAP_Declaration &
    get_OP_MAP_Declaration (std::string const & opMAPName)
        throw (std::string const &);

    OP_DAT_Declaration &
    get_OP_DAT_Declaration (std::string const & opDATName)
        throw (std::string const &);

    OP_GBL_Declaration &
    get_OP_GBL_Declaration (std::string const & opGBLName)
        throw (std::string const &);

    /*
     * Traverses the supplied Fortran files to discover OP_DECL_SET, OP_DECL_MAP,
     * OP_DECL_DAT, OP_DECL_GBL function calls and store their actual arguments
     */
    void
    visit (SgNode * node);
};

#endif

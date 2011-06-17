/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * This class stores all OP2 declared variables and
 * subroutines encountered while parsing the user-supplied
 * source code
 */

#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <rose.h>
#include <OP2Variables.h>

class Declarations: public AstSimpleProcessing
{
  private:

    /*
     * ======================================================
     * The project which the source-to-source translator
     * operates upon
     * ======================================================
     */
    SgProject * project;

    /*
     * ======================================================
     * References to all subroutines found during the visit
     * of the input files
     * ======================================================
     */
    std::vector <SgProcedureHeaderStatement *> subroutinesInSourceCode;

    /*
     * ======================================================
     * Actual arguments passed to every OP_DECL_SET in the
     * program
     * ======================================================
     */
    std::vector <OP_SET_Declaration *> OP_SET_Declarations;

    /*
     * ======================================================
     * Actual arguments passed to every OP_DECL_MAP in the
     * program
     * ======================================================
     */
    std::vector <OP_MAP_Declaration *> OP_MAP_Declarations;

    /*
     * ======================================================
     * Actual arguments passed to every OP_DECL_DAT in the
     * program
     * ======================================================
     */
    std::vector <OP_DAT_Declaration *> OP_DAT_Declarations;

    /*
     * ======================================================
     * Actual arguments passed to every OP_DECL_GBL in the
     * program
     * ======================================================
     */
    std::vector <OP_GBL_Declaration *> OP_GBL_Declarations;

  public:

    OP_SET_Declaration *
    get_OP_SET_Declaration (std::string const & opSETName)
        throw (std::string const &);

    OP_MAP_Declaration *
    get_OP_MAP_Declaration (std::string const & opMAPName)
        throw (std::string const &);

    OP_DAT_Declaration *
    get_OP_DAT_Declaration (std::string const & opDATName)
        throw (std::string const &);

    OP_GBL_Declaration *
    get_OP_GBL_Declaration (std::string const & opGBLName)
        throw (std::string const &);

    std::vector <SgProcedureHeaderStatement *>::const_iterator
    first_SubroutineInSourceCode ();

    std::vector <SgProcedureHeaderStatement *>::const_iterator
    last_SubroutineInSourceCode ();

    /*
     * ======================================================
     * Traverses the supplied files to discover
     * OP_DECL_SET, OP_DECL_MAP, OP_DECL_DAT, OP_DECL_GBL calls
     * and store their actual arguments
     * ======================================================
     */
    void
    visit (SgNode * node);

    Declarations (SgProject * project);
};

#endif

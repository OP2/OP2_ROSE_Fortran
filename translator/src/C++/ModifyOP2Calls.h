/*
 * Written by Adam Betts and Carlo Bertolli
 *
 * Allows debug and verbose options to be passed on ROSE command line.
 * Could be extended to include others
 */

#ifndef MODIFY_OP2_CALLS_H
#define MODIFY_OP2_CALLS_H

#include <rose.h>

class ModifyOP2Calls: public AstSimpleProcessing
{
  private:

    SgProject * project;

  public:

    /*
     * ======================================================
     * Over-riding implementation of the AST vertex traversal
     * function
     * ======================================================
     */
    virtual void
    visit (SgNode * node);

    ModifyOP2Calls (SgProject * project);
};

#endif

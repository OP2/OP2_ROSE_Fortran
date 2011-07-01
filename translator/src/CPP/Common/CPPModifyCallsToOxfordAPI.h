/*
 * Written by Adam Betts and Carlo Bertolli
 */

#ifndef CPP_MODIFY_CALLS_TO_OXFORD_API
#define CPP_MODIFY_CALLS_TO_OXFORD_API

#include <rose.h>

class CPPModifyCallsToOxfordAPI: public AstSimpleProcessing
{
  private:

    virtual void
    visit (SgNode * node);

  public:

    CPPModifyCallsToOxfordAPI ()
    {
    }
};

#endif

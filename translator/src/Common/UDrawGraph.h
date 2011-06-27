#ifndef UDRAWGRAPH_H
#define UDRAWGRAPH_H

#include <rose.h>

class UDrawGraph: public AstSimpleProcessing
{

  private:

    SgProject * project;

  public:

    virtual void
    visit (SgNode * node);

    UDrawGraph ();
};

#endif

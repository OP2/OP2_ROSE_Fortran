#pragma once
#ifndef CPP_PREPROCESS_H
#define CPP_PREPROCESS_H

#include <rose.h>

class CPPProgramDeclarationsAndDefinitions;

class CPPPreProcess: public AstSimpleProcessing
{
  private:
    
    CPPProgramDeclarationsAndDefinitions* declarations;
    
    virtual void
    visit (SgNode* node);
    
  public:
    
    CPPPreProcess (SgProject* project,
                   CPPProgramDeclarationsAndDefinitions* declarations);
};

#endif
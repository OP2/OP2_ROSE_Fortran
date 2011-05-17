
#ifndef ROSE_OP2SOURCE_H
#define ROSE_OP2SOURCE_H

#include <rose.h>
#include <map>
#include <string>

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;

class OPSource : public AstSimpleProcessing 
{
  private:
    SgProject *project;
    SgSourceFile *file;
    
  public:
    OPSource();
    virtual void visit(SgNode *n);
    virtual void atTraversalEnd();

    void fixParLoops(SgNode *n);
		void fixOpFunctions(SgNode *n);
    void fixOpStructs(SgNode *n);

    void setProject(SgProject *p);
    void unparse();

    map<string, SgFunctionDeclaration*> cudaFunctionDeclarations;
};

#endif

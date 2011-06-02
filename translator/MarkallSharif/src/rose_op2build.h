
#ifndef ROSE_OP2BUILD_H
#define ROSE_OP2BUILD_H

#include <rose.h>
#include <vector>
#include "rose_op2parloop.h"

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;

struct constVar
{
	int dim;
	SgType* type;
	constVar(int dim, SgType* type) { this->dim = dim; this->type = type; }
};

class OPBuild : public AstSimpleProcessing
{  
  private:
    SgProject *project;
		SgProject *out_project;
		OPParLoop *pl;
    vector<SgDeclarationStatement*> sharedConstVariables;

  public:
    OPBuild();
    virtual void visit(SgNode *n);
    virtual void atTraversalEnd();

		SgType* getTypeFromExpression(SgExpression* i);
		void setParLoop(OPParLoop *pl);
		void setProject(SgProject *p);
		void generateBuildFile();
		void unparse();
		void setOPParLoop(OPParLoop* pl);
};

#endif

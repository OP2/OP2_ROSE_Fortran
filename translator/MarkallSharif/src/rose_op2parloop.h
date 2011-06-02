
#ifndef ROSE_OP2PARLOOP_H
#define ROSE_OP2PARLOOP_H

#include <rose.h>
#include <vector>
#include <map>
#include <string>

#include "rose_op2argument.h"

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;

class op_par_loop
{
  public:
    static const int num_params;
		string prev_name;

    SgFunctionRefExp *kernel;
    //string label;
    SgVarRefExp *set;
    vector<op_argument*> args;					// All args
    vector<op_argument*> ind_args;			// Args that do uses indirection
		vector<op_argument*> planContainer;	// Arguments are passed into the loop in such a way so that args that uses same op_dat reside together.
																				// This container stores the starting argument of a op_dat category. The size of the container will therefore
																				// give us the number of category there are. This neglects all arguments that do not use indirection

    int numArgs() { return args.size(); }
    int numIndArgs() { return ind_args.size(); }

		void updatePlanContainer(op_argument* arg);
};

// The ParLoop class inherits from AstSimpleProcessing, which provides a mechanism for 
// traversing the AST of the program in ROSE. We use this for convenience, as opposed to
// trying to walk through the AST "manually".

class OPParLoop : public AstSimpleProcessing 
{
  private:
    SgProject *project;
    vector<SgProject*> kernels;

  public:
    OPParLoop();
    virtual void visit(SgNode *n);
    virtual void atTraversalEnd();
    
    void generateSpecial(SgFunctionCallExp *fn, op_par_loop *pl);
    void generateStandard(SgFunctionCallExp *fn, op_par_loop *pl);
    void generateGlobalKernelsHeader();
    inline string getName(SgFunctionRefExp *fn);
    void setProject(SgProject *p);
    void unparse();


    void forwardDeclareUtilFunctions(SgGlobal* globalScope, SgType* op_set, SgType* op_dat, SgType* op_ptr, SgType* op_access, SgType* op_plan);
    void preHandleConstAndGlobalData(SgFunctionCallExp *fn, op_par_loop *pl, SgBasicBlock *body);
    void postHandleConstAndGlobalData(SgFunctionCallExp *fn, op_par_loop *pl, SgBasicBlock *body);
		void preKernelGlobalDataHandling(SgFunctionCallExp *fn, op_par_loop *pl, SgBasicBlock *body);	
		void postKernelGlobalDataHandling(SgFunctionCallExp *fn, op_par_loop *pl, SgBasicBlock *body);


    map<string, SgFunctionDeclaration*> cudaFunctionDeclarations;
};

#endif

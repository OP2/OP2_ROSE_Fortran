
#ifndef ROSE_OP2ARGUMENT_H
#define ROSE_OP2ARGUMENT_H

#include <rose.h>

using namespace std;
using namespace SageBuilder;
using namespace SageInterface;

// Enums and classes for storing information about the arguments to op_par_loop_* internally
//enum op_dat_category { OP_DAT, OP_DAT_GBL, OP_DAT_CONST };
enum op_access   { OP_READ, OP_WRITE, OP_RW, OP_INC, OP_MIN, OP_MAX };

class op_argument
{
  public:
    static const int num_params;
  
    SgVarRefExp *arg;
    int index;
    SgExpression *ptr;
    int dim;
    SgType* type;
    op_access access;
		bool global;

		int plan_index;
		int own_index;
     
    bool usesIndirection();
    bool consideredAsReduction();
		bool isGlobal();
		bool isNotGlobal();
		bool consideredAsConst();

    
    // Constructor
    op_argument(SgExpressionPtrList::iterator &i);
  
  protected:
    SgVarRefExp* getSgVarRefExp(SgExpression* i);
    SgType* getSgTypeFromVarRef(SgVarRefExp* arg);
		int getDimFromVarRef(SgVarRefExp* arg, bool isGlobal);
};

#endif


#include <stdlib.h>
#include <stdio.h>

#include "op_datatypes.h"

//
// Extern Methods
//
extern void fixup_op_set(op_set* set);
extern void fixup_op_ptr(op_ptr* ptr);
extern void op_decl_const_i(const char* dat, int size, char const *name);

//
// OP Structure Methods
//

// Constructor
_op_set::_op_set()
{
  size = 0;
  name = "null";
  index = 0;
  partinfo = NULL;
}

// Constructor
_op_set::_op_set(unsigned int size, std::vector<int>* partinfo, char const *name){
  this->size = size;
  this->name = name;
  this->partinfo = partinfo;

	// Validate part info
	if(partinfo != NULL)
	{
		unsigned int total = 0;
		for(int i = 0; i < partinfo->size(); i++) {
			total += partinfo->at(i);
		}
		if(total != size) {
			printf("\n op_set:%s error: Invalid Partitioning Metadata", name);
			exit(1);
		}
	}
  
  fixup_op_set(this);
}

// Destructor
_op_set::~_op_set()
{

}

// IsNULL
bool _op_set::is_null()
{
  if(size == 0 && index == 0 && name == NULL)
    return true;
  return false;
}


//
// OP Ptr Structure Methods
//

// Constructor
_op_ptr::_op_ptr()
{
  this->from = _op_set();
  this->to = _op_set();
  this->dim = 0;
  this->ptr = NULL;
  this->name = "id";
  this->index = -1;
}

// Constructor op_ptr
_op_ptr::_op_ptr(op_set from, op_set to, int dim, int *ptr, char const *name)
{
  this->from = from;
  this->to   = to;
  this->dim  = dim;
  this->ptr  = ptr;
  this->name = name;
  
  fixup_op_ptr(this);
}

// Destructor
_op_ptr::~_op_ptr()
{

}

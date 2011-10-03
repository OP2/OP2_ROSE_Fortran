#include "op_lib.cu"
#import "op_datatypes.cpp"
__constant__
float gam;
__constant__
float gm1;
__constant__
float cfl;
__constant__
float eps;
__constant__
float mach;
__constant__
float alpha;
__constant__
myconst air_const;
#include "adt_calc_kernel.cu"
#include "res_calc_kernel.cu"
#include "save_soln_kernel.cu"
#include "update_kernel.cu"

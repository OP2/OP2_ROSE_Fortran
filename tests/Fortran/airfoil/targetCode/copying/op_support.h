
/* header for C++ functions called from C */

#ifndef __OP_SUPPORT_H
#define __OP_SUPPORT_H

op_plan * plan(char const * name, op_set set, int nargs, op_dat *args, int *idxs,
											op_map *maps, int *dims, char const **typs, op_access *accs, int ninds, int *inds);


#endif

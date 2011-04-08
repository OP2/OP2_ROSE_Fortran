/*
  Open source copyright declaration based on BSD open source template:
  http://www.opensource.org/licenses/bsd-license.php

* Copyright (c) 2011, Florian Rathgeber
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of Mike Giles may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mike Giles ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mike Giles BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#include <petscmat.h>
#include <petscvec.h>

#include "op_datatypes.h"

extern "C" {

void op_decl_sparsity ( op_sparsity * sparsity, op_map * rowmap, op_map * colmap )
{
  // FIXME add consistency checks
  if ( rowmap->from.index != colmap->from.index ) {
    printf("op_decl_sparsity: row map and col map do not map from the same set\n");
    exit(1);
  }
  
  const size_t nrows = rowmap->to.size;
  const size_t ncols = colmap->to.size;
  size_t max_nonzeros = 0;

  // Create and populate auxiliary data structure: for each element of the from
  // set, for each row pointed to by the row map, add all columns pointed to by
  // the col map
  std::vector< std::set< int > > s(nrows);
  for ( int e = 0; e < rowmap->from.size; ++e ) {
    for ( int i = 0; i < rowmap->dim; ++i ) {
      int row = rowmap->map[i + e*rowmap->dim];
      s[row].insert( colmap->map + e*colmap->dim, colmap->map + (e+1)*colmap->dim );
    }
  }

  // Create final sparsity structure
  int *nnz = (int*)malloc(nrows * sizeof(int));
  int *rowptr = (int*)malloc((nrows+1) * sizeof(int));
  rowptr[0] = 0;
  for ( size_t row = 0; row < nrows; ++row ) {
    nnz[row] = s[row].size();
    rowptr[row+1] = rowptr[row] + nnz[row];
    if ( max_nonzeros < s[row].size() ) max_nonzeros = s[row].size();
  }
  int *colidx = (int*)malloc(rowptr[nrows] * sizeof(int));
  for ( size_t row = 0; row < nrows; ++row ) {
    std::copy(s[row].begin(), s[row].end(), colidx + rowptr[row]);
  }
  
  sparsity->nrows = nrows;
  sparsity->ncols = ncols;
  sparsity->nnz = nnz;
  sparsity->rowptr = rowptr;
  sparsity->colidx = colidx;
  sparsity->max_nonzeros = max_nonzeros;
}

void op_decl_mat( op_dat * mat, op_sparsity * sparsity, char const * name ) {
  Mat p_mat;
  // Create a PETSc CSR sparse matrix and pre-allocate storage
  MatCreateSeqAIJ(PETSC_COMM_SELF,
      sparsity->nrows,
      sparsity->ncols,
      sparsity->max_nonzeros,
      (const PetscInt*)sparsity->nnz,
      &p_mat);
  // Set the column indices (FIXME: benchmark is this is worth it)
  MatSeqAIJSetColumnIndices(p_mat, (PetscInt*)sparsity->colidx);

  initialise_dat(mat, NULL, 1, sizeof(PetscScalar), p_mat, name);
}

void op_mat_addto( op_dat * mat, const void* values, int nrows, const int *irows, int ncols, const int *icols ) {
  for (int i = 0; i < nrows; ++i) {
    for (int j = 0; j < nrows; ++j) {
      printf("(%d,%d)->(%d,%d): %f\n", i, j, ((const PetscInt *)irows)[i], ((const PetscInt *)icols)[j], ((const PetscScalar*)values)[i*ncols+j]);
    }
  }
  MatSetValues((Mat) mat->dat, nrows, (const PetscInt *)irows, ncols, (const PetscInt *)icols, (const PetscScalar *)values, ADD_VALUES);
}

void op_mat_assemble( op_dat * mat ) {
  MatAssemblyBegin((Mat) mat->dat,MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd((Mat) mat->dat,MAT_FINAL_ASSEMBLY);
  MatView((Mat) mat->dat,PETSC_VIEWER_STDOUT_WORLD);
}

void op_create_vec ( op_dat * vec ) {
  Vec p_vec;
  // Create a PETSc vector and pass it the user-allocated storage
  VecCreateSeqWithArray(MPI_COMM_SELF,vec->dim * vec->set.size,(PetscScalar*)vec->dat,&p_vec);
  VecAssemblyBegin(p_vec);
  VecAssemblyEnd(p_vec);

  vec->dat = p_vec;
}

void op_mat_mult ( const op_dat * mat, const op_dat * v_in, op_dat * v_out ) {
  MatMult((Mat) mat->dat,(Vec) v_in->dat,(Vec) v_out->dat);
  VecView((Vec) v_out->dat,PETSC_VIEWER_STDOUT_WORLD);
}

} /* extern "C" */


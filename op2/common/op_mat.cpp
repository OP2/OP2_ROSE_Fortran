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

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <vector>

#include <petscksp.h>

#include "op_datatypes.h"

extern "C" {

void op_decl_sparsity ( op_sparsity * sparsity, op_map * rowmap, op_map * colmap )
{
  assert(sparsity && rowmap && colmap);

  // FIXME add consistency checks
  if ( rowmap->from->index != colmap->from->index ) {
    printf("op_decl_sparsity: row map and col map do not map from the same set\n");
    exit(1);
  }
  
  const size_t nrows = rowmap->to->size;
  const size_t ncols = colmap->to->size;
  size_t max_nonzeros = 0;

  // Create and populate auxiliary data structure: for each element of the from
  // set, for each row pointed to by the row map, add all columns pointed to by
  // the col map
  std::vector< std::set< int > > s(nrows);
  for ( int e = 0; e < rowmap->from->size; ++e ) {
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

void op_decl_mat( op_dat * data, op_set * rowset, op_set * colset, int dim, int type_size, op_sparsity * sparsity, char const * name ) {
  assert( data && rowset && colset && sparsity );
  assert( rowset->size == sparsity->nrows && colset->size == sparsity->ncols );

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

  initialise_dat( data,
                  2,
                  rowset,
                  colset,
                  dim,
                  type_size,
                  p_mat,
                  name );
}

void op_mat_addto( op_dat * mat, const void* values, int nrows, const int *irows, int ncols, const int *icols ) {
  assert( mat && values && irows && icols );
  assert( mat->rank == 2 );

  //for (int i = 0; i < nrows; ++i) {
    //for (int j = 0; j < nrows; ++j) {
      //printf("(%d,%d)->(%d,%d): %f\n", i, j, ((const PetscInt *)irows)[i], ((const PetscInt *)icols)[j], ((const PetscScalar*)values)[i*ncols+j]);
    //}
  //}
  MatSetValues((Mat) mat->dat, nrows, (const PetscInt *)irows, ncols, (const PetscInt *)icols, (const PetscScalar *)values, ADD_VALUES);
}

void op_mat_assemble( op_dat * mat ) {
  assert( mat && mat->rank == 2 );

  MatAssemblyBegin((Mat) mat->dat,MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd((Mat) mat->dat,MAT_FINAL_ASSEMBLY);
  //MatView((Mat) mat->dat,PETSC_VIEWER_STDOUT_WORLD);
}

static Vec op_create_vec ( const op_dat * vec ) {
  assert( vec && vec->rank == 1 );

  Vec p_vec;
  // Create a PETSc vector and pass it the user-allocated storage
  VecCreateSeqWithArray(MPI_COMM_SELF,vec->dim * vec->set[0]->size,(PetscScalar*)vec->dat,&p_vec);
  VecAssemblyBegin(p_vec);
  VecAssemblyEnd(p_vec);

  return p_vec;
}

void op_mat_mult ( const op_dat * mat, const op_dat * v_in, op_dat * v_out ) {
  assert( mat && v_in && v_out );
  assert( mat->rank == 2 && v_in->rank == 1 && v_out->rank == 1 );

  Vec p_v_in = op_create_vec(v_in);
  Vec p_v_out = op_create_vec(v_out);

  MatMult((Mat) mat->dat, p_v_in, p_v_out);

  //VecView(p_v_out, PETSC_VIEWER_STDOUT_WORLD);
  
  VecDestroy(p_v_in);
  VecDestroy(p_v_out);
}

void op_solve ( const op_dat * mat, const op_dat * b, op_dat * x ) {
  assert( mat && b && x );
  assert( mat->rank == 2 && b->rank == 1 && x->rank == 1 );
  printf("Solving system A*x=b of size %dx%d\n", mat->set[0]->size, mat->set[1]->size);

  Vec p_b = op_create_vec(b);
  Vec p_x = op_create_vec(x);
  Mat A = (Mat) mat->dat;
  KSP ksp;
  PC pc;
  int its;

  KSPCreate(PETSC_COMM_WORLD,&ksp);
  KSPSetOperators(ksp,A,A,DIFFERENT_NONZERO_PATTERN);
  KSPGetPC(ksp,&pc);
  PCSetType(pc,PCJACOBI);
  KSPSetTolerances(ksp,1.e-7,PETSC_DEFAULT,PETSC_DEFAULT,PETSC_DEFAULT);

  KSPSolve(ksp,p_b,p_x);
  KSPGetIterationNumber(ksp,&its);
  printf("Converged in %d iterations\n", its);

  //KSPView(ksp,PETSC_VIEWER_STDOUT_WORLD);
  MatView((Mat) mat->dat,PETSC_VIEWER_STDOUT_WORLD);
  //VecView(p_x, PETSC_VIEWER_STDOUT_WORLD);
  //VecView(p_b, PETSC_VIEWER_STDOUT_WORLD);

  VecDestroy(p_b);
  VecDestroy(p_x);
  KSPDestroy(ksp);
}

} /* extern "C" */


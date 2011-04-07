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
  std::vector< std::set< size_t > > s(nrows);
  for ( int e = 0; e < rowmap->from.size; ++e ) {
    for ( int i = 0; i < rowmap->dim; ++i ) {
      int row = rowmap->map[i + e*rowmap->dim];
      s[row].insert( colmap->map + e*colmap->dim, colmap->map + (e+1)*colmap->dim );
    }
  }
  std::ostream_iterator<size_t> ositer(std::cout, " ");
  for ( size_t row = 0; row < nrows; ++row ) {
    std::copy(s[row].begin(), s[row].end(), ositer);
    std::cout << s[row].size() << std::endl;
  }

  // Create final sparsity structure
  size_t *nnz = (size_t*)malloc(nrows * sizeof(size_t));
  size_t *rowptr = (size_t*)malloc((nrows+1) * sizeof(size_t));
  rowptr[0] = 0;
  for ( size_t row = 0; row < nrows; ++row ) {
    nnz[row] = s[row].size();
    rowptr[row+1] = rowptr[row] + nnz[row];
    if ( max_nonzeros < s[row].size() ) max_nonzeros = s[row].size();
  }
  size_t *colidx = (size_t*)malloc(rowptr[nrows] * sizeof(size_t));
  for ( size_t row = 0; row < nrows; ++row ) {
    std::copy(s[row].begin(), s[row].end(), colidx + rowptr[row]);
  }
  
  sparsity->nrows = nrows;
  sparsity->ncols = ncols;
  sparsity->nnz = nnz;
  sparsity->rowptr = rowptr;
  sparsity->colidx = colidx;
  sparsity->max_nonzeros = max_nonzeros;
  std::copy(sparsity->nnz, sparsity->nnz+nrows, ositer);
  std::cout << std::endl;
  std::copy(sparsity->rowptr, sparsity->rowptr+nrows+1, ositer);
  std::cout << std::endl;
  std::copy(sparsity->colidx, sparsity->colidx + sparsity->rowptr[nrows], ositer);
  std::cout << std::endl;
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

  initialise_dat(mat, NULL, 1, sizeof(float), p_mat, name);
}

void op_mat_addto( op_dat * mat, const void* values, int nrows, const int *irows, int ncols, const int *icols ) {
  MatSetValues((Mat) mat->dat, nrows, (const PetscInt *)irows, ncols, (const PetscInt *)icols, (const PetscScalar *)values, ADD_VALUES);
}

} /* extern "C" */


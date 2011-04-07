// test program demonstrating assembly of op_sparse_matrix for FE
// discretisation of a 1D Laplace operator and spmv

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// OP header file

#include <op2.h>

// kernel routines for parallel loops

#include "laplace.h"

// define problem size

#define NN       2
#define NITER    2

// main program

int main(int argc, char **argv) {

  int   nnode = (NN+1);

  int    *p_elem_node = (int *)malloc(2*sizeof(int)*NN);
  float  *p_xn = (float *)malloc(sizeof(float)*nnode);
  float  *p_x  = (float *)malloc(sizeof(float)*nnode);
  float  *p_y  = (float *)malloc(sizeof(float)*nnode);

  // create element -> node mapping
  for (int i = 0; i < NN; ++i) {
    p_elem_node[2*i] = i;
    p_elem_node[2*i+1] = i+1;
  }

  // create coordinates and populate x with -1/pi^2*sin(pi*x)
  for (int i = 0; i < nnode; ++i) {
    p_xn[i] = sin(0.5*M_PI*i/NN);
    p_x[i] = -1./(M_PI*M_PI)*sin(M_PI*p_xn[i]);
  }

  // OP initialisation

  op_init(argc,argv,2);

  // declare sets, pointers, and datasets

  op_set nodes, elements;
  op_map elem_node;
  op_dat x, y, xn, mat;
  op_sparsity mat_sparsity;

  op_decl_set(&nodes, nnode, "nodes");
  op_decl_set(&elements, NN, "elements");

  op_decl_map(&elem_node, &elements, &nodes, 2, p_elem_node, "elem_node");

  dump_map(&elem_node, "map");

  op_decl_dat(&x, &nodes, 1, sizeof(float), p_x, "x");
  op_decl_dat(&y, &nodes, 1, sizeof(float), p_y, "y");
  op_decl_dat(&xn, &nodes, 1, sizeof(float), p_xn, "xn");

  op_decl_sparsity(&mat_sparsity, &elem_node, &elem_node);

  dump_sparsity(&mat_sparsity, "sparsity");

  op_decl_mat(&mat, &mat_sparsity, "matrix");

  // construct the matrix
  op_par_loop_2((void(*)(void*,void*))laplace, "laplace", &elements,
                op_construct_mat_arg(&mat, OP_ALL, &elem_node, OP_ALL, &elem_node, OP_INC),
                op_construct_vec_arg(&xn, 0, &elem_node, OP_READ));

  // spmv
  /*op_mat_mult(mat, x, y);*/

  op_exit();
}


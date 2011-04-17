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

typedef double Real;

// kernel routines for parallel loops

#include "laplace.h"

// define problem size

#define NN       6
#define NITER    2

// main program

int main(int argc, char **argv) {

  int   nnode = (NN+1);

  int   *p_elem_node = (int *)malloc(2*sizeof(int)*NN);
  Real  *p_xn = (Real *)malloc(sizeof(Real)*nnode);
  Real  *p_x  = (Real *)malloc(sizeof(Real)*nnode);
  Real  *p_xref = (Real *)malloc(sizeof(Real)*nnode);
  Real  *p_y  = (Real *)malloc(sizeof(Real)*nnode);

  // create element -> node mapping
  for (int i = 0; i < NN; ++i) {
    p_elem_node[2*i] = i;
    p_elem_node[2*i+1] = i+1;
  }

  // create coordinates and populate x with -1/pi^2*sin(pi*x)
  for (int i = 0; i < nnode; ++i) {
    /*p_xn[i] = sin(0.5*M_PI*i/NN);*/
    p_xn[i] = (Real)i/NN;
    p_x[i] = (1./(M_PI*M_PI))*sin(M_PI*p_xn[i]);
    p_xref[i] = sin(M_PI*p_xn[i]);
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

  /*dump_map(&elem_node, "map");*/

  op_decl_vec(&x, &nodes, 1, sizeof(Real), p_x, "x");
  op_decl_vec(&y, &nodes, 1, sizeof(Real), p_y, "y");
  op_decl_vec(&xn, &nodes, 1, sizeof(Real), p_xn, "xn");

  op_decl_sparsity(&mat_sparsity, &elem_node, &elem_node);

  /*dump_sparsity(&mat_sparsity, "sparsity");*/

  op_decl_mat(&mat, &nodes, &nodes, 1, sizeof(Real), &mat_sparsity, "matrix");

  /*dump_dat(&mat, "matrix");*/

  op_diagnostic_output();
  
  // Fix the values of the boundary nodes to get a unique solution
  Real val = 1e308;
  int idx = 0;
  op_mat_addto(&mat, &val, 1, &idx, 1, &idx);
  idx = NN;
  op_mat_addto(&mat, &val, 1, &idx, 1, &idx);

  // construct the matrix
  op_par_loop_2((void(*)(void*,void*))laplace, "laplace", &elements,
                op_construct_mat_arg(&mat, OP_ALL, &elem_node, OP_ALL, &elem_node, OP_INC),
                op_construct_vec_arg(&xn, 0, &elem_node, OP_READ));

  // spmv
  /*op_mat_mult(&mat, &x, &y);*/

  // solve
  op_solve(&mat, &x, &y);

  for (int i = 0; i < nnode; ++i) {
    printf("%f\n", p_x[i]);
  }

  op_exit();
}


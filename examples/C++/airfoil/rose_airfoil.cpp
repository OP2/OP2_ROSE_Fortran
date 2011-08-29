/*
  Open source copyright declaration based on BSD open source template:
  http://www.opensource.org/licenses/bsd-license.php
* Copyright (c) 2009-2011, Mike Giles
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
//
//     Nonlinear airfoil lift calculation
//
//     Written by Mike Giles, 2010-2011, based on FORTRAN code
//     by Devendra Ghate and Mike Giles, 2005
//
//
// standard headers
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
// global constants
float gam;
float gm1;
float cfl;
float eps;
float mach;
float alpha;
float qinf[4UL];
//
// OP header file
//
#include "op2.h"
//
// kernel routines for parallel loops
//
#include "save_soln.h"
#include "adt_calc.h"
#include "res_calc.h"
#include "bres_calc.h"
#include "update.h"
// main program

int main(int argc,char **argv)
{
  int *becell;
  int *ecell;
  int *bound;
  int *bedge;
  int *edge;
  int *cell;
  float *x;
  float *q;
  float *qold;
  float *adt;
  float *res;
  int nnode;
  int ncell;
  int nedge;
  int nbedge;
  int niter;
  float rms;
// read in grid
  printf("reading in grid \n");
  FILE *fp;
  if ((fp = fopen("new_grid.dat","r")) == ((0L))) {
    printf("can\'t open file new_grid.dat\n");
    exit((-1));
  }
  if (fscanf(fp,"%d %d %d %d \n",&nnode,&ncell,&nedge,&nbedge) != 4) {
    printf("error reading from new_grid.dat\n");
    exit((-1));
  }
  cell = ((int *)(malloc(((((4 * ncell)) * (sizeof(int )))))));
  edge = ((int *)(malloc(((((2 * nedge)) * (sizeof(int )))))));
  ecell = ((int *)(malloc(((((2 * nedge)) * (sizeof(int )))))));
  bedge = ((int *)(malloc(((((2 * nbedge)) * (sizeof(int )))))));
  becell = ((int *)(malloc((((nbedge) * (sizeof(int )))))));
  bound = ((int *)(malloc((((nbedge) * (sizeof(int )))))));
  x = ((float *)(malloc(((((2 * nnode)) * (sizeof(float )))))));
  q = ((float *)(malloc(((((4 * ncell)) * (sizeof(float )))))));
  qold = ((float *)(malloc(((((4 * ncell)) * (sizeof(float )))))));
  res = ((float *)(malloc(((((4 * ncell)) * (sizeof(float )))))));
  adt = ((float *)(malloc((((ncell) * (sizeof(float )))))));
  for (int n = 0; n < nnode; n++) {
    if (fscanf(fp,"%f %f \n",(x + (2 * n)),(x + ((2 * n) + 1))) != 2) {
      printf("error reading from new_grid.dat\n");
      exit((-1));
    }
  }
  for (int n = 0; n < ncell; n++) {
    if (fscanf(fp,"%d %d %d %d \n",(cell + (4 * n)),(cell + ((4 * n) + 1)),(cell + ((4 * n) + 2)),(cell + ((4 * n) + 3))) != 4) {
      printf("error reading from new_grid.dat\n");
      exit((-1));
    }
  }
  for (int n = 0; n < nedge; n++) {
    if (fscanf(fp,"%d %d %d %d \n",(edge + (2 * n)),(edge + ((2 * n) + 1)),(ecell + (2 * n)),(ecell + ((2 * n) + 1))) != 4) {
      printf("error reading from new_grid.dat\n");
      exit((-1));
    }
  }
  for (int n = 0; n < nbedge; n++) {
    if (fscanf(fp,"%d %d %d %d \n",(bedge + (2 * n)),(bedge + ((2 * n) + 1)),(becell + n),(bound + n)) != 4) {
      printf("error reading from new_grid.dat\n");
      exit((-1));
    }
  }
  fclose(fp);
// set constants and initialise flow field and residual
  printf("initialising flow field \n");
  gam = 1.4f;
  gm1 = (gam - 1.0f);
  cfl = 0.9f;
  eps = 0.05f;
  float mach = 0.4f;
  float alpha = (((3.0f) * atan((1.0f))) / (45.0f));
  float p = 1.0f;
  float r = 1.0f;
  float u = (sqrt((((gam * p) / r))) * (mach));
  float e = ((p / (r * gm1)) + ((0.5f * u) * u));
  qinf[0] = r;
  qinf[1] = (r * u);
  qinf[2] = 0.0f;
  qinf[3] = (r * e);
  for (int n = 0; n < ncell; n++) {
    for (int m = 0; m < 4; m++) {
      q[(4 * n) + m] = (qinf[m]);
      res[(4 * n) + m] = 0.0f;
    }
  }
// OP initialisation
  op_init(argc,argv,2);
// declare sets, pointers, datasets and global constants
  op_set nodes = op_decl_set(nnode);
  op_set edges = op_decl_set(nedge);
  op_set bedges = op_decl_set(nbedge);
  op_set cells = op_decl_set(ncell);
  op_map pedge = op_decl_map(edges,nodes,2,edge);
  op_map pecell = op_decl_map(edges,cells,2,ecell);
  op_map pbedge = op_decl_map(bedges,nodes,2,bedge);
  op_map pbecell = op_decl_map(bedges,cells,1,becell);
  op_map pcell = op_decl_map(cells,nodes,4,cell);
  op_dat p_bound = op_decl_dat(bedges,1,bound);
  op_dat p_x = op_decl_dat(nodes,2,x);
  op_dat p_q = op_decl_dat(cells,4,q);
  op_dat p_qold = op_decl_dat(cells,4,qold);
  op_dat p_adt = op_decl_dat(cells,1,adt);
  op_dat p_res = op_decl_dat(cells,4,res);
  op_decl_const(1,&gam);
  op_decl_const(1,&gm1);
  op_decl_const(1,&cfl);
  op_decl_const(1,&eps);
  op_decl_const(1,&mach);
  op_decl_const(1,&alpha);
  op_decl_const(4,qinf);
  op_diagnostic_output();
// main time-marching loop
  niter = 1000;
  for (int iter = 1; iter <= niter; iter++) {
//  save old flow solution
    op_par_loop(save_soln,cells,op_arg_dat(p_q,(-1), *((&OP_ID)),OP_READ),op_arg_dat(p_qold,(-1), *((&OP_ID)),OP_WRITE));
//  predictor/corrector update loop
    for (int k = 0; k < 2; k++) {
//    calculate area/timstep
      op_par_loop(adt_calc,cells,op_arg_dat(p_x,0,pcell,OP_READ),op_arg_dat(p_x,1,pcell,OP_READ),op_arg_dat(p_x,2,pcell,OP_READ),op_arg_dat(p_x,3,pcell,OP_READ),op_arg_dat(p_q,(-1), *((&OP_ID)),OP_READ),op_arg_dat(p_adt,(-1), *((&OP_ID)),OP_WRITE));
//    calculate flux residual
      op_par_loop(res_calc,edges,op_arg_dat(p_x,0,pedge,OP_READ),op_arg_dat(p_x,1,pedge,OP_READ),op_arg_dat(p_q,0,pecell,OP_READ),op_arg_dat(p_q,1,pecell,OP_READ),op_arg_dat(p_adt,0,pecell,OP_READ),op_arg_dat(p_adt,1,pecell,OP_READ),op_arg_dat(p_res,0,pecell,OP_INC),op_arg_dat(p_res,1,pecell,OP_INC));
      op_par_loop(bres_calc,bedges,op_arg_dat(p_x,0,pbedge,OP_READ),op_arg_dat(p_x,1,pbedge,OP_READ),op_arg_dat(p_q,0,pbecell,OP_READ),op_arg_dat(p_adt,0,pbecell,OP_READ),op_arg_dat(p_res,0,pbecell,OP_INC),op_arg_dat(p_bound,(-1), *((&OP_ID)),OP_READ));
//    update flow field
      rms = (0.0);
      op_par_loop(update,cells,op_arg_dat(p_qold,(-1), *((&OP_ID)),OP_READ),op_arg_dat(p_q,(-1), *((&OP_ID)),OP_WRITE),op_arg_dat(p_res,(-1), *((&OP_ID)),OP_RW),op_arg_dat(p_adt,(-1), *((&OP_ID)),OP_READ),op_arg_gbl(&rms,OP_INC));
    }
//  print iteration history
    rms = ((sqrt(((rms / ((float )ncell))))));
    if ((iter % 100) == 0) 
      printf(" %d  %10.5e \n",iter,(rms));
  }
  op_timing_output();
  return 0;
}
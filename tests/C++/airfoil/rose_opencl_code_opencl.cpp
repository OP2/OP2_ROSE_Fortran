#define ROUND_UP(bytes) (((bytes) + 15) & ~15)
#define MIN(a,b) ((a<b) ? (a) : (b))
#define ZERO_float 0.0f
#include <CL/cl.h>
#include "op_opencl_rt_support.h"
extern float alpha;
extern float cfl;
extern float eps;
extern float gam;
extern float gm1;
extern float mach;
extern float qinf[4UL];
int threadsPerBlockSize_bres_calc = 512;
int setPartitionSize_bres_calc = 512;
int threadsPerBlockSize_fusedOne = 512;
int setPartitionSize_fusedOne = 512;
int threadsPerBlockSize_fusedTwo = 512;
int setPartitionSize_fusedTwo = 512;
int threadsPerBlockSize_res_calc = 512;
int setPartitionSize_res_calc = 512;
#ifdef OP_WARPSIZE_0
#define OP_WARPSIZE OP_WARPSIZE_0
#endif
int threadsPerBlockSize_update = 512;

__kernel void ReductionFloat4(__global float *volatile reductionResult,__private float inputValue,__private int reductionOperation,__local float *sharedFloat4)
{
  __local float *volatile volatileSharedFloat4;
  int i1;
  int threadID;
  threadID = get_local_id(0);
  i1 = get_local_size(0) >> 1;
  barrier(CLK_LOCAL_MEM_FENCE);
  sharedFloat4[threadID] = inputValue;
  for (; i1 > OP_WARPSIZE; i1 >>= 1) {
    barrier(CLK_LOCAL_MEM_FENCE);
    if (threadID < i1) {
      switch(reductionOperation){
        case 0:
{
          sharedFloat4[threadID] = sharedFloat4[threadID] + sharedFloat4[threadID + i1];
          break; 
        }
        case 1:
{
          if (sharedFloat4[threadID + i1] < sharedFloat4[threadID]) {
            sharedFloat4[threadID] = sharedFloat4[threadID + i1];
          }
          break; 
        }
        case 2:
{
          if (sharedFloat4[threadID + i1] > sharedFloat4[threadID]) {
            sharedFloat4[threadID] = sharedFloat4[threadID + i1];
          }
          break; 
        }
      }
    }
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  volatileSharedFloat4 = sharedFloat4;
  if (threadID < OP_WARPSIZE) {
    for (; i1 > 0; i1 >>= 1) {
      if (threadID < i1) {
        switch(reductionOperation){
          case 0:
{
            volatileSharedFloat4[threadID] = volatileSharedFloat4[threadID] + volatileSharedFloat4[threadID + i1];
            break; 
          }
          case 1:
{
            if (volatileSharedFloat4[threadID + i1] < volatileSharedFloat4[threadID]) {
              volatileSharedFloat4[threadID] = volatileSharedFloat4[threadID + i1];
            }
            break; 
          }
          case 2:
{
            if (volatileSharedFloat4[threadID + i1] > volatileSharedFloat4[threadID]) {
              volatileSharedFloat4[threadID] = volatileSharedFloat4[threadID + i1];
            }
            break; 
          }
        }
      }
    }
  }
  if (threadID == 0) {
    switch(reductionOperation){
      case 0:
{
         *reductionResult =  *reductionResult + volatileSharedFloat4[0];
        break; 
      }
      case 1:
{
        if (sharedFloat4[0] <  *reductionResult) {
           *reductionResult = volatileSharedFloat4[0];
        }
        break; 
      }
      case 2:
{
        if (sharedFloat4[0] >  *reductionResult) {
           *reductionResult = volatileSharedFloat4[0];
        }
        break; 
      }
    }
  }
}

inline void bres_calc_modified(__local float *x1,__local float *x2,__local float *q1,__local float *adt1,float *res1,__global int *bound,__constant float *gm1,__constant float  *qinf,__constant float *eps)
{
  typedef int float ;
  float dx;
  float dy;
  float mu;
  float ri;
  float p1;
  float vol1;
  float p2;
  float vol2;
  float f;
  dx = (x1[0] - x2[0]);
  dy = (x1[1] - x2[1]);
  ri = (1.0f / q1[0]);
  p1 = ( *gm1 * (q1[3] - ((0.5f * ri) * ((q1[1] * q1[1]) + (q1[2] * q1[2])))));
  if ( *bound == 1) {
    res1[1] += (+p1 * dy);
    res1[2] += (-p1 * dx);
  }
  else {
    vol1 = (ri * ((q1[1] * dy) - (q1[2] * dx)));
    ri = (1.0f / qinf[0]);
    p2 = ( *gm1 * (qinf[3] - ((0.5f * ri) * ((qinf[1] * qinf[1]) + (qinf[2] * qinf[2])))));
    vol2 = (ri * ((qinf[1] * dy) - (qinf[2] * dx)));
    mu = ( *adt1 *  *eps);
    f = ((0.5f * ((vol1 * q1[0]) + (vol2 * qinf[0]))) + (mu * (q1[0] - qinf[0])));
    res1[0] += f;
    f = ((0.5f * ((((vol1 * q1[1]) + (p1 * dy)) + (vol2 * qinf[1])) + (p2 * dy))) + (mu * (q1[1] - qinf[1])));
    res1[1] += f;
    f = ((0.5f * ((((vol1 * q1[2]) - (p1 * dx)) + (vol2 * qinf[2])) - (p2 * dx))) + (mu * (q1[2] - qinf[2])));
    res1[2] += f;
    f = ((0.5f * ((vol1 * (q1[3] + p1)) + (vol2 * (qinf[3] + p2)))) + (mu * (q1[3] - qinf[3])));
    res1[3] += f;
  }
}

__kernel void bres_calc_kernel(__global float *opDat1,__global float *opDat3,__global float *opDat4,__global float *opDat5,__global int *opDat6,__global int *ind_maps1,__global int *ind_maps3,__global int *ind_maps4,__global int *ind_maps5,__global short *mappingArray1,__global short *mappingArray2,__global short *mappingArray3,__global short *mappingArray4,__global short *mappingArray5,__global int *pindSizes,__global int *pindOffs,__global int *pblkMap,__global int *poffset,__global int *pnelems,__global int *pnthrcol,__global int *pthrcol,__private int blockOffset,__local float *shared_bres_calc,__constant float *gm1,__constant float  *qinf,__constant float *eps)
{
  float opDat5Local[4];
  __local int sharedMemoryOffset;
  __local int numberOfActiveThreads;
  int nbytes;
  int blockID;
  int i1;
  __global  int* __local  opDat1IndirectionMap;
  __global  int* __local  opDat3IndirectionMap;
  __global  int* __local  opDat4IndirectionMap;
  __global  int* __local  opDat5IndirectionMap;
  __local int opDat1SharedIndirectionSize;
  __local int opDat3SharedIndirectionSize;
  __local int opDat4SharedIndirectionSize;
  __local int opDat5SharedIndirectionSize;
  __local  float * __local  opDat1SharedIndirection;
  __local  float * __local  opDat3SharedIndirection;
  __local  float * __local  opDat4SharedIndirection;
  __local  float * __local  opDat5SharedIndirection;
  __local int numOfColours;
  __local int numberOfActiveThreadsCeiling;
  int colour1;
  int colour2;
  int i2;
  if (get_local_id(0) == 0) {
    blockID = pblkMap[get_group_id(0) + blockOffset];
    numberOfActiveThreads = pnelems[blockID];
    numberOfActiveThreadsCeiling = get_local_size(0) * (1 + (numberOfActiveThreads - 1) / get_local_size(0));
    numOfColours = pnthrcol[blockID];
    sharedMemoryOffset = poffset[blockID];
    opDat1SharedIndirectionSize = pindSizes[0 + blockID * 4];
    opDat3SharedIndirectionSize = pindSizes[1 + blockID * 4];
    opDat4SharedIndirectionSize = pindSizes[2 + blockID * 4];
    opDat5SharedIndirectionSize = pindSizes[3 + blockID * 4];
    opDat1IndirectionMap = ind_maps1 + pindOffs[0 + blockID * 4];
    opDat3IndirectionMap = ind_maps3 + pindOffs[1 + blockID * 4];
    opDat4IndirectionMap = ind_maps4 + pindOffs[2 + blockID * 4];
    opDat5IndirectionMap = ind_maps5 + pindOffs[3 + blockID * 4];
    nbytes = 0;
    opDat1SharedIndirection = &shared_bres_calc[nbytes / sizeof(float )];
    nbytes += ROUND_UP(opDat1SharedIndirectionSize * (sizeof(float ) * 2));
    opDat3SharedIndirection = &shared_bres_calc[nbytes / sizeof(float )];
    nbytes += ROUND_UP(opDat3SharedIndirectionSize * (sizeof(float ) * 4));
    opDat4SharedIndirection = &shared_bres_calc[nbytes / sizeof(float )];
    nbytes += ROUND_UP(opDat4SharedIndirectionSize * (sizeof(float ) * 1));
    opDat5SharedIndirection = &shared_bres_calc[nbytes / sizeof(float )];
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (i1 = get_local_id(0); i1 < opDat1SharedIndirectionSize * 2; i1 += get_local_size(0)) {
    opDat1SharedIndirection[i1] = opDat1[i1 % 2 + opDat1IndirectionMap[i1 / 2] * 2];
  }
  for (i1 = get_local_id(0); i1 < opDat3SharedIndirectionSize * 4; i1 += get_local_size(0)) {
    opDat3SharedIndirection[i1] = opDat3[i1 % 4 + opDat3IndirectionMap[i1 / 4] * 4];
  }
  for (i1 = get_local_id(0); i1 < opDat4SharedIndirectionSize * 1; i1 += get_local_size(0)) {
    opDat4SharedIndirection[i1] = opDat4[i1 % 1 + opDat4IndirectionMap[i1 / 1] * 1];
  }
  for (i1 = get_local_id(0); i1 < opDat5SharedIndirectionSize * 4; i1 += get_local_size(0)) {
    opDat5SharedIndirection[i1] = 0.00000F;
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (i1 = get_local_id(0); i1 < numberOfActiveThreadsCeiling; i1 += get_local_size(0)) {
    colour2 = -1;
    if (i1 < numberOfActiveThreads) {
      for (i2 = 0; i2 < 4; ++i2) {
        opDat5Local[i2] = 0.00000F;
      }
      bres_calc_modified(opDat1SharedIndirection + mappingArray1[i1 + sharedMemoryOffset] * 2,opDat1SharedIndirection + mappingArray2[i1 + sharedMemoryOffset] * 2,opDat3SharedIndirection + mappingArray3[i1 + sharedMemoryOffset] * 4,opDat4SharedIndirection + mappingArray4[i1 + sharedMemoryOffset] * 1,opDat5Local,opDat6 + (i1 + sharedMemoryOffset) * 1,gm1,qinf,eps);
      colour2 = pthrcol[i1 + sharedMemoryOffset];
    }
    for (colour1 = 0; colour1 < numOfColours; ++colour1) {
      if (colour2 == colour1) {
        for (i2 = 0; i2 < 4; ++i2) {
          opDat5SharedIndirection[i2 + mappingArray5[i1 + sharedMemoryOffset] * 4] += opDat5Local[i2];
        }
      }
      barrier(CLK_LOCAL_MEM_FENCE);
    }
  }
  for (i1 = get_local_id(0); i1 < opDat5SharedIndirectionSize * 4; i1 += get_local_size(0)) {
    opDat5[i1 % 4 + opDat5IndirectionMap[i1 / 4] * 4] += opDat5SharedIndirection[i1];
  }
}

void bres_calc_host(const char *userSubroutine,op_set set,op_arg opDat1,op_arg opDat2,op_arg opDat3,op_arg opDat4,op_arg opDat5,op_arg opDat6)
{
#ifdef OP_BLOCK_SIZE_0
  threadsPerBlockSize_bres_calc = OP_BLOCK_SIZE_0;
#endif
#ifdef OP_PART_SIZE_0
  setPartitionSize_bres_calc = OP_PART_SIZE_0;
#endif
  size_t blocksPerGrid;
  size_t threadsPerBlock;
  size_t totalThreadNumber;
  size_t dynamicSharedMemorySize;
  cl_int errorCode;
  cl_event event;
  cl_kernel kernelPointer;
  int i3;
  op_arg opDatArray[6];
  int indirectionDescriptorArray[6];
  op_plan *planRet;
  int blockOffset;
  opDatArray[0] = opDat1;
  opDatArray[1] = opDat2;
  opDatArray[2] = opDat3;
  opDatArray[3] = opDat4;
  opDatArray[4] = opDat5;
  opDatArray[5] = opDat6;
  indirectionDescriptorArray[0] = 0;
  indirectionDescriptorArray[1] = 0;
  indirectionDescriptorArray[2] = 1;
  indirectionDescriptorArray[3] = 2;
  indirectionDescriptorArray[4] = 3;
  indirectionDescriptorArray[5] = -1;
  planRet = op_plan_get(userSubroutine,set,setPartitionSize_bres_calc,6,opDatArray,4,indirectionDescriptorArray);
  cl_mem gm1_d;
  gm1_d = op_allocate_constant(&gm1,sizeof(float ));
  cl_mem qinf_d;
  qinf_d = op_allocate_constant(&qinf,4 * sizeof(float));
  cl_mem eps_d;
  eps_d = op_allocate_constant(&eps,sizeof(float ));
  blockOffset = 0;
  double cpu_t1;
  double cpu_t2;
  double wall_t1;
op_timers(&cpu_t1, &wall_t1);
  double wall_t2;
  for (i3 = 0; i3 < planRet -> ncolors; ++i3) {
    blocksPerGrid = planRet -> ncolblk[i3];
    dynamicSharedMemorySize = planRet -> nshared;
    threadsPerBlock = threadsPerBlockSize_bres_calc;
    totalThreadNumber = threadsPerBlock * blocksPerGrid;
    kernelPointer = getKernel("bres_calc_kernel");
    errorCode = clSetKernelArg(kernelPointer,0,sizeof(cl_mem ),&opDat1.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,1,sizeof(cl_mem ),&opDat3.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,2,sizeof(cl_mem ),&opDat4.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,3,sizeof(cl_mem ),&opDat5.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,4,sizeof(cl_mem ),&opDat6.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,5,sizeof(cl_mem ),&planRet -> ind_maps[0]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,6,sizeof(cl_mem ),&planRet -> ind_maps[1]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,7,sizeof(cl_mem ),&planRet -> ind_maps[2]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,8,sizeof(cl_mem ),&planRet -> ind_maps[3]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,9,sizeof(cl_mem ),&planRet -> loc_maps[0]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,10,sizeof(cl_mem ),&planRet -> loc_maps[1]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,11,sizeof(cl_mem ),&planRet -> loc_maps[2]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,12,sizeof(cl_mem ),&planRet -> loc_maps[3]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,13,sizeof(cl_mem ),&planRet -> loc_maps[4]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,14,sizeof(cl_mem ),&planRet -> ind_sizes);
    errorCode = errorCode | clSetKernelArg(kernelPointer,15,sizeof(cl_mem ),&planRet -> ind_offs);
    errorCode = errorCode | clSetKernelArg(kernelPointer,16,sizeof(cl_mem ),&planRet -> blkmap);
    errorCode = errorCode | clSetKernelArg(kernelPointer,17,sizeof(cl_mem ),&planRet -> offset);
    errorCode = errorCode | clSetKernelArg(kernelPointer,18,sizeof(cl_mem ),&planRet -> nelems);
    errorCode = errorCode | clSetKernelArg(kernelPointer,19,sizeof(cl_mem ),&planRet -> nthrcol);
    errorCode = errorCode | clSetKernelArg(kernelPointer,20,sizeof(cl_mem ),&planRet -> thrcol);
    errorCode = errorCode | clSetKernelArg(kernelPointer,21,sizeof(int ),&blockOffset);
    errorCode = errorCode | clSetKernelArg(kernelPointer,22,dynamicSharedMemorySize,NULL);
    errorCode = errorCode | clSetKernelArg(kernelPointer,23,sizeof(cl_mem ),&gm1_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,24,sizeof(cl_mem ),&qinf_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,25,sizeof(cl_mem ),&eps_d);
    assert_m(errorCode == CL_SUCCESS,"Error setting OpenCL kernel arguments");
    errorCode = clEnqueueNDRangeKernel(cqCommandQueue,kernelPointer,1,NULL,&totalThreadNumber,&threadsPerBlock,0,NULL,&event);
    assert_m(errorCode == CL_SUCCESS,"Error executing OpenCL kernel");
    errorCode = clFinish(cqCommandQueue);
    assert_m(errorCode == CL_SUCCESS,"Error completing device command queue");
    blockOffset += blocksPerGrid;
  }
op_timers(&cpu_t2, &wall_t2);
op_timing_realloc(0);
  OP_kernels[0].name = userSubroutine;
  OP_kernels[0].count = OP_kernels[0].count + 1;
  OP_kernels[0].time = OP_kernels[0].time + (wall_t2 - wall_t1);
  OP_kernels[0].transfer = OP_kernels[0].transfer + planRet -> transfer;
  OP_kernels[0].transfer = OP_kernels[0].transfer + planRet -> transfer2;
}

inline void fusedOne_modified(__global float *q,__global float *qold,__local float *x1,__local float *x2,__local float *x3,__local float *x4,__global float *adt,__constant float *gam,__constant float *gm1,__constant float *cfl)
{
  for (int n = 0; n < 4; n++) {
    qold[n] = q[n];
  }
  float dx;
  float dy;
  float ri;
  float u;
  float v;
  float c;
  ri = (1.0f / q[0]);
  u = (ri * q[1]);
  v = (ri * q[2]);
  c = (sqrt((( *gam *  *gm1) * ((ri * q[3]) - (0.5f * ((u * u) + (v * v)))))));
  dx = (x2[0] - x1[0]);
  dy = (x2[1] - x1[1]);
   *adt = (fabs(((u * dy) - (v * dx))) + (c * sqrt(((dx * dx) + (dy * dy)))));
  dx = (x3[0] - x2[0]);
  dy = (x3[1] - x2[1]);
   *adt += (fabs(((u * dy) - (v * dx))) + (c * sqrt(((dx * dx) + (dy * dy)))));
  dx = (x4[0] - x3[0]);
  dy = (x4[1] - x3[1]);
   *adt += (fabs(((u * dy) - (v * dx))) + (c * sqrt(((dx * dx) + (dy * dy)))));
  dx = (x1[0] - x4[0]);
  dy = (x1[1] - x4[1]);
   *adt += (fabs(((u * dy) - (v * dx))) + (c * sqrt(((dx * dx) + (dy * dy)))));
   *adt = ( *adt /  *cfl);
}

__kernel void fusedOne_kernel(__global float *opDat1,__global float *opDat2,__global float *opDat3,__global float *opDat7,__global int *ind_maps3,__global short *mappingArray3,__global short *mappingArray4,__global short *mappingArray5,__global short *mappingArray6,__global int *pindSizes,__global int *pindOffs,__global int *pblkMap,__global int *poffset,__global int *pnelems,__global int *pnthrcol,__global int *pthrcol,__private int blockOffset,__local float *shared_fusedOne,__constant float *gam,__constant float *gm1,__constant float *cfl)
{
  __local int sharedMemoryOffset;
  __local int numberOfActiveThreads;
  int nbytes;
  int blockID;
  int i1;
  __global  int* __local  opDat3IndirectionMap;
  __local int opDat3SharedIndirectionSize;
  __local  float * __local  opDat3SharedIndirection;
  if (get_local_id(0) == 0) {
    blockID = pblkMap[get_group_id(0) + blockOffset];
    numberOfActiveThreads = pnelems[blockID];
    sharedMemoryOffset = poffset[blockID];
    opDat3SharedIndirectionSize = pindSizes[0 + blockID * 1];
    opDat3IndirectionMap = ind_maps3 + pindOffs[0 + blockID * 1];
    nbytes = 0;
    opDat3SharedIndirection = &shared_fusedOne[nbytes / sizeof(float )];
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (i1 = get_local_id(0); i1 < opDat3SharedIndirectionSize * 2; i1 += get_local_size(0)) {
    opDat3SharedIndirection[i1] = opDat3[i1 % 2 + opDat3IndirectionMap[i1 / 2] * 2];
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (i1 = get_local_id(0); i1 < numberOfActiveThreads; i1 += get_local_size(0)) {
    fusedOne_modified(opDat1 + (i1 + sharedMemoryOffset) * 4,opDat2 + (i1 + sharedMemoryOffset) * 4,opDat3SharedIndirection + mappingArray3[i1 + sharedMemoryOffset] * 2,opDat3SharedIndirection + mappingArray4[i1 + sharedMemoryOffset] * 2,opDat3SharedIndirection + mappingArray5[i1 + sharedMemoryOffset] * 2,opDat3SharedIndirection + mappingArray6[i1 + sharedMemoryOffset] * 2,opDat7 + (i1 + sharedMemoryOffset) * 1,gam,gm1,cfl);
  }
}

void fusedOne_host(const char *userSubroutine,op_set set,op_arg opDat1,op_arg opDat2,op_arg opDat3,op_arg opDat4,op_arg opDat5,op_arg opDat6,op_arg opDat7)
{
#ifdef OP_BLOCK_SIZE_1
  threadsPerBlockSize_fusedOne = OP_BLOCK_SIZE_1;
#endif
#ifdef OP_PART_SIZE_1
  setPartitionSize_fusedOne = OP_PART_SIZE_1;
#endif
  size_t blocksPerGrid;
  size_t threadsPerBlock;
  size_t totalThreadNumber;
  size_t dynamicSharedMemorySize;
  cl_int errorCode;
  cl_event event;
  cl_kernel kernelPointer;
  int i3;
  op_arg opDatArray[7];
  int indirectionDescriptorArray[7];
  op_plan *planRet;
  int blockOffset;
  opDatArray[0] = opDat1;
  opDatArray[1] = opDat2;
  opDatArray[2] = opDat3;
  opDatArray[3] = opDat4;
  opDatArray[4] = opDat5;
  opDatArray[5] = opDat6;
  opDatArray[6] = opDat7;
  indirectionDescriptorArray[0] = -1;
  indirectionDescriptorArray[1] = -1;
  indirectionDescriptorArray[2] = 0;
  indirectionDescriptorArray[3] = 0;
  indirectionDescriptorArray[4] = 0;
  indirectionDescriptorArray[5] = 0;
  indirectionDescriptorArray[6] = -1;
  planRet = op_plan_get(userSubroutine,set,setPartitionSize_fusedOne,7,opDatArray,1,indirectionDescriptorArray);
  cl_mem gam_d;
  gam_d = op_allocate_constant(&gam,sizeof(float ));
  cl_mem gm1_d;
  gm1_d = op_allocate_constant(&gm1,sizeof(float ));
  cl_mem cfl_d;
  cfl_d = op_allocate_constant(&cfl,sizeof(float ));
  blockOffset = 0;
  double cpu_t1;
  double cpu_t2;
  double wall_t1;
op_timers(&cpu_t1, &wall_t1);
  double wall_t2;
  for (i3 = 0; i3 < planRet -> ncolors; ++i3) {
    blocksPerGrid = planRet -> ncolblk[i3];
    dynamicSharedMemorySize = planRet -> nshared;
    threadsPerBlock = threadsPerBlockSize_fusedOne;
    totalThreadNumber = threadsPerBlock * blocksPerGrid;
    kernelPointer = getKernel("fusedOne_kernel");
    errorCode = clSetKernelArg(kernelPointer,0,sizeof(cl_mem ),&opDat1.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,1,sizeof(cl_mem ),&opDat2.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,2,sizeof(cl_mem ),&opDat3.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,3,sizeof(cl_mem ),&opDat7.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,4,sizeof(cl_mem ),&planRet -> ind_maps[0]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,5,sizeof(cl_mem ),&planRet -> loc_maps[2]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,6,sizeof(cl_mem ),&planRet -> loc_maps[3]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,7,sizeof(cl_mem ),&planRet -> loc_maps[4]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,8,sizeof(cl_mem ),&planRet -> loc_maps[5]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,9,sizeof(cl_mem ),&planRet -> ind_sizes);
    errorCode = errorCode | clSetKernelArg(kernelPointer,10,sizeof(cl_mem ),&planRet -> ind_offs);
    errorCode = errorCode | clSetKernelArg(kernelPointer,11,sizeof(cl_mem ),&planRet -> blkmap);
    errorCode = errorCode | clSetKernelArg(kernelPointer,12,sizeof(cl_mem ),&planRet -> offset);
    errorCode = errorCode | clSetKernelArg(kernelPointer,13,sizeof(cl_mem ),&planRet -> nelems);
    errorCode = errorCode | clSetKernelArg(kernelPointer,14,sizeof(cl_mem ),&planRet -> nthrcol);
    errorCode = errorCode | clSetKernelArg(kernelPointer,15,sizeof(cl_mem ),&planRet -> thrcol);
    errorCode = errorCode | clSetKernelArg(kernelPointer,16,sizeof(int ),&blockOffset);
    errorCode = errorCode | clSetKernelArg(kernelPointer,17,dynamicSharedMemorySize,NULL);
    errorCode = errorCode | clSetKernelArg(kernelPointer,18,sizeof(cl_mem ),&gam_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,19,sizeof(cl_mem ),&gm1_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,20,sizeof(cl_mem ),&cfl_d);
    assert_m(errorCode == CL_SUCCESS,"Error setting OpenCL kernel arguments");
    errorCode = clEnqueueNDRangeKernel(cqCommandQueue,kernelPointer,1,NULL,&totalThreadNumber,&threadsPerBlock,0,NULL,&event);
    assert_m(errorCode == CL_SUCCESS,"Error executing OpenCL kernel");
    errorCode = clFinish(cqCommandQueue);
    assert_m(errorCode == CL_SUCCESS,"Error completing device command queue");
    blockOffset += blocksPerGrid;
  }
op_timers(&cpu_t2, &wall_t2);
op_timing_realloc(1);
  OP_kernels[1].name = userSubroutine;
  OP_kernels[1].count = OP_kernels[1].count + 1;
  OP_kernels[1].time = OP_kernels[1].time + (wall_t2 - wall_t1);
  OP_kernels[1].transfer = OP_kernels[1].transfer + planRet -> transfer;
  OP_kernels[1].transfer = OP_kernels[1].transfer + planRet -> transfer2;
}

inline void fusedTwo_modified(__global float *qold,__global float *q,__global float *res,__global float *adt,float *rms,__local float *x1,__local float *x2,__local float *x3,__local float *x4,__constant float *gam,__constant float *gm1,__constant float *cfl)
{
  float del;
  float adti;
  float dx;
  float dy;
  float ri;
  float u;
  float v;
  float c;
  adti = (1.0f /  *adt);
  for (int n = 0; n < 4; n++) {
    del = (adti * res[n]);
    q[n] = (qold[n] - del);
    res[n] = 0.0f;
     *rms += (del * del);
  }
  ri = (1.0f / q[0]);
  u = (ri * q[1]);
  v = (ri * q[2]);
  c = (sqrt((( *gam *  *gm1) * ((ri * q[3]) - (0.5f * ((u * u) + (v * v)))))));
  dx = (x2[0] - x1[0]);
  dy = (x2[1] - x1[1]);
   *adt = (fabs(((u * dy) - (v * dx))) + (c * sqrt(((dx * dx) + (dy * dy)))));
  dx = (x3[0] - x2[0]);
  dy = (x3[1] - x2[1]);
   *adt += (fabs(((u * dy) - (v * dx))) + (c * sqrt(((dx * dx) + (dy * dy)))));
  dx = (x4[0] - x3[0]);
  dy = (x4[1] - x3[1]);
   *adt += (fabs(((u * dy) - (v * dx))) + (c * sqrt(((dx * dx) + (dy * dy)))));
  dx = (x1[0] - x4[0]);
  dy = (x1[1] - x4[1]);
   *adt += (fabs(((u * dy) - (v * dx))) + (c * sqrt(((dx * dx) + (dy * dy)))));
   *adt = ( *adt /  *cfl);
}

__kernel void fusedTwo_kernel(__global float *opDat1,__global float *opDat2,__global float *opDat3,__global float *opDat4,__global float *reductionArrayDevice5,__global float *opDat6,__global int *ind_maps6,__global short *mappingArray6,__global short *mappingArray7,__global short *mappingArray8,__global short *mappingArray9,__global int *pindSizes,__global int *pindOffs,__global int *pblkMap,__global int *poffset,__global int *pnelems,__global int *pnthrcol,__global int *pthrcol,__private int blockOffset,__local float *shared_fusedTwo,__constant float *gam,__constant float *gm1,__constant float *cfl)
{
  __local int sharedMemoryOffset;
  __local int numberOfActiveThreads;
  int nbytes;
  int blockID;
  int i1;
  float opDat5Local[1];
  __local float reductionTemporaryArray5[2048];
  __global  int* __local  opDat6IndirectionMap;
  __local int opDat6SharedIndirectionSize;
  __local  float * __local  opDat6SharedIndirection;
  if (get_local_id(0) == 0) {
    blockID = pblkMap[get_group_id(0) + blockOffset];
    numberOfActiveThreads = pnelems[blockID];
    sharedMemoryOffset = poffset[blockID];
    opDat6SharedIndirectionSize = pindSizes[0 + blockID * 1];
    opDat6IndirectionMap = ind_maps6 + pindOffs[0 + blockID * 1];
    nbytes = 0;
    opDat6SharedIndirection = &shared_fusedTwo[nbytes / sizeof(float )];
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (i1 = get_local_id(0); i1 < opDat6SharedIndirectionSize * 2; i1 += get_local_size(0)) {
    opDat6SharedIndirection[i1] = opDat6[i1 % 2 + opDat6IndirectionMap[i1 / 2] * 2];
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (i1 = get_local_id(0); i1 < numberOfActiveThreads; i1 += get_local_size(0)) {
    fusedTwo_modified(opDat1 + (i1 + sharedMemoryOffset) * 4,opDat2 + (i1 + sharedMemoryOffset) * 4,opDat3 + (i1 + sharedMemoryOffset) * 4,opDat4 + (i1 + sharedMemoryOffset) * 1,opDat5Local,opDat6SharedIndirection + mappingArray6[i1 + sharedMemoryOffset] * 2,opDat6SharedIndirection + mappingArray7[i1 + sharedMemoryOffset] * 2,opDat6SharedIndirection + mappingArray8[i1 + sharedMemoryOffset] * 2,opDat6SharedIndirection + mappingArray9[i1 + sharedMemoryOffset] * 2,gam,gm1,cfl);
  }
}

void fusedTwo_host(const char *userSubroutine,op_set set,op_arg opDat1,op_arg opDat2,op_arg opDat3,op_arg opDat4,op_arg opDat5,op_arg opDat6,op_arg opDat7,op_arg opDat8,op_arg opDat9)
{
#ifdef OP_BLOCK_SIZE_2
  threadsPerBlockSize_fusedTwo = OP_BLOCK_SIZE_2;
#endif
#ifdef OP_PART_SIZE_2
  setPartitionSize_fusedTwo = OP_PART_SIZE_2;
#endif
  size_t blocksPerGrid;
  size_t threadsPerBlock;
  size_t totalThreadNumber;
  size_t dynamicSharedMemorySize;
  cl_int errorCode;
  cl_event event;
  cl_kernel kernelPointer;
  int i3;
  op_arg opDatArray[9];
  int indirectionDescriptorArray[9];
  op_plan *planRet;
  int blockOffset;
  int i1;
  int i2;
  int reductionBytes;
  int reductionSharedMemorySize;
  float *reductionArrayHost5;
  reductionBytes = 0;
  reductionSharedMemorySize = 0;
  reductionArrayHost5 = ((float *)opDat5.data);
  reductionBytes += ROUND_UP(blocksPerGrid * sizeof(float ) * 1);
  reductionSharedMemorySize = MAX(reductionSharedMemorySize,sizeof(float ));
  reallocReductArrays(reductionBytes);
  reductionBytes = 0;
  opDat5.data = OP_reduct_h + reductionBytes;
  opDat5.data_d = ((char *)OP_reduct_d) + reductionBytes;
  for (i1 = 0; i1 < blocksPerGrid; ++i1) {
    for (i2 = 0; i2 < 1; ++i2) {
      ((float *)opDat5.data)[i2 + i1 * 1] = 0.00000F;
    }
  }
  reductionBytes += ROUND_UP(blocksPerGrid * sizeof(float ) * 1);
  mvReductArraysToDevice(reductionBytes);
  opDatArray[0] = opDat1;
  opDatArray[1] = opDat2;
  opDatArray[2] = opDat3;
  opDatArray[3] = opDat4;
  opDatArray[4] = opDat5;
  opDatArray[5] = opDat6;
  opDatArray[6] = opDat7;
  opDatArray[7] = opDat8;
  opDatArray[8] = opDat9;
  indirectionDescriptorArray[0] = -1;
  indirectionDescriptorArray[1] = -1;
  indirectionDescriptorArray[2] = -1;
  indirectionDescriptorArray[3] = -1;
  indirectionDescriptorArray[4] = -1;
  indirectionDescriptorArray[5] = 0;
  indirectionDescriptorArray[6] = 0;
  indirectionDescriptorArray[7] = 0;
  indirectionDescriptorArray[8] = 0;
  planRet = op_plan_get(userSubroutine,set,setPartitionSize_fusedTwo,9,opDatArray,1,indirectionDescriptorArray);
  cl_mem gam_d;
  gam_d = op_allocate_constant(&gam,sizeof(float ));
  cl_mem gm1_d;
  gm1_d = op_allocate_constant(&gm1,sizeof(float ));
  cl_mem cfl_d;
  cfl_d = op_allocate_constant(&cfl,sizeof(float ));
  blockOffset = 0;
  double cpu_t1;
  double cpu_t2;
  double wall_t1;
op_timers(&cpu_t1, &wall_t1);
  double wall_t2;
  for (i3 = 0; i3 < planRet -> ncolors; ++i3) {
    blocksPerGrid = planRet -> ncolblk[i3];
    dynamicSharedMemorySize = planRet -> nshared;
    threadsPerBlock = threadsPerBlockSize_fusedTwo;
    totalThreadNumber = threadsPerBlock * blocksPerGrid;
    kernelPointer = getKernel("fusedTwo_kernel");
    errorCode = clSetKernelArg(kernelPointer,0,sizeof(cl_mem ),&opDat1.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,1,sizeof(cl_mem ),&opDat2.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,2,sizeof(cl_mem ),&opDat3.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,3,sizeof(cl_mem ),&opDat4.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,4,sizeof(cl_mem ),&opDat5.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,5,sizeof(cl_mem ),&opDat6.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,6,sizeof(cl_mem ),&planRet -> ind_maps[0]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,7,sizeof(cl_mem ),&planRet -> loc_maps[5]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,8,sizeof(cl_mem ),&planRet -> loc_maps[6]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,9,sizeof(cl_mem ),&planRet -> loc_maps[7]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,10,sizeof(cl_mem ),&planRet -> loc_maps[8]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,11,sizeof(cl_mem ),&planRet -> ind_sizes);
    errorCode = errorCode | clSetKernelArg(kernelPointer,12,sizeof(cl_mem ),&planRet -> ind_offs);
    errorCode = errorCode | clSetKernelArg(kernelPointer,13,sizeof(cl_mem ),&planRet -> blkmap);
    errorCode = errorCode | clSetKernelArg(kernelPointer,14,sizeof(cl_mem ),&planRet -> offset);
    errorCode = errorCode | clSetKernelArg(kernelPointer,15,sizeof(cl_mem ),&planRet -> nelems);
    errorCode = errorCode | clSetKernelArg(kernelPointer,16,sizeof(cl_mem ),&planRet -> nthrcol);
    errorCode = errorCode | clSetKernelArg(kernelPointer,17,sizeof(cl_mem ),&planRet -> thrcol);
    errorCode = errorCode | clSetKernelArg(kernelPointer,18,sizeof(int ),&blockOffset);
    errorCode = errorCode | clSetKernelArg(kernelPointer,19,dynamicSharedMemorySize,NULL);
    errorCode = errorCode | clSetKernelArg(kernelPointer,20,sizeof(cl_mem ),&gam_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,21,sizeof(cl_mem ),&gm1_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,22,sizeof(cl_mem ),&cfl_d);
    assert_m(errorCode == CL_SUCCESS,"Error setting OpenCL kernel arguments");
    errorCode = clEnqueueNDRangeKernel(cqCommandQueue,kernelPointer,1,NULL,&totalThreadNumber,&threadsPerBlock,0,NULL,&event);
    assert_m(errorCode == CL_SUCCESS,"Error executing OpenCL kernel");
    errorCode = clFinish(cqCommandQueue);
    assert_m(errorCode == CL_SUCCESS,"Error completing device command queue");
    blockOffset += blocksPerGrid;
  }
op_timers(&cpu_t2, &wall_t2);
op_timing_realloc(2);
  OP_kernels[2].name = userSubroutine;
  OP_kernels[2].count = OP_kernels[2].count + 1;
  OP_kernels[2].time = OP_kernels[2].time + (wall_t2 - wall_t1);
  OP_kernels[2].transfer = OP_kernels[2].transfer + planRet -> transfer;
  OP_kernels[2].transfer = OP_kernels[2].transfer + planRet -> transfer2;
  mvReductArraysToHost(reductionBytes);
  for (i1 = 0; i1 < blocksPerGrid; ++i1) {
    for (i2 = 0; i2 < 1; ++i2) {
      reductionArrayHost5[i2] += ((float *)opDat5.data)[i2 + i1 * 1];
    }
  }
}

inline void res_calc_modified(__local float *x1,__local float *x2,__local float *q1,__local float *q2,__local float *adt1,__local float *adt2,float *res1,float *res2,__constant float *gm1,__constant float *eps)
{
  float dx;
  float dy;
  float mu;
  float ri;
  float p1;
  float vol1;
  float p2;
  float vol2;
  float f;
  dx = (x1[0] - x2[0]);
  dy = (x1[1] - x2[1]);
  ri = (1.0f / q1[0]);
  p1 = ( *gm1 * (q1[3] - ((0.5f * ri) * ((q1[1] * q1[1]) + (q1[2] * q1[2])))));
  vol1 = (ri * ((q1[1] * dy) - (q1[2] * dx)));
  ri = (1.0f / q2[0]);
  p2 = ( *gm1 * (q2[3] - ((0.5f * ri) * ((q2[1] * q2[1]) + (q2[2] * q2[2])))));
  vol2 = (ri * ((q2[1] * dy) - (q2[2] * dx)));
  mu = ((0.5f * ( *adt1 +  *adt2)) *  *eps);
  f = ((0.5f * ((vol1 * q1[0]) + (vol2 * q2[0]))) + (mu * (q1[0] - q2[0])));
  res1[0] += f;
  res2[0] -= f;
  f = ((0.5f * ((((vol1 * q1[1]) + (p1 * dy)) + (vol2 * q2[1])) + (p2 * dy))) + (mu * (q1[1] - q2[1])));
  res1[1] += f;
  res2[1] -= f;
  f = ((0.5f * ((((vol1 * q1[2]) - (p1 * dx)) + (vol2 * q2[2])) - (p2 * dx))) + (mu * (q1[2] - q2[2])));
  res1[2] += f;
  res2[2] -= f;
  f = ((0.5f * ((vol1 * (q1[3] + p1)) + (vol2 * (q2[3] + p2)))) + (mu * (q1[3] - q2[3])));
  res1[3] += f;
  res2[3] -= f;
}

__kernel void res_calc_kernel(__global float *opDat1,__global float *opDat3,__global float *opDat5,__global float *opDat7,__global int *ind_maps1,__global int *ind_maps3,__global int *ind_maps5,__global int *ind_maps7,__global short *mappingArray1,__global short *mappingArray2,__global short *mappingArray3,__global short *mappingArray4,__global short *mappingArray5,__global short *mappingArray6,__global short *mappingArray7,__global short *mappingArray8,__global int *pindSizes,__global int *pindOffs,__global int *pblkMap,__global int *poffset,__global int *pnelems,__global int *pnthrcol,__global int *pthrcol,__private int blockOffset,__local float *shared_res_calc,__constant float *gm1,__constant float *eps)
{
  float opDat7Local[4];
  float opDat8Local[4];
  __local int sharedMemoryOffset;
  __local int numberOfActiveThreads;
  int nbytes;
  int blockID;
  int i1;
  __global  int* __local  opDat1IndirectionMap;
  __global  int* __local  opDat3IndirectionMap;
  __global  int* __local  opDat5IndirectionMap;
  __global  int* __local  opDat7IndirectionMap;
  __local int opDat1SharedIndirectionSize;
  __local int opDat3SharedIndirectionSize;
  __local int opDat5SharedIndirectionSize;
  __local int opDat7SharedIndirectionSize;
  __local  float * __local  opDat1SharedIndirection;
  __local  float * __local  opDat3SharedIndirection;
  __local  float * __local  opDat5SharedIndirection;
  __local  float * __local  opDat7SharedIndirection;
  __local int numOfColours;
  __local int numberOfActiveThreadsCeiling;
  int colour1;
  int colour2;
  int i2;
  if (get_local_id(0) == 0) {
    blockID = pblkMap[get_group_id(0) + blockOffset];
    numberOfActiveThreads = pnelems[blockID];
    numberOfActiveThreadsCeiling = get_local_size(0) * (1 + (numberOfActiveThreads - 1) / get_local_size(0));
    numOfColours = pnthrcol[blockID];
    sharedMemoryOffset = poffset[blockID];
    opDat1SharedIndirectionSize = pindSizes[0 + blockID * 4];
    opDat3SharedIndirectionSize = pindSizes[1 + blockID * 4];
    opDat5SharedIndirectionSize = pindSizes[2 + blockID * 4];
    opDat7SharedIndirectionSize = pindSizes[3 + blockID * 4];
    opDat1IndirectionMap = ind_maps1 + pindOffs[0 + blockID * 4];
    opDat3IndirectionMap = ind_maps3 + pindOffs[1 + blockID * 4];
    opDat5IndirectionMap = ind_maps5 + pindOffs[2 + blockID * 4];
    opDat7IndirectionMap = ind_maps7 + pindOffs[3 + blockID * 4];
    nbytes = 0;
    opDat1SharedIndirection = &shared_res_calc[nbytes / sizeof(float )];
    nbytes += ROUND_UP(opDat1SharedIndirectionSize * (sizeof(float ) * 2));
    opDat3SharedIndirection = &shared_res_calc[nbytes / sizeof(float )];
    nbytes += ROUND_UP(opDat3SharedIndirectionSize * (sizeof(float ) * 4));
    opDat5SharedIndirection = &shared_res_calc[nbytes / sizeof(float )];
    nbytes += ROUND_UP(opDat5SharedIndirectionSize * (sizeof(float ) * 1));
    opDat7SharedIndirection = &shared_res_calc[nbytes / sizeof(float )];
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (i1 = get_local_id(0); i1 < opDat1SharedIndirectionSize * 2; i1 += get_local_size(0)) {
    opDat1SharedIndirection[i1] = opDat1[i1 % 2 + opDat1IndirectionMap[i1 / 2] * 2];
  }
  for (i1 = get_local_id(0); i1 < opDat3SharedIndirectionSize * 4; i1 += get_local_size(0)) {
    opDat3SharedIndirection[i1] = opDat3[i1 % 4 + opDat3IndirectionMap[i1 / 4] * 4];
  }
  for (i1 = get_local_id(0); i1 < opDat5SharedIndirectionSize * 1; i1 += get_local_size(0)) {
    opDat5SharedIndirection[i1] = opDat5[i1 % 1 + opDat5IndirectionMap[i1 / 1] * 1];
  }
  for (i1 = get_local_id(0); i1 < opDat7SharedIndirectionSize * 4; i1 += get_local_size(0)) {
    opDat7SharedIndirection[i1] = 0.00000F;
  }
  barrier(CLK_LOCAL_MEM_FENCE);
  for (i1 = get_local_id(0); i1 < numberOfActiveThreadsCeiling; i1 += get_local_size(0)) {
    colour2 = -1;
    if (i1 < numberOfActiveThreads) {
      for (i2 = 0; i2 < 4; ++i2) {
        opDat7Local[i2] = 0.00000F;
      }
      for (i2 = 0; i2 < 4; ++i2) {
        opDat8Local[i2] = 0.00000F;
      }
      res_calc_modified(opDat1SharedIndirection + mappingArray1[i1 + sharedMemoryOffset] * 2,opDat1SharedIndirection + mappingArray2[i1 + sharedMemoryOffset] * 2,opDat3SharedIndirection + mappingArray3[i1 + sharedMemoryOffset] * 4,opDat3SharedIndirection + mappingArray4[i1 + sharedMemoryOffset] * 4,opDat5SharedIndirection + mappingArray5[i1 + sharedMemoryOffset] * 1,opDat5SharedIndirection + mappingArray6[i1 + sharedMemoryOffset] * 1,opDat7Local,opDat8Local,gm1,eps);
      colour2 = pthrcol[i1 + sharedMemoryOffset];
    }
    for (colour1 = 0; colour1 < numOfColours; ++colour1) {
      if (colour2 == colour1) {
        for (i2 = 0; i2 < 4; ++i2) {
          opDat7SharedIndirection[i2 + mappingArray7[i1 + sharedMemoryOffset] * 4] += opDat7Local[i2];
        }
        for (i2 = 0; i2 < 4; ++i2) {
          opDat7SharedIndirection[i2 + mappingArray8[i1 + sharedMemoryOffset] * 4] += opDat8Local[i2];
        }
      }
      barrier(CLK_LOCAL_MEM_FENCE);
    }
  }
  for (i1 = get_local_id(0); i1 < opDat7SharedIndirectionSize * 4; i1 += get_local_size(0)) {
    opDat7[i1 % 4 + opDat7IndirectionMap[i1 / 4] * 4] += opDat7SharedIndirection[i1];
  }
}

void res_calc_host(const char *userSubroutine,op_set set,op_arg opDat1,op_arg opDat2,op_arg opDat3,op_arg opDat4,op_arg opDat5,op_arg opDat6,op_arg opDat7,op_arg opDat8)
{
#ifdef OP_BLOCK_SIZE_3
  threadsPerBlockSize_res_calc = OP_BLOCK_SIZE_3;
#endif
#ifdef OP_PART_SIZE_3
  setPartitionSize_res_calc = OP_PART_SIZE_3;
#endif
  size_t blocksPerGrid;
  size_t threadsPerBlock;
  size_t totalThreadNumber;
  size_t dynamicSharedMemorySize;
  cl_int errorCode;
  cl_event event;
  cl_kernel kernelPointer;
  int i3;
  op_arg opDatArray[8];
  int indirectionDescriptorArray[8];
  op_plan *planRet;
  int blockOffset;
  opDatArray[0] = opDat1;
  opDatArray[1] = opDat2;
  opDatArray[2] = opDat3;
  opDatArray[3] = opDat4;
  opDatArray[4] = opDat5;
  opDatArray[5] = opDat6;
  opDatArray[6] = opDat7;
  opDatArray[7] = opDat8;
  indirectionDescriptorArray[0] = 0;
  indirectionDescriptorArray[1] = 0;
  indirectionDescriptorArray[2] = 1;
  indirectionDescriptorArray[3] = 1;
  indirectionDescriptorArray[4] = 2;
  indirectionDescriptorArray[5] = 2;
  indirectionDescriptorArray[6] = 3;
  indirectionDescriptorArray[7] = 3;
  planRet = op_plan_get(userSubroutine,set,setPartitionSize_res_calc,8,opDatArray,4,indirectionDescriptorArray);
  cl_mem gm1_d;
  gm1_d = op_allocate_constant(&gm1,sizeof(float ));
  cl_mem eps_d;
  eps_d = op_allocate_constant(&eps,sizeof(float ));
  blockOffset = 0;
  double cpu_t1;
  double cpu_t2;
  double wall_t1;
op_timers(&cpu_t1, &wall_t1);
  double wall_t2;
  for (i3 = 0; i3 < planRet -> ncolors; ++i3) {
    blocksPerGrid = planRet -> ncolblk[i3];
    dynamicSharedMemorySize = planRet -> nshared;
    threadsPerBlock = threadsPerBlockSize_res_calc;
    totalThreadNumber = threadsPerBlock * blocksPerGrid;
    kernelPointer = getKernel("res_calc_kernel");
    errorCode = clSetKernelArg(kernelPointer,0,sizeof(cl_mem ),&opDat1.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,1,sizeof(cl_mem ),&opDat3.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,2,sizeof(cl_mem ),&opDat5.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,3,sizeof(cl_mem ),&opDat7.data_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,4,sizeof(cl_mem ),&planRet -> ind_maps[0]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,5,sizeof(cl_mem ),&planRet -> ind_maps[1]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,6,sizeof(cl_mem ),&planRet -> ind_maps[2]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,7,sizeof(cl_mem ),&planRet -> ind_maps[3]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,8,sizeof(cl_mem ),&planRet -> loc_maps[0]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,9,sizeof(cl_mem ),&planRet -> loc_maps[1]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,10,sizeof(cl_mem ),&planRet -> loc_maps[2]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,11,sizeof(cl_mem ),&planRet -> loc_maps[3]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,12,sizeof(cl_mem ),&planRet -> loc_maps[4]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,13,sizeof(cl_mem ),&planRet -> loc_maps[5]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,14,sizeof(cl_mem ),&planRet -> loc_maps[6]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,15,sizeof(cl_mem ),&planRet -> loc_maps[7]);
    errorCode = errorCode | clSetKernelArg(kernelPointer,16,sizeof(cl_mem ),&planRet -> ind_sizes);
    errorCode = errorCode | clSetKernelArg(kernelPointer,17,sizeof(cl_mem ),&planRet -> ind_offs);
    errorCode = errorCode | clSetKernelArg(kernelPointer,18,sizeof(cl_mem ),&planRet -> blkmap);
    errorCode = errorCode | clSetKernelArg(kernelPointer,19,sizeof(cl_mem ),&planRet -> offset);
    errorCode = errorCode | clSetKernelArg(kernelPointer,20,sizeof(cl_mem ),&planRet -> nelems);
    errorCode = errorCode | clSetKernelArg(kernelPointer,21,sizeof(cl_mem ),&planRet -> nthrcol);
    errorCode = errorCode | clSetKernelArg(kernelPointer,22,sizeof(cl_mem ),&planRet -> thrcol);
    errorCode = errorCode | clSetKernelArg(kernelPointer,23,sizeof(int ),&blockOffset);
    errorCode = errorCode | clSetKernelArg(kernelPointer,24,dynamicSharedMemorySize,NULL);
    errorCode = errorCode | clSetKernelArg(kernelPointer,25,sizeof(cl_mem ),&gm1_d);
    errorCode = errorCode | clSetKernelArg(kernelPointer,26,sizeof(cl_mem ),&eps_d);
    assert_m(errorCode == CL_SUCCESS,"Error setting OpenCL kernel arguments");
    errorCode = clEnqueueNDRangeKernel(cqCommandQueue,kernelPointer,1,NULL,&totalThreadNumber,&threadsPerBlock,0,NULL,&event);
    assert_m(errorCode == CL_SUCCESS,"Error executing OpenCL kernel");
    errorCode = clFinish(cqCommandQueue);
    assert_m(errorCode == CL_SUCCESS,"Error completing device command queue");
    blockOffset += blocksPerGrid;
  }
op_timers(&cpu_t2, &wall_t2);
op_timing_realloc(3);
  OP_kernels[3].name = userSubroutine;
  OP_kernels[3].count = OP_kernels[3].count + 1;
  OP_kernels[3].time = OP_kernels[3].time + (wall_t2 - wall_t1);
  OP_kernels[3].transfer = OP_kernels[3].transfer + planRet -> transfer;
  OP_kernels[3].transfer = OP_kernels[3].transfer + planRet -> transfer2;
}

inline void update_modified(float *qold,float *q,float *res,__global float *adt,float *rms)
{
  float del;
  float adti;
  adti = (1.0f /  *adt);
  for (int n = 0; n < 4; n++) {
    del = (adti * res[n]);
    q[n] = (qold[n] - del);
    res[n] = 0.0f;
     *rms += (del * del);
  }
}

__kernel void update_kernel(__global float *opDat1,__global float *opDat2,__global float *opDat3,__global float *opDat4,__global float *reductionArrayDevice5,int sharedMemoryOffset,int setSize,__local char *shared_update)
{
  float opDat1Local[4];
  float opDat2Local[4];
  float opDat3Local[4];
  __local char *sharedPointer_update;
  float opDat5Local[1];
  __local float reductionTemporaryArray5[2048];
  int i1;
  int i2;
  int localOffset;
  int numberOfActiveThreads;
  int threadID;
  threadID = get_local_id(0) % OP_WARPSIZE;
  for (i1 = 0; i1 < 1; ++i1) {
    opDat5Local[i1] = 0.00000F;
  }
  sharedPointer_update = shared_update + sharedMemoryOffset * (get_local_id(0) / OP_WARPSIZE);
  for (i1 = get_global_id(0); i1 < setSize; i1 += get_global_size(0)) {
    localOffset = i1 - threadID;
    numberOfActiveThreads = MIN(OP_WARPSIZE,setSize - localOffset);
    for (i2 = 0; i2 < 4; ++i2) {
      ((float *)sharedPointer_update)[threadID + i2 * numberOfActiveThreads] = opDat1[threadID + i2 * numberOfActiveThreads + localOffset * 4];
    }
    for (i2 = 0; i2 < 4; ++i2) {
      opDat1Local[i2] = ((float *)sharedPointer_update)[i2 + threadID * 4];
    }
    for (i2 = 0; i2 < 4; ++i2) {
      ((float *)sharedPointer_update)[threadID + i2 * numberOfActiveThreads] = opDat3[threadID + i2 * numberOfActiveThreads + localOffset * 4];
    }
    for (i2 = 0; i2 < 4; ++i2) {
      opDat3Local[i2] = ((float *)sharedPointer_update)[i2 + threadID * 4];
    }
    update_modified(opDat1Local,opDat2Local,opDat3Local,opDat4 + i1,opDat5Local);
    for (i2 = 0; i2 < 4; ++i2) {
      ((float *)sharedPointer_update)[i2 + threadID * 4] = opDat2Local[i2];
    }
    for (i2 = 0; i2 < 4; ++i2) {
      opDat2[threadID + i2 * numberOfActiveThreads + localOffset * 4] = ((float *)sharedPointer_update)[threadID + i2 * numberOfActiveThreads];
    }
    for (i2 = 0; i2 < 4; ++i2) {
      ((float *)sharedPointer_update)[i2 + threadID * 4] = opDat3Local[i2];
    }
    for (i2 = 0; i2 < 4; ++i2) {
      opDat3[threadID + i2 * numberOfActiveThreads + localOffset * 4] = ((float *)sharedPointer_update)[threadID + i2 * numberOfActiveThreads];
    }
  }
  for (i1 = 0; i1 < 1; ++i1) {
    ReductionFloat4(&reductionArrayDevice5[i1 + get_group_id(0) * 1],opDat5Local[i1],0,reductionTemporaryArray5);
  }
}

void update_host(const char *userSubroutine,op_set set,op_arg opDat1,op_arg opDat2,op_arg opDat3,op_arg opDat4,op_arg opDat5)
{
#ifdef OP_BLOCK_SIZE_4
  threadsPerBlockSize_update = OP_BLOCK_SIZE_4;
#endif
  size_t blocksPerGrid;
  size_t threadsPerBlock;
  size_t totalThreadNumber;
  size_t dynamicSharedMemorySize;
  cl_int errorCode;
  cl_event event;
  cl_kernel kernelPointer;
  int sharedMemoryOffset;
  int i1;
  int i2;
  int reductionBytes;
  int reductionSharedMemorySize;
  float *reductionArrayHost5;
  double cpu_t1;
  double cpu_t2;
  double wall_t1;
op_timers(&cpu_t1, &wall_t1);
  double wall_t2;
  blocksPerGrid = 200;
  threadsPerBlock = threadsPerBlockSize_update;
  totalThreadNumber = threadsPerBlock * blocksPerGrid;
  dynamicSharedMemorySize = 0;
  dynamicSharedMemorySize = MAX(dynamicSharedMemorySize,sizeof(float ) * 4);
  dynamicSharedMemorySize = MAX(dynamicSharedMemorySize,sizeof(float ) * 4);
  dynamicSharedMemorySize = MAX(dynamicSharedMemorySize,sizeof(float ) * 4);
  dynamicSharedMemorySize = MAX(dynamicSharedMemorySize,sizeof(float ) * 4);
  sharedMemoryOffset = dynamicSharedMemorySize * OP_WARPSIZE;
  dynamicSharedMemorySize = dynamicSharedMemorySize * threadsPerBlock;
  reductionBytes = 0;
  reductionSharedMemorySize = 0;
  reductionArrayHost5 = ((float *)opDat5.data);
  reductionBytes += ROUND_UP(blocksPerGrid * sizeof(float ) * 1);
  reductionSharedMemorySize = MAX(reductionSharedMemorySize,sizeof(float ));
  reallocReductArrays(reductionBytes);
  reductionBytes = 0;
  opDat5.data = OP_reduct_h + reductionBytes;
  opDat5.data_d = ((char *)OP_reduct_d) + reductionBytes;
  for (i1 = 0; i1 < blocksPerGrid; ++i1) {
    for (i2 = 0; i2 < 1; ++i2) {
      ((float *)opDat5.data)[i2 + i1 * 1] = 0.00000F;
    }
  }
  reductionBytes += ROUND_UP(blocksPerGrid * sizeof(float ) * 1);
  mvReductArraysToDevice(reductionBytes);
  kernelPointer = getKernel("update_kernel");
  errorCode = clSetKernelArg(kernelPointer,0,sizeof(cl_mem ),&opDat1.data_d);
  errorCode = errorCode | clSetKernelArg(kernelPointer,1,sizeof(cl_mem ),&opDat2.data_d);
  errorCode = errorCode | clSetKernelArg(kernelPointer,2,sizeof(cl_mem ),&opDat3.data_d);
  errorCode = errorCode | clSetKernelArg(kernelPointer,3,sizeof(cl_mem ),&opDat4.data_d);
  errorCode = errorCode | clSetKernelArg(kernelPointer,4,sizeof(cl_mem ),&opDat5.data_d);
  errorCode = errorCode | clSetKernelArg(kernelPointer,5,sizeof(int ),&sharedMemoryOffset);
  errorCode = errorCode | clSetKernelArg(kernelPointer,6,sizeof(int ),&set -> size);
  errorCode = errorCode | clSetKernelArg(kernelPointer,7,dynamicSharedMemorySize,NULL);
  assert_m(errorCode == CL_SUCCESS,"Error setting OpenCL kernel arguments");
  errorCode = clEnqueueNDRangeKernel(cqCommandQueue,kernelPointer,1,NULL,&totalThreadNumber,&threadsPerBlock,0,NULL,&event);
  assert_m(errorCode == CL_SUCCESS,"Error executing OpenCL kernel");
  errorCode = clFinish(cqCommandQueue);
  assert_m(errorCode == CL_SUCCESS,"Error completing device command queue");
  mvReductArraysToHost(reductionBytes);
  for (i1 = 0; i1 < blocksPerGrid; ++i1) {
    for (i2 = 0; i2 < 1; ++i2) {
      reductionArrayHost5[i2] += ((float *)opDat5.data)[i2 + i1 * 1];
    }
  }
op_timers(&cpu_t2, &wall_t2);
op_timing_realloc(4);
  OP_kernels[4].name = userSubroutine;
  OP_kernels[4].count = OP_kernels[4].count + 1;
  OP_kernels[4].time = OP_kernels[4].time + (wall_t2 - wall_t1);
  OP_kernels[4].transfer = OP_kernels[4].transfer + ((float )(set -> size)) * opDat1.size * 1.00000F;
  OP_kernels[4].transfer = OP_kernels[4].transfer + ((float )(set -> size)) * opDat2.size * 1.00000F;
  OP_kernels[4].transfer = OP_kernels[4].transfer + ((float )(set -> size)) * opDat3.size * 2.00000F;
  OP_kernels[4].transfer = OP_kernels[4].transfer + ((float )(set -> size)) * opDat4.size * 1.00000F;
}

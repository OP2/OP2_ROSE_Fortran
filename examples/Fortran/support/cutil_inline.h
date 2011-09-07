/*
 
 This is my own personal version of cutil_inline.h 
 based on what is in the CUDA 3.1 SDK but including
 just the bits needed for OP2 so that OP2 is now
 completely independent of the CUDA SDK
 
 */

#ifndef _CUTIL_INLINE_H_
#define _CUTIL_INLINE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <cuda.h>
#include <cuda_runtime_api.h>

#define cutilSafeCall(err) __cudaSafeCall(err,__FILE__,__LINE__)
#define cutilCheckMsg(msg) __cutilCheckMsg(msg,__FILE__,__LINE__)

inline void __cudaSafeCall(cudaError err,
                           const char *file, const int line){
  if(cudaSuccess != err) {
    printf("%s(%i) : cutilSafeCall() Runtime API error : %s.\n",
           file, line, cudaGetErrorString(err) );
    exit(-1);
  }
}

inline void __cutilCheckMsg(const char *errorMessage,
                            const char *file, const int line) {
  cudaError_t err = cudaGetLastError();
  if( cudaSuccess != err) {
    printf("%s(%i) : cutilCheckMsg() error : %s : %s.\n",
           file, line, errorMessage, cudaGetErrorString(err) );
    exit(-1);
  }
}


inline void cutilDeviceInit(int argc, char **argv) {
  int deviceCount;
  cutilSafeCall(cudaGetDeviceCount(&deviceCount));
  if (deviceCount == 0) {
    printf("cutil error: no devices supporting CUDA\n");
    exit(-1);
  }
	
  cudaDeviceProp deviceProp;
  cutilSafeCall(cudaGetDeviceProperties(&deviceProp,0));
	
  printf("\n Using CUDA device: %s\n", deviceProp.name);
  cutilSafeCall(cudaSetDevice(0));
}



#endif

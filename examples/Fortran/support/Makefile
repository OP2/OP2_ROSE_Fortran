IC = nvcc 
FC = pgfortran 

IC_OPT = -lcudart -O2 -use_fast_math
FC_OPT = -Mcuda=cuda3.1 -fast -O2 -Mcuda=ptxinfo -Minform=inform

# the following generates corresponding intermediate C CUDA code
# -Mcuda=keepgpu

all: cppfunctions cfunctions iflux_comp link

cfunctions: op_seq.h op_seq.c debug.c
	$(IC) $(IC_OPT) -c  op_seq.c debug.c

cppfunctions: cutil_inline.h op_support.h op_support.cpp
	$(IC) $(IC_OPT) -c  op_support.cpp

iflux_comp: rose_HydraConstants.F90 rose_convertDatFiles.F90 cudaConfigurationParams.F95 op2_c.F95 rose_cuda_code.CUF rose_main_op2.f90
	$(FC) $(FC_OPT) -c rose_HydraConstants.F90 rose_convertDatFiles.F90 cudaConfigurationParams.F95 op2_c.F95 rose_cuda_code.CUF rose_main_op2.f90

link: rose_HydraConstants.o rose_convertDatFiles.o cudaConfigurationParams.o op_support.o op_seq.o debug.o op2_c.o rose_cuda_code.o rose_main_op2.o
	$(FC) $(FC_OPT) rose_HydraConstants.o rose_convertDatFiles.o cudaConfigurationParams.o op_support.o op_seq.o debug.o op2_c.o rose_cuda_code.o rose_main_op2.o -o iflux_cuda

clean:
	rm -f *.o
	rm -f *.mod
	rm -f iflux_cuda
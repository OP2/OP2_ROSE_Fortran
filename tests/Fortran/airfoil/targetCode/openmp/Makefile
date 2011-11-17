IC = gcc -g 
FC = gfortran -g

OPENMP = -fopenmp
PREPROCESSOR = -fpp
EXEC = airfoil_openmp
CPPLINK = -lstdc++

all: cfunctions airfoil_seq link

cfunctions: op_support.cpp op_support.h op_seq.h op_seq.c debug.c
	$(IC) $(DEBUG) $(OPENMP) -c op_support.cpp op_seq.c debug.c

airfoil_seq: OP2Profiling.f90 input.f90 constvars.f90 op2_c.f90 airfoil_seqfun.f90 save_soln_openmp.f90 adt_calc_openmp.f90 res_calc_openmp.f90 bres_calc_openmp.f90 update_openmp.f90 airfoil.f90
	$(FC) $(DEBUG) $(OPENMP) -c  OP2Profiling.f90 input.f90 constvars.f90 op2_c.f90 airfoil_seqfun.f90 save_soln_openmp.f90 adt_calc_openmp.f90 res_calc_openmp.f90 bres_calc_openmp.f90 update_openmp.f90 airfoil.f90

link:   OP2Profiling.o input.o constvars.o op_support.o op2_c.o airfoil_seqfun.o save_soln_openmp.o adt_calc_openmp.o res_calc_openmp.o bres_calc_openmp.o update_openmp.o airfoil.o
	$(FC) $(DEBUG) $(OPENMP) $(CPPLINK) OP2Profiling.o op_support.o op_seq.o debug.o input.o constvars.o op2_c.o airfoil_seqfun.o save_soln_openmp.o adt_calc_openmp.o res_calc_openmp.o bres_calc_openmp.o update_openmp.o airfoil.o -o $(EXEC)
	
clean:
	rm -f *.o
	rm -f *.mod
	rm -f airfoil_seq


OP2_INC         = -I$(OP2_INSTALL_PATH)/c/include
OP2_LIB         = -L$(OP2_INSTALL_PATH)/c/lib

EXEC = airfoil

CPP           = g++
CPPFLAGS      = -g -fPIC -DUNIX -Wall #-Wextra

all: airfoil_seq link


airfoil_seq: kernels.cpp kernels.h airfoil.cpp Makefile
	$(CPP) $(CPPFLAGS) -DREAL_IS_DOUBLE kernels.cpp airfoil.cpp $(OP2_INC) $(OP2_LIB) -lop2_seq -c

link:  kernels.o airfoil.o Makefile
	$(CPP) $(CPPFLAGS) -DREAL_IS_DOUBLE kernels.o airfoil.o $(OP2_INC) $(OP2_LIB) -lop2_seq -o airfoil_seq

clean:
	rm -f airfoil_seq *.o

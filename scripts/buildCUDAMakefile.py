# Written by Adam Betts
#
# Generates two Makefiles:
# 1) For the source-to-source translation of Fortran files using the ROSE framework.
# 2) To compile a Fortran application with CUDA calls using the PGI compiler suite.
#    In particular, this Makefile requires 'pgcc' and 'pgfortran' to be in the path 
#    to compile the executable.

import os
from FileDependencies import getBaseFileName, determineModuleDependencies
from Graph import Graph

# Work out the dependencies between Fortran and CUDA-Fortran modules 
g = determineModuleDependencies(['F95', 'f95', 'CUF'])

# Create the Makefile  
CUDAMakefile = open("Makefile.CUDA", "w")
   
# Makefile variables
CUDAMakefile.write("CC      = pgcc\n")
CUDAMakefile.write("FC      = pgfortran\n")
CUDAMakefile.write("LD      = $(FC)\n")
CUDAMakefile.write("DEBUG   = -g\n")
CUDAMakefile.write("LDFLAGS = -Mcuda=cuda3.1 -g\n")
CUDAMakefile.write("OUT     = airfoil_cuda\n\n")

# PHONY targets
CUDAMakefile.write("# Phony targets\n")
CUDAMakefile.write(".PHONY: all clean\n\n")
CUDAMakefile.write("all: $(OUT)\n\n")
CUDAMakefile.write("clean:\n\t")
CUDAMakefile.write("rm -f *.o *.mod *.MOD $(OUT)\n\n")

# Object files to be created
CUDAMakefile.write("# C, Fortran, and CUDA Fortran object files\n")
CUDAMakefile.write("C_OBJS       := $(patsubst %.c,%.o,$(wildcard *.c))\n")
CUDAMakefile.write("FORTRAN_OBJS := $(patsubst %.F95,%.o,$(wildcard *.F95))\n")
CUDAMakefile.write("CUDA_OBJS    := $(patsubst %.CUF,%.o,$(wildcard *.CUF))\n\n")

# File suffixes to recognise
CUDAMakefile.write("# Clear out all suffixes\n")
CUDAMakefile.write(".SUFFIXES:\n")
CUDAMakefile.write("# List only suffixes we use\n")
CUDAMakefile.write(".SUFFIXES: .o .c .F95 .CUF\n\n")

# New suffix rules to generate object files from .F95, .CUF, .c files
CUDAMakefile.write("# Suffix rules\n")
CUDAMakefile.write(".c.o:\n\t")
CUDAMakefile.write("@echo \"\\n===== BUILDING OBJECT FILE $@ =====\"\n\t")
CUDAMakefile.write("$(CC) $(DEBUG) -c $< -o $@\n\n")

CUDAMakefile.write(".F95.o:\n\t")
CUDAMakefile.write("@echo \"\\n===== BUILDING OBJECT FILE $@ =====\"\n\t")
CUDAMakefile.write("$(FC) $(DEBUG) -c $< -o $@\n\n")

CUDAMakefile.write(".CUF.o:\n\t")
CUDAMakefile.write("@echo \"\\n===== BUILDING OBJECT FILE $@ =====\"\n\t")
CUDAMakefile.write("$(FC) $(DEBUG) -c $< -o $@\n\n")

# How to link object files together
CUDAMakefile.write("# Link target\n")
CUDAMakefile.write("$(OUT): $(C_OBJS) $(FORTRAN_OBJS) $(CUDA_OBJS)\n\t")
CUDAMakefile.write("@echo \"\\n===== LINKING $(OUT) =====\"\n\t")
CUDAMakefile.write("$(FC) $(LDFLAGS) $(DEBUG) $(C_OBJS) $(FORTRAN_OBJS) $(CUDA_OBJS) -o $(OUT)\n\n")

# The Fortran and CUDA object files depend on the C object files
CUDAMakefile.write("# Dependencies\n")
CUDAMakefile.write("$(FORTRAN_OBJS) $(CUDA_OBJS): $(C_OBJS)\n\n")

CUDAMakefile.write("# Per-file dependencies\n")
# Loop through each vertex in the graph
for v in g.getVertices():
    CUDAMakefile.write(getBaseFileName(v.getFileName()) + ".o: ")
    for p in v.getPredecessors():
        if p != "dummy":
            CUDAMakefile.write(getBaseFileName(p) + ".o ")
    CUDAMakefile.write("\n\n")

CUDAMakefile.close()
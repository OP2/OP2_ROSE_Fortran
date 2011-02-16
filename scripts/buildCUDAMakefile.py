# Written by Adam Betts
#
# Generates a Makefile for a Fortran application with CUDA calls.
# The Makefile requires the PGI compiler suite, in particular 'pgcc' and
# 'pgfortran' must be in the path to compile the executable.

def writeFileDependencies(makefile):
    dependencyFile = open("dependencies", "r")
    fileList = []
    for line in dependencyFile:
        fileName = line.strip()
        dotCharacter = fileName.find('.')
        # Guard against files without extensions
        if dotCharacter == - 1:
            dotCharacter = len(fileName))
        fileName = fileName[:dotCharacter]
        
        if len(fileList) > 0:
            # Chain the dependencies together by making this file depend on the
            # previous one
            makefile.write("%s.o: " % (fileName))
            makefile.write("%s.o " % (fileList[len(fileList)-1]))
            makefile.write("\n\n")
        
        fileList.append(fileName)

# Create the Makefile    
makefile = open("Makefile", "w")

# Makefile variables
makefile.write("CC      = pgcc\n")
makefile.write("FC      = pgfortran\n")
makefile.write("LD      = $(FC)\n")
makefile.write("DEBUG   = -g\n")
makefile.write("LDFLAGS = -Mcuda=cuda3.1 -g\n")
makefile.write("OUT     = airfoil_cuda\n\n")

# PHONY targets
makefile.write("# Phony targets\n")
makefile.write(".PHONY: all clean\n\n")
makefile.write("all: $(OUT)\n\n")
makefile.write("clean:\n\t")
makefile.write("rm -f *.o *.mod *.MOD $(OUT)\n\n")

# Object files to be created
makefile.write("# C, Fortran, and CUDA Fortran object files\n")
makefile.write("C_OBJS       := $(patsubst %.c,%.o,$(wildcard *.c))\n")
makefile.write("FORTRAN_OBJS := $(patsubst %.F95,%.o,$(wildcard *.F95))\n")
makefile.write("CUDA_OBJS    := $(patsubst %.CUF,%.o,$(wildcard *.CUF))\n\n")

# File suffixes to recognise
makefile.write("# Clear out all suffixes\n")
makefile.write(".SUFFIXES:\n")
makefile.write("# List only suffixes we use\n")
makefile.write(".SUFFIXES: .o .c .F95 .CUF\n\n")

# New suffix rules to generate object files from .F95, .CUF, .c files
makefile.write("# Suffix rules\n")
makefile.write(".c.o:\n\t")
makefile.write("@echo \"\\n===== BUILDING OBJECT FILE $@ =====\"\n\t")
makefile.write("$(CC) $(DEBUG) -c $< -o $@\n\n")

makefile.write(".F95.o:\n\t")
makefile.write("@echo \"\\n===== BUILDING OBJECT FILE $@ =====\"\n\t")
makefile.write("$(FC) $(DEBUG) -c $< -o $@\n\n")

makefile.write(".CUF.o:\n\t")
makefile.write("@echo \"\\n===== BUILDING OBJECT FILE $@ =====\"\n\t")
makefile.write("$(FC) $(DEBUG) -c $< -o $@\n\n")

# How to link object files together
makefile.write("# Link target\n")
makefile.write("$(OUT): $(C_OBJS) $(FORTRAN_OBJS) $(CUDA_OBJS)\n\t")
makefile.write("@echo \"\\n===== LINKING $(OUT) =====\"\n\t")
makefile.write("$(FC) $(LDFLAGS) $(DEBUG) $(C_OBJS) $(FORTRAN_OBJS) $(CUDA_OBJS) -o $(OUT)\n\n")

# The Fortran and CUDA object files depend on the C object files
makefile.write("# Dependencies\n")
makefile.write("$(FORTRAN_OBJS) $(CUDA_OBJS): $(C_OBJS)\n\n")

# Write out the dependencies between Fortran modules 
writeFileDependencies(makefile)

makefile.close()

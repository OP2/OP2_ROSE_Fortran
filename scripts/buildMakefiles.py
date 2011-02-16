# Written by Adam Betts
#
# Generates a Makefile for a Fortran application with CUDA calls.
# The Makefile requires the PGI compiler suite, in particular 'pgcc' and
# 'pgfortran' must be in the path to compile the executable.

import glob
import os
import re
from collections import defaultdict

# Create the Makefiles   
CUDAMakefile = open("Makefile.CUDA", "w")
ROSEMakefile = open("Makefile.ROSE", "w")
dependencyList = []

def getBaseFileName (fileName):
    lastDirectorySeparator = fileName.rfind('/') 
    fileExtensionCharacter = fileName.rfind('.')
    return fileName[lastDirectorySeparator+1:fileExtensionCharacter]

def determineModuleDependencies ():
    moduleNameToFileName = {}
    fileNameToModuleDependencies = defaultdict(set)
    
    fileList = glob.glob(os.getcwd() + os.sep + "*.[fF]95") + glob.glob(os.getcwd() + os.sep + "*.CUF")
    for fileName in fileList:
        file = open(fileName)
            
        use_line_regex = re.compile("^\s*use")
        module_line_regex = re.compile("^\s*module\s+(\S+)\s*$")
        
        for line in file:
            if use_line_regex.search(line):
                lexemes = line.split()
                fileNameToModuleDependencies[fileName].add(lexemes[len(lexemes) - 1])
            
            if module_line_regex.search(line):
                lexemes = line.split()
                moduleNameToFileName[lexemes[len(lexemes) - 1]] = fileName
        file.close()
    
    for fileName in fileList:
        # Ignore any CUDA files lying around
        if os.path.splitext(fileName)[1] != ".CUF":
            baseFileName = getBaseFileName(fileName)
            if fileName in fileNameToModuleDependencies:
                CUDAMakefile.write(baseFileName + ".o: ")
                writeToDependencyList = True
                for moduleName in fileNameToModuleDependencies[fileName]:
                    if moduleName in moduleNameToFileName:
                        baseModuleFileName = getBaseFileName(moduleNameToFileName[moduleName])
                        CUDAMakefile.write(baseModuleFileName + ".o ")
                        if baseModuleFileName not in dependencyList:
                            writeToDependencyList = False
                        
                CUDAMakefile.write("\n\n")
                
                if writeToDependencyList:
                    dependencyList.append(baseFileName)
                
            else:
                dependencyList.append(baseFileName)
    
    # This is a REAL hack! It naively assumes that all of the 
    # dependencies between files have been resolved and, therefore,
    # we can write out the remaining files. 
    # The proper way to do this is to build an acyclic graph and then 
    # do a topological sort
    for fileName in fileList:
        # Ignore any CUDA files lying around
        if os.path.splitext(fileName)[1] != ".CUF":
            baseFileName = getBaseFileName(fileName)
            if baseFileName not in dependencyList:
                dependencyList.append(baseFileName)
        

def createCUDAMakefile():    
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
    
def createROSEMakefile():
    # Makefile variables
    ROSEMakefile.write("FC    = translator\n")
    ROSEMakefile.write("DEBUG = 0\n\n")
    
    # PHONY targets
    ROSEMakefile.write("# Phony targets\n")
    ROSEMakefile.write(".PHONY: all clean\n\n")
    ROSEMakefile.write("all: clean s2s\n\n")
    ROSEMakefile.write("clean:\n\t")
    ROSEMakefile.write("rm -f *.CUF *.o *.mod *.rmod rose* *_postprocessed* *~\n\n")
    
    # Translation of Fortran into Fortran with CUDA
    ROSEMakefile.write("# Suffix rules\n")
    ROSEMakefile.write("s2s: ")
    for file in dependencyList:
        ROSEMakefile.write(file + ".F95 ")
    ROSEMakefile.write("\n\t")
    ROSEMakefile.write("@echo \"\\n===== TRANSLATING =====\"\n\t")
    ROSEMakefile.write("$(FC) -d $(DEBUG) ")
    for file in dependencyList:
        ROSEMakefile.write(file + ".F95 ")
    ROSEMakefile.write("\n\n")

# Write out the variables and rules 
createCUDAMakefile()

# Work out the dependencies between Fortran modules 
# And write them to the CUDA Makefile so that the correct compilation 
# order is maintained
determineModuleDependencies()

# Create the ROSE Makefile. Note that it must be called AFTER
# module dependencies have been resolved
createROSEMakefile() 

CUDAMakefile.close()
ROSEMakefile.close()
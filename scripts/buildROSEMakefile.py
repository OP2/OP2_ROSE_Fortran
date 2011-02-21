# Written by Adam Betts
#
# Generates a Makefile which controls source-to-source translation of 
# Fortran files using the ROSE framework.

import os
from FileDependencies import determineModuleDependencies
from Graph import Graph

# Work out the dependencies between Fortran 95 modules 
g = determineModuleDependencies(['F95', 'f95'])
topSort = g.getTopologicalSort()

# Create the Makefiles   
ROSEMakefile = open("Makefile.ROSE", "w")

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
for file in topSort:
    lastDirectorySeparator = file.rfind(os.sep)
    ROSEMakefile.write(file[lastDirectorySeparator+1:] + " ")
ROSEMakefile.write("\n\t")
ROSEMakefile.write("@echo \"\\n===== TRANSLATING =====\"\n\t")
ROSEMakefile.write("$(FC) -d $(DEBUG) ")
for file in topSort:
    lastDirectorySeparator = file.rfind(os.sep)
    ROSEMakefile.write(file[lastDirectorySeparator+1:] + " ")
ROSEMakefile.write("\n\n")

ROSEMakefile.close()
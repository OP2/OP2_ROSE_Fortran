# Written by Adam Betts
#
# Generates a Makefile which controls source-to-source translation of 
# Fortran files using the ROSE framework.

import os
import sys

# Add the 'src' directory to the module search and PYTHONPATH
sys.path.append(sys.path[0] + os.sep + "src")

from FileDependencies import determineModuleDependencies
from Graph import Graph

# Obtain from the user where the source-to-source translator resides
exe = raw_input("Enter path to the Fortran source-to-source translator: ")
if not (os.path.exists(exe) and os.access(exe, os.X_OK)):
    print("Could not find executable '" + exe + "'.")
    sys.exit(1)

# Work out the dependencies between Fortran 95 modules 
g = determineModuleDependencies(['F95', 'f95'])
topSort = g.getTopologicalSort()

# Create the Makefiles   
ROSEMakefile = open("Makefile.ROSE", "w")

# Makefile variables
ROSEMakefile.write("FC    = " + exe + "\n")
ROSEMakefile.write("DEBUG = 0\n\n")

# PHONY targets
ROSEMakefile.write("# Phony targets\n")
ROSEMakefile.write(".PHONY: all clean\n\n")
ROSEMakefile.write("all: clean s2s\n\n")
ROSEMakefile.write("clean:\n\t")
ROSEMakefile.write("rm -f *.CUF *.o *.rmod *.mod rose_* *_postprocessed* *~\n\n")

# Translation of Fortran into Fortran with CUDA
ROSEMakefile.write("# Source-to-source translation rule\n")
ROSEMakefile.write("s2s: ")
for file in topSort:
    lastDirectorySeparator = file.rfind(os.sep)
    ROSEMakefile.write(file[lastDirectorySeparator + 1:] + " ")
ROSEMakefile.write("\n\t")
ROSEMakefile.write("@echo \"\\n===== TRANSLATING =====\"\n\t")
ROSEMakefile.write("$(FC) -d $(DEBUG) ")
for file in topSort:
    lastDirectorySeparator = file.rfind(os.sep)
    ROSEMakefile.write(file[lastDirectorySeparator + 1:] + " ")
ROSEMakefile.write("2> stderr.txt\n\n")

ROSEMakefile.close()

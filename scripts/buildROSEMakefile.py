# Written by Adam Betts
#
# Generates a Makefile for the ROSE compiler

# Get the dependencies into list
dependencyFile = open("dependencies", "r")
fileList = []
for line in dependencyFile:
    fileName = line.strip()
    dotCharacter = fileName.find('.')
    # Guard against files without extensions
    if dotCharacter == - 1:
        dotCharacter = len(fileName)
    fileList.append(fileName[:dotCharacter])

# Create the Makefile    
makefile = open("Makefile", "w")

# Makefile variables
makefile.write("FC    = translator\n")
makefile.write("DEBUG = 0\n\n")

# PHONY targets
makefile.write("# Phony targets\n")
makefile.write(".PHONY: all clean\n\n")
makefile.write("all: clean s2s\n\n")
makefile.write("clean:\n\t")
makefile.write("rm -f *.CUF *.o *.mod *.rmod rose* *_postprocessed* *~\n\n")

# Translation of Fortran into Fortran with CUDA
makefile.write("# Suffix rules\n")
makefile.write("s2s: ")
for f in fileList:
    makefile.write("%s.F95 " % (f))
makefile.write("\n\t")
makefile.write("@echo \"\\n===== TRANSLATING =====\"\n\t")
makefile.write("$(FC) -d $(DEBUG) ")
for f in fileList:
    makefile.write("%s.F95 " % (f))
makefile.write("\n\n")

makefile.close()
import os
import sys

def executableExists (exeName):
    for path in os.environ["PATH"].split(os.pathsep):
        exe_file = os.path.join(path, exeName)
        if os.access(exe_file, os.X_OK):
            return True
    return False

# First of all check the following executables are in the path
executables = ["pgcc", "pgfortran"]
for exeName in executables:
    if not executableExists(exeName):
        print("WARNING: could not find '%s' in your path (%s)") % (exeName, os.environ["PATH"])
        #sys.exit(1)

# Create the Makefile    
makefile = open("Makefile", "w")

# Useful functions for Makefile output
def newLine ():
    makefile.write("\n")
def doubleNewLine ():
    makefile.write("\n\n")
def tabbedNewLine ():
    makefile.write("\n\t")
        
# Makefile variables
CCompilerVariable = "CC"
FortranCompilerVariable = "FC"
executableName = "airfoil"

# Write Makefile variables
makefile.write("%s = pgcc -g" % (CCompilerVariable))
newLine()
makefile.write("%s = pgfortran -Mcuda=cuda3.1 -fast -Minform=inform" % (FortranCompilerVariable))

# Target 'cfiles'
doubleNewLine()
makefile.write("cOBJS: op_seq.h op_seq.c debug.c")
tabbedNewLine()
makefile.write("$(%s) -c op_seq.c debug.c" % (CCompilerVariable))

# Target 'fortranfiles'
doubleNewLine()
makefile.write("fortranOBJS: $(wildcard *{.F95,.CUF})")
tabbedNewLine()
makefile.write("$(%s) -c $(fortranOBJS)" % (FortranCompilerVariable))

# Target 'link'
doubleNewLine()
makefile.write("link: $(cOBJS) $(fortranOBJS)")
tabbedNewLine()
makefile.write("$(%s) $(cOBJS) $(fortranOBJS) -o %s" % (FortranCompilerVariable, executableName))

# PHONY targets
doubleNewLine()
makefile.write(".PHONY: all clean")

# Target 'all'
doubleNewLine()
makefile.write("all: cOBJS fortranOBJS link") 

# Target 'clean'
doubleNewLine()
makefile.write("clean:")
tabbedNewLine()
makefile.write("rm -f *.o *.mod *.rmod %s" % (executableName))

makefile.close()
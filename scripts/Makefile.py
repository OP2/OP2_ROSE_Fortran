#!/usr/bin/python

import os
import re
import sys	
import glob
from optparse import OptionParser
from subprocess import Popen, PIPE

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

openclFlag        = "--opencl"
cudaFlag          = "--cuda"
openmpFlag        = "--openmp"
allBackendOptions = [openclFlag, cudaFlag, openmpFlag]

helpShortFlag = "-h"

parser.add_option("--clean",
                  action="store_true",
                  dest="clean",
                  help="Remove generated files. Use this flag in conjunction with one of %s to remove files generated for that particular backend." % allBackendOptions,
                  default=False)

parser.add_option("-C",
		  "--compile",
                  action="store_true",
                  dest="compile",
                  help="Runs the source-to-source compiler.",
                  default=False)

parser.add_option(cudaFlag,
                  action="store_true",
                  dest="cuda",
                  help="Generate code for CUDA backend.",
                  default=False)

parser.add_option(openclFlag,
                  action="store_true",
                  dest="opencl",
                  help="Generate code for OpenCL backend.",
                  default=False)

parser.add_option("-d",
                  "--debug",
                  action="store",
                  dest="debug",
		  type="int",
                  help="Set the debug parameter of the compiler. [Default is %default].",
                  default=0,
		  metavar="<INT>")

parser.add_option(openmpFlag,
                  action="store_true",
                  dest="openmp",
                  help="Generate code for OpenMP backend.",
                  default=False)

parser.add_option(helpShortFlag,
                  "--help",
                  action="help",
                  help="Display this help message.")

parser.add_option("-v",
                 "--verbose",
                 action="store_true",
                 dest="verbose",
                 help="Be verbose.",
                 default=False)

parser.add_option("-f",
                 "--format",
                 action="store",
		 type="int",
                 dest="format",
		 metavar="<INT>",
                 help="Format the generated code.")

parser.add_option("-M",
                 "--makefile",
                 action="store_true",
                 dest="makefile",
                 help="Generate a Makefile for the target backend.",
                 default=False)

(opts, args) = parser.parse_args(sys.argv[1:])

# Cleans out files generated during the compilation process
def clean ():
	filesToRemove = []
	filesToRemove.extend(glob.glob('BLANK*.[fF?]*'))
	filesToRemove.extend(glob.glob('rose_[!^openmp]*.[fF?]*'))
	filesToRemove.extend(glob.glob('[!^BLANK]*_postprocessed.[fF?]*'))
	filesToRemove.extend(glob.glob('*.rmod'))
	filesToRemove.extend(glob.glob('*.mod'))
	filesToRemove.extend(glob.glob('hs_err_pid*.log'))
	filesToRemove.extend(glob.glob('~*'))

	if opts.cuda:
		filesToRemove.extend(glob.glob('rose*.CUF'))
	
	if opts.openmp:			
		filesToRemove.extend(glob.glob('rose*_ope.[fF?]*'))

	for file in filesToRemove:
		if opts.verbose:
			print("Removing file: '" + file + "'") 
		os.remove(file)

def exitMessage (str):
	print(str + ".")
	exit(1)

def verboseMessage (str):
	if opts.verbose:
		print(str + ".")

def outputStdout (stdoutLines):
	print('==================================== STANDARD OUTPUT ===========================================')
	for line in stdoutLines.splitlines():
		print(line)
	print('================================================================================================')

# Runs the compiler
def compile ():
	allBackends       = (opts.cuda, opts.openmp, opts.opencl)
	backendsSelected  = [] 

	for backend in allBackends:
		if backend:
			backendsSelected.append(backend)

	if len(backendsSelected) == 0:
		exitMessage("You must specify one of %s on the command line." % allBackendOptions)
	elif len(backendsSelected) > 1:
		exitMessage("You specified multiple backends on the command line. Please only specify one of these." % backendsSelected)

	configFile = 'config'
	if not os.path.isfile(configFile):
		exitMessage("Unable to find configuration file '%s' with the path to source-to-source translator and files to translate." % (configFile))

	translatorPath = None
	filesToCompile = []
	constantsFile  = None

	for line in open(configFile, 'r'):
		line = line.strip()
		words = line.split('=')
		if line.startswith('translator'):
			translatorPath = words[1].strip()
			if not os.path.isfile(translatorPath):
				exitMessage("'" + translatorPath + "' does not exist.")
		elif line.startswith('files'):
			files = words[1].strip().split(' ')
			for f in files:
				f = f.strip()
				filesToCompile.append(f)
				if not os.path.isfile(f):
					exitMessage("File '" + f + "' does not exist.")
		elif line.startswith('constants'):
			constantsFile = words[1].strip()

	if translatorPath is None:
		exitMessage("You did not specify a path to the translator. Use 'translator=<path/to/translator>' in the configuration file.")

	if not filesToCompile:
		exitMessage("You did not specify which files need to compiled. Use files=<list/of/files> in the configuration file.")

	cmd = translatorPath + ' -d ' + str(opts.debug) + ' '

	if opts.cuda:
		cmd += cudaFlag + ' '
	elif opts.openmp:
		cmd += openmpFlag + ' '
	elif opts.opencl:
		cmd += openclFlag + ' '

	for f in filesToCompile:
		cmd += f + ' '

	if constantsFile is not None:
		cmd += "--constants " + constantsFile 

	verboseMessage("Running: '" + cmd + "'")

	# Run the compiler in a bash shell as it needs the environment variables such as
	# LD_LIBRARY_PATH
	proc = Popen(cmd,
	     	     shell=True,
             	     executable='/bin/bash',
             	     stderr=PIPE,
             	     stdout=PIPE)

	# Grab both standard output and standard error streams from the process
	stdoutLines, stderrLines = proc.communicate()

	# If a non-zero return code is detected then the compiler choked
	if proc.returncode != 0:
		print('Problem running compiler.')

		outputStdout (stdoutLines)
		
		print('==================================== STANDARD ERROR ============================================')
		lines = stderrLines.splitlines()
		print(lines[len(lines)-1])
		print('================================================================================================') 
        	exit(1)

	if opts.debug > 0:
		outputStdout (stdoutLines)

def writeLine (f, line, indent):
	baseIndex       = 0
	maxLineLength   = opts.format
	ampersandNeeded = False

	while len(line) - baseIndex > maxLineLength:		
		f.write(" " * indent)						
		if ampersandNeeded:
			f.write("&")				
		f.write(line[baseIndex:baseIndex+maxLineLength])
		tempLine = line[baseIndex:baseIndex+maxLineLength].strip()
		if tempLine.strip()[len(tempLine)-1] is not '&':
			f.write("&")
		f.write("\n")
		baseIndex = baseIndex + maxLineLength		
		ampersandNeeded = True
	
	f.write(" " * indent)
	if ampersandNeeded:
			f.write("&")
	f.write(line[baseIndex:])
	
def formatCode (files):
	end_regex        = re.compile("\s*end\s", re.IGNORECASE)
	subroutine_regex = re.compile("\ssubroutine\s", re.IGNORECASE)	
	do_regex         = re.compile("\s*do\s", re.IGNORECASE)
	select_regex     = re.compile("\s*select\s", re.IGNORECASE)
	module_regex     = re.compile("\s*module\s", re.IGNORECASE)
	program_regex    = re.compile("\s*program\s", re.IGNORECASE)
	# Have to match any character before the 'IF' because it can be on the same line as a 'CASE' statement
	if_regex         = re.compile(".*if\s", re.IGNORECASE) 
	type_regex       = re.compile("\s*type\s", re.IGNORECASE)
	call_regex       = re.compile("\s*call\s", re.IGNORECASE)
	implicit_regex   = re.compile("\s*implicit none\s", re.IGNORECASE)
	
	for fileName in files:
		verboseMessage("Formatting '" + fileName + "'")

		moduleOrProgramFound = False
		newLineNeeded        = False
		callFound            = False
		indent               = 0
		f2                   = open("_" + os.path.basename(fileName), "w")
		f                    = open(fileName, "r")

		for line in f:
			if line.strip() and (line.strip()[0] == '#' or line.strip()[0] == '!'):
				# Only output comments within the module or program scope
				if moduleOrProgramFound:
					f2.write(line)

			elif end_regex.match(line):
				if not type_regex.search(line):
					indent = indent - 2
				writeLine(f2, line, indent)
				f2.write("\n")
				newLineNeeded = False
	
			elif module_regex.match(line) or program_regex.match(line) or subroutine_regex.search(line) or \
				do_regex.match(line) or if_regex.match(line) or select_regex.match(line):
				
				if module_regex.match(line) or program_regex.match(line):
					moduleOrProgramFound = True

				if newLineNeeded:
					f2.write("\n")
		
				writeLine(f2, line, indent)
				indent = indent + 2	
				newLineNeeded = True

			elif not re.compile("\n").match(line):
				if implicit_regex.match(line):
					f2.write("\n")
					writeLine(f2, line, indent)
					f2.write("\n")
				else:
					writeLine(f2, line, indent)	
					newLineNeeded = True	
				
				if callFound and line.endswith(")\n"):
					f2.write("\n")
					callFound = False
					newLineNeeded = False
				elif call_regex.match(line):
					if not line.endswith("&\n"):
						f2.write("\n")
						newLineNeeded = False
					else:
						callFound = True			

		f.close()
		f2.close()
		os.rename(f2.name,f.name)

def generateBackendMakefile (files):
	# Add the 'src' directory to the module search and PYTHONPATH
	sys.path.append(sys.path[0] + os.sep + "src")

	from FileDependencies import getBaseFileName, determineModuleDependencies
	from Graph import Graph

	verboseMessage("Generating Makefile for backend")

	# Work out the dependencies between modules 
	g = determineModuleDependencies(files)

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
	    dependencies = []
	    # Discover non-dummy dependencies in the graph. This is 
	    # because we currently add a dummy start vertex to the graph
	    # and connect it to every vertex without predecessors.
	    # (This is needed for the topological sort.)
	    for p in v.getPredecessors():
		if p != Graph.dummyFileName:
		    dependencies.append(p)
		            
	    if len(dependencies) > 0:
		CUDAMakefile.write(getBaseFileName(v.getFileName()) + ".o: ")
		for d in dependencies:
		    CUDAMakefile.write(getBaseFileName(d) + ".o ")
		CUDAMakefile.write("\n\n")

	CUDAMakefile.close()

	return CUDAMakefile.name

if opts.clean:
	clean()

if opts.format:
	if opts.format < 40:
		exitMessage("Formatting length must be positive number greater than or equal to 40. Currently set to " + str(opts.format))

if opts.compile:
	compile()

	# The files generated by our compiler
	files = glob.glob(os.getcwd() + os.sep + "rose_*")
	
	if opts.format > 0:
		formatCode (files)
	
	if opts.makefile:
		files.append (generateBackendMakefile (files))

if not opts.clean and not opts.compile:
	exitMessage("No actions selected. Use %s for options." % helpShortFlag)

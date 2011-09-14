#!/usr/bin/python

import os
import sys	

# Add the 'src' directory to the module search and PYTHONPATH
sys.path.append(sys.path[0] + os.sep + "src")

import glob
from optparse import OptionParser
from subprocess import Popen, PIPE
from Debug import Debug

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

helpShortFlag = "-h"

parser.add_option("--clean",
                  action="store_true",
                  dest="clean",
                  help="Remove generated files.",
                  default=False)

parser.add_option("--run",
                  action="store_true",
                  dest="run",
                  help="Run the tests.",
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

(opts, args) = parser.parse_args(sys.argv[1:])

debug = Debug(opts.verbose)

generatedFilesDirectory = "generatedFiles"

# Cleans out files generated during the compilation process
def clean ():
	filesToRemove = []
	filesToRemove.extend(glob.glob('BLANK*.[fF?]*'))
	filesToRemove.extend(glob.glob('[!^BLANK]*_postprocessed.[fF?]*'))
	filesToRemove.extend(glob.glob('*.rmod'))
	filesToRemove.extend(glob.glob('*.mod'))
	filesToRemove.extend(glob.glob('hs_err_pid*.log'))
	filesToRemove.extend(glob.glob('~*'))
	filesToRemove.extend(glob.glob('rose*.[fF?]*'))
	filesToRemove.extend(glob.glob('rose*.CUF'))

	for file in filesToRemove:
		if opts.verbose:
			print("Removing file: '" + file + "'") 
		os.remove(file)

def generateCUDAMakefile (generatedFiles, executableName, testNum):
	from FileDependencies import getBaseFileName, determineModuleDependencies
	from Graph import Graph

	debug.verboseMessage("Generating Makefile for CUDA")

	CObjectFiles        = ['op_support.o', 'op_seq.o', 'debug.o']
	fortranSupportFiles = ['cudaConfigurationParams.F95', 'op2_c.F95']
	op2Directory        = "OP2_DIR"
	linkTarget          = "link"
	fortranTarget       = "fortranFiles"

	# Work out the dependencies between modules 
	g = determineModuleDependencies(generatedFiles)

	# Create the Makefile  
	CUDAMakefile = open("Makefile." + str(testNum), "w")
	   
	# Makefile variables
	CUDAMakefile.write("FC      = pgfortran\n")
	CUDAMakefile.write("FC_OPT  = -Mcuda=cuda3.1 -fast -O2 -Mcuda=ptxinfo -Minform=inform\n")
	CUDAMakefile.write("OUT     = %s\n" % (executableName))
	CUDAMakefile.write("%s = \n\n" % (op2Directory))

	# PHONY targets
	CUDAMakefile.write("# Phony targets\n")
	CUDAMakefile.write(".PHONY: all clean\n\n")
	CUDAMakefile.write("all: %s %s\n\n" % (fortranTarget, linkTarget))
	CUDAMakefile.write("clean:\n\t")
	CUDAMakefile.write("rm -f *.o *.mod *.MOD $(OUT)\n\n")

	# Fortran target
	fortranLine = ""
	for f in fortranSupportFiles:
		fortranLine += f + " "
	for f in generatedFiles:
		fortranLine += os.path.basename(f) + " "

	CUDAMakefile.write(fortranTarget + ": ")
	CUDAMakefile.write(fortranLine + "\n\t")
	CUDAMakefile.write("$(FC) $(FC_OPT) -c ")
	CUDAMakefile.write(fortranLine + "\n\n")

	# Link target
	linkLine = ""	
	for f in CObjectFiles:
		linkLine += "$(%s)/%s " % (op2Directory, f)
	for f in fortranSupportFiles:
		linkLine += f[:-4] + ".o "
	for f in generatedFiles:
		basename = os.path.basename(f)
		linkLine += basename[:-4] + ".o "

	CUDAMakefile.write(linkTarget + ": ")
	CUDAMakefile.write(linkLine + "\n\t")
	CUDAMakefile.write("$(FC) $(FC_OPT) ")
	CUDAMakefile.write(linkLine + "-o " + executableName + "\n\n")

	CUDAMakefile.close()

	return CUDAMakefile.name

# Runs the compiler
def runTests ():
	from FormatFortranCode import FormatFortranCode	

	testsFile = 'testsToRun'
	if not os.path.isfile(testsFile):
		debug.exitMessage("Unable to find file containing tests to run. It should be called '%s'" % (testsFile))

	configFile = 'config'
	if not os.path.isfile(configFile):
		debug.exitMessage("Unable to find configuration file '%s' with the absolute path to source-to-source translator" % (configFile))

	translatorPath = None
	for line in open(configFile, 'r'):
		line = line.strip()
		words = line.split('=')
		if line.startswith('translator'):
			translatorPath = words[1].strip()
			if not os.path.isfile(translatorPath):
				debug.exitMessage("'" + translatorPath + "' does not exist")

	if translatorPath is None:
		debug.exitMessage("You did not specify a path to the translator. Use 'translator=<path/to/translator>' in the configuration file")

	failLexeme = 'FAIL'
	passLexeme = 'PASS'
	i = 0
	
	for line in open(testsFile, 'r'):
		i = i + 1
		debug.verboseMessage("========== Test %s ==========" % i)
		tokens     = line.split(' ')
		testResult = tokens[1].strip()
		testFile   = tokens[0].strip()	

		if testResult != failLexeme and testResult != passLexeme:
			debug.exitMessage("The outcome of testing '%s' is '%s' which is not understood. It either has to be '%s' or '%s'" % (testFile, testResult, failLexeme, passLexeme))

		if not os.path.isfile(testFile):
			debug.exitMessage("File '" + testFile + "' does not exist")		

		filesToCompile = ['ISO_C_BINDING.F95', 'OP2.F95', 'UserKernels.F95']
		filesToCompile.append(testFile)		

		cmd = translatorPath + ' --CUDA '
		for f in filesToCompile:
			cmd += f + ' '

		debug.verboseMessage("Running: '" + cmd + "'")

		# Run the compiler in a bash shell as it needs the environment variables such as
		# LD_LIBRARY_PATH
		proc = Popen(cmd,
		     	     shell=True,
		     	     executable='/bin/bash',
		     	     stderr=PIPE,
		     	     stdout=PIPE)

		# Grab both standard output and standard error streams from the process
		stdoutLines, stderrLines = proc.communicate()

		if proc.returncode != 0:
			lines = stderrLines.splitlines()
			debug.verboseMessage("Last error message '%s'" % (lines[len(lines)-1]))
			if testResult == failLexeme:
				debug.verboseMessage("Test on file '%s' passed" % (testFile))
			else:
				debug.verboseMessage("Test on file '%s' did NOT pass" % (testFile))
		else:
			if testResult == passLexeme:				
				debug.verboseMessage("Test on file '%s' passed" % (testFile))

				# The files generated by our compiler
				files = []
 				files.append("rose_cuda_code.CUF")
				files.append("rose_" + testFile)
				f = FormatFortranCode (files) 
			
				if not os.path.exists(generatedFilesDirectory):
					os.makedirs(generatedFilesDirectory)

				renamedFiles = []
				for f in files:
					destName = generatedFilesDirectory + os.sep + testFile[:-4] + "_" + f
					debug.verboseMessage("Keeping file '%s'" % (destName))
					os.rename(f, destName)
					renamedFiles.append(destName)
				
				makefile = generateCUDAMakefile(renamedFiles, "program", i)
				destName = generatedFilesDirectory + os.sep + makefile
				debug.verboseMessage("Generating Makefile '%s'" % (destName))				
				os.rename(makefile, destName)
			else:
				debug.verboseMessage("Test on file '%s' did NOT pass" % (testFile))
		debug.verboseMessage("")

if opts.clean:
	clean()

if opts.run:
	runTests()

if not opts.clean and not opts.run:
	debug.exitMessage("No actions selected. Use %s for options" % helpShortFlag)

#!/usr/bin/python

import os
import sys
import string	

# Add the 'src' directory to the module search and PYTHONPATH
sys.path.append(sys.path[0] + os.sep + "src")

import glob
from optparse import OptionParser
from subprocess import Popen, PIPE
from Debug import Debug

# Global variables
parser                  = OptionParser(add_help_option=False)
helpShortFlag           = "-h"
generatedFilesDirectory = "generatedFiles"

# Add options to the command line
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

parser.add_option("-T",
		  "--tests",
		  dest="tests",
		  action="append",
		  help="Run only this test number. (To run multiple tests, specify this option multiple times.)")

# Parse command-line options
(opts, args) = parser.parse_args(sys.argv[1:])

# Add debug support
debug = Debug(opts.verbose)

def testInteger(message, data):
	try:
		int(data)
	except ValueError:
		debug.exitMessage(message)

# Ensure the test numbers supplied by the user are indeed integers
if opts.tests:
	for testNum in opts.tests:
		testInteger("Supplied test number '%s' is not an integer" % (testNum), testNum)

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

def generateCUDAMakefile (generatedFiles, testNum):
	from FileDependencies import getBaseFileName, determineModuleDependencies
	from Graph import Graph

	debug.verboseMessage("Generating Makefile for CUDA")

	CObjectFiles        = ['op_support.o', 'op_seq.o', 'debug.o']
	fortranSupportFiles = ['cudaConfigurationParams.F95', 'op2_c.F95']
	op2Directory        = "OP2_DIR"
	linkTarget          = "link"
	fortranTarget       = "fortranFiles"
	executableName      = "Test" + testNum

	# Work out the dependencies between modules 
	g = determineModuleDependencies(generatedFiles)

	# Create the Makefile  
	CUDAMakefile = open("Makefile." + testNum, "w")
	   
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

	translatorEnvVariable  = 'IMPERIAL_TRANSLATOR_HOME'
	translatorHome         = string.split(os.environ.get(translatorEnvVariable), os.pathsep)[0]

	print (translatorHome)
	if translatorHome is None:
		debug.exitMessage("Unable to find the root directory of the source-to-source translator. Please set environment variable '%s'" % translatorEnvVariable)
	elif not os.path.isdir(translatorHome):
		debug.exitMessage("The source-to-source translator path '%s' is not a directory" % (translatorHome))

	translatorPath   = translatorHome + os.sep + 'translator' + os.sep + 'bin' + os.sep + 'translator'
	op2Path          = translatorHome + os.sep + 'support' + os.sep + 'Fortran'

	if not os.path.isfile(translatorPath):
		debug.exitMessage("Unable to find the translator binary '" + translatorPath + "'. (Check the git repository directory structure has not changed. If so, modify this script!)")
	if not os.path.isdir(op2Path):
		debug.exitMessage("Unable to find the directory '" + op2Path + "' needed to compile Fortran programs using our translator. (Check the git repository directory structure has not changed. If so, modify this script!)")

	failLexeme = 'FAIL'
	passLexeme = 'PASS'
	
	testReportLines  = []
	for line in open(testsFile, 'r'):
		tokens     = line.split(' ')
		testNum    = tokens[0].strip()
		testResult = tokens[1].strip()	
		run        = False

		testInteger("The test number '%s' in the file '%s' is not an integer" % (testNum, testsFile), testNum)

		if opts.tests:
			# If the user has specified to run certain tests then only run those
			if testNum in opts.tests:
				run = True
		else:
			# Otherwise run everything
			run = True
				
		if run:
			debug.verboseMessage("========== Test %s ==========" % (testNum))
			if testResult != failLexeme and testResult != passLexeme:
				debug.exitMessage("The outcome of test case '%s' is '%s' which is not understood. It either has to be '%s' or '%s'" % (testNum, testResult, failLexeme, passLexeme))

			FortranFiles = []
			for i in range(2, len(tokens)):
				f = tokens[i].strip()
				if not os.path.isfile(f):
					debug.exitMessage("File '" + f + "' does not exist")	
				else:
					FortranFiles.append(f)	

			filesToCompile = ['ISO_C_BINDING.F95', 'OP2.F95']

			cmd = translatorPath + ' --CUDA '
			for f in filesToCompile:
				cmd += op2Path + os.sep + f + ' '
			cmd += 'UserKernels.F95 '
			cmd += ''.join(FortranFiles)

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
				if testResult == failLexeme:
					debug.verboseMessage("Test case #%s passed" % (testNum))
				else:
					debug.verboseMessage("Test case #%s did NOT pass" % (testNum))
					testReportLines.append("========== Test %s ==========\n" % (testNum))
					for line in stderrLines.splitlines():
						testReportLines.append(line + "\n")
					testReportLines.append("\n\n")
			else:
				if testResult == passLexeme:				
					debug.verboseMessage("Test case #%s passed" % (testNum))

					# The files generated by our compiler
					files = []
	 				files.append("rose_cuda_code.CUF")
					files.append("rose_" + tokens[len(tokens)-1].strip())
					f = FormatFortranCode (files) 
			
					if not os.path.exists(generatedFilesDirectory):
						os.makedirs(generatedFilesDirectory)

					renamedFiles = []
					j = 0
					for f in files:
						j = j + 1
						destName = generatedFilesDirectory + os.sep + "Test" + testNum + "File" + str(j) + f[-4:]
						debug.verboseMessage("Keeping file '%s'" % (destName))
						os.rename(f, destName)
						renamedFiles.append(destName)
				
					makefile = generateCUDAMakefile(renamedFiles, testNum)
					destName = generatedFilesDirectory + os.sep + makefile
					debug.verboseMessage("Generating Makefile '%s'" % (destName))				
					os.rename(makefile, destName)
				else:
					debug.verboseMessage("Test case #%s did NOT pass" % (testNum))
			debug.verboseMessage("")

	if testReportLines:
		testReport = open("testReport.txt", "w")
		for line in testReportLines:
			testReport.write(line)
		testReport.close()

if opts.clean:
	clean()

if opts.run:
	runTests()

if not opts.clean and not opts.run:
	debug.exitMessage("No actions selected. Use %s for options" % helpShortFlag)

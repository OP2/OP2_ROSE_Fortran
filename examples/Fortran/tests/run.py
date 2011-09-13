#!/usr/bin/python

import os
import re
import sys	
import glob
from optparse import OptionParser
from subprocess import Popen, PIPE

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
def runTests ():
	testsFile = 'testsToRun'
	if not os.path.isfile(testsFile):
		exitMessage("Unable to find file containing tests to run. It should be called '%s'" % (testsFile))

	configFile = 'config'
	if not os.path.isfile(configFile):
		exitMessage("Unable to find configuration file '%s' with the absolute path to source-to-source translator" % (configFile))

	translatorPath = None
	for line in open(configFile, 'r'):
		line = line.strip()
		words = line.split('=')
		if line.startswith('translator'):
			translatorPath = words[1].strip()
			if not os.path.isfile(translatorPath):
				exitMessage("'" + translatorPath + "' does not exist")

	if translatorPath is None:
		exitMessage("You did not specify a path to the translator. Use 'translator=<path/to/translator>' in the configuration file")

	failLexeme = 'FAIL'
	passLexeme = 'PASS'
	
	for line in open(testsFile, 'r'):
		tokens     = line.split(' ')
		testResult = tokens[1].strip()
		testFile   = tokens[0].strip()	

		if testResult != failLexeme and testResult != passLexeme:
			exitMessage("The outcome of testing '%s' is '%s' which is not understood. It either has to be '%s' or '%s'" % (testFile, testResult, failLexeme, passLexeme))

		if not os.path.isfile(testFile):
			exitMessage("File '" + testFile + "' does not exist")		

		filesToCompile = ['ISO_C_BINDING.F95', 'OP2.F95', 'UserKernels.F95']
		filesToCompile.append(testFile)		

		cmd = translatorPath + ' --CUDA '
		for f in filesToCompile:
			cmd += f + ' '

		print("Running: '" + cmd + "'")

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
			print("Last error message '%s'" % (lines[len(lines)-1]))
			if testResult == failLexeme:
				print("Test on file '%s' passed" % (testFile))
			else:
				print("Test on file '%s' did NOT pass" % (testFile))
		else:
			if testResult == passLexeme:
				print("Test on file '%s' passed" % (testFile))
			else:
				print("Test on file '%s' did NOT pass" % (testFile))		
		
		print("")

if opts.clean:
	clean()

if opts.run:
	runTests()

if not opts.clean and not opts.run:
	exitMessage("No actions selected. Use %s for options." % helpShortFlag)

#!/usr/bin/env python

import os
import sys

# Add the 'src' directory to the module search and PYTHONPATH
sys.path.append(sys.path[0] + os.sep + "src")
	
from optparse import OptionParser
from Debug import Debug

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

(opts, args) = parser.parse_args(sys.argv[1:])

debug = Debug(opts.verbose)

# Cleans out files generated during the compilation process
def clean ():
	from glob import glob

	filesToRemove = []
	filesToRemove.extend(glob('*_postprocessed.[fF?]*'))
	filesToRemove.extend(glob('*.rmod'))
	filesToRemove.extend(glob('*.mod'))
	filesToRemove.extend(glob('hs_err_pid*.log'))
	filesToRemove.extend(glob('~*'))

	if opts.cuda or opts.openmp:
		filesToRemove.extend(glob('rose*.[fF?]*')) 

	for file in filesToRemove:
		if os.path.exists(file):
			debug.verboseMessage("Removing file: '" + file + "'") 
			os.remove(file)

def outputStdout (stdoutLines):
	print('==================================== STANDARD OUTPUT ===========================================')
	for line in stdoutLines.splitlines():
		print(line)
	print('================================================================================================')

# Runs the compiler
def compile ():
	from subprocess import Popen, PIPE
	from string import split

	allBackends       = (opts.cuda, opts.openmp, opts.opencl)
	backendsSelected  = [] 

	for backend in allBackends:
		if backend:
			backendsSelected.append(backend)

	if len(backendsSelected) == 0:
		debug.exitMessage("You must specify one of %s on the command line." % allBackendOptions)
	elif len(backendsSelected) > 1:
		debug.exitMessage("You specified multiple backends on the command line. Please only specify one of these." % backendsSelected)

	configFile = 'config'
	if not os.path.isfile(configFile):
		debug.exitMessage("Unable to find configuration file '%s' with the path to source-to-source translator and files to translate." % (configFile))

	translatorEnvVariable = 'IMPERIAL_TRANSLATOR_HOME'
	translatorHome        = split(os.environ.get(translatorEnvVariable), os.pathsep)[0]

	if translatorHome is None:
		debug.exitMessage("Unable to find the root directory of the source-to-source translator. Please set environment variable '%s'" % translatorEnvVariable)
	elif not os.path.isdir(translatorHome):
		debug.exitMessage("The source-to-source translator path '%s' is not a directory" % (translatorHome))

	translatorPath = translatorHome + os.sep + 'translator' + os.sep + 'bin' + os.sep + 'translator'
	op2Path        = translatorHome + os.sep + 'support' + os.sep + 'Fortran'

	filesToCompile = []

	for line in open(configFile, 'r'):
		line = line.strip()
		words = line.split('=')
		if line.startswith('files'):
			files = words[1].strip().split(' ')
			for f in files:
				f = f.strip()
				filesToCompile.append(f)
				if not os.path.isfile(f):
					debug.exitMessage("File '" + f + "' does not exist.")

	if not filesToCompile:
		debug.exitMessage("You did not specify which files need to compiled. Use files=<list/of/files> in the configuration file.")

	cmd = translatorPath + ' -d ' + str(opts.debug) + ' '

	if opts.cuda:
		cmd += cudaFlag + ' '
	elif opts.openmp:
		cmd += openmpFlag + ' '
	elif opts.opencl:
		cmd += openclFlag + ' '

	auxiliaryFiles = ['ISO_C_BINDING.F95', 'OP2.F95']
	for f in auxiliaryFiles:
		cmd += op2Path + os.sep + f + ' '

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

if opts.clean:
	clean()

if opts.format:
	if opts.format < 40:
		debug.exitMessage("Formatting length must be positive number greater than or equal to 40. Currently set to " + str(opts.format))

if opts.compile:
	from FormatFortranCode import FormatFortranCode
	from glob import glob
	from shutil import move

	compile()

	# The files generated by our compiler
	files = glob(os.getcwd() + os.sep + "rose_*")
	
	if opts.format > 0:
		f = FormatFortranCode (files, opts.format)

	generatedCodeName = "rose_cuda_code.F95"
	for f in files:
		if os.path.basename(f) == generatedCodeName:
			destinationName = os.path.dirname(f) + os.sep + generatedCodeName[:-4] + ".CUF" 
			debug.verboseMessage("Moving '%s' into '%s'" % (f, destinationName))
			move(f, destinationName)

if not opts.clean and not opts.compile:
	debug.exitMessage("No actions selected. Use %s for options." % helpShortFlag)

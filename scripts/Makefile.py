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
	
	# The translator generates a file specific to each backend
	translatorCUDAFileName = ".translator.cuda"
	translatorOpenCLFileName = ".translator.opencl"	
	translatorOpenMPFileName = ".translator.openmp"

	generatedFile = None
	if os.path.exists(translatorCUDAFileName) and opts.cuda:
		generatedFile = open(translatorCUDAFileName, 'r')	
	if os.path.exists(translatorOpenCLFileName) and opts.opencl:
		generatedFile = open(translatorOpenCLFileName, 'r')
	if  os.path.exists(translatorOpenMPFileName) and opts.openmp:
		generatedFile = open(translatorOpenMPFileName, 'r')

	if generatedFile is not None:
		for line in generatedFile:
			line  = line.strip()
			if line.startswith("files="):
				words = line.split('=')
				files = words[1].strip().split(' ')
				for compilationUnitFileName in files:
					compilationUnitFileName = compilationUnitFileName.strip()
					if compilationUnitFileName.startswith("rose_"):
						filesToRemove.append(compilationUnitFileName)
		generatedFile.close()

	for file in filesToRemove:
		if os.path.exists(file):
			debug.verboseMessage("Removing file: '" + file + "'") 
			os.remove(file)

def outputStdout (stdoutLines):
	print('==================================== STANDARD OUTPUT ===========================================')
	for line in stdoutLines.splitlines():
		print(line)
	print('================================================================================================')

# Check that the backend selected is sane before returning which was chosen
def getBackendTarget ():
	allBackends      = (opts.cuda, opts.openmp, opts.opencl)
	backendsSelected = [] 

	for backend in allBackends:
		if backend:
			backendsSelected.append(backend)

	if len(backendsSelected) == 0:
		debug.exitMessage("You must specify one of %s on the command line." % allBackendOptions)
	elif len(backendsSelected) > 1:
		debug.exitMessage("You specified multiple backends on the command line. Please only specify one of these." % backendsSelected)
	
	if opts.cuda:
		return cudaFlag + ' '
	elif opts.openmp:
		return openmpFlag + ' '
	elif opts.opencl:
		return openclFlag + ' '

# Check that the path to the translator can be found before returning its absolute path
def getTranslatorHome ():
	from string import split

	translatorEnvVariable = 'IMPERIAL_TRANSLATOR_HOME'
	translatorHome        = split(os.environ.get(translatorEnvVariable), os.pathsep)[0]

	if translatorHome is None:
		debug.exitMessage("Unable to find the root directory of the source-to-source translator. Please set environment variable '%s'" % translatorEnvVariable)
	elif not os.path.isdir(translatorHome):
		debug.exitMessage("The source-to-source translator path '%s' is not a directory" % (translatorHome))

	return translatorHome

# Get the files to translate from the mandatory 'config' file
def getCompilationFiles ():
	from string import split
	
	configFile = 'config'
	if not os.path.isfile(configFile):
		debug.exitMessage("Unable to find configuration file '%s' with the files to translate." % (configFile))

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

	return filesToCompile

# Get the name of the file containing free variables referenced in the kernel functions
def getFreeVariablesFilename ():
	from string import split
	
	configFile = 'config'
	if not os.path.isfile(configFile):
		debug.exitMessage("Unable to find configuration file '%s' with the files to translate." % (configFile))

	filename = None

	f = open(configFile, 'r')
	for line in f:
		line = line.strip()
		words = line.split('=')
		if line.startswith('freeVariables'):
			filename = words[1].strip().split(' ')[0]
			if not os.path.isfile(filename):
				debug.exitMessage("File '" + filename + "' does not exist.")
	f.close ()

	if filename is None:
		debug.exitMessage("You did not specify which file contains the free variables. Use freeVariables=<filename> in the configuration file.")

	return filename	

# Get the command to source-to-source translate the Fortran code
def getFortranCompilationCommand (filesToCompile, freeVariablesFilename):
	translatorHome = getTranslatorHome ()
	translatorPath = translatorHome + os.sep + 'translator' + os.sep + 'bin' + os.sep + 'translator'
	op2Path        = translatorHome + os.sep + 'support' + os.sep + 'Fortran'

	cmd = translatorPath + ' -d ' + str(opts.debug) + ' ' + getBackendTarget ()

	auxiliaryFiles = ['ISO_C_BINDING.F95', 'OP2.F95']

	for f in auxiliaryFiles:
		cmd += op2Path + os.sep + f + ' '

	for f in filesToCompile:
		cmd += f + ' '

	cmd += '-f ' + freeVariablesFilename
	
	return cmd	

# Get the command to source-to-source translate the Fortran code
def getCPPCompilationCommand (filesToCompile):
	translatorHome = getTranslatorHome ()
	translatorPath = translatorHome + os.sep + 'translator' + os.sep + 'bin' + os.sep + 'translator'
	includePath    = translatorHome + os.sep + 'support' + os.sep + 'C++'

	cmd = translatorPath + ' -d ' + str(opts.debug) + ' ' + getBackendTarget () + ' -I' + includePath + ' '
	for f in filesToCompile:
		cmd += f + ' '
	
	return cmd	

def runCompiler (cmd):
	from subprocess import Popen, PIPE

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
		for line in lines:
			print (line)		
		print('================================================================================================') 
        	exit(1)

	if opts.debug > 0:
		outputStdout (stdoutLines)

def renameFortranCUDAfile (generatedFiles):
	from shutil import move

	cudaCodeName = "rose_cuda_code.F95"
	debug.verboseMessage("Looking for the ROSE generated file '%s'" % cudaCodeName)

	for f in generatedFiles:
		if os.path.basename(f) == cudaCodeName:
			destinationName = os.path.dirname(f) + os.sep + cudaCodeName[:-4] + ".CUF" 
			debug.verboseMessage("Moving '%s' into '%s'" % (f, destinationName))
			move(f, destinationName)

def preprocessOP2HeaderFiles (filesToCompile, oxfordOP2HeaderInclude):
	from tempfile import mkstemp
	from shutil import move
	from os import remove, close

	for file in filesToCompile:
		#Create temp file
		debug.verboseMessage("Analysing file '%s'" % file)
		fh, abs_path = mkstemp()
		new_file = open(abs_path,'w')
		old_file = open(file)
		for line in old_file:
			new_file.write(line.replace('#include "op_lib_cpp.h"', oxfordOP2HeaderInclude))

		new_file.close()
		close(fh)
		old_file.close()
		remove(file)
		move(abs_path, file)

def postprocessOP2Declarations (generatedFiles, oxfordOP2HeaderInclude):
	from tempfile import mkstemp
	from shutil import move
	from os import remove, close

	for file in generatedFiles:
		#Create temp file
		debug.verboseMessage("Analysing file '%s'" % file)
		fh, abs_path = mkstemp()
		new_file = open(abs_path,'w')
		old_file = open(file)
		for line in old_file:
			if oxfordOP2HeaderInclude in line:
				new_file.write("\n")
			else:
				new_file.write(line.replace("*(&OP_ID)", "OP_ID"))

		new_file.close()
		close(fh)
		old_file.close()
		remove(file)
		move(abs_path, file)

def createMakefile ():
	# The translator generates a file specific to each backend
	translatorCUDAFileName = ".translator.cuda"
	translatorOpenCLFileName = ".translator.opencl"	
	translatorOpenMPFileName = ".translator.openmp"

	generatedFile = None
	if os.path.exists(translatorCUDAFileName) and opts.cuda:
		generatedFile = open(translatorCUDAFileName, 'r')	
	if os.path.exists(translatorOpenCLFileName) and opts.opencl:
		generatedFile = open(translatorOpenCLFileName, 'r')
	if  os.path.exists(translatorOpenMPFileName) and opts.openmp:
		generatedFile = open(translatorOpenMPFileName, 'r')

	mainCompilationUnit = None
	compilationUnits    = []
	if generatedFile is not None:
		for line in generatedFile:
			line  = line.strip()
			if line.startswith("main="):
				words               = line.split('=')
				mainCompilationUnit = words[1].strip().split(' ')[0]
			elif line.startswith("files="):
				words = line.split('=')
				files = words[1].strip().split(' ')
				for compilationUnitFileName in files:
					compilationUnitFileName = compilationUnitFileName.strip()
					compilationUnits.append(compilationUnitFileName)
	generatedFile.close()
	
	makefile = None

	if opts.cuda:
		makefile = open('Makefile.cuda', 'w')	
		makefile.write("OUT = binary_cuda\n")

	elif opts.opencl:
		makefile = open('Makefile.opencl', 'w')
		makefile.write("OUT = binary_opencl\n")

	elif opts.openmp:
		makefile = open('Makefile.openmp', 'w')
		makefile.write("OUT      = binary_openmp\n")
		makefile.write("CPP      = g++\n")
		makefile.write("CPPFLAGS = -g -fPIC -Wall -DUNIX -fopenmp\n")
		makefile.write("\n")
		
		makefile.write("$(OUT): ")
		for file in compilationUnits:
			makefile.write("%s " % file)
		
		makefile.write("\n")
		makefile.write("\t")
		makefile.write("$(CPP) %s $(CPPFLAGS) -I$(OP2_INSTALL_PATH)/c/include -L$(OP2_INSTALL_PATH)/c/lib -lm -lop2_openmp -o $(OUT)\n" % mainCompilationUnit)

	makefile.close()

def handleFortranProject (filesToCompile):
	from FormatFortranCode import FormatFortranCode	
	from glob import glob

	freeVariablesFilename = getFreeVariablesFilename ()
	cmd = getFortranCompilationCommand (filesToCompile, freeVariablesFilename)
	runCompiler (cmd)

	# The files generated by our compiler
	generatedFiles = glob(os.getcwd() + os.sep + "rose_*")
	
	if opts.format > 0:
		f = FormatFortranCode (generatedFiles, opts.format)

	if opts.cuda:
		renameFortranCUDAfile (generatedFiles)

def handleCPPProject (filesToCompile):
	from glob import glob

	oxfordOP2HeaderInclude = '#include "OP2_OXFORD.h"'
	preprocessOP2HeaderFiles (filesToCompile, oxfordOP2HeaderInclude)

	cmd = getCPPCompilationCommand (filesToCompile)
	runCompiler (cmd)

	# The files generated by our compiler
	generatedFiles = glob(os.getcwd() + os.sep + "rose_*")

	postprocessOP2Declarations (generatedFiles, oxfordOP2HeaderInclude)

	createMakefile ()

if opts.clean:
	clean()

if opts.format:
	if opts.format < 40:
		debug.exitMessage("Formatting length must be positive number greater than or equal to 40. Currently set to " + str(opts.format))

if opts.compile:
	import re

	filesToCompile = getCompilationFiles ()

	fortranGeneration = False
	char_regex        = re.compile("[f|F][0-9][0-9]")

	for f in filesToCompile:
		if char_regex.match(f[-3:]):
			fortranGeneration = True

	if fortranGeneration:
		handleFortranProject (filesToCompile)
	else:
		handleCPPProject (filesToCompile)

if not opts.clean and not opts.compile:
	debug.exitMessage("No actions selected. Use %s for options." % helpShortFlag)

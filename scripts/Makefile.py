#!/usr/bin/env python

from os import sep
from sys import path, argv

# Add the 'src' directory to the module search and PYTHONPATH
path.append(path[0] + sep + "src")
	
from optparse import OptionParser
from Debug import Debug

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

openclFlag        = "--opencl"
cudaFlag          = "--cuda"
openmpFlag        = "--openmp"
allBackendOptions = [openclFlag, cudaFlag, openmpFlag]

helpShortFlag = "-h"

parser.add_option("-c",
                  "--config",
                  action="store",
                  dest="config",
		  type="string",
                  help="Use this configuration file.",
                  default=0,
		  metavar="<STRING>")

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

(opts, args) = parser.parse_args(argv[1:])

debug = Debug(opts.verbose)

# The translator generates a file specific to each backend
translatorCUDAFileName   = ".translator.cuda"
translatorOpenCLFileName = ".translator.opencl"	
translatorOpenMPFileName = ".translator.openmp"

def getBackendSpecificFileName (fileName):
	if opts.cuda:
		return fileName[:-4] + "_cuda.cpp"
	elif opts.openmp:
		return fileName[:-4] + "_openmp.cpp"
	elif opts.opencl:
		return fileName[:-4] + "_opencl.cpp"

# Cleans out files generated during the compilation process
def clean ():
	from glob import glob
	from os import path, remove 

	filesToRemove = []
	filesToRemove.extend(glob('*_postprocessed.[fF?]*'))
	filesToRemove.extend(glob('*.rmod'))
	filesToRemove.extend(glob('*.mod'))
	filesToRemove.extend(glob('hs_err_pid*.log'))
	filesToRemove.extend(glob('~*'))

	hiddenFile = None
	if path.exists(translatorCUDAFileName) and opts.cuda:
		hiddenFile = open(translatorCUDAFileName, 'r')	
	if path.exists(translatorOpenCLFileName) and opts.opencl:
		hiddenFile = open(translatorOpenCLFileName, 'r')
	if path.exists(translatorOpenMPFileName) and opts.openmp:
		hiddenFile = open(translatorOpenMPFileName, 'r')

	compilerGeneratedFiles = []
	if hiddenFile is not None:
		for line in hiddenFile:
			line  = line.strip()
			if line.startswith("files="):
				words = line.split('=')
				files = words[1].strip().split(' ')
				for compilationUnitFileName in files:
					compilationUnitFileName = compilationUnitFileName.strip()
					newFileName = getBackendSpecificFileName(compilationUnitFileName)
					compilerGeneratedFiles.append(newFileName)
	hiddenFile.close()

	filesToRemove.extend(compilerGeneratedFiles)
	for file in filesToRemove:
		if path.exists(file):
			debug.verboseMessage("Removing file: '" + file + "'") 
			remove(file)

def outputStdout (stdoutLines):
	print('==================================== STANDARD OUTPUT ===========================================')
	for line in stdoutLines.splitlines():
		print(line)
	print('================================================================================================')

def getBackendTarget ():
	# Check that the backend selected is sane before returning which was chosen
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

def getTranslatorHome ():
	# Check that the path to the translator can be found before returning its absolute path
	from string import split
	from os import environ, path, pathsep

	translatorEnvVariable = 'IMPERIAL_TRANSLATOR_HOME'

	if not environ.has_key(translatorEnvVariable):
		errorMessage = "=" * 80 + \
"\nYou need to set the environment variable '" + translatorEnvVariable + "' to point to the base directory of the translator infrastructure." + \
"\nThis is the path ending with the directory 'OP2_ROSE_Fortran'." + \
"\nFor example, 'export IMPERIAL_TRANSLATOR_HOME=/usr/joe/bloggs/subdir/OP_ROSE_FORTRAN'\n" + "=" * 80 
		debug.exitMessage(errorMessage)
	
	translatorHome = split(environ.get(translatorEnvVariable), pathsep)[0]
	if not path.isdir(translatorHome):
		debug.exitMessage("The source-to-source translator path '%s' is not a directory" % (translatorHome))

	return translatorHome

def getCompilationFiles ():		
	# Get the files to translate from the mandatory 'config' file
	from os import path
	from string import split
	
	configFile = None
	if opts.config:
		configFile = opts.config
	else:
		configFile = 'config'

	if not path.isfile(configFile):
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
				if not path.isfile(f):
					debug.exitMessage("File '" + f + "' does not exist.")

	if not filesToCompile:
		debug.exitMessage("You did not specify which files need to compiled. Use files=<list/of/files> in the configuration file.")

	return filesToCompile

def getFreeVariablesFilename ():
	# Get the name of the file containing free variables referenced in the kernel functions
	from string import split
	from os import path
	
	configFile = 'config'
	if not path.isfile(configFile):
		debug.exitMessage("Unable to find configuration file '%s' with the files to translate." % (configFile))

	filename = None

	f = open(configFile, 'r')
	for line in f:
		line = line.strip()
		words = line.split('=')
		if line.startswith('freeVariables'):
			filename = words[1].strip().split(' ')[0]
			if not path.isfile(filename):
				debug.exitMessage("File '" + filename + "' does not exist.")
	f.close ()

	if filename is None:
		debug.exitMessage("You did not specify which file contains the free variables. Use freeVariables=<filename> in the configuration file.")

	return filename	

def getFortranCompilationCommand (filesToCompile, freeVariablesFilename):
	# Get the command to source-to-source translate the Fortran code
	translatorHome = getTranslatorHome ()
	translatorPath = translatorHome + sep + 'translator' + sep + 'bin' + sep + 'translator'
	op2Path        = translatorHome + sep + 'support' + sep + 'Fortran'

	cmd = translatorPath + ' -d ' + str(opts.debug) + ' ' + getBackendTarget ()

	auxiliaryFiles = ['ISO_C_BINDING.F95', 'OP2.F95']

	for f in auxiliaryFiles:
		cmd += op2Path + sep + f + ' '

	for f in filesToCompile:
		cmd += f + ' '

	cmd += '-f ' + freeVariablesFilename
	
	return cmd	

def getCPPCompilationCommand (filesToCompile):
	# Get the command to source-to-source translate the Fortran code
	translatorHome = getTranslatorHome ()
	translatorPath = translatorHome + sep + 'translator' + sep + 'bin' + sep + 'translator'
	includePath    = translatorHome + sep + 'support' + sep + 'C++'

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
	from os import path

	cudaCodeName = "rose_cuda_code.F95"
	debug.verboseMessage("Looking for the ROSE generated file '%s'" % cudaCodeName)

	for f in generatedFiles:
		if path.basename(f) == cudaCodeName:
			destinationName = path.dirname(f) + sep + cudaCodeName[:-4] + ".CUF" 
			debug.verboseMessage("Moving '%s' into '%s'" % (f, destinationName))
			move(f, destinationName)

def switchHeadersInGivenFiles (filesToCompile, completeOP2Header, reducedOP2Header, preprocess):
	from tempfile import mkstemp
	from shutil import move
	from os import remove, close

	for file in filesToCompile:
		#Create temp file
		if preprocess:
			debug.verboseMessage("Pre-processing OP2 header includes in file '%s'" % file)
		else:
			debug.verboseMessage("Post-processing OP2 header includes in file '%s'" % file)

		fh, abs_path = mkstemp()
		new_file = open(abs_path,'w')
		old_file = open(file)
		for line in old_file:
			# If pre-processing the files then replace the OP2 header with the fake one
			if preprocess:
				new_file.write(line.replace(completeOP2Header, reducedOP2Header))
			else:
			# Otherwise post-processing, and the inverse is done
				new_file.write(line.replace(reducedOP2Header, completeOP2Header))

		new_file.close()
		close(fh)
		old_file.close()
		remove(file)
		move(abs_path, file)

def postprocessGeneratedFiles (generatedFiles, completeOP2Header, reducedOP2Header):
	from tempfile import mkstemp
	from shutil import move
	from os import remove, close

	mainCompilationUnits = generatedFiles[0]

	for file in mainCompilationUnits:
		#Create temp file
		debug.verboseMessage("Postprocessing generated file '%s'" % file)
		fh, abs_path = mkstemp()
		new_file = open(abs_path,'w')
		old_file = open(file)
		for line in old_file:
			if reducedOP2Header in line:
				new_file.write(line.replace(reducedOP2Header, completeOP2Header))
			else:
				new_file.write(line.replace("*(&OP_ID)", "OP_ID"))

		new_file.close()
		close(fh)
		old_file.close()
		remove(file)
		move(abs_path, file)

def getGeneratedFiles ():
	from shutil import move
	
	hiddenFile = None
	if opts.cuda:
		hiddenFile = open(translatorCUDAFileName, 'r')	
	elif opts.openmp:
		hiddenFile = open(translatorOpenMPFileName, 'r')
	elif opts.opencl:
		hiddenFile = open(translatorOpenCLFileName, 'r')

	compilationUnits         = []
	generatedCompilationUnit = None

	for line in hiddenFile:
		line  = line.strip()
		if line.startswith("generated="):
			words                    = line.split('=')
			oldFileName              = words[1].strip().split(' ')[0]
			newFileName              = getBackendSpecificFileName (oldFileName)
			generatedCompilationUnit = newFileName
			move(oldFileName, newFileName)
		elif line.startswith("files="):
			words = line.split('=')
			files = words[1].strip().split(' ')
			for compilationUnitFileName in files:
				compilationUnitFileName = compilationUnitFileName.strip()
				newFileName             = getBackendSpecificFileName (compilationUnitFileName)
				if newFileName != generatedCompilationUnit:
					move (compilationUnitFileName, newFileName)
					compilationUnits.append(newFileName)
	hiddenFile.close()

	if opts.cuda:
		# Move the generated file so that it has the correct suffix as required by CUDA compilers
		cudaCompilationUnit = generatedCompilationUnit[:-4] + ".cu"
		move(generatedCompilationUnit, cudaCompilationUnit)
		generatedCompilationUnit = cudaCompilationUnit

	return (compilationUnits, generatedCompilationUnit)

def createMakefile (generatedFiles):
	from shutil import move
	from os import environ
	from sys import stdout

	compilationUnits        = generatedFiles[0]
	generatedCompilatonUnit = generatedFiles[1]

	makefile = None

	if opts.cuda:
		cudaObjectFile  = generatedCompilatonUnit[:-3] + ".o"

		makefileName = 'Makefile.cuda'
		makefile = open(makefileName, 'w')	
		makefile.write("OUT       = binary_cuda\n")
		makefile.write("CPP       = g++\n")
		makefile.write("CPPFLAGS  = -O3 -Wall\n")
		makefile.write("NVCC      = nvcc\n")
		makefile.write("NVCCFLAGS = -O3 -arch=sm_20 -Xptxas=-v -use_fast_math\n")	
		makefile.write("\n")

		makefile.write("$(OUT): ")
		makefile.write(cudaObjectFile + " ")
		for file in compilationUnits:		
			objectFile = file[:-4] + ".o"
			makefile.write(objectFile + " ")

		makefile.write("\n")
		makefile.write("\t")
		makefile.write("$(CPP) ")	
		makefile.write(cudaObjectFile + " ")	
		for file in compilationUnits:
			objectFile = file[:-4] + ".o"
			makefile.write(objectFile + " ")
		makefile.write("-L$(OP2_INSTALL_PATH)/c/lib -lcudart -lop2_cuda -o $(OUT)\n")

		makefile.write("\n")
		makefile.write(cudaObjectFile + ": " + generatedCompilatonUnit)
		makefile.write("\n")
		makefile.write("\t")
		makefile.write("$(NVCC) $(NVCCFLAGS) -I$(OP2_INSTALL_PATH)/c/include -c %s -o %s" % (generatedCompilatonUnit, cudaObjectFile))
		makefile.write("\n")

		for file in compilationUnits:
			if file != generatedCompilatonUnit:
				objectFile = file[:-4] + ".o"
				makefile.write("\n")
				makefile.write(objectFile + ": " + file)
				makefile.write("\n")
				makefile.write("\t")
				makefile.write("$(CPP) $(CPPFLAGS) -I$(OP2_INSTALL_PATH)/c/include -c %s -o %s\n" % (file, objectFile))

	elif opts.opencl:
		makefileName = 'Makefile.opencl'
		makefile = open(makefileName, 'w')
		makefile.write("OUT = binary_opencl\n")

	elif opts.openmp:
		makefileName = 'Makefile.openmp'
		makefile = open(makefileName, 'w')
		makefile.write("OUT      = binary_openmp\n")
		makefile.write("CPP      = g++\n")
		makefile.write("CPPFLAGS = -O3 -Wall -fopenmp\n")
		makefile.write("\n")
		
		makefile.write("$(OUT): ")		
		makefile.write(generatedCompilatonUnit + " ")
		for file in compilationUnits:
			makefile.write(file + " ")
		
		makefile.write("\n")
		makefile.write("\t")
		makefile.write("$(CPP) ")		
		makefile.write(generatedCompilatonUnit + " ")
		for file in compilationUnits:
			makefile.write(file + " ")
		makefile.write("$(CPPFLAGS) -I$(OP2_INSTALL_PATH)/c/include -L$(OP2_INSTALL_PATH)/c/lib -lm -lop2_openmp -o $(OUT)\n")

	# Every Makefile has a clean target
	makefile.write("\n")
	makefile.write("clean:")
	makefile.write("\n")
	makefile.write("\t")
	makefile.write("rm -f $(OUT) *.o")

	makefile.close()

	op2InstallPath = "OP2_INSTALL_PATH"
	message = """\n**************************************************************** WARNING ****************************************************************
* Good news: I am generating a Makefile for you called '%s' to compile the code I just generated on your behalf!
* Bad news: in order to succesfully compile the generated code via this Makefile, you must set an environment variable '%s'.
* This environment variable should point to the directory '<prefix>/OP2-Common/op2'.
* Otherwise, the OP2 include and library directories will not be found and the make process is doomed to fail.
*****************************************************************************************************************************************
""" % (makefileName, op2InstallPath)
	stdout.write(message)

	return makefile.name

def makeArchive (makefile, generatedFiles):
	import zipfile

	compilationUnits        = generatedFiles[0]
	generatedCompilatonUnit = generatedFiles[1]

	fileName = None
	if opts.cuda:
		fileName = "files_cuda.zip"
	elif opts.opencl:
		fileName = "files_opencl.zip"	
	elif opts.openmp:
		fileName = "files_openmp.zip"

	z = zipfile.ZipFile(fileName, "w")

	z.write(makefile)
	z.write(generatedCompilatonUnit)
	for file in compilationUnits:
		z.write(file)

	z.close()

def handleFortranProject (filesToCompile):
	from FormatFortranCode import FormatFortranCode	
	from glob import glob
	from os import getcwd

	freeVariablesFilename = getFreeVariablesFilename ()
	cmd = getFortranCompilationCommand (filesToCompile, freeVariablesFilename)
	runCompiler (cmd)

	# The files generated by our compiler
	generatedFiles = glob(getcwd() + sep + "rose_*")
	
	if opts.format > 0:
		f = FormatFortranCode (generatedFiles, opts.format)

	if opts.cuda:
		renameFortranCUDAfile (generatedFiles)

def handleCPPProject (filesToCompile):
	from glob import glob

	completeOP2Header = '#include "op_lib_cpp.h"'
	reducedOP2Header  = '#include "OP2_OXFORD.h"'

	# Replace OP2 header to simpler version that passes through the ROSE frontend
	switchHeadersInGivenFiles (filesToCompile, completeOP2Header, reducedOP2Header, True)

	cmd = getCPPCompilationCommand (filesToCompile)
	runCompiler (cmd)		

	# Reverse the above header replacement to leave user files untouched
	switchHeadersInGivenFiles (filesToCompile, completeOP2Header, reducedOP2Header, False)

	# The files generated by our compiler
	generatedFiles = getGeneratedFiles ()

	postprocessGeneratedFiles (generatedFiles, completeOP2Header, reducedOP2Header)

	# Create a backend-specific Makefile
	makefile = createMakefile (generatedFiles)

	makeArchive (makefile, generatedFiles)

def main ():
	if opts.clean:
		clean()

	if opts.format:
		if opts.format < 40:
			debug.exitMessage("Formatting length must be positive number greater than or equal to 40. Currently set to " + str(opts.format))

	if opts.compile:
		from re import compile

		filesToCompile    = getCompilationFiles ()
		fortranGeneration = False
		char_regex        = compile("[f|F][0-9][0-9]")

		for f in filesToCompile:
			if char_regex.match(f[-3:]):
				fortranGeneration = True

		if fortranGeneration:
			handleFortranProject (filesToCompile)
		else:
			handleCPPProject (filesToCompile)

	if not opts.clean and not opts.compile:
		debug.exitMessage("No actions selected. Use %s for options." % helpShortFlag)

if __name__ == "__main__":
	main ()


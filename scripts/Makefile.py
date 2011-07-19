#!/usr/bin/python

import os
import re
import glob
from optparse import OptionParser
from subprocess import Popen, PIPE
from sys import argv

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

cudaFlag      = "--cuda"
openmpFlag    = "--openmp"
helpShortFlag = "-h"

parser.add_option("--clean",
                  action="store_true",
                  dest="clean",
                  help="Remove generated files. Use this flag in conjunction with %s or %s to remove files generated for that particular backend." % (cudaFlag, openmpFlag),
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
                 help="Format the generated code. [Default is %default].",
                 default=80)

(opts, args) = parser.parse_args(argv[1:])

# Cleans out files generated during the compilation process
def clean ():
	filesToRemove = []
	filesToRemove.extend(glob.glob('BLANK*.[fF?]95'))
	filesToRemove.extend(glob.glob('rose_*.[fF?]95'))
    	filesToRemove.extend(glob.glob('BLANK*.[fF?]90'))
    	filesToRemove.extend(glob.glob('rose_*.[fF?]90'))
	filesToRemove.extend(glob.glob('[!^BLANK]*_postprocessed.[fF?]95'))
    	filesToRemove.extend(glob.glob('[!^BLANK]*_postprocessed.[fF?]90'))
	filesToRemove.extend(glob.glob('*.rmod'))
	filesToRemove.extend(glob.glob('*.mod'))
	filesToRemove.extend(glob.glob('hs_err_pid*.log'))
	filesToRemove.extend(glob.glob('~*'))

	if opts.cuda:
		filesToRemove.extend(glob.glob('[!^BLANK]*_cudafor.CUF'))
	
	if opts.openmp:			
		filesToRemove.extend(glob.glob('[!^BLANK]*_openmp.[fF?]95'))

	for file in filesToRemove:
		if opts.verbose:
			print("Removing file: '" + file + "'") 
		os.remove(file)

def exitMessage (str):
	print(str)
	exit(1)

def verboseMessage (str):
	if opts.verbose:
		print(str)

def outputStdout (stdoutLines):
	print('==================================== STANDARD OUTPUT ===========================================')
	for line in stdoutLines.splitlines():
		print(line)
	print('================================================================================================')

# Runs the compiler
def compile ():
	if not opts.cuda and not opts.openmp:
		exitMessage("You must specify either %s or %s on the command line." % (cudaFlag, openmpFlag))
	elif opts.cuda and opts.openmp:
		exitMessage("You specified both %s and %s on the command line. Please only specify one of these." % (cudaFlag, openmpFlag))

	configFile = 'config'
	if not os.path.isfile(configFile):
		exitMessage("Unable to find configuration file '%s' with the path to source-to-source translator and files to translate." % (configFile))

	translatorPath = None
	filesToCompile = []

	for line in open(configFile, 'r'):
		words = line.split('=')
		if line.startswith('translator'):
			translatorPath = words[1].strip()
			if not os.path.isfile(translatorPath):
				exitMessage("'" + translatorPath + "' does not exist.")
		elif line.startswith('files'):
			files = words[1].split(' ')
			for f in files:
				f = f.strip()
				filesToCompile.append(f)
				if not os.path.isfile(f):
					exitMessage("'" + f + "' does not exist.")

	if translatorPath is None:
		exitMessage("You did not specify a path to the translator. Use 'translator=<path/to/translator>' in the configuration file.")

	if not filesToCompile:
		exitMessage("You did not specify which files need to compiled. Use files=<list/of/files> in the configuration file.")

	cmd = translatorPath + ' -d ' + str(opts.debug) + ' '

	if opts.cuda:
		cmd += cudaFlag + ' '
	elif opts.openmp:
		cmd+= openmpFlag + ' '

	for f in filesToCompile:
		cmd += f + ' '

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
		tempLine = line.strip()
		if tempLine.strip()[len(tempLine)-1] is not '&':
			f.write("&")
		f.write("\n")
		baseIndex = baseIndex + maxLineLength		
		ampersandNeeded = True
	
	f.write(" " * indent)
	if ampersandNeeded:
			f.write("&")
	f.write(line[baseIndex:])
	

def formatCode ():
	end_regex        = re.compile("\s*end\s", re.IGNORECASE)
	subroutine_regex = re.compile("\ssubroutine\s", re.IGNORECASE)	
	do_regex         = re.compile("\s*do\s", re.IGNORECASE)
	module_regex     = re.compile("\s*module\s", re.IGNORECASE)
	if_regex         = re.compile("\s*if\s", re.IGNORECASE)
	type_regex       = re.compile("\s*type\s", re.IGNORECASE)
	call_regex       = re.compile("\s*call\s", re.IGNORECASE)
	implicit_regex   = re.compile("\s*implicit none\s", re.IGNORECASE)

	files = glob.glob(os.getcwd() + os.sep + "*.CUF")
	
	for fileName in files:
		verboseMessage("Formatting '" + fileName + "'")

		newLineNeeded = False
		callFound     = False
		indent        = 0
		f2            = open("_" + os.path.basename(fileName), "w")
		f             = open(fileName, "r")

		for line in f:
			if end_regex.match(line):
				if not type_regex.search(line):
					indent = indent - 2
				writeLine(f2, line, indent)
				f2.write("\n")
				newLineNeeded = False
	
			elif subroutine_regex.search(line) or module_regex.match(line) or do_regex.match(line) or if_regex.match(line):	
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
				elif call_regex.match(line):
					if not line.endswith("&\n"):
						f2.write("\n")
					else:
						callFound = True			

		f.close()
		f2.close()
		os.rename(f2.name,f.name)

if opts.clean:
	clean()

if opts.compile:
	compile()
	
	if opts.format:
		formatCode ()

if not opts.clean and not opts.compile:
	exitMessage("No actions selected. Use %s for options." % helpShortFlag)

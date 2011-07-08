#!/usr/bin/python2.6

import os
import re
from optparse import OptionParser
from sys import argv

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

parser.add_option("-d",
                  "--debug",
                  action="store_true",
                  dest="debug",
                  help="Debug mode.",
                  default=False)

parser.add_option("-h",
                  "--help",
                  action="help",
                  help="Display this help message.")

parser.add_option("-v",
                 "--verbose",
                 action="store_true",
                 dest="verbose",
                 help="Be verbose.",
                 default=False)

(opts, args) = parser.parse_args(argv[1:])

def debugMessage (message):
	if opts.debug:
		print(message)

def verboseMessage (message):
	if opts.verbose:
		print(message)

def errorMessage (message):
	print(message)
	exit(1)

files = []

for f in args:
	if not (os.path.exists(f)):
		errorMessage("The file '" + f + "' does not exist")
	else:
		files.append(f)

include_regex = re.compile("\s*#include \"const.inc\"")
end_regex     = re.compile("\s*end\s")

for f in set(files):
	newFile = open("transformed_" + f, "w")
	lastDirectorySeparator = f.rfind(os.sep) 
    	fileExtensionCharacter = f.rfind('.')
	newFile.write("MODULE " + f[lastDirectorySeparator+1:fileExtensionCharacter] + "\n\n")
	newFile.write("use HydraConstants\n\n")
	newFile.write("contains\n\n")

	passedFile = open(f, "r")
	lastLine = None
	for line in passedFile:		
		if line[0] == 'c':
			newFile.write("!" + line[1:])
		elif include_regex.search(line):
			newFile.write("\n")			
		elif "&" in line:
			newFile.write(lastLine[:-1] + " &\n")
			lastLine = line
		else:
			if lastLine:
				if end_regex.search(lastLine):
					newFile.write(lastLine[:-1] + " subroutine\n")
				else:
					newFile.write(lastLine)
			lastLine = line
	passedFile.close()

	newFile.write("END MODULE" + "\n")
	newFile.close()

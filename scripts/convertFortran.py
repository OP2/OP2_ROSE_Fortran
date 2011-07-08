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
	# First work out which lines need the continuation symbol "&" appended

	previousLines   = {}
	appendAmpersand = {}
	index           = 0
	passedFile      = open(f, "r")
	for line in passedFile:
		appendAmpersand[index] = False
		if "&" in line:
			if "#" in previousLines[index-1]:
				appendAmpersand[index-2] = True
				appendAmpersand[index-4] = True
			else:
				appendAmpersand[index-1] = True
		previousLines[index] = line
		index += 1
	passedFile.close()

	# Now output the lines to the file

	newFile = open("transformed_" + f, "w")
	lastDirectorySeparator = f.rfind(os.sep) 
    	fileExtensionCharacter = f.rfind('.')
	newFile.write("MODULE " + f[lastDirectorySeparator+1:fileExtensionCharacter] + "\n\n")
	newFile.write("use HydraConstants\n\n")
	newFile.write("contains\n\n")

	for index, line in previousLines.items():	
		if line[0] == 'c':
			newFile.write("!" + line[1:])
		elif include_regex.search(line):
			newFile.write("\n")
		elif end_regex.search(line):
			newFile.write(line[:-1] + " subroutine\n")			
		elif appendAmpersand[index]:
			newFile.write(line[:-1] + " &\n")
		else:
			newFile.write(line)

	newFile.write("END MODULE" + "\n")
	newFile.close()

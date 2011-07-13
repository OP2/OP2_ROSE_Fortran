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

parser.add_option("-c",
                  "--constants",
                  action="store",
                  type="string",
                  dest="constants",
                  help="The file containing constants.",
                  metavar="<FILENAME>")

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

def modifyFiles (constantsModuleName):
	files = []
	for f in args:
		if not (os.path.exists(f)):
			errorMessage("The file '" + f + "' does not exist")
		else:
			files.append(f)

	include_regex = re.compile("\s*#include \"%s\"" % opts.constants)
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
		newFile.write("use %s\n\n" % constantsModuleName)
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

def modifyConstantsFile (constantsModuleName):
	commonStr    = "common"
	charStr      = "character"
	integerStr   = "integer"
	realStr      = "real"
	logicalStr   = "logical"
	parameterStr = "parameter"

	integerDecl   = False
	realDecl      = False
	charDecl      = False
	logicalDecl   = False
	parameterDecl = False
	commonBlock   = False

	commonDeclarations = {}
	charDeclarations   = {}
	integers           = []
	floats             = []
	doubles            = []
	logicals           = []
	constants          = []

	def handleIntegerDeclaration (line):
		None		
		#print(line)

	def handleRealDeclaration (line):
		None		
		#print(line)

	def handleCharacterDeclaration (line):
		line = line.strip("& ").strip()

		numOfCommas = line.count(',')
		numOfTokens = 0		

		if line[0] == '*':
			# The size n of the character array is declared as "character * n"

			# Skip over all the digits at the start of the string and also find
			# the size of the character array
			i    = 1
			size = ""
			while line[i].isdigit():
				size += line[i]
				i = i + 1

			# Find the character declarations
			tokens = line[i:].split(',')
			for token in tokens:
				if token is not "":
					numOfTokens = numOfTokens + 1
					print(token.strip() + " " + size)
		else:
			# The size n of the character array is declared after the variable name, e.g. "variableName * n"
			tokens = line.split(',')
			for token in tokens:
				if token is not "":
					numOfTokens = numOfTokens + 1
					i     = token.find("*")
					if i != -1:
						size  = token[i+1:].strip()	
						token = token[:i]
						print(token.strip() + " " + size)
					else:
						print(token.strip())

		print("Found " + str(numOfTokens) + " tokens and " + str(numOfCommas) + " token delimiters")
		return numOfCommas == numOfTokens

	def handleLogicalDeclaration (line):
		None		
		#print(line)

	def handleParameterDeclaration (line):
		None		
		#print(line)

	def handleCommonDeclaration (line):
		currentDeclaration = None
		line               = line.strip("& ").strip()
		tokens             = line.split(',')
		numOfCommas        = line.count(',')
		numOfTokens        = 0
	
		for token in tokens:
			token = token.strip()
			if token is not '':
				if currentDeclaration is None:
					i = token.find('(')
					if i == -1:
						numOfTokens = numOfTokens + 1 
						commonDeclarations[token] = None
						verboseMessage(token + " => None")
					else:
						j = token.find(')')
						if j == -1:
							numOfCommas = numOfCommas - 1
							currentDeclaration = token[:i]
							commonDeclarations[currentDeclaration] = token[i:] + ","
						else:
							numOfTokens = numOfTokens + 1 
							commonDeclarations[token[:i]] = token[i:j+1]
							verboseMessage(token[:i] + " => " + commonDeclarations[token[:i]])
				else:
					j = token.find(')')
					if j == -1:
						numOfCommas = numOfCommas - 1
						commonDeclarations[currentDeclaration] += token + ","				
					else:
						numOfTokens = numOfTokens + 1
						commonDeclarations[currentDeclaration] += token
						verboseMessage(currentDeclaration + " => " + commonDeclarations[currentDeclaration])
						currentDeclaration = None

		debugMessage("Found " + str(numOfTokens) + " tokens and " + str(numOfCommas) + " token delimiters")
		return numOfCommas == numOfTokens

	newFile    = open("transformed_" + opts.constants, "w")
	passedFile = open(opts.constants, "r")

	newFile.write("MODULE %s\n\n" % constantsModuleName)
	
	for line in passedFile:
		if line[0] == 'c':
			newFile.write("!" + line[1:])
		else:	
			if commonBlock:
				commonBlock = handleCommonDeclaration(line)
			elif charDecl:
				charDecl = handleCharacterDeclaration (line)

			if commonStr in line:
				i = line.rfind("/")
				commonBlock = handleCommonDeclaration(line[i+1:])
			elif charStr in line:
				i = line.find(charStr)
				charDecl = handleCharacterDeclaration (line[i+len(charStr):])

	newFile.write("END MODULE" + "\n")

	newFile.close()
	passedFile.close()

if not opts.constants:
	errorMessage("You must supply a file containing Fortran constants")

constantsModuleName = "HYDRA_CONSTANTS"

modifyFiles (constantsModuleName)

modifyConstantsFile (constantsModuleName)

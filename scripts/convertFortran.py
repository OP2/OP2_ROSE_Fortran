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
		newFile = open("transformed_" + os.path.basename(f), "w")
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

class CommonDeclarations:

	def __init__ (self):
		self.variableNameToDimensions = {}

	def output (self):		
		print("===== SCALARS =====")
		for key, value in self.variableNameToDimensions.items():
			if value is None:
				print(key)
		print("===== ARRAYS =====")
		for key, value in self.variableNameToDimensions.items():
			if value is not None:
				print(key + " with dimensions " + value[1:-1])

	def isArray (self, variableName):
		if variableName not in self.variableNameToDimensions:
			return False
		else:
			return self.variableNameToDimensions[variableName] is not None

	def getDimension (self, variableName):
		return self.variableNameToDimensions[variableName]

	def extract (self, line):
		currentVariable = None
		line            = line.strip("& ").strip()
		tokens          = line.split(',')
		numOfCommas     = line.count(',')
		numOfTokens     = 0

		for token in tokens:
			token = token.strip()
			if token is not '':
				if currentVariable is None:
					i = token.find('(')
					if i == -1:
						numOfTokens = numOfTokens + 1 
						self.variableNameToDimensions[token] = None
						verboseMessage(token + " => None")
					else:
						j = token.find(')')
						if j == -1:
							numOfCommas = numOfCommas - 1
							currentVariable = token[:i]
							self.variableNameToDimensions[currentVariable] = token[i:] + ","
						else:
							numOfTokens = numOfTokens + 1 
							self.variableNameToDimensions[token[:i]] = token[i:j+1]
							verboseMessage(token[:i] + " => " + self.variableNameToDimensions[token[:i]])
				else:
					j = token.find(')')
					if j == -1:
						numOfCommas = numOfCommas - 1
						self.variableNameToDimensions[currentVariable] += token + ","				
					else:
						numOfTokens = numOfTokens + 1
						self.variableNameToDimensions[currentVariable] += token
						verboseMessage(currentVariable + " => " + self.variableNameToDimensions[currentVariable])
						currentVariable = None

		debugMessage("Found " + str(numOfTokens) + " tokens and " + str(numOfCommas) + " token delimiters")
		return numOfCommas == numOfTokens

class CharacterDeclarations:
	
	def __init__ (self):
		self.currentSize = ""	
		self.variableNameToSize = {}

	def output (self):		
		print("===== CHARACTERS =====")
		for key, value in self.variableNameToSize.items():
			print("Size(" + key + ") = " + value)

	def contains (self, variableName):
		return variableName in self.variableNameToSize

	def getSize (self, variableName):
		return self.variableNameToSize[variableName]

	def extract (self, line):
		line = line.strip("& ").strip()

		numOfCommas = line.count(',')
		numOfTokens = 0		

		if line[0] == '*':
			# The size n of the character array is declared as "character * n"

			# Skip over all the digits at the start of the string and also find
			# the size of the character array
			i = 1
			while line[i].isdigit():
				self.currentSize += line[i]
				i = i + 1

			# Find the character declarations
			tokens = line[i:].split(',')
			for token in tokens:
				if token is not "":
					numOfTokens = numOfTokens + 1
					self.variableNameToSize[token.strip()] = self.currentSize
		else:
			tokens = line.split(',')
			for token in tokens:
				if token is not "":
					numOfTokens = numOfTokens + 1
					i           = token.find("*")
					if i != -1:
						# The size n of the character array is declared after the variable name, e.g. "variableName * n"
						size  = token[i+1:].strip()	
						token = token[:i]
						self.variableNameToSize[token.strip()] = size
					else:
						self.variableNameToSize[token.strip()] = self.currentSize

		debugMessage("Found " + str(numOfTokens) + " tokens and " + str(numOfCommas) + " token delimiters")
		
		finished = numOfCommas == numOfTokens
		if not finished:
			self.currentSize = ""
		return finished

class ParameterDeclarations:
	
	def __init__ (self):
		self.variableNameToValue = {}

	def output (self):
		print("===== PARAMETER =====")
		for key, value in self.variableNameToValue.items():
			print("Value(" + key + ") = " + value)

	def contains (self, variableName):
		return variableName in self.variableNameToValue

	def getValue (self, variableName):
		return self.variableNameToValue[variableName]

	def getValuesOfVariables (self, tokens):
		lastVariable = ""
		i            = 0
		for token in tokens:				
		# Even number tokens represent a variable name		
			if i % 2 == 0:
				lastVariable = token.strip()
			else:
				assert(lastVariable is not "")
				self.variableNameToValue[lastVariable] = token.strip()
			i = i + 1

	def extract (self, line):
		line = line.replace('(', ' ', 1).replace(')', ' ', 1).strip()
		numOfCommas = line.count(',')
		numOfTokens = 0

		if numOfCommas > 0:
			tokensWithEquals = line.split(',')
			for token1 in tokensWithEquals:
				if token1 != "":
					numOfTokens = numOfTokens + 1
					tokens      = token1.strip().split('=')			
					self.getValuesOfVariables (tokens)
		else:
			numOfTokens = 1
			tokens = line.split('=')
			self.getValuesOfVariables(tokens)

		debugMessage("Found " + str(numOfTokens) + " tokens and " + str(numOfCommas) + " token delimiters")
		
		return False

class TypeDeclarations:
	
	def __init__ (self):
		self.currentSize = ""
		self.variableNameToSize = {}

	def output (self, typeName):
		print("===== " + typeName + " =====")
		for key, value in self.variableNameToSize.items():
			if value is not "":
				print("Size(" + key + ") = " + value)
			else:
				print(key)

	def contains (self, variableName):
		return variableName in self.variableNameToSize

	def getSize (self, variableName):
		return self.variableNameToSize[variableName]

	def extract (self, line):
		line = line.strip("& ").strip()

		numOfCommas = line.count(',')
		numOfTokens = 0		

		if line[0] == '*':
			# The size n of the type is declared as "type * n"
			self.currentSize = line[1]
			assert(self.currentSize.isdigit())

			# Find the declarations
			tokens = line[2:].split(',')
			for token in tokens:
				if token is not "":
					numOfTokens = numOfTokens + 1
					self.variableNameToSize[token.strip()] = self.currentSize
		else:
			tokens = line.split(',')
			for token in tokens:
				if token is not "":
					numOfTokens = numOfTokens + 1
					self.variableNameToSize[token.strip()] = self.currentSize

		debugMessage("Found " + str(numOfTokens) + " tokens and " + str(numOfCommas) + " token delimiters")
		
		finished = numOfCommas == numOfTokens
		if not finished:
			self.currentSize = ""
		return finished

def outputConstantsFile (integerDecl, realDecl, logicalDecl, charDecl, parameterDecl, commonDecl):
	newFile = open("transformed_" + os.path.basename(opts.constants), "w")
	newFile.write("MODULE %s\n\n" % constantsModuleName)

	for key, value in integerDecl.variableNameToSize.items():
		newFile.write("INTEGER (KIND=" + value + ")")

		if parameterDecl.contains(key):
			newFile.write(", parameter")

		newFile.write(" :: " + key)

		if commonDecl.isArray(key):
			newFile.write(commonDecl.getDimension(key))

		if parameterDecl.contains(key):
			newFile.write(" = " + parameterDecl.getValue(key))

		newFile.write("\n")

	newFile.write("\n")

	for key, value in realDecl.variableNameToSize.items():
		newFile.write("REAL (KIND=" + value + ")")

		if parameterDecl.contains(key):
			newFile.write(", parameter")

		newFile.write(" :: " + key)

		if commonDecl.isArray(key):
			newFile.write(commonDecl.getDimension(key))

		if parameterDecl.contains(key):
			newFile.write(" = " + parameterDecl.getValue(key))

		newFile.write("\n")

	newFile.write("\n")

	for key, value in charDecl.variableNameToSize.items():
		newFile.write("CHARACTER (LEN=" + value + ")")

		if parameterDecl.contains(key):
			newFile.write(", parameter")

		newFile.write(" :: " + key)

		if commonDecl.isArray(key):
			newFile.write(commonDecl.getDimension(key))

		if parameterDecl.contains(key):
			newFile.write(" = " + parameterDecl.getValue(key))

		newFile.write("\n")

	newFile.write("\n")

	for key, value in logicalDecl.variableNameToSize.items():
		newFile.write("LOGICAL")

		if parameterDecl.contains(key):
			newFile.write(", parameter")

		newFile.write(" :: " + key)

		if commonDecl.isArray(key):
			newFile.write(commonDecl.getDimension(key))

		if parameterDecl.contains(key):
			newFile.write(" = " + parameterDecl.getValue(key))

		newFile.write("\n")

	newFile.write("\nEND MODULE" + "\n")
	newFile.close()

def modifyConstantsFile (constantsModuleName):
	commonStr    = "common"
	charStr      = "character"
	integerStr   = "integer"
	realStr      = "real"
	logicalStr   = "logical"
	parameterStr = "parameter"

	common_regex    = re.compile("\s*%s\s*/" % commonStr)
	char_regex      = re.compile("\s*%s\s*" % charStr)
	integer_regex   = re.compile("\s*%s\s*" % integerStr)
	real_regex      = re.compile("\s*%s\s*" % realStr)
	logical_regex   = re.compile("\s*%s\s*" % logicalStr)
	parameter_regex = re.compile("\s*%s\s*\(" % parameterStr)

	commonBlock    = False
	integerBlock   = False
	realBlock      = False
	charBlock      = False
	logicalBlock   = False
	parameterBlock = False

	commonDecl    = CommonDeclarations ()
	charDecl      = CharacterDeclarations ()
	integerDecl   = TypeDeclarations ()
	realDecl      = TypeDeclarations ()
	logicalDecl   = TypeDeclarations ()
	parameterDecl = ParameterDeclarations ()

	passedFile = open(opts.constants, "r")
	
	for line in passedFile:
		if line[0] == 'c':
			# Ignore Fortran comments
			pass
		else:	
			if commonBlock:
				commonBlock = commonDecl.extract(line)
			elif charBlock:
				charBlock = charDecl.extract(line)
			elif integerBlock:
				integerBlock = integerDecl.extract(line)
			elif realBlock:
				realBlock = realDecl.extract(line)
			elif logicalBlock:
				logicalBlock = logicalDecl.extract(line)
			elif parameterBlock:
				parameterBlock = parameterDecl.extract(line)
			else:
				if common_regex.match(line):
					i = line.rfind("/")
					commonBlock = commonDecl.extract(line[i+1:])
				elif char_regex.match(line):
					i = line.find(charStr)
					charBlock = charDecl.extract(line[i+len(charStr):])
				elif integer_regex.match(line):
					i = line.find(integerStr)
					integerBlock = integerDecl.extract(line[i+len(integerStr):])
				elif real_regex.match(line):
					i = line.find(realStr)
					realBlock = realDecl.extract(line[i+len(realStr):])
				elif logical_regex.match(line):
					i = line.find(logicalStr)
					logicalBlock = logicalDecl.extract(line[i+len(logicalStr):])
				elif parameter_regex.match(line):
					i = line.find(parameterStr)
					parameterBlock = parameterDecl.extract(line[i+len(parameterStr):])

	passedFile.close()

	if opts.debug:
		commonDecl.output()
		charDecl.output()
		integerDecl.output("INTEGER")
		realDecl.output("REAL")
		logicalDecl.output("LOGICAL")
		parameterDecl.output()

	outputConstantsFile (integerDecl, realDecl, logicalDecl, charDecl, parameterDecl, commonDecl)

if not opts.constants:
	errorMessage("You must supply a file containing Fortran constants")

constantsModuleName = "HYDRA_CONSTANTS"

modifyFiles (constantsModuleName)

modifyConstantsFile (constantsModuleName)

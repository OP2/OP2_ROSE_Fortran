import glob
import os
import re
from Graph import Graph

def getBaseFileName (fileName):
    lastDirectorySeparator = fileName.rfind(os.sep) 
    fileExtensionCharacter = fileName.rfind('.')
    return fileName[lastDirectorySeparator+1:fileExtensionCharacter]

def determineModuleDependencies (fileList):
	# The graph models the dependencies and is returned by this function
	g = Graph()

	# Mapping from a module name to its file name
	moduleNameToFileName = {}
    
	module_line_regex = re.compile("^\s*module\s+(\S+)\s*$", re.IGNORECASE)
	program_line_regex = re.compile("^\s*program\s+(\S+)\s*$", re.IGNORECASE)
	use_line_regex = re.compile("^\s*use", re.IGNORECASE)              

	for fileName in fileList:      
		g.addVertex(fileName)
		f = open(fileName)  

        	for line in f:
			if module_line_regex.search(line) or program_line_regex.search(line):
				lexemes    = line.split()
				moduleName = lexemes[len(lexemes) - 1]
				moduleNameToFileName[moduleName] = fileName
				print("Module '%s' in file '%s'" % (moduleName, fileName))
		f.close()
        
	for fileName in fileList:
		f = open(fileName)	
		for line in f:
			if use_line_regex.search(line):
			        lexemes = line.split()
			        moduleName = lexemes[len(lexemes) - 1]
				if moduleName in moduleNameToFileName:
					print("Adding edge %s to %s" % (moduleNameToFileName[moduleName], fileName))
					g.addEdge(moduleNameToFileName[moduleName], fileName)            
		f.close()

	return g

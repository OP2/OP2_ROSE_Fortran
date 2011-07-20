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
    
    for fileName in fileList:      
        g.addVertex(fileName)
        file = open(fileName)
        module_line_regex = re.compile("^\s*module\s+(\S+)\s*$", re.IGNORECASE)
        
        for line in file:
            if module_line_regex.search(line):
                lexemes = line.split()
                moduleNameToFileName[lexemes[len(lexemes) - 1]] = fileName
        file.close()
        
    for fileName in fileList:    
        file = open(fileName)
        use_line_regex = re.compile("^\s*use", re.IGNORECASE)        
        
        for line in file:
            if use_line_regex.search(line):
                lexemes = line.split()
                moduleName = lexemes[len(lexemes) - 1]
                if moduleName in moduleNameToFileName:
                    g.addEdge(moduleNameToFileName[moduleName], fileName)            
        file.close()
    
    return g

class FortranDependencies ():
	def __init__ (self, files):
		from Graph import Graph

        	self.files = files

		# The graph models the dependencies and is returned by this function
		self.g = Graph()

		self.__compute__()

	def getDependencyGraph (self):
		return self.g

	def __compute__ (self):
		from re import IGNORECASE, compile

		# Mapping from a module name to its file name
		moduleNameToFileName = {}
	    
		module_line_regex = compile("^\s*module\s+(\S+)\s*$", IGNORECASE)
		program_line_regex = compile("^\s*program\s+(\S+)\s*$", IGNORECASE)
		use_line_regex = compile("^\s*use", IGNORECASE)              

		for fileName in self.files:      
			self.g.addVertex(fileName)

			f = open(fileName)  
			for line in f:
				if module_line_regex.search(line) or program_line_regex.search(line):
					lexemes    = line.split()
					moduleName = lexemes[len(lexemes) - 1]
					moduleNameToFileName[moduleName] = fileName
					print("Module '%s' in file '%s'" % (moduleName, fileName))
			f.close()
		
		for fileName in self.files:
			f = open(fileName)	
			for line in f:
				if use_line_regex.search(line):
					lexemes = line.split()
					moduleName = lexemes[len(lexemes) - 1]
					if moduleName in moduleNameToFileName:
						print("Adding edge %s to %s" % (moduleNameToFileName[moduleName], fileName))
						self.g.addEdge(moduleNameToFileName[moduleName], fileName)            
			f.close()

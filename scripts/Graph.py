# Written by Adam Betts
#
# Very simple graph API to add dependencies between
# Fortran modules 

from Vertex import Vertex

dummyFileName = "dummy" 

class Graph():    
    def __init__ (self):
        self.vertices = {}
    
    def getVertices (self):
        vertices = [] 
        for key in self.vertices:
            vertices.append(self.vertices[key])
        return vertices
        
    def addVertex (self, fileName):
        v = Vertex(fileName)
        self.vertices[fileName] = v
        
    def getVertex (self, fileName):
        return self.vertices[fileName]
    
    def addEdge (self, predName, succName):
        p = self.vertices[predName]
        s = self.vertices[succName]
        p.addSuccessor(succName)
        s.addPredecessor(predName)
    
    def addDummyStartVertex (self):
        dummyv = Vertex(dummyFileName)
        self.vertices[dummyFileName] = dummyv
        
        for key in self.vertices:
            v = self.vertices[key]
            if v.numberOfPredecessors() == 0 and v.getFileName() != dummyFileName:
                self.addEdge(dummyFileName, v.getFileName())
                
    def visit (self, visited, sort, v):
        visited[v.getFileName()] = True
        for s in v.getSuccessors():
            if not visited[s]:
                self.visit(visited, sort, self.getVertex(s))
                
        if v.getFileName() != dummyFileName:
            sort.insert(0,v.getFileName())
                
    def getTopologicalSort (self):
        self.addDummyStartVertex()
        sort    = []
        visited = {}
        for key in self.vertices:
            visited[key] = False
        
        self.visit(visited, sort, self.getVertex(dummyFileName))   
        
        return sort 
    

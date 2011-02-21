# Written by Adam Betts
#
# Very simple vertex API for graphs 

class Vertex():
    def __init__ (self, fileName):
        self.fileName = fileName
        self.predecessors = []
        self.successors = []
    
    def getFileName (self):
        return self.fileName
     
    def addSuccessor (self, succName):
        self.successors.append(succName)
        
    def addPredecessor (self, predName):
        self.predecessors.append(predName)
    
    def numberOfSuccessors (self):
        return len(self.successors)
    
    def numberOfPredecessors (self):
        return len(self.predecessors)
        
    def getSuccessors (self):
        return self.successors
    
    def getPredecessors (self):
        return self.predecessors

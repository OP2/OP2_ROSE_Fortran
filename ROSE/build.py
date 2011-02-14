import glob
import os
import re
import sys
import string
from subprocess import Popen

def getNewDirectory (msg):
	return raw_input(msg)

roseVersion       = glob.glob(os.getcwd() + "/*[0-9]?")
defaultBuildDir   = roseVersion[0] + "_build"
defaultInstallDir = roseVersion[0] + "_inst"

print("\n======================================================================")
print("Will put ROSE build files into '%s'." % (defaultBuildDir))
print("Will put ROSE installation files into '%s'." % (defaultInstallDir))
print("Do you want to change these paths?")

possibleAnswers = ("y", "yes", "n", "no") 
answer = None
while True: 
    answer = raw_input("Please enter '[Y]es' or '[N]o?': ")  
    if answer.lower() not in possibleAnswers[:]: 
		print("Invalid answer.")
    else:
		break
        
if answer in possibleAnswers[:2]:
	defaultBuildDir   = getNewDirectory("Enter path where to build ROSE: ")	
	defaultInstallDir = getNewDirectory("Enter path where to install ROSE: ")

if not os.path.exists(defaultBuildDir):
	os.mkdir(defaultBuildDir)
if not os.path.exists(defaultInstallDir):
	os.mkdir(defaultInstallDir)
	
boostInstallDir = None
pattern = re.compile("boost")
LD_LIBRARY_PATH = string.split( os.environ.get("LD_LIBRARY_PATH"), ':')
for path in LD_LIBRARY_PATH:
	if pattern.search(path):
		boostInstallDir = path
		
if boostInstallDir is None:
	print("Could not find BOOST in your LD_LIBRARY_PATH")
	sys.exit(1)
	
configureString  = """%s/configure --prefix=%s --with-boost=%s --enable-cuda --enable-opencl --with-java --without-haskell --with-CXX_DEBUG=-g --with-CXX_OPTIMIZE=-O0""" % (roseVersion[0], defaultInstallDir, boostInstallDir[:-4]) 
					
configureCommand = Popen(args=configureString, cwd=defaultBuildDir, shell='/bin/bash') 
if configureCommand.wait() != 0:
	print("The configure command '%s' failed" % (configureString))
	sys.exit(1)

makeCommand = Popen(args='make', cwd=defaultBuildDir+'/src', shell='/bin/bash') 
if makeCommand.wait() != 0:
	print("Command 'make' failed")
	sys.exit(1)
	
makeInstallCommand = Popen(args='make install', cwd=defaultBuildDir+'/src', shell='/bin/bash') 
if makeCommand.wait() != 0:
	print("Command 'make install' failed")
	sys.exit(1)

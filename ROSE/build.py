import glob
import os
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
	
configureString  = "../configure"
configureCommand = Popen(args='ls -al', cwd=defaultBuildDir, shell='/bin/bash') 
if configureCommand.wait() != 0:
	print("The configure command '%s' failed" % (configureString))
import glob
import os
import re
import sys
import string
from subprocess import Popen

# There might be several supported versions of ROSE in the repository.
# In which case it is the user's responsibility to select the version to install 
selectedRoseVersion = None
roseVersions = glob.glob(os.getcwd() + "/*[0-9]?")
if len(roseVersions) > 1:
	print("======================================================================")
	print("Detected the following ROSE versions:")
	for i, version in enumerate(roseVersions):
		print("%s) %s" % (i, version))
	while True:
		try:
			answer = int(raw_input("Please choose which to build and install: "))
			if answer > len(roseVersions) - 1 or answer < 0:
				print("Invalid answer. Choose between 0..%s" % (len(roseVersions) - 1))
			else:
				selectedRoseVersion = roseVersions[answer]
				break;
		except ValueError:
			print("Invalid answer. Choose an integer.")
else:
	selectedRoseVersion = roseVersions[0]

# These are the ROSE build and installation directories
defaultBuildDir = selectedRoseVersion + "_build"
defaultInstallDir = selectedRoseVersion + "_inst"

print("\n======================================================================")
print("Will put ROSE build files into '%s'." % (defaultBuildDir))
print("Will put ROSE installation files into '%s'." % (defaultInstallDir))
print("Do you want to change these paths?")

# See if the user wants to change the default build and installation directories
possibleAnswers = ("y", "yes", "n", "no") 
answer = None
while True: 
    answer = raw_input("Please enter '[Y]es' or '[N]o?': ")  
    if answer.lower() not in possibleAnswers[:]: 
		print("Invalid answer.")
    else:
		break
        
if answer in possibleAnswers[:2]:
	defaultBuildDir = raw_input("Enter path where to build ROSE: ")	
	defaultInstallDir = raw_input("Enter path where to install ROSE: ")

# Make the build and installation directories if they do not exist 
if not os.path.exists(defaultBuildDir):
	os.mkdir(defaultBuildDir)
if not os.path.exists(defaultInstallDir):
	os.mkdir(defaultInstallDir)

# Try to find the BOOST library as this is needed as a configure option to ROSE
boostInstallDir = None
pattern = re.compile("boost")
LD_LIBRARY_PATH = string.split(os.environ.get("LD_LIBRARY_PATH"), os.pathsep)
for path in LD_LIBRARY_PATH:
	if pattern.search(path):
		boostInstallDir = path
		
if boostInstallDir is None:
	print("Could not find BOOST in your LD_LIBRARY_PATH")
	sys.exit(1)
	
configureString = "%s/configure \
--prefix=%s \
--with-boost=%s \
--enable-cuda \
--enable-opencl \
--with-java \
--without-haskell \
--with-CXX_DEBUG=-g \
--with-CXX_OPTIMIZE=-O0" % (selectedRoseVersion, defaultInstallDir, boostInstallDir[:-4]) 

# Run the ROSE configure command in the build directory
print("====== CONFIGURE ======\n")
configureCommand = Popen(args=configureString, cwd=defaultBuildDir, shell="/bin/bash") 
if configureCommand.wait() != 0:
	print("The configure command '%s' failed" % (configureString))
	sys.exit(1)

# Run the 'make' command in the 'src' tree of the build directory
print("====== MAKE ======\n")
makeCommand = Popen(args="make", cwd=defaultBuildDir + os.sep + "src", shell="/bin/bash") 
if makeCommand.wait() != 0:
	print("Command 'make' failed")
	sys.exit(1)

# Run the 'make install' command in the 'src' tree of the build directory
print("====== MAKE INSTALL ======\n")
makeInstallCommand = Popen(args="make install", cwd=defaultBuildDir + os.sep + "src", shell="/bin/bash") 
if makeCommand.wait() != 0:
	print("Command 'make install' failed")
	sys.exit(1)

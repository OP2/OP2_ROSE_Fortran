import glob
import os
import re
import sys
import string
from subprocess import Popen, PIPE

LD_LIBRARY_PATH = string.split(os.environ.get("LD_LIBRARY_PATH"), os.pathsep)

def selectROSEVersion ():
	# There might be several supported versions of ROSE in the repository.
	# In which case it is the user's responsibility to select the version to install 
	roseVersions  = glob.glob(os.getcwd() + "/*[0-9]?")
	
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
	return selectedRoseVersion

def getBoostPath ():
	boostInstallDirectory = None
	boostPattern          = re.compile("boost")
	
	for path in LD_LIBRARY_PATH:
		if boostPattern.search(path):
			boostInstallDirectory = path

	if boostInstallDirectory is None:
		print("Could not find BOOST in your LD_LIBRARY_PATH")
		sys.exit(1)
	
	return boostInstallDirectory

def checkEnvironment ():
	print("====== CHECKING G++ VERSION ======")
	
	proc = Popen("g++ --version",
	     	     shell=True,
             	     executable='/bin/bash',
             	     stderr=PIPE,
             	     stdout=PIPE)

	stdoutLines, stderrLines = proc.communicate()

	versionNumberPattern = re.compile("[0-9]\.[0-9]\.[0-9]$")
	
	for line in stdoutLines.splitlines():
		tokens = line.split()
		for token in tokens:
			if versionNumberPattern.match(token):
				numbers = token.split('.')
				majorVersion = int(numbers[0])
				minorVersion = int(numbers[1])
				
				if majorVersion != 4:
					print("Major version of g++ has to be 4 to successfully compile ROSE. Yours is currently '%s'" % numbers[0])
					sys.exit(1)

				if minorVersion > 4:
					print("Minor version of g++ has to be between 0 and 4 to successfully compile ROSE. Yours is currently '%s'" % numbers[1])
					sys.exit(1)
				
				print("g++ version %s passes" % token)
				break

def buildROSE (ROSEVersion, boostInstallDirectory):
	ROSEBuildDirectory   = ROSEVersion + "_build" 
	ROSEInstallDirectory = ROSEVersion + "_inst"

	if not os.path.exists(ROSEBuildDirectory):
		os.mkdir(ROSEBuildDirectory)

	if not os.path.exists(ROSEInstallDirectory):
		os.mkdir(ROSEInstallDirectory)

	print("Build ROSE in '%s'" % ROSEBuildDirectory)	
	print("Installing ROSE in '%s'" % ROSEInstallDirectory)

	configureString = "%s/configure --prefix=%s --with-boost=%s --enable-cuda --enable-edg-cuda --enable-opencl --enable-edg-opencl --with-java --without-haskell" % (ROSEVersion, ROSEInstallDirectory, boostInstallDirectory[:-4])

	print("Configuring ROSE with '%s'" % configureString)

	# Run the ROSE configure command in the build directory
	print("====== CONFIGURE ======")
	configureCommand = Popen(args=configureString, cwd=ROSEBuildDirectory, shell="/bin/bash") 
	if configureCommand.wait() != 0:
		print("The configure command '%s' failed" % (configureString))
		sys.exit(1)

	# Run the 'make' command in the 'src' tree of the build directory
	print("====== MAKE ======")
	makeCommand = Popen(args="make", cwd=ROSEBuildDirectory + os.sep + "src", shell="/bin/bash") 
	if makeCommand.wait() != 0:
		print("Command 'make' failed")
		sys.exit(1)

	# Run the 'make install' command in the 'src' tree of the build directory
	print("====== MAKE INSTALL ======")
	makeInstallCommand = Popen(args="make install", cwd=ROSEBuildDirectory + os.sep + "src", shell="/bin/bash") 
	if makeCommand.wait() != 0:
		print("Command 'make install' failed")
		sys.exit(1)

	print("====== MY WORK IS DONE ======")

# Main
checkEnvironment()
ROSEVersion           = selectROSEVersion ()
boostInstallDirectory = getBoostPath ()
buildROSE (ROSEVersion, boostInstallDirectory)

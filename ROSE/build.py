import sys
import os
import tarfile
import glob
import re
import string
import shutil

from optparse import OptionParser
from subprocess import Popen, PIPE

sys.path.append(os.path.dirname(os.getcwd()) + os.sep + 'scripts' + os.sep + 'src')
from Debug import Debug

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

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

(opts, args) = parser.parse_args(sys.argv[1:])

debug = Debug(opts.verbose)

def checkEnvironment ():
	debug.verboseMessage("Checking G++ version")
	
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
					debug.exitMessage("Major version of g++ has to be 4 to successfully compile ROSE. Yours is currently '%s'" % numbers[0])

				if minorVersion > 4:
					debug.exitMessage("Minor version of g++ has to be between 0 and 4 to successfully compile ROSE. Yours is currently '%s'" % numbers[1])
				
				debug.verboseMessage("g++ version %s passes" % token)
				break

def getBoostPath ():	
	LD_LIBRARY_PATH = string.split(os.environ.get("LD_LIBRARY_PATH"), os.pathsep)
	boostDirectory  = None
	boostPattern    = re.compile("boost")
	
	for path in LD_LIBRARY_PATH:
		if boostPattern.search(path):
			boostDirectory = path

	if boostDirectory is None:
		debug.exitMessage("Unable to find BOOST in your LD_LIBRARY_PATH")
	
	return boostDirectory

def selectROSEVersion ():
	debug.verboseMessage("Selecting ROSE tarball")

	tarballs = glob.glob('rose*.tar.gz')
	if len(tarballs) == 0:
		debug.exitMessage("Unable to find ROSE tarballs")
	
	chosenTarball = None
	if len(tarballs) > 1:
		print("Detected the following ROSE tarballs:")
		for i, file in enumerate(tarballs):
			print("%s) %s" % (i, file))
		while True:
			try:
				answer = int(raw_input("Please choose which to build and install: "))
				if answer > len(tarballs) - 1 or answer < 0:
					print("Invalid answer. Choose between 0..%s" % (len(tarballs) - 1))
				else:
					chosenTarball = tarballs[answer]
					break;
			except ValueError:
				print("Invalid answer. Choose an integer.")
	else:
		chosenTarball = tarballs[0]
	return chosenTarball

def extractTarball (chosenTarball):
	debug.verboseMessage("Uncompressing tarball '%s'" % chosenTarball)
	tarball = tarfile.open(chosenTarball)
	tarball.extractall()
	tarball.close()

	debug.verboseMessage("Obtaining extracted ROSE directory name")
	tarExtension  = ".tar.gz"
	tokens        = chosenTarball.split('-')
	roseVersion   = tokens[len(tokens) - 1]
	roseDirectory = tokens[0] + "-" + tokens[1] + "-" + roseVersion[:-len(tarExtension)]
	
	if not os.path.isdir(roseDirectory):
		debug.exitMessage("Unable to find extracted ROSE directory '%s" % roseDirectory)
	
	return roseDirectory

def getRoseDirectories (roseDirectory, create=False):
	roseDirectoryBuild   = roseDirectory + "_build"
	roseDirectoryInstall = roseDirectory + "_inst"

	if create:
		if not os.path.exists(roseDirectoryBuild):
			debug.verboseMessage("Making directory '%s'" % roseDirectoryBuild)	
			os.mkdir(roseDirectoryBuild)
		if not os.path.exists(roseDirectoryInstall):
			debug.verboseMessage("Making directory '%s'" % roseDirectoryInstall)
			os.mkdir(roseDirectoryInstall)

	return roseDirectoryBuild, roseDirectoryInstall 

def copyModifiedROSEFiles (roseDirectory):
	imperialDirectory = "ImperialModifications"
	fileLocationsFile = "fileLocations.txt"
	f = open(os.getcwd() + os.sep + imperialDirectory + os.sep + fileLocationsFile, 'r')
	for line in f:
		tokens      = line.split('=')
		sourceFile  = os.getcwd() + os.sep + imperialDirectory + os.sep + tokens[0]
		destination = os.getcwd() + os.sep + roseDirectory + os.sep + tokens[1].strip() + os.sep + tokens[0]
		debug.verboseMessage("Moving Imperial changed file '%s' into '%s'" % (sourceFile, destination))
		shutil.copy(sourceFile, destination)
	f.close()

def buildROSE (roseDirectory, boostDirectory):
	roseDirectoryBuild, roseDirectoryInstall = getRoseDirectories(roseDirectory, True)

	debug.verboseMessage("Build ROSE in '%s'" % roseDirectoryBuild)	
	debug.verboseMessage("Installing ROSE in '%s'" % roseDirectoryInstall)

	configureString = "%s/configure --prefix=%s --with-boost=%s --enable-static --enable-cuda --enable-edg-cuda --enable-opencl --enable-edg-opencl --with-java --without-haskell" % (os.getcwd() + os.sep + roseDirectory, os.getcwd() + os.sep + roseDirectoryInstall, boostDirectory)

	debug.verboseMessage("Configuring ROSE with command '%s'" % (configureString))

	# Run the ROSE configure command in the build directory
	proc = Popen(args=configureString, 
		     cwd=roseDirectoryBuild, 
		     shell="/bin/bash",
		     stderr=PIPE,
	             stdout=PIPE) 
			
	proc.communicate()
	if proc.returncode != 0:
		debug.exitMessage("The configure command '%s' failed" % (configureString))

	# Run the 'make' command in the 'src' tree of the build directory
	makeString = "make -j 4"

	debug.verboseMessage("Building ROSE with command '%s'" % (makeString))

	proc = Popen(args=makeString, 
		     cwd=roseDirectoryBuild + os.sep + "src", 
		     shell="/bin/bash",
		     stderr=PIPE,
		     stdout=PIPE)
 
	proc.communicate()
	if proc.returncode != 0:
		debug.exitMessage("Command '%s' failed" % makeString)

	# Run the 'make install' command in the 'src' tree of the build directory
	makeInstallString = "make install"

	debug.verboseMessage("Installing ROSE with command '%s'" % (makeInstallString))

	proc = Popen(args=makeInstallString, 
		     cwd=roseDirectoryBuild + os.sep + "src", 
		     shell="/bin/bash",
		     stderr=PIPE,
		     stdout=PIPE)
 
	proc.communicate()
	if proc.returncode != 0:
		debug.exitMessage("Command '%s' failed" % makeInstallString)

def copyRosePublicConfigHeader (roseDirectory):
	roseDirectoryBuild, roseDirectoryInstall = getRoseDirectories(roseDirectory)
	sourceFile          = os.getcwd() + os.sep + roseDirectory + os.sep + "rosePublicConfig.h"
	destinatonDirectory = os.getcwd() + os.sep + roseDirectoryInstall + os.sep + "include"
	debug.verboseMessage("Moving file '%s' into '%s'" % (sourceFile, destinatonDirectory))
	shutil.copy(sourceFile, destinatonDirectory)

# Main
checkEnvironment()
boostDirectory = getBoostPath ()
tarball        = selectROSEVersion ()
roseDirectory  = extractTarball (tarball)
copyModifiedROSEFiles (roseDirectory)
buildROSE (roseDirectory, boostDirectory[:-4])
copyRosePublicConfigHeader (roseDirectory)

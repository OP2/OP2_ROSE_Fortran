import sys
import os
import tarfile
import glob
import re
import string
import shutil
import hashlib
import platform

from optparse import OptionParser
from subprocess import Popen, PIPE

sys.path.append(os.path.dirname(os.getcwd()) + os.sep + 'scripts' + os.sep + 'src')
from Debug import Debug

# The name of the sub-directory to the directory 'ROSE' where changes to ROSE have been implemented by Imperial College
# and the name of the file containing file-location pairs
imperialDirectory = "ImperialModifications"
fileLocationsFile = "fileLocations.txt"

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

parser.add_option("-b",
                 "--build",
                 action="store_true",
                 dest="build",
                 help="Build ROSE from scratch (i.e. the tarball).",
                 default=False)

parser.add_option("-c",
                 "--check",
                 action="store_true",
                 dest="check",
                 help="Check environment is suitable for ROSE build and installation.",
                 default=False)

parser.add_option("-h",
                  "--help",
                  action="help",
                  help="Display this help message.")

parser.add_option("-u",
                 "--update",
                 action="store_true",
                 dest="update",
                 help="Update the ROSE source tree with local modifications found in '%s' and re-compile ROSE." % imperialDirectory,
                 default=False)

parser.add_option("-v",
                 "--verbose",
                 action="store_true",
                 dest="verbose",
                 help="Be verbose.",
                 default=False)

(opts, args) = parser.parse_args(sys.argv[1:])

debug = Debug(opts.verbose)

def queryYesNo(question, default="yes"):
    valid = {"yes":"yes", "y":"yes", "ye":"yes", "no":"no","n":"no"}
    if default == None:
        prompt = " [y/n] "
    elif default == "yes":
        prompt = " [Y/n] "
    elif default == "no":
        prompt = " [y/N] "
    else:
        raise ValueError("Invalid default answer: '%s'" % default)

    while True:
        sys.stdout.write("=== %s ===" % question + prompt)
        choice = raw_input().lower()
        if default is not None and choice == '':
            return default
        elif choice in valid.keys():
            return valid[choice]
        else:
            sys.stdout.write("Please respond with 'yes/y' or 'no/n'.\n")

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
    osLibraryPathString = None
    if platform.system() == "Darwin":
        debug.verboseMessage("Darwin OS detected")
        osLibraryPathString = "DYLD_LIBRARY_PATH"
    else:
        debug.verboseMessage("Linux OS assumed")
        osLibraryPathString = "LD_LIBRARY_PATH"

    osLibraryPath = os.environ.get(osLibraryPathString)
    if not osLibraryPath:
        debug.exitMessage("%s has not been set in your environment. Set it to contain your Boost libraries." % osLibraryPathString)
    LD_LIBRARY_PATH = string.split(osLibraryPath, os.pathsep)
    boostDirectory  = None
    boostPattern    = re.compile("libboost")

    try:
        for path in LD_LIBRARY_PATH:
            for file in os.listdir(path):
                if boostPattern.match(file):
                    boostDirectory = path
                    raise StopIteration ()
    except StopIteration:
        pass

    translatorRequiredLibraries = ['libboost_filesystem.so', 'libboost_system.so']
    for libraryName in translatorRequiredLibraries:
        match  = False
        for file in os.listdir(boostDirectory):
            if libraryName == file:
                match = True
        if match:
            debug.verboseMessage("Found Boost library '%s' needed to compile source-to-source translator" % libraryName)
        else:
            debug.exitMessage("Unable to find Boost library '%s' in your Boost libraries. This is needed to compile source-to-source translator" % libraryName)

    if boostDirectory is None:
        debug.exitMessage("Unable to find BOOST in your Library Path '%s'" % osLibraryPathString)
    else:
        debug.verboseMessage("Boost libraries found in '%s'" % boostDirectory)

    return boostDirectory

def selectOption (options, msg):
    sys.stdout.write("=== %s ===\n" % msg)
    for i, option in enumerate(options):
        sys.stdout.write("%s) %s\n" % (i, option))
    while True:
        try:
            answer = int(raw_input())
            if answer > len(options) - 1 or answer < 0:
                sys.stdout.write("Invalid answer. Choose between 0..%s.\n" % (len(options) - 1))
            else:
                return answer
                break;
        except ValueError:
            sys.stdout.write("Invalid answer. Choose an integer.\n")

def selectROSETarball ():
    tarballs = glob.glob('rose*.tar.gz')
    if len(tarballs) == 0:
        debug.exitMessage("Unable to find ROSE tarballs")

    answer = 0
    if len(tarballs) > 1:
        answer = selectOption(tarballs, "Please choose which tarball to build and install")
    return tarballs[answer]

def selectROSEVersion ():
    directories = glob.glob('rose-[0-9].[0-9].[0-9]a-[0-9][0-9]*[!_a-zA-Z]')
    if len(directories) == 0:
        debug.exitMessage("Unable to find ROSE installation")

    answer = 0
    if len(directories) > 1:
        answer = selectOption(directories, "Please choose which ROSE version you want to update")
    return directories[answer]

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
    fullPath = os.getcwd() + os.sep + imperialDirectory + os.sep + fileLocationsFile
    fileListFile = open(fullPath, 'r')
    for line in fileListFile:
        tokens              = line.split('=')
        sourceFileName      = os.getcwd() + os.sep + imperialDirectory + os.sep + tokens[0]
        destinationFileName = os.getcwd() + os.sep + roseDirectory + os.sep + tokens[1].strip() + os.sep + tokens[0]
        debug.verboseMessage("Moving Imperial changed file '%s' into '%s'" % (sourceFileName, destinationFileName))
        shutil.copy(sourceFileName, destinationFileName)
    fileListFile.close()

def configureROSE (roseDirectory, boostDirectory):
    roseDirectoryBuild, roseDirectoryInstall = getRoseDirectories(roseDirectory, True)

    debug.verboseMessage("Build ROSE in '%s'" % roseDirectoryBuild)
    debug.verboseMessage("Installing ROSE in '%s'" % roseDirectoryInstall)

    configureString = "%s/configure --prefix=%s --with-boost=%s --enable-cuda --enable-edg-cuda --enable-opencl --enable-edg-opencl --with-java --without-haskell" % (os.getcwd() + os.sep + roseDirectory, os.getcwd() + os.sep + roseDirectoryInstall, boostDirectory)

    if queryYesNo("Shall I build a STATIC library for ROSE? (This is not strictly needed and will consume A LOT of extra space.)", "no") == "yes":
	configureString = " --enable-static" 

    debug.verboseMessage("Configuring ROSE with command '%s'" % (configureString))

    # Run the ROSE configure command in the build directory
    proc = Popen(args=configureString,
             cwd=roseDirectoryBuild,
             shell="/bin/bash",
             stderr=PIPE,
                 stdout=PIPE)

    proc.communicate()
    if proc.returncode != 0:
        debug.exitMessage("The configure command '%s' failed\nCheck '%s/config.log' for errors." % (configureString, roseDirectoryBuild))

def makeROSE (roseDirectory):
    roseDirectoryBuild, roseDirectoryInstall = getRoseDirectories(roseDirectory)

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

def makeInstallROSE (roseDirectory):
    roseDirectoryBuild, roseDirectoryInstall = getRoseDirectories(roseDirectory)

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

def installAdditionalROSEHeaderFiles (roseDirectory):
    roseDirectoryBuild, roseDirectoryInstall = getRoseDirectories(roseDirectory)

    makeInstallCommand1 = "make install-includeHEADERS"

    debug.verboseMessage("Installing rosePublicConfig.h with command '%s'" % (makeInstallCommand1))

    proc = Popen(args=makeInstallCommand1,
             cwd=roseDirectoryBuild,
             shell="/bin/bash",
             stderr=PIPE,
             stdout=PIPE)

    proc.communicate()
    if proc.returncode != 0:
        debug.exitMessage("Command '%s' failed" % makeInstallCommand1)

    makeInstallCommand2 = "make install-data-local"

    debug.verboseMessage("Installing GCC/CUDA/OpenCL headers with command '%s'" % (makeInstallCommand2))

    proc = Popen(args=makeInstallCommand2,
             cwd=roseDirectoryBuild,
             shell="/bin/bash",
             stderr=PIPE,
             stdout=PIPE)

    proc.communicate()
    if proc.returncode != 0:
        debug.exitMessage("Command '%s' failed" % makeInstallCommand2)


def computeChecksum (fileName):
    sourceFile = open(fileName, 'r')
    md5        = hashlib.md5()
    while True:
        data = sourceFile.read(128)
        if not data:
            break
        md5.update(data)
    sourceFile.close()
    return md5.hexdigest()

def copyFilesIfNeeded (roseDirectory):
    makeNeeded   = False
    fullPath     = os.getcwd() + os.sep + imperialDirectory + os.sep + fileLocationsFile
    fileListFile = open(fullPath, 'r')
    for line in fileListFile:
        tokens              = line.split('=')
        sourceFileName      = os.getcwd() + os.sep + imperialDirectory + os.sep + tokens[0]
        destinationFileName = os.getcwd() + os.sep + roseDirectory + os.sep + tokens[1].strip() + os.sep + tokens[0]

        if not os.path.exists(sourceFileName):
            debug.exitMessage("The source file '%s' does not exist. Check that this file exists in the git repository." % sourceFileName)

        if not os.path.exists(destinationFileName):
            debug.exitMessage("The destination file '%s' does not exist. Check that this ROSE revision has been decompressed and exists in this location." % destinationFileName)

        sourceMD5           = computeChecksum(sourceFileName)
        destinationMD5      = computeChecksum(destinationFileName)
        debug.debugMessage("MD5 of '%s' is '%s'" % (sourceFileName, sourceMD5), 1)
        debug.debugMessage("MD5 of '%s' is '%s'" % (destinationFileName, destinationMD5), 1)

        if sourceMD5 != destinationMD5:
            debug.verboseMessage("MD5s differ: moving Imperial changed file '%s' into '%s'" % (sourceFileName, destinationFileName))
            shutil.copy(sourceFileName, destinationFileName)
            makeNeeded = True
    fileListFile.close()
    return makeNeeded

def rebuildTranslator ():
    # Run the 'scons' command in the 'translator' directory
    sconsString = "scons -j 6"

    debug.verboseMessage("Rebuilding the translator with '%s'" % (sconsString))

    proc = Popen(args=sconsString,
             cwd=os.path.dirname(os.getcwd()) + os.sep + "translator",
             shell="/bin/bash",
             stderr=PIPE,
             stdout=PIPE)

    proc.communicate()
    if proc.returncode != 0:
        debug.exitMessage("Command '%s' failed" % sconsString)

def buildAction ():
    checkEnvironment ()
    boostDirectory = getBoostPath ()
    tarball        = selectROSETarball ()
    roseDirectory  = extractTarball (tarball)
    copyModifiedROSEFiles (roseDirectory)
    configureROSE (roseDirectory, boostDirectory[:-4])
    makeROSE (roseDirectory)
    makeInstallROSE (roseDirectory)
    installAdditionalROSEHeaderFiles (roseDirectory)

def updateAction ():
    roseDirectory = selectROSEVersion ()
    makeNeeded = copyFilesIfNeeded (roseDirectory)
    if makeNeeded:
        makeROSE (roseDirectory)
        makeInstallROSE (roseDirectory)
        if queryYesNo("ROSE installation has changed. Shall I rebuild the translator?") == "yes":
            rebuildTranslator ()
        else:
            debug.verboseMessage("ROSE installation has changed but the translator has NOT been rebuilt. Therefore, there may be errors when using the translator.")
    else:
        debug.verboseMessage("None of the files in the ROSE distribution have changed. Therefore, nothing to do.")

if opts.check:
    checkEnvironment ()
    boostDirectory = getBoostPath ()
    debug.verboseMessage("Your environment is sane.")

if opts.build:
    buildAction ()
    debug.verboseMessage("ROSE build completed.")

if opts.update:
    updateAction ()
    debug.verboseMessage("Updating completed.")

if not opts.build and not opts.update and not opts.check:
    debug.exitMessage("No actions selected. Use -h for options")


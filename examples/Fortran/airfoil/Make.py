import os
import glob
from optparse import OptionParser
from subprocess import Popen, PIPE
from sys import argv

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

parser.add_option("--clean",
                  action="store_true",
                  dest="clean",
                  help="Remove generated files.",
                  default=False)

parser.add_option("--compile",
                  action="store_true",
                  dest="compile",
                  help="Runs the source-to-source compiler.",
                  default=False)

parser.add_option("--cuda",
                  action="store_true",
                  dest="cuda",
                  help="Generate code for CUDA backend.",
                  default=False)

parser.add_option("-d",
                  "--debug",
                  action="store",
                  dest="debug",
		  type="int",
                  help="Set the debug parameter of the compiler. [Default is %default].",
                  default=0,
		  metavar="<INT>")

parser.add_option("--openmp",
                  action="store_true",
                  dest="openmp",
                  help="Generate code for OpenMP backend.",
                  default=False)

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

(opts, args) = parser.parse_args(argv[1:])

# Cleans out files generated during the compilation process
def clean ():
	filesToRemove = []
	filesToRemove.extend(glob.glob('BLANK*.[fF?]95'))
	filesToRemove.extend(glob.glob("rose_*.[fF?]95"))
	filesToRemove.extend(glob.glob("[!^BLANK]*_postprocessed.[fF?]95"))
	filesToRemove.extend(glob.glob("*.rmod"))
	filesToRemove.extend(glob.glob("*.mod"))
	filesToRemove.extend(glob.glob("hs_err_pid*.log"))
	filesToRemove.extend(glob.glob("~*"))

	for file in filesToRemove:
		if opts.verbose:
			print("Removing file: '" + file + "'") 
		os.remove(file)

# Runs the compiler
def compile ():
	if not opts.cuda and not opts.openmp:
		print("You must specify either --cuda or --openmp on the command line.")
		exit(1)

	cmd = '/home/abetts/SILOET/OP2_ROSE_Fortran/translator/bin/translator -d '
	cmd += str(opts.debug) + ' '

	if opts.cuda:
		cmd += '--cuda '
	elif opts.openmp:
		cmd+= '--openmp '

	cmd += 'output.F95 input.F95 iso_c_binding.F95 op_fake.F95 constvars.F95 airfoil_seqfun.F95 airfoil.F95'

	if opts.verbose:
		print("Running: '" + cmd + "'")

	# Run the compiler in a bash shell as it needs the environment variables such as
	# LD_LIBRARY_PATH
	proc = Popen(cmd,
	     	     shell=True,
             	     executable='/bin/bash',
             	     stderr=PIPE,
             	     stdout=PIPE)

	# Grab both standard output and standard error streams from the process
	stdoutdata, stderrdata = proc.communicate()

	if opts.debug > 0:
		print('==================================== STANDARD OUTPUT ===========================================')
		for line in stdoutdata.splitlines():
			print(line)
		print('================================================================================================')

	# If a non-zero return code is detected then the compiler choked
	if proc.returncode != 0:
		print('Problem running compiler.')
		print('==================================== STANDARD ERROR ============================================')
		for line in stderrdata.splitlines():
			print(line)
		print('================================================================================================') 
        	exit(1)

if opts.clean:
	clean()

if opts.compile:
	compile()

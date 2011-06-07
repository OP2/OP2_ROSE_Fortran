import os
from subprocess import Popen, PIPE
from optparse import OptionParser
from sys import argv

# The command-line parser and its options
parser = OptionParser(add_help_option=False)

parser.add_option("-v",
                 "--verbose",
                 action="store_true",
                 dest="verbose",
                 help="Be verbose.",
                 default=False)

(opts, args) = parser.parse_args(argv[1:])

for path, dirs, files in os.walk(os.path.abspath(os.curdir + os.sep + 'src')):
	for file in files:
		fileBasename, fileExtension = os.path.splitext(file)
		fullPath = os.path.join(path, file)

		if fileExtension == '.cpp' or fileExtension == '.h':
			if opts.verbose:
				print('Adding %s to Git repository' % (fullPath))
		
			cmd = 'git add ' + fullPath

			proc = Popen(cmd,
				     shell=True,
                                     executable='/bin/bash',
                                     stderr=PIPE,
                                     stdout=PIPE)

			proc.communicate()

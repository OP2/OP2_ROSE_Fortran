#!/bin/bash -e

# If non-empty, then trace execution
debug=
[ -n "$debug" ] && set -x

###############################################################################################################################
# Functions
###############################################################################################################################

# Convert number of seconds to HH:MM:SS
sec_to_hms () {
    local sec="$1"
    printf "%02d:%02d:%02ds" $[sec/3600] $[sec%3600/60] $[sec%60]
}

# Make output more pretty. Reads standard input, indents and writes to standard output.
filter () {
    local start_time=$SECONDS
    echo ""
    echo "********************************************************************************"
    echo "*** $*"
    echo "********************************************************************************"

	cat
}

filterStep () {
    local start_time=$SECONDS
    ( echo ) 2>&1 |filter "Starting $* step"

	cat

    ( echo "Duration: $(sec_to_hms $SECONDS)" )
    ( echo ) 2>&1 |filter "Ending $* step"
}

# Clearly indicate death
killStep () {
    ( echo ) 2>&1 |filter "FAILED $* step"
    exit 1
}

# Restrict 'rm -rf' to be executed only within Hudson's workspace or the MasterInstallTree
safe_rm_rf () {
    set +e
        echo "$1" |grep -q "^/export/tmp.hudson-rose/hudson/workspace/\|^${HOME}/MasterInstallTree/"
        local OK=$?
    set -e

    if test $OK -eq 0 ; then
        echo "rm -rf $1"
        rm -rf $1
    else
        echo "Error: unsafe rm -rf '$1'"
        exit 1
    fi
}

###############################################################################################################################
# Functions
###############################################################################################################################



#########################################################################################
#
#  Dump Hudson environment
#  This script should be sourced in other Hudson run test scripts to
#  uniformly dump the Hudson environment.
#

#  TOO (3/16/2011): Centralized environment information to make it more
#   useful for debugging
#  DQ (1/14/2010): Support for dumping the enviroment so that it 
#   can be easily sourced to permit Hudson tests to be reproduced.
#
#########################################################################################
dumpHudsonEnvironment() {
#########################################################################################
if test $# = 1 ; then
  local env_dump_file=$1
  [ -f $env_dump_file ] && rm $env_dump_file

  (
	echo ""
	echo "This output is dumped in:"   
	echo ""
	echo "$machine_name@$env_dump_file"   
	echo ""
	echo ""
	echo "-------------------------------------------------------------------"
	echo "                                                          [General]"
	echo ""
	echo ""
	echo "machine                           = $machine"
	echo "machine_os                        = $machine_os"
	echo "machine_name                      = $machine_name"
	echo "start_time                        = $start_time"
	echo "hudson_start_time_seconds         = $hudson_start_time_seconds"

	echo ""
	echo ""
	echo "-------------------------------------------------------------------"
	echo "                                                             [ROSE]"
	echo ""
	echo ""
	set +e
        env |grep "^ROSE_"
	set -e

	echo ""
	echo ""
	echo "-------------------------------------------------------------------"
	echo "                                                              [Git]"
	echo ""
	echo ""
        [ ! -n "$ROSE_SHOW_LAST_N_COMMITS" ] || [ $ROSE_SHOW_LAST_N_COMMITS -le 0 ] && ROSE_SHOW_LAST_N_COMMITS=5
	echo "Last ($ROSE_SHOW_LAST_N_COMMITS) commits:"
        git log -n$ROSE_SHOW_LAST_N_COMMITS --pretty=format:'
          + Committed %ar by %an%n
            %s'

	echo ""
	echo ""
	# tps (02/04/2010) : Checking the EDG submodule version
	echo "git submodule status (EDG version) ="
	echo "  `git submodule status`"

	echo ""
	echo "-------------------------------------------------------------------"
	echo "                                                     [Library Path]"
	echo ""
	echo ""
	if test "x$label" = "x64bit-macos-10.6" ; then
	 echo "DYLD_LIBRARY_PATH ="
	 for a_path in `echo $DYLD_LIBRARY_PATH | sed 's/:/ /g'` ; do
	   echo "    $a_path"
	 done
	echo ""
	 echo "Copy me: export DYLD_LIBRARY_PATH=\"$DYLD_LIBRARY_PATH:\$DYLD_LIBRARY_PATH\""
	else
	 echo "LD_LIBRARY_PATH ="
	 for a_path in `echo $LD_LIBRARY_PATH | sed 's/:/ /g'` ; do
	   echo "    $a_path"
	 done
	echo ""
	echo "Copy me: export LD_LIBRARY_PATH=\"$LD_LIBRARY_PATH:\$LD_LIBRARY_PATH\""
	fi

	echo ""
	echo ""
	echo "-------------------------------------------------------------------"
	echo "                                                  [Executable Path]"
	echo ""
	echo ""
	echo "PATH ="
	 for a_path in `echo $PATH | sed 's/:/ /g'` ; do
	   echo "    $a_path"
	 done
	echo ""
	echo "Copy me: export PATH=\"$PATH:\$PATH\""
	echo ""
	echo ""
	echo `gcc --version | head -1`
	echo `automake --version | head -1`
	echo `autoconf --version | head -1`
	echo `aclocal --version | head -1`
	echo `git --version`
       
 

	echo ""
	echo ""
	echo "-------------------------------------------------------------------"
	echo "                                                   [Configure line]"
	echo ""
	echo ""
	echo "./configure"
         [ ! -n "$CONFIGURE_FLAGS" ] && echo "CONFIGURE_FLAGS is empty='$CONFIGURE_FLAGS'" && exit 1 
	 for an_option in ${CONFIGURE_FLAGS} ; do 
	   echo "    $an_option"
	 done
	echo ""
	echo "Copy me: ./configure `echo ${CONFIGURE_FLAGS} |sed 's/  //'`"

	echo ""
	echo ""
	echo "-------------------------------------------------------------------"
	echo "                                                     [Install tree]"
	echo ""
	echo ""
	echo "ROSE_CONFIGURE_PREFIX (--prefix)=" 
	echo ""
	echo "    $ROSE_CONFIGURE_PREFIX"
	echo ""
	echo ""
  ) 2>&1 |tee $env_dump_file |filterStep "Hudson environment output"
else
    echo "[Error] Usage: dumpHudsonEnvironment <output-file-name>"
    exit 1
fi
}

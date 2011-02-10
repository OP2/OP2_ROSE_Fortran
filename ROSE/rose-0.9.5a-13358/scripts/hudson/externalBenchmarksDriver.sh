#!/bin/bash
# A hudson-friendly script to 
# x) install ROSE and test identityTranslator on a set of external benchmarks
# 
# The assumptions include
#   an installed copy of spec cpu 2006 under /home/hudson-rose/opt
#   a predefined configuration file to use rose to build the benchmarks
# 
# It only runs on 32-bit machines for now
#  since rose-gcc41.cfg is 32-bit specific.
# Liao 2/10/2010

# set up 64-bit environment
#--------------------------------------
# installation path of spec cpu and the config file for using rose
SPEC_CPU2006_INS="/home/hudson-rose/opt/spec_cpu2006_ins"
SPEC_CPU2006_CONFIG="rose-gcc41.cfg"

# sometimes, we want to skip the time consuming part
# and reuse previous build to test this script
SKIP_COMPILATION=0

set +e
source /usr/apps/qt/4.5.1/setup.sh  # this causes the script to abort if set +e is not used

# extra work to find libjvm.so on different platforms
PROCESSOR_TYPE=`uname -p`
if [ "$PROCESSOR_TYPE" == "i686" ] ; then
  JAVA_PLATFORM_STRING="i386"
elif  [ "$PROCESSOR_TYPE" == "x86_64" ] ; then
  JAVA_PLATFORM_STRING="amd64"
else
  echo "unrecognized processor type: $PROCESSOR_TYPE"
  exit 1 
fi

export JAVA_HOME=/usr/apps/java/jdk1.6.0_11
export BOOST_ROOT=/export/tmp.hudson-rose/opt/boost_1_40_0-inst
export LD_LIBRARY_PATH="/usr/apps/java/jdk1.6.0_11/lib:${JAVA_HOME}/jre/lib/$JAVA_PLATFORM_STRING/server:$LD_LIBRARY_PATH"
export LD_LIBRARY_PATH="${BOOST_ROOT}/lib:${LD_LIBRARY_PATH}"
export PATH="$JAVA_HOME/bin:/export/tmp.hudson-rose/opt/git-1.6.5.2-inst/bin:/usr/apps/automake/1.9.6/bin:$PATH"
PROCESS_NUM=8

# prepare the source tree 
#--------------------------------------
# git pull # this will cause problem since hudson test may run on a ghost branch

set -e

ROSE_SOURCE_PATH=`pwd`
echo `pwd`

if [ -d ${ROSE_SOURCE_PATH} ]; then
  if [ -f ${ROSE_SOURCE_PATH}/build ] ;
   then
     echo ${ROSE_SOURCE_PATH} is verified to be a rose source tree
   else
     echo Fatal error: ${ROSE_SOURCE_PATH} does not seem to be a rose source tree!
     exit 3
  fi
else
  echo Fatal error: ${ROSE_SOURCE_PATH} does not exist!
  exit 3
fi

# always rebuild
if [ $SKIP_COMPILATION -ne 1 ]; then
./build
fi

# build tree:  configure and build
#--------------------------------------
# always build fresh
if [ $SKIP_COMPILATION -ne 1 ]; then
  if test -e ROSE-build ; then chmod -R u+wx ROSE-build ; fi
  rm -rf ROSE-build
  mkdir ROSE-build
fi

cd ROSE-build
ROSE_BUILD_PATH=`pwd`

# we install rose under top_src/ROSE-build/install
ROSE_INSTALL_PATH=$ROSE_BUILD_PATH/install
if [ $SKIP_COMPILATION -ne 1 ]; then
  # only reconfigure as needed
  ../configure --with-boost=${BOOST_ROOT} --with-CXX_DEBUG=-g --with-CXX_WARNINGS=-Wall --with-pch --prefix=$ROSE_INSTALL_PATH
#../configure --with-boost=${BOOST_ROOT} --with-CXX_DEBUG=-g --with-CXX_WARNINGS=-Wall --enable-dq-developer-tests --with-ROSE_LONG_MAKE_CHECK_RULE=yes --with-qt=/usr/apps/qt/4.5.1 --with-roseQt --with-haskell=/home/liao6/opt/ghc-6.10.4/bin --with-pch --with-gomp_omp_runtime_library=/home/liao6/opt/gcc-svn/lib/ --prefix=$HOME/.hudson/tempInstall
  
  make -j${PROCESS_NUM} && \
  make -j${PROCESS_NUM} install
fi

if [ $? -ne 0 ]; then
  echo "Error in make && make install, aborting...."
  exit 3
fi


# --------- test rose on external benchmarks
set -e
$ROSE_SOURCE_PATH/scripts/testOnExternalBenchmarks.sh ${ROSE_INSTALL_PATH} $SPEC_CPU2006_INS $SPEC_CPU2006_CONFIG 

if [ $? -ne 0 ]; then
  echo "Error in testOnExternalBenchmarks.sh aborting ...."
  exit 3
fi


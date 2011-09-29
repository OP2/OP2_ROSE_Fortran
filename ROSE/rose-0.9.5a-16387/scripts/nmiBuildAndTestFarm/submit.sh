
# Check command line for option file
if [[ "$1" == "" || !( -f $1 )]]
then
	echo "Usage: $0 OPTION_FILE [tarball]"
	exit 1
fi

. $1

# Check if all necessary options are set

if [[ "$TITLE" == "" ]]
then
	echo "please set TITLE in your option file"
	exit 1
fi

if [[ "$DESCRIPTION" == "" ]]
then
	echo "please set DESCRIPTION in your option file"
	exit 1
fi

#if [[ "$PLATFORMS" == "" ]]
#then
#	echo "please set PLATFORMS in yout option file"
#	exit 1
#fi

# optional options, however most likely these need to be set
# a warning is emitted
if [[ "$PREREQS" == "" ]]
then
	echo "WARNING: PREREQS is not set"
fi

if [[ "$CONFIGURE_OPTIONS" == "" ]]
then
	echo "WARNING: CONFIGURE_OPTIONS is not set"
fi

# Determine HEAD revision number
REVISION=`svn -r HEAD log https://outreach.scidac.gov/svn/rose/trunk/ | grep '^Load rose' | cut -d'-' -f 3 | cut -d' ' -f 1`
PLATFORMS=`arr=($(echo $1 | awk -F"/" '{$1=$1; print}')); echo ${arr[${#arr[*]}-2]}`


# create env.pl
mkdir -p env
ENV_PL_TEMPDIR=$PWD/`mktemp -d env/XXXXXXXX`
ENV_PL=${ENV_PL_TEMPDIR}/env.pl
touch $ENV_PL
echo "#!/usr/bin/env perl" > $ENV_PL

# Test if this is required since the NMI environment should set it
# and it appears that we over write it in the perl environment.
# echo '$ENV{JAVA_HOME} = "'${JAVA_HOME}'";' >> $ENV_PL
echo '$ENV{ACLOCAL_INCLUDES} = "'${ACLOCAL_INCLUDES}'";' >> $ENV_PL
echo 'return 1;' >> $ENV_PL


# determine inputs
if [[ "$2" == "" ]]; then
    ROSE_SOURCE='svn'
else
    ROSE_SOURCE='scp'

    if [[ !( -f $2 ) ]]; then
        echo "Couldn't find tarball $2"
        exit 1
    fi
    mkdir -p rose-tarballs
    ROSE_TARBALL_TEMPDIR=$PWD/`mktemp -d rose-tarballs/XXXXXXXX`
    cp $2 ${ROSE_TARBALL_TEMPDIR}/rose.tar.gz
fi



# exporting options, to be recognized in the submit and input scripts
export _NMI_TITLE=${TITLE}
export _NMI_DESCRIPTION=${DESCRIPTION}
export _NMI_PLATFORMS=${PLATFORMS}
export _NMI_PREREQS=${PREREQS}
export _NMI_CONFIGURE_OPTIONS=${CONFIGURE_OPTIONS}

export _NMI_REVISION=${REVISION}

export _NMI_HOME=$HOME
export _NMI_HOSTNAME=$HOSTNAME
export _NMI_SUBMITDIR=$PWD
export _NMI_ENV_PL_TEMPDIR=${ENV_PL_TEMPDIR}

export _NMI_ROSE_TARBALL_TEMPDIR=${ROSE_TARBALL_TEMPDIR}
export _NMI_ROSE_SOURCE=${ROSE_SOURCE}


# submit the build job
echo "Submitting the build job ..."
echo "current revision: ${REVISION}"
echo "configuration: $1"
nmi_submit rose-build.submit > .build_submit.log
RID=`grep "Run ID:" .build_submit.log | cut -f 3`
GID=`grep "Global ID:" .build_submit.log | cut -f 2`
cat .build_submit.log

# export Run ID and Global to be recognized by the test input file
export _NMI_RID=${RID}
export _NMI_GID=${GID}

# submit the test job
#echo -e "\nSubmitting the test job ..." 
#nmi_submit rose-test.submit 

exit

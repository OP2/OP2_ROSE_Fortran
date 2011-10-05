#!/bin/sh

#set -x
quiet=0
leakcheck=0
timing=0

#OCLOGFILE=/dev/null
OCLOGFILE="" ; export OCLOGFILE

# Capture arguments
srcdir="$1"
builddir="$2"
mode="$3"
if test "x$4" = "x" ; then cache=1 ; else cache=0; fi
longtests="$5"

if test "x$timing" = "x1" ; then leakcheck=0; fi

#locate the testdata and expected directory
if test "$cache" = 0 ; then
CACHE=""
expected3="${srcdir}/nocacheremote3"
expected4="${srcdir}/nocacheremote4"
else
CACHE="[cache]"
expected3="${srcdir}/expectremote3"
expected4="${srcdir}/expectremote4"
fi

# get the list of test files
##################################################
# Remote test info
##################################################
# For now, only do only following test sets
if test -n "$longtests"; then
WHICHTESTS="L1 LC1"
else
WHICHTESTS="S1 C1"
fi

# For special testing
REMOTEURLX="http://test.opendap.org:8080/dods/dts"
REMOTETESTSX="test.67"

# These shorter tests are always run
REMOTEURLS1="http://test.opendap.org:8080/dods/dts"
REMOTETESTSS1="\
test.01 test.02 test.04 test.05 test.06a test.07a test.07 \
test.21 test.22 test.23 \
test.31 \
test.50 test.53 test.55 test.56 test.57 \
test.66 test.67 test.68 test.69"

# These longer tests are optional
REMOTEURLL1="$REMOTEURLS1"
REMOTETESTSL1="\
test.03 \
b31 b31a D1 Drifters EOSDB \
ingrid nestedDAS NestedSeq NestedSeq2 OverideExample \
SimpleDrdsExample test.an1 \
test.dfp1 test.gr1 \
test.gr2 test.gr3 test.gr4 test.gr5 \
test.sds1 test.sds2 test.sds3 \
test.sds4 test.sds5 \
test.vs1 test.vs2 test.vs3 test.vs4 test.vs5 \
whoi"


# Anything larger than about 100k will not be in the distribution
TOOBIG="parserBug0001 test.satimage Sat_Images test.06 test.32"

# Following contain %XX escapes which I cannot handle yet
ESCAPEDFAIL="test.dfr1 test.dfr2 test.dfr3 test.GridFile test.PointFile test.SwathFile test.sds6 test.sds7"

REMOTEURLA="http://test.opendap.org/dap/data/nc"
REMOTETESTSA="\
test.nc\
"

# Following tests are to check constraint handling
REMOTEURLC1="http://test.opendap.org:8080/dods/dts"
REMOTETESTSC1="\
test.01;1;f64 \
test.02;1;b[1:2:10] \
test.03;1;i32[0:1][1:2][0:2] \
test.04;1;types.i32 \
test.05;1;types.floats.f32 \
test.06;1;ThreeD \
test.07;1;person.age \
test.07;3;person \
test.07;4;types[0:2:10].f32"

# Constrained long tests
REMOTEURLLC1="http://test.opendap.org:8080/dods/dts"
REMOTETESTSLC1="\
test.03;2;s1"

# Unknown problem: test.07;2;&age>2
IGNORE="test.07.2" 

# Temporarily suppress
XFAILTESTS3=""
XFAILTESTS4=""

# Misc tests not currently used
REMOTEURL0="http://test.opendap.org/dap/netcdf/examples"
REMOTETESTS0="\
cami_0000-09-01_64x128_L26_c030918.nc \
ECMWF_ERA-40_subset.nc \
sresa1b_ncar_ccsm3_0_run1_200001.nc \
tos_O1_2001-2002.nc
123bears.nc \
ber-2002-10-01.nc \
ceopL2AIRS2-2.nc \
ceopL2AIRS2.nc \
data.nc \
fnoc1.nc \
in1.nc \
in_2.nc \
in.nc \
in_no_three_double_dmn.nc \
in_v.nc \
saco1.nc \
testfile.nc \
text.nc \
"

case "$mode" in
3)
    EXPECTED="$expected3"
    TITLE="DAP to netCDF-3 translation"
    PARAMS="${CACHE}[netcdf3]"
    XFAILTESTS="$XFAILTESTS3"
    ;;
4)
    EXPECTED="$expected4"
    TITLE="DAP to netCDF-4 translation"
    PARAMS="${CACHE}[netcdf4]"
    XFAILTESTS="$XFAILTESTS4"
    ;;
esac

RESULTSDIR="./results"
# Locate some tools
NCDUMP="${builddir}/ncdump/ncdump"
if test "x$leakcheck" = x1 ; then
VALGRIND="valgrind -q --error-exitcode=2 --leak-check=full"
else
VALGRIND=
fi
if test "x$timing" = "x1" ; then TIMECMD="time"; else TIMECMD=""; fi

rm -fr ${RESULTSDIR}
mkdir "${RESULTSDIR}"

passcount=0
xfailcount=0
failcount=0

echo "*** Testing $TITLE "
echo "        Base URL: ${TESTURL}"
echo "        Client Parameters: ${PARAMS}"
if test "$cache" = 0; then
echo "        Caching: off"
else
echo "        Caching: on"
fi
echo "    Note: The remote tests may be slow or even fail if the server is overloaded"

status=0

for i in $WHICHTESTS ; do
  constrained=0
  ncconstrained=0
  case "$i" in
  S1) TESTURL="$REMOTEURLS1" ; TESTSET="$REMOTETESTSS1" ;;
  S2) TESTURL="$REMOTEURLS2" ; TESTSET="$REMOTETESTSS2" ;;
  L1) TESTURL="$REMOTEURLL1" ; TESTSET="$REMOTETESTSL1" ;;
  L2) TESTURL="$REMOTEURLL2" ; TESTSET="$REMOTETESTSL2" ;;
  C1) TESTURL="$REMOTEURLC1" ; TESTSET="$REMOTETESTSC1" ; constrained=1 ;ncconstrained=0 ;;
  LC1) TESTURL="$REMOTEURLLC1" ; TESTSET="$REMOTETESTSLC1" ; constrained=1 ;ncconstrained=0 ;;
  X) TESTURL="$REMOTEURLX" ; TESTSET="$REMOTETESTSX" ; constrained=0 ; ncconstrained=0 ;;
  esac

cd ${RESULTSDIR}
for t in ${TESTSET} ; do
  # see if we are using constraints
  if test "x$constrained" = "x0" ; then # No constraint
    testname=$t
    ce=
  else # Constrained
    testname=`echo $t | cut -d ';' -f1`
    testno=`echo $t | cut -d ';' -f2`
    ce=`echo $t | cut -d ';' -f3-`
    if test "x$ncconstrained" = "x1" ; then
      ce=`echo $ce | tr '[]' '()'`
    fi
  fi
  if test "x$constrained" = "x0" ; then
    name="${testname}"
    url="${PARAMS}${TESTURL}/$testname"
  else
    name="${testname}.${testno}"
    if test "x$ncconstrained" = "x0" ; then
      url="${PARAMS}${TESTURL}/$testname?${ce}"
    else
      url="[ce=${ce}]${PARAMS}${TESTURL}/$testname"
    fi
  fi
  if test "x$quiet" = "x0" ; then echo "*** Testing: ${name}"; fi
  if test "x$quiet" = "x0" ; then echo "*** URL: ${url}"; fi

  # determine if this is an xfailtest
  isxfail=0
  for x in ${XFAILTESTS} ; do
    if test "x${name}" = "x${x}" ; then isxfail=1; fi
  done
  status=0

  if ${TIMECMD} ${VALGRIND} ${NCDUMP} "${url}" > ${name}.dmp ; then status=$status; else status=1; fi
  # compare with expected
  if diff -w ${EXPECTED}/${name}.dmp ${name}.dmp
    then status=$status; else status=1; fi
  if test "x$status" = "x1" ; then
    if test "x$isxfail" = "x1" ; then status=2; fi  # xfail
  fi

  case "$status" in
  0)
    passcount=`expr $passcount + 1`
    if test "x$quiet" = "x" ; then echo "*** SUCCEED: ${name}"; fi
    ;;
  1)
    failcount=`expr $failcount + 1`
    echo "*** FAIL:  ${name}"
    ;;
  2)
    xfailcount=`expr $xfailcount + 1`
    echo "*** XFAIL: ${name}"
    ;;
  esac

done
cd ..

done

totalcount=`expr $passcount + $failcount + $xfailcount`
okcount=`expr $passcount + $xfailcount`

echo "*** PASSED: ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"

# Ignore failures for now
#failcount=0
if test "$failcount" -gt 0 ; then
  exit 1
else
  exit 0
fi

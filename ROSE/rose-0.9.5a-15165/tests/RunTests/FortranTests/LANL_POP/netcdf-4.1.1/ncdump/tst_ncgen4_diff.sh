#!/bin/sh

. ${srcdir}/tst_ncgen4_shared.sh

ALLXFAIL="${XFAILTESTS} ${SPECIALTESTS}"

if test "${MODE}" = 3 ; then
TESTSET="${TESTS3}"
else
TESTSET="${TESTS3} ${TESTS4} ${ALLXFAIL}"
fi

echo "*** Testing ncgen with -k${KFLAG}"

cd ${RESULTSDIR}
for x in ${TESTSET} ; do
  test $verbose = 1 && echo "*** Testing: ${x}"
	# determine if we need the specflag set
	specflag=
	for s in ${SPECIALTESTS} ; do
	if test "x${s}" = "x${x}" ; then specflag="-s"; fi
	done
	# determine if this is an xfailtest
	isxfail=
	for t in ${ALLXFAIL} ; do
	if test "x${t}" = "x${x}" ; then isxfail=1; fi
	done
  rm -f ${x}.nc ${x}.dmp
  ${builddir}/../ncgen/ncgen -k${KFLAG} -o ${x}.nc ${cdl}/${x}.cdl
  # dump .nc file
  ${builddir}/../ncdump/ncdump ${specflag} ${x}.nc > ${x}.dmp
  # compare with expected
  if diff -w ${expected}/${x}.dmp ${x}.dmp ; then
    test $verbose = 1 && echo "*** SUCCEED: ${x}"
    passcount=`expr $passcount + 1`
  elif test "x${isxfail}" = "x1" ; then
    echo "*** XFAIL: ${x}"
    xfailcount=`expr $xfailcount + 1`
  else
    echo "*** FAIL: ${x}"
    failcount=`expr $failcount + 1`
  fi
done
cd ..

totalcount=`expr $passcount + $failcount + $xfailcount`
okcount=`expr $passcount + $xfailcount`

echo "*** PASSED: ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"

if test $failcount -gt 0 ; then
  exit 1
else
  exit 0
fi

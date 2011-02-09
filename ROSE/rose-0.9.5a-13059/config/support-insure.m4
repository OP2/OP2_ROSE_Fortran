AC_DEFUN([ROSE_SUPPORT_INSURE],
[
# Begin macro ROSE_SUPPORT_INSURE.

# We may be asked to guess the correct flags,
# so we must know our host and the compiler used.
AC_REQUIRE([AC_CANONICAL_HOST])
AC_REQUIRE([AC_PROG_CXX])
AC_REQUIRE([BTNG_INFO_CXX_ID])
AC_REQUIRE([BTNG_CXX_AR])

dnl *********************************************************************
dnl * Use INSURE
dnl *********************************************************************
# AC_ARG_ENABLE(insure-api,
#  [  --enable-insure-api ....................... Enable insure API in code.])
AC_ARG_ENABLE(insure,
 [  --enable-insure ....................... Augment the linker with insure.])

# if test "$enable_insure_api" = yes ; then
#   AC_DEFINE([USE_INSURE],[],[Support for Insure])
#   AC_DEFINE([USE_INSURE_API],[],[Support for Insure API (Compiling with the Insure library).])
# fi

dnl DQ added so that EDG can detect use of linker and use the api 
dnl while the rest of ROSE does not.  EDG only supports use of API
dnl not just the linker option.
if test "$enable_insure" = yes ; then
  AC_DEFINE([USE_INSURE],[],[Link-time support for Insure.])
fi



dnl *********************************************************************
dnl * Specify the location of INSURE
dnl *********************************************************************
# Let user specify where to find insure.
# Specify by --with-insure= or setting insure_PREFIX.
AC_MSG_CHECKING(for insure)
AC_ARG_WITH(insure,
[  --with-insure=PATH	Specify the prefix where insure is installed],
, test "$insure_PREFIX" && with_insure="$insure_PREFIX")
test "$with_insure" && test ! "$with_insure" = no && insure_PREFIX="$with_insure"
AC_MSG_RESULT($insure_PREFIX)
echo "insure_PREFIX is $insure_PREFIX" >&5
if test "$insure_PREFIX" ; then
  insure_INCLUDES="-I$insure_PREFIX"
  insure_LIBS="$insure_PREFIX/insure_stubs.a"
  insure_BIN="$insure_PREFIX/insure"
fi
AC_SUBST(insure_PREFIX)
AC_SUBST(insure_INCLUDES)
AC_SUBST(insure_LIBS)
AC_SUBST(insure_BIN)
echo "insure_INCLUDES is $insure_INCLUDES" >&5
echo "insure_LIBS is $insure_LIBS" >&5
echo "insure_BIN is $insure_BIN" >&5



dnl Allows insure windows to be disabled, as it is too much load over the network.
# AC_ARG_ENABLE(insure-windows,
#   [  --enable-insure-windows ............... turn on use of INSURE windows option])
# test ! "$enable_insure_windows" = yes && insure_windows_option='-windows=no';



dnl *********************************************************************
dnl * Specify options for INSURE
dnl *********************************************************************
dnl  Added by DQ to support INSURE (using my favorite options and paths which might be platform specific)
dnl note that we have to use the ${} instead of $() for this to work on LINUX and IBM (other platforms do not seem to case)
dnl  INSURE_RUN_AT_EXIT="-run-at-exit=\"if %z; then echo \\\"%v: %e errors, %l+%L bytes leaked.\\\"; fi\" "
AC_ARG_WITH(insure-options,
   [  --with-insure-options=ARG ................ manually set INSURE options to ARG],
INSURE_OPTIONS=$with_insure_options,
INSURE_OPTIONS=""
)
AC_SUBST(INSURE_OPTIONS)


# if test "$enable_insure_linker" ; then
#   AUX_LINKER="${insure_BIN} ${INSURE_OPTIONS}"
# fi
# AC_SUBST(AUX_LINKER)



# if test "$enable_insure_linker" = yes ; then
# if test "$with_insure" = yes ; then
if test "$enable_insure" = yes ; then
# Setup compiler specific options for using insure
# Different compilers (e.g KCC) require options in different order.
  AUX_LINKER="${insure_BIN} ${INSURE_OPTIONS}"
  case $CXX in
    g++)
    # See comment below about syntax change for Linux
      CC="insure"
      CCLD="$CC"
      CXX="insure"
      CXXLD="$CXX"
      ;;
    KCC | mpKCC)
    # insure specific option required and clear the AUX_LINKER
      CCLD="$CC --link_command_prefix '${insure_BIN} ${INSURE_OPTIONS}' -o"
      CXXLD="$CXX --link_command_prefix '${insure_BIN} ${INSURE_OPTIONS}' -o"
      AUX_LINKER=""
      ;;
    CC)
    # No insure specific option required
    # Can't use "$(xxx)" syntax since it only works when build proecess is run on SUNs
    # Linux requires a different syntax: "$xxx" instead.
    # CCLD="$(AUX_LINKER) $(CXX)"
    # CXXLD="$(AUX_LINKER) $(CXX)"
      CCLD="$AUX_LINKER $CXX"
      CXXLD="$AUX_LINKER $CXX"
    ;;
  esac
else
     CCLD="$CC"
# Liao: 2/17/2009, enable CXXLD environment variable to be accepted during configuration,
# especially when roseTranslator is used and it is not ready to be used as a linker
      CXXLD="$CXXLD"
      if test "x$CXXLD" = x; then
         CXXLD="$CXX"
      fi 
# BP : 10/29/2001, the above two lines were originally 
#      CCLD="$(CC)"
#      CXXLD="$(CXX)"
# but on Linux systems this seems to not work
fi

AC_SUBST(AUX_LINKER)

echo "In macro ROSE SUPPORT INSURE: CC    = $CC"
echo "In macro ROSE SUPPORT INSURE: CCLD  = $CCLD"
echo "In macro ROSE SUPPORT INSURE: CXX   = $CXX"
echo "In macro ROSE SUPPORT INSURE: CXXLD = $CXXLD"

AC_SUBST(CCLD)
AC_SUBST(CXXLD)

# End macro ROSE_SUPPORT_INSURE.
]
)


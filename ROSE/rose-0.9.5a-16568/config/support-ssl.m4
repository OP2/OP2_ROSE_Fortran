AC_DEFUN([ROSE_SUPPORT_SSL],
[
# Begin macro ROSE_SUPPORT_SSL.

# Adding support for the SSL library to provide access to MD5 checksum support for binary library identification.

AC_MSG_CHECKING(for ssl)
AC_ARG_ENABLE(ssl,
[  --enable-ssl ................................ Enable use of SSL library (MD5 checksums)],
,
if test ! "$enable_ssl" ; then
   enable_ssl=no
fi
)

#if test "$enable_ssl" = yes ; then
#  AC_DEFINE([USE_ROSE_SSL_SUPPORT],[],[Support for ssl])
#fi

echo "In ROSE SSL SUPPORT MACRO: enable_ssl = $enable_ssl"

if test "$enable_ssl" = no; then
   # If SSL is not specified, then don't use it.
   echo "Skipping use of SSL (libssl) support!"
else
   echo "Setup SSL support in ROSE! path = $enable_ssl"
   AC_DEFINE([USE_ROSE_SSL_SUPPORT],1,[Controls use of ROSE support for SSL (libssl -- MD5) library (incompatable with Java, so disables Fortran support).])
fi

# AC_SUBST(dwarf_path)

# End macro ROSE_SUPPORT_DWARF.
]
)

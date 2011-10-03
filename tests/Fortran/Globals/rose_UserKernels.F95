# 1 "/home/abetts/SILOET/OP2_ROSE_Fortran/examples/Fortran/tests/Globals/UserKMfRDX2.F90"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/abetts/SILOET/OP2_ROSE_Fortran/examples/Fortran/tests/Globals/UserKMfRDX2.F90"
MODULE UserKernels
CONTAINS
SUBROUTINE integer4Scalar_integer4Scalar(x,y)
IMPLICIT NONE
INTEGER(kind=4) :: x
INTEGER(kind=4) :: y
x = x + y
END SUBROUTINE integer4Scalar_integer4Scalar

SUBROUTINE integer4Scalar_integer4Scalar_max(x,y)
IMPLICIT NONE
INTEGER(kind=4) :: x
INTEGER(kind=4) :: y
x = max(x * y,x + y)
END SUBROUTINE integer4Scalar_integer4Scalar_max

SUBROUTINE integer4Scalar_integer4Scalar_min(x,y)
IMPLICIT NONE
INTEGER(kind=4) :: x
INTEGER(kind=4) :: y
x = min(x - y + 100,x + y - 15)
END SUBROUTINE integer4Scalar_integer4Scalar_min

SUBROUTINE real8Array_integer4Array(x,y)
IMPLICIT NONE
REAL(kind=8) :: x(*)
INTEGER(kind=4) :: y(*)
x(1) = x(1) + y(1) + y(2)
END SUBROUTINE real8Array_integer4Array

SUBROUTINE real8Array_integer4Scalar(x,y)
IMPLICIT NONE
REAL(kind=8) :: x(*)
INTEGER(kind=4) :: y
x(1) = x(1) + y
END SUBROUTINE real8Array_integer4Scalar

SUBROUTINE real8Array(x)
IMPLICIT NONE
REAL(kind=8) :: x(*)
x(1) = x(1) * 10
END SUBROUTINE real8Array

SUBROUTINE real8Array_integer4Array_real8Array(a,b,c)
REAL(kind=8) :: a(4)
INTEGER(kind=4) :: b(7)
REAL(kind=8) :: c(11)
INTEGER(kind=4) :: i
DO i = 1, 4
c(i) = a(i)
END DO
DO i = 5, 11
c(i) = int(b(i - 4))
END DO
END SUBROUTINE real8Array_integer4Array_real8Array

END MODULE UserKernels


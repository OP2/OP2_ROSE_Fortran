module UserKernels

contains

  subroutine  integer4Scalar_integer4Scalar ( x, y )
    implicit none

    integer(4) :: x
    integer(4) :: y

    x = x + y

  end subroutine integer4Scalar_integer4Scalar 

  subroutine  integer4Scalar_integer4Scalar_max ( x, y )
    implicit none

    integer(4) :: x
    integer(4) :: y

    x = max(x * y, x + y)

  end subroutine integer4Scalar_integer4Scalar_max

  subroutine  integer4Scalar_integer4Scalar_min ( x, y )
    implicit none

    integer(4) :: x
    integer(4) :: y

    x = min(x - y + 100, x + y - 15)

  end subroutine integer4Scalar_integer4Scalar_min

  subroutine real8Array_integer4Array ( x, y )
    implicit none

    real(8), dimension(*) :: x
    integer(4), dimension(*) :: y

    x(1) = x(1) + y(1) + y(2)

  end subroutine real8Array_integer4Array
  
  subroutine real8Array_integer4Scalar ( x, y )
    implicit none

    real(8), dimension(*) :: x
    integer(4) :: y

    x(1) = x(1) + y

  end subroutine real8Array_integer4Scalar

  subroutine real8Array ( x )
    implicit none

    real(8), dimension(*) :: x

    x(1) = x(1) * 10

  end subroutine real8Array

  subroutine real8Array_integer4Array_real8Array ( a, b, c )
  
    real(8),    dimension(4)  :: a
    integer(4), dimension(7)  :: b
    real(8),    dimension(11) :: c
    
    integer(4) :: i
    
    do i = 1, 4
      c(i) = a(i)
    end do

    do i = 5, 11
      c(i) = int ( b(i-4) )
    end do
    
  end subroutine real8Array_integer4Array_real8Array

end module UserKernels

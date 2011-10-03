module UserKernels

contains

   subroutine real8Array_real8Array_real8Array_kernel1 ( x, y, z )
    real(8) :: x(6), y(6), z(6)

    do i = 1, 6
      z(i) = z(i) + x(i) + y(i)
    end do
  end subroutine

  subroutine real8Array_real8Array_real8Array_kernel2 ( x, y, k )
    real(8) :: x(6), y(6), k(6)

    do i = 1, 6
      k(i) = k(i) + x(i) - z(i)
    end do
  end subroutine

  subroutine real8Array_real8Array_real8Array_real8Array_kernel1 ( x, y, z, k )
    real(8) :: x(6), y(6), z(6), k(6)

    do i = 1, 6
      z(i) = z(i) + x(i) + y(i)
    end do

    do i = 1, 6
      k(i) = k(i) + x(i) - z(i)
    end do
  end subroutine

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

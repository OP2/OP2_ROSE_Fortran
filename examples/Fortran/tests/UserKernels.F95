module UserKernels

contains
  
  subroutine kernel_2parameter_floatArray_floatScalar ( x, y ) BIND(C)
    implicit none

    real(8), dimension(*) :: x
    integer(4) :: y

    x(1) = x(1) + y

  end subroutine kernel_2parameter_floatArray_floatScalar

  subroutine kernel_1parameter_floatArray ( x ) BIND(C)
    implicit none

    real(8), dimension(*) :: x

    x(1) = x(1) * 10

  end subroutine kernel_1parameter_floatArray

end module UserKernels

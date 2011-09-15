module UserKernels

contains

  subroutine  kernel_2parameter_integerScalar_integerScalar ( x, y )
    implicit none

    integer(4) :: x
    integer(4) :: y

    x = x + y

  end subroutine kernel_2parameter_integerScalar_integerScalar  


  subroutine kernel_2parameter_floatArray_floatArray ( x, y )
    implicit none

    real(8), dimension(*) :: x
    integer(4), dimension(*) :: y

    x(1) = x(1) + y(1) + y(2)

  end subroutine kernel_2parameter_floatArray_floatArray
  
  subroutine kernel_2parameter_floatArray_integerScalar ( x, y )
    implicit none

    real(8), dimension(*) :: x
    integer(4) :: y

    x(1) = x(1) + y

  end subroutine kernel_2parameter_floatArray_integerScalar

  subroutine kernel_1parameter_floatArray ( x )
    implicit none

    real(8), dimension(*) :: x

    x(1) = x(1) * 10

  end subroutine kernel_1parameter_floatArray

end module UserKernels

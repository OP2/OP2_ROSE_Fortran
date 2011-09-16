module UserKernels

contains

  subroutine newKernel ( a, b, c )
  
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
    
  end subroutine newKernel
  
end module UserKernels

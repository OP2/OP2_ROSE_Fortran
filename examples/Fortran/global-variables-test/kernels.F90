
module kernels

contains

  ! read
  subroutine kernelr81 ( a, b )
  
    real(8), dimension(4) :: a
    real(8) :: b
    
    a(1) = 10 + b
    a(2) = 20 + b
    a(3) = 30 + b
    a(4) = 40 + b
  
  end subroutine

  subroutine kernelr82 ( a, b )
  
    real(8), dimension(4) :: a
    real(8), dimension(10) :: b
    
    a(1) = 10 + b(1) + b(5) + b(9)
    a(2) = 20 + b(2) + b(6) + b(10)
    a(3) = 30 + b(3) + b(7)
    a(4) = 40 + b(4) + b(8)
  
  end subroutine

  ! inc
  subroutine kernelr83 ( a, b )
  
    real(8), dimension(4) :: a
    real(8) :: b
    
    a(1) = 10 + b
    a(2) = 20 + b
    a(3) = 30 + b
    a(4) = 40 + b
    b = b + a(1) + a(2)
  
  end subroutine

  subroutine kernelr84 ( a, b )
  
    real(8), dimension(4) :: a
    real(8), dimension(10) :: b
    
    integer(4) :: i
    
    a(1) = 10 + b(1) + b(5) + b(9)
    a(2) = 20 + b(2) + b(6) + b(10)
    a(3) = 30 + b(3) + b(7)
    a(4) = 40 + b(4) + b(8)

    do i = 1, 10
    
      b(i) = b(i) + a(1)
    
    end do
  
  end subroutine

  ! min
  subroutine kernelr85 ( a, b )
  
    real(8), dimension(4) :: a
    real(8) :: b
    
    a(1) = 10 + b
    a(2) = 20 + b
    a(3) = 30 + b
    a(4) = 40 + b
    b = a(1) + a(2)
  
  end subroutine

  subroutine kernelr86 ( a, b )
  
    real(8), dimension(4) :: a
    real(8), dimension(10) :: b
    
    integer(4) :: i
    
    a(1) = 10 + b(1) + b(5) + b(9)
    a(2) = 20 + b(2) + b(6) + b(10)
    a(3) = 30 + b(3) + b(7)
    a(4) = 40 + b(4) + b(8)

    do i = 1, 10
    
      b(i) = a(1)
    
    end do
  
  end subroutine



  ! read
  subroutine kerneli41 ( a, b )
  
    real(8), dimension(4) :: a
    integer(4) :: b
    
    a(1) = 10 + b
    a(2) = 20 + b
    a(3) = 30 + b
    a(4) = 40 + b
  
  end subroutine

  subroutine kerneli42 ( a, b )
  
    real(8), dimension(4) :: a
    integer(4), dimension(10) :: b
    
    a(1) = 10 + b(1) + b(5) + b(9)
    a(2) = 20 + b(2) + b(6) + b(10)
    a(3) = 30 + b(3) + b(7)
    a(4) = 40 + b(4) + b(8)
  
  end subroutine

  ! inc
  subroutine kerneli43 ( a, b )
  
    real(8), dimension(4) :: a
    integer(4) :: b
    
    a(1) = 10 + b
    a(2) = 20 + b
    a(3) = 30 + b
    a(4) = 40 + b
    b = b + a(1) + a(2)
  
  end subroutine

  subroutine kerneli44 ( a, b )
  
    real(8), dimension(4) :: a
    integer(4), dimension(10) :: b
    
    integer(4) :: i
    
    a(1) = 10 + b(1) + b(5) + b(9)
    a(2) = 20 + b(2) + b(6) + b(10)
    a(3) = 30 + b(3) + b(7)
    a(4) = 40 + b(4) + b(8)

    do i = 1, 10
    
      b(i) = b(i) + a(1)
    
    end do
  
  end subroutine

  ! min
  subroutine kerneli45 ( a, b )
  
    real(8), dimension(4) :: a
    integer(4) :: b
    
    a(1) = 10 + b
    a(2) = 20 + b
    a(3) = 30 + b
    a(4) = 40 + b
    b = a(1) + a(2)
  
  end subroutine

  subroutine kerneli46 ( a, b )
  
    real(8), dimension(4) :: a
    integer(4), dimension(10) :: b
    
    integer(4) :: i
    
    a(1) = 10 + b(1) + b(5) + b(9)
    a(2) = 20 + b(2) + b(6) + b(10)
    a(3) = 30 + b(3) + b(7)
    a(4) = 40 + b(4) + b(8)

    do i = 1, 10
    
      b(i) = a(1)
    
    end do
  
  end subroutine





end module kernels
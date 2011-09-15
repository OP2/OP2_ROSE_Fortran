
program main

  use OP2_C
  use kernel

  integer(4) :: i

  type(op_set) :: iterationSet
  integer(4) :: setSize
  
  real(8), dimension(:), allocatable :: A, C
  integer(4), dimension(:), allocatable :: B
  type(op_dat) :: p_A, p_B, p_C
  
  setSize = 20

  allocate ( A ( setSize * 4 ) )
  allocate ( B ( setSize * 7 ) )
  allocate ( C ( setSize * 11 ) )
  
  do i = 1, setSize * 4
  
    A ( i ) = mod ( i-1, 4 ) 
  
  end do

  do i = 1, setSize * 7
  
    B ( i ) = mod ( i-1, 7 )
  
  end do
  
  call op_decl_set ( setSize, iterationSet )
  
  call op_decl_dat ( iterationSet, 4, A, p_A  )
  call op_decl_dat ( iterationSet, 7, B, p_B  )
  call op_decl_dat ( iterationSet, 11, C, p_C  )
  
  call op_init ()
      
  ! r8
  call op_par_loop_3 ( sumKernel, iterationSet, &
                     & p_A, -1, OP_ID, OP_READ, &
                     & p_B, -1, OP_ID, OP_READ, &
                     & p_C, -1, OP_ID, OP_WRITE  &                     
                   & )

  do i = 1, 11 * setSize
  
    print *, C(i)
  
  end do


end program main
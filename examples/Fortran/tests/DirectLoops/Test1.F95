program Test1
  use OP2_C
  use UserKernels

  integer(4) :: i

  type(op_set) :: iterationSet
  integer(4) :: setSize
  
  real(8), dimension(:), allocatable :: A, C
  integer(4), dimension(:), allocatable :: B
  type(op_dat) :: opDatA, opDatB, opDatC
  
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
  
  call op_decl_set ( setSize, iterationSet, "" )
  
  call op_decl_dat ( iterationSet, 4, A, opDatA, ""  )
  call op_decl_dat ( iterationSet, 7, B, opDatB, ""  )
  call op_decl_dat ( iterationSet, 11, C, opDatC, ""  )
  
  call op_init ()
      
  call op_par_loop_3 ( newKernel, iterationSet, &
                     & opDatA, -1, OP_ID, OP_READ, &
                     & opDatB, -1, OP_ID, OP_READ, &
                     & opDatC, -1, OP_ID, OP_WRITE  &                     
                   & )

  do i = 1, 11 * setSize
    print *, C(i)
  end do

end program Test1

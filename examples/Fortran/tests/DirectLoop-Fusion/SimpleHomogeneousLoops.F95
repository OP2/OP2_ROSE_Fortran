


program SimpleLoops

  use OP2_C
  use UserKernels

  integer(4) :: iter

  type(op_set) :: edges
  type(op_dat) :: x, y, z, k

  integer(4) :: numEdges, dimX, dimY, dimZ, dimK
  
  real(8), dimension(:), allocatable :: xd, yd, zd, kd

  character(len=2) :: name = 'NO'

  numEdges = 1000000
  dimX = 6
  dimY = 6
  dimZ = 6
  dimK = 6
  
  allocate ( xd ( dimX * numEdges ) )
  allocate ( yd ( dimY * numEdges ) )
  allocate ( zd ( dimZ * numEdges ) )
  allocate ( kd ( dimK * numEdges ) )

  do iter = 1, numEdges * 6

    xd(iter) = iter
    yd(iter) = iter
    zd(iter) = iter
    kd(iter) = iter

  end do

  call op_decl_set ( numEdges, edges, name )

  call op_decl_dat ( edges, 6, xd, x, name )
  call op_decl_dat ( edges, 6, yd, y, name  )
  call op_decl_dat ( edges, 6, zd, z, name  )
  call op_decl_dat ( edges, 6, kd, k, name  )

  do iter = 1, 1000

    call op_par_loop_3 ( ker1, edges, &
                      & x, -1, OP_ID, OP_READ, &
                      & y, -1, OP_ID, OP_READ, &
                      & z, -1, OP_ID, OP_INC &
                    & )

    call op_par_loop_3 ( ker2, edges, &
                      & x, -1, OP_ID, OP_READ, &
                      & y, -1, OP_ID, OP_READ, &
                      & k, -1, OP_ID, OP_INC &
                    & )

  !  call op_par_loop_4 ( ker1, edges, &
  !                     & x, -1, OP_ID, OP_READ, &
  !                     & y, -1, OP_ID, OP_READ, &
  !                     & z, -1, OP_ID, OP_INC, &
  !                     & k, -1, OP_ID, OP_INC, &
  !                   & )
  end do

end program
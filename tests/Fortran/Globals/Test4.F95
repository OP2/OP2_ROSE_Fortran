! This program contains:
! 1) An OP_DAT with WRITE access
! 2) An OP_GBL (array) with READ access

program Test4 
  use OP2_C
  use UserKernels
  
  ! Host declarations
  integer (4) :: setCardinality
  integer (4), dimension(:), allocatable :: hostGlobal
  real (8), dimension (:), allocatable :: hostData

  integer (4) :: i
  
  ! OP2 declarations
  type(op_set) :: set
  type(op_dat) :: deviceData
  type(op_dat) :: global

  setCardinality = 10

  allocate ( hostGlobal (2) )
  allocate ( hostData (4 * setCardinality) )

  print *, "Initialising host data"
  do i = 1, 2
    hostGlobal(i) = i * 2
    print *, "hostGlobal(", i, ")", hostGlobal(i)
  end do

  do i = 1, 4 * setCardinality
    hostData(i) = i
    print *, "hostData(", i, ")", hostData(i)
  end do

  call op_init ()
 
  call op_decl_set ( setCardinality, set, "" )
  call op_decl_dat ( set, 4, hostData, deviceData, "")
  call op_decl_gbl ( hostGlobal, 2, global )

  call op_par_loop_2 ( real8Array_integer4Array, set, &
                     & deviceData, -1, OP_ID,  OP_WRITE, &
                     & global,     -1, OP_GBL, OP_READ)

  print *, "After kernel execution"
  do i = 1, 4 * setCardinality
    print *, "hostData(", i, ")", hostData(i)
  end do

end program Test4


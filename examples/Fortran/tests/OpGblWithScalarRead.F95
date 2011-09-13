program OpGblWithScalarRead
  use OP2
  use UserKernels
  use, intrinsic :: ISO_C_BINDING
  implicit none
  ! Host declarations
  character(kind=c_char,len=7) :: setName = C_CHAR_'theSet'//C_NULL_CHAR
  character(kind=c_char,len=8) :: dataName = C_CHAR_'theData'//C_NULL_CHAR
  integer (4) :: setCardinality
  integer (4) :: hostGlobal
  integer (4) :: i
  real (8), dimension (:), allocatable :: hostData
  ! OP2 declarations
  type(op_set) :: set
  type(op_dat) :: deviceData
  type(op_dat) :: global
  allocate ( hostData (4 * setCardinality) )

  do i = 1, 4 * setCardinality
    hostData(i) = i
  end do

  call op_init ()
  call op_decl_set ( setCardinality, set, setName )
  call op_decl_dat ( set, 4, hostData, deviceData, dataName)
  call op_decl_gbl ( hostGlobal, global )
  call op_par_loop_2 ( kernel_2parameter_floatArray_floatScalar, set, &
  & deviceData, -1, OP_ID,  OP_WRITE, &
  & global,     -1, OP_GBL, OP_READ)

  do i = 1, 4 * setCardinality
    print *, hostData(i)
  end do

end program OpGblWithScalarRead


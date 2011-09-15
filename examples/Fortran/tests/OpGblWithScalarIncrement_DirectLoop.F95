program OpGblWithScalarRead
  use OP2_C
  use UserKernels
  use, intrinsic :: ISO_C_BINDING
  implicit none
  
  ! Host declarations
  character(kind=c_char,len=7) :: setName = C_CHAR_'theSet'//C_NULL_CHAR
  
  integer (4) :: setCardinality
  integer (4) :: hostGlobal1
  integer (4) :: hostGlobal2
  
  ! OP2 declarations
  type(op_set) :: set
  type(op_dat) :: global1
  type(op_dat) :: global2  

  setCardinality = 10
  hostGlobal1 = 0
  hostGlobal2 = 10

  call op_init ()
  
  call op_decl_set ( setCardinality, set, setName )
  call op_decl_gbl ( hostGlobal1, global1 )
  call op_decl_gbl ( hostGlobal2, global2 )
  
  call op_par_loop_2 ( kernel_2parameter_integerScalar_integerScalar, set, &
                     & global1, -1, OP_GBL, OP_INC, &
                     & global2, -1, OP_GBL, OP_READ)

  print *, hostGlobal1

end program OpGblWithScalarRead


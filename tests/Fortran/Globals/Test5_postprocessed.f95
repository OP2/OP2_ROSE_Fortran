# 1 "/home/abetts/SILOET/OP2_ROSE_Fortran/tests/Fortran/Globals/Test51ogLQW.F90"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "/home/abetts/SILOET/OP2_ROSE_Fortran/tests/Fortran/Globals/Test51ogLQW.F90"
! This program contains:
! 1) An OP_GBL (scalar) with INCREMENT access
! 2) An OP_GBL (scalar) with READ access

program Test5 
  use OP2_C
  use UserKernels
  
  ! Host declarations
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
  
  call op_decl_set ( setCardinality, set, "" )
  call op_decl_gbl ( hostGlobal1, global1 )
  call op_decl_gbl ( hostGlobal2, global2 )
  
  call op_par_loop_2 ( integer4Scalar_integer4Scalar, set, &
                     & global1, -1, OP_GBL, OP_INC, &
                     & global2, -1, OP_GBL, OP_READ)

  print *, hostGlobal1

end program Test5


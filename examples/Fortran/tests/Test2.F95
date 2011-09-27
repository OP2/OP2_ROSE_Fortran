! This program contains a read/write to an OP_GBL which is not allowed
! and should therefore be flagged as an error by the compiler

program Test2 

use OP2_C
use UserKernels

use, intrinsic :: ISO_C_BINDING

  implicit none

  ! Host declarations
  integer (4) :: setCardinality
  real(8), dimension(:), allocatable, target :: hostGlobal

  ! OP2 declarations
  type(op_set) :: set
  type(op_dat) :: global

  setCardinality = 10

  allocate ( hostGlobal ( 1 ) )
	
  call op_init ()

  call op_decl_set ( setCardinality, set, "" )
  call op_decl_gbl ( hostGlobal, 1, global )

  call op_par_loop_1 ( real8Array, set, &
                       & global,    -1, OP_GBL, OP_RW &
                       & )

end program Test2

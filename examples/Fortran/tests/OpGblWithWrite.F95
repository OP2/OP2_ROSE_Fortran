program OpGblWithWrite

use OP2_C
use UserKernels

use, intrinsic :: ISO_C_BINDING

  implicit none

  ! Host declarations
  character(kind=c_char,len=7) :: setName = C_CHAR_'theSet'//C_NULL_CHAR
  integer (4) :: setCardinality
  real(8), dimension(:), allocatable, target :: hostGlobal

  ! OP2 declarations
  type(op_set) :: set
  type(op_dat) :: global

  allocate ( hostGlobal ( 1 ) )

  call op_init ()
	
  call op_decl_set ( setCardinality, set, setName )
  call op_decl_gbl ( hostGlobal, 1, global )

  call op_par_loop_1 ( kernel_1parameter_floatArray, set, &
                       & global,    -1, OP_GBL, OP_WRITE &
                       & )

end program OpGblWithWrite

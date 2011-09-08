


program main

  use OP2_C
  use kernels

  integer(4) :: i

  type(op_set) :: iterationSet
  integer(4) :: setSize
  
  real(8), dimension(:), allocatable :: opdata
  type(op_dat) :: opdataD
  
  real(8) :: r8ScalarRead, r8ArrayRead(10), r8ScalarInc, r8ArrayInc(10), r8ScalarMin, r8ArrayMin(10)
  integer(4) :: i4ScalarRead, i4ArrayRead(10), i4ScalarInc, i4ArrayInc(10), i4ScalarMin, i4ArrayMin(10)
  
  type(op_dat) :: r8ScalarReadD, r8ArrayReadD, r8ScalarIncD, r8ArrayIncD, r8ScalarMinD, r8ArrayMinD
  type(op_dat) :: i4ScalarReadD, i4ArrayReadD, i4ScalarIncD, i4ArrayIncD, i4ScalarMinD, i4ArrayMinD
  
  setSize = 1000

  allocate ( opdata ( setSize * 4 ) )
  
  do i = 1, setSize * 4
  
    opdata ( i ) = i
  
  end do
  
  call op_decl_set ( setSize, iterationSet )
  
  call op_decl_dat ( iterationSet, 4, opdata, opdataD )
  
  call op_decl_gbl ( r8ScalarRead, r8ScalarReadD )
  call op_decl_gbl ( r8ArrayRead, 10, r8ArrayReadD )
  call op_decl_gbl ( r8ScalarInc, r8ScalarIncD )
  call op_decl_gbl ( r8ArrayInc, 10, r8ArrayIncD )
  call op_decl_gbl ( r8ScalarMin, r8ScalarMinD )
  call op_decl_gbl ( r8ArrayMin, 10, r8ArrayMinD )

  call op_decl_gbl ( i4ScalarRead, i4ScalarReadD )
  call op_decl_gbl ( i4ArrayRead, 10, i4ArrayReadD )
  call op_decl_gbl ( i4ScalarInc, i4ScalarIncD )
  call op_decl_gbl ( i4ArrayInc, 10, i4ArrayIncD )
  call op_decl_gbl ( i4ScalarMin, i4ScalarMinD )
  call op_decl_gbl ( i4ArrayMin, 10, i4ArrayMinD )
  
  call op_init ()
  
  
  ! r8
  call op_par_loop_2 ( kernelr81, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & r8ScalarReadD, -1, OP_GBL, OP_READ &
                   & )

  call op_par_loop_2 ( kernelr82, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & r8ArrayReadD, -1, OP_GBL, OP_READ &
                   & )

  call op_par_loop_2 ( kernelr83, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & r8ScalarIncD, -1, OP_GBL, OP_INC &
                   & )

  call op_par_loop_2 ( kernelr84, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & r8ArrayIncD, -1, OP_GBL, OP_INC &
                  &  )

  call op_par_loop_2 ( kernelr85, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & r8ScalarMinD, -1, OP_GBL, OP_MIN &
                   & )

  call op_par_loop_2 ( kernelr86, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & r8ArrayMinD, -1, OP_GBL, OP_MIN &
                   & )

  ! i4
  call op_par_loop_2 ( kerneli41, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & i4ScalarReadD, -1, OP_GBL, OP_READ &
                   & )

  call op_par_loop_2 ( kerneli42, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & i4ArrayReadD, -1, OP_GBL, OP_READ &
                   & )

  call op_par_loop_2 ( kerneli43, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & i4ScalarIncD, -1, OP_GBL, OP_INC &
                   & )

  call op_par_loop_2 ( kerneli44, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & i4ArrayIncD, -1, OP_GBL, OP_INC &
                  &  )

  call op_par_loop_2 ( kerneli45, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & i4ScalarMinD, -1, OP_GBL, OP_MIN &
                   & )

  call op_par_loop_2 ( kerneli46, iterationSet, &
                     & opdataD, -1, OP_ID, OP_WRITE, &
                     & i4ArrayMinD, -1, OP_GBL, OP_MIN &
                   & )


                     

end program main
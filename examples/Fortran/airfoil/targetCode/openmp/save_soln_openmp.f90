
module save_soln_openmp

#ifdef _OPENMP
	use omp_lib
#endif

	use OP2_C
	use airfoil_seq
	use OP2Profiling

	real(8), dimension(:), pointer :: argument1
	real(8), dimension(:), pointer :: argument2

	logical :: isFirstTimeExecuting_save_soln = .true.

	contains
	
		subroutine save_soln_caller (	callerArgument1, &
															  & callerArgument2, &
															  & sliceStart, sliceEnd &
														  & )
			real(8), dimension(0:*) :: callerArgument1
			real(8), dimension(0:*) :: callerArgument2
			
			integer(kind = OMP_integer_kind) :: sliceStart
			integer(kind = OMP_integer_kind) :: sliceEnd

			integer(kind = OMP_integer_kind) :: sliceIterator

			integer(kind = OMP_integer_kind) :: threadid
			
			threadid = OMP_get_thread_num() 

			! apply the kernel to each element of the assigned slice of set
			do sliceIterator = sliceStart, sliceEnd-1

				call save_soln ( callerArgument1 ( sliceIterator * 4: sliceIterator * 4 + 4 - 1 ), &
											 & callerArgument2 ( sliceIterator * 4: sliceIterator * 4 + 4 - 1 ) &
										 & )
			end do
			

		
		end subroutine save_soln_caller

	
		function op_par_loop_save_soln ( subroutineName, set, &
																	 & arg1, idx1, ptr1, access1, &
																	 & arg2, idx2, ptr2, access2 &
																 & )

			! use directives	
			use, intrinsic :: ISO_C_BINDING

			! mandatory	
			implicit none

			type(profInfo) :: op_par_loop_save_soln
			
			! formal arguments
			character, dimension(*), intent(in) :: subroutineName
			
			! data set on which we loop
			type(op_set), intent(in) :: set

			! data ids used in the function
			type(op_dat) :: arg1, arg2
			
			! index to be used in first and second pointers
			integer(4), intent(in) :: idx1, idx2
			
			! ptr ids for indirect access to data
			type(op_map) :: ptr1, ptr2
			
			! access values for arguments
			integer(4), intent(in) :: access1, access2

			! local variables
			
			! number of threads
			integer(kind = OMP_integer_kind) :: nthreads = 0

			! thread index
			integer(kind = OMP_integer_kind) :: threadIndex = -1

			! bounds of set slice assigned to each thread
			integer(kind = OMP_integer_kind) :: sliceStart = -1
			integer(kind = OMP_integer_kind) :: sliceEnd = -1

			! initialise timers
			! skipped for now..
			
			! get number of threads
#ifdef _OPENMP
			nthreads = omp_get_max_threads ()
#else
			nthreads = 1
#endif
			
			
			if ( isFirstTimeExecuting_save_soln .eqv. .true. ) then
			
				call c_f_pointer ( arg1%dat, argument1, (/set%size/) )
				call c_f_pointer ( arg2%dat, argument2, (/set%size/) )
			
				isFirstTimeExecuting_save_soln = .false.
			
			end if
			
			
			! apply kernel to each set element (no plan in direct loops)
			
			! private is required in fortran because the declaration is global, and we can't declare local thread
			! variables inside the do below
			
			! notice also that the DO pragma, without PARALLEL, does not include a synchronisation
			
			!$OMP PARALLEL DO PRIVATE(sliceStart,sliceEnd)
			do threadIndex = 0, nthreads-1
			
				sliceStart = ( set%size * (threadIndex) ) / nthreads;
				sliceEnd = ( set%size * ( (threadIndex) + 1 ) ) / nthreads;
				
				call save_soln_caller ( argument1, &
															& argument2, &
															& sliceStart, sliceEnd &
														& )
				
			end do
			!$OMP END PARALLEL DO
						
						
						
			! timing end
			op_par_loop_save_soln%hostTime = 0.0
			op_par_loop_save_soln%kernelTime = 0.0

		end function op_par_loop_save_soln

end module save_soln_openmp
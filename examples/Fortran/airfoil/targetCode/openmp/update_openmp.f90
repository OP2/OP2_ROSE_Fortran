
module update_openmp

#ifdef _OPENMP
	use omp_lib
#endif

	use OP2_C
	use airfoil_seq
	use OP2Profiling

	real(8), dimension(:), pointer :: argument1
	real(8), dimension(:), pointer :: argument2
	real(8), dimension(:), pointer :: argument3
	real(8), dimension(:), pointer :: argument4
	real(8), dimension(:), pointer :: argument5

	logical :: isFirstTimeExecuting_update = .true.

	contains
	
		subroutine update_caller ( callerArgument1, &
														 & callerArgument2, &
														 & callerArgument3, &
														 & callerArgument4, &
														 & callerArgument5, &
														 & sliceStart, sliceEnd &
													 & )

			real(8), dimension(0:*) :: callerArgument1
			real(8), dimension(0:*) :: callerArgument2
			real(8), dimension(0:*) :: callerArgument3
			real(8), dimension(0:*) :: callerArgument4
			real(8), dimension(0:*) :: callerArgument5
			
			integer(kind = OMP_integer_kind) :: sliceStart
			integer(kind = OMP_integer_kind) :: sliceEnd

			integer(kind = OMP_integer_kind) :: sliceIterator

			! apply the kernel to each element of the assigned slice of set
			do sliceIterator = sliceStart, sliceEnd-1

				call update ( callerArgument1 ( sliceIterator * 4: sliceIterator * 4 + 4 - 1 ), &
										& callerArgument2 ( sliceIterator * 4: sliceIterator * 4 + 4 - 1 ), &
										& callerArgument3 ( sliceIterator * 4: sliceIterator * 4 + 4 - 1 ), &
										& callerArgument4 ( sliceIterator * 1: sliceIterator * 1 + 1 - 1 ), &
										& callerArgument5 &
									& )
			end do

		end subroutine update_caller

	
		function op_par_loop_update ( subroutineName, set, &
															  & arg1, idx1, ptr1, access1, &
															  & arg2, idx2, ptr2, access2, &
																& arg3, idx3, ptr3, access3, &
																& arg4, idx4, ptr4, access4, &
																& arg5, idx5, ptr5, access5 &
															& )

			! use directives	
			use, intrinsic :: ISO_C_BINDING

			! mandatory	
			implicit none

			type(profInfo) :: op_par_loop_update
			
			! formal arguments
			character, dimension(*), intent(in) :: subroutineName
			
			! data set on which we loop
			type(op_set), intent(in) :: set

			! data ids used in the function
			type(op_dat) :: arg1, arg2, arg3, arg4, arg5
			
			! index to be used in first and second pointers
			integer(4), intent(in) :: idx1, idx2, idx3, idx4, idx5
			
			! ptr ids for indirect access to data
			type(op_map) :: ptr1, ptr2, ptr3, ptr4, ptr5
			
			! access values for arguments
			integer(4), intent(in) :: access1, access2, access3, access4, access5

			! local variables
			
			! number of threads
			integer(kind = OMP_integer_kind) :: nthreads = 0

			! thread index
			integer(kind = OMP_integer_kind) :: threadIndex = -1

			! bounds of set slice assigned to each thread
			integer(kind = OMP_integer_kind) :: sliceStart = -1
			integer(kind = OMP_integer_kind) :: sliceEnd = -1

			real(8), dimension ( 0:(1 + 64 * 64) -1 ) :: arg5_l

			integer(4) :: iter1, iter2

			! initialise timers
			! skipped for now..
			
			! get number of threads
#ifdef _OPENMP
			nthreads = omp_get_max_threads ()
#else
			nthreads = 1
#endif

			if ( isFirstTimeExecuting_update .eqv. .true. ) then
			
				call c_f_pointer ( arg1%dat, argument1, (/set%size/) )
				call c_f_pointer ( arg2%dat, argument2, (/set%size/) )
				call c_f_pointer ( arg3%dat, argument3, (/set%size/) )
				call c_f_pointer ( arg4%dat, argument4, (/set%size/) )
				call c_f_pointer ( arg5%dat, argument5, (/arg5%dim/) )

				isFirstTimeExecuting_update = .false.
			
			end if

			! apply kernel to each set element (no plan in direct loops)
			
			! private is required in fortran because the declaration is global, and we can't declare local thread
			! variables inside the do below
			
			! notice also that the DO pragma, without PARALLEL, does not include a synchronisation
			
			do iter1 = 0, nthreads-1
				do iter2 = 0, 1 - 1
				
				  arg5_l ( iter2 + iter1 * 64 ) = 0

				end do
			end do
			
			
			!$OMP PARALLEL DO PRIVATE(sliceStart,sliceEnd)
			do threadIndex = 0, nthreads-1
			
				sliceStart = ( set%size * (threadIndex) ) / nthreads;
				sliceEnd = ( set%size * ( (threadIndex) + 1 ) ) / nthreads;

				call update_caller ( argument1, &
													 & argument2, &
													 & argument3, &
													 & argument4, &
													 & arg5_l ( threadIndex * 64: ), &
													 & sliceStart, sliceEnd &
												 & )
			
			end do
			!$OMP END PARALLEL DO
			
			! combine reduction data
			do iter1 = 0, nthreads - 1
				do iter2 = 0, 1 - 1
				
						argument5 ( 1 + iter2 ) = argument5 ( 1 + iter2 ) + arg5_l ( iter2 + iter1 * 64 )

				end do
			end do

		

			! timing end
			op_par_loop_update%hostTime = 0.0
			op_par_loop_update%kernelTime = 0.0

		end function op_par_loop_update

end module update_openmp
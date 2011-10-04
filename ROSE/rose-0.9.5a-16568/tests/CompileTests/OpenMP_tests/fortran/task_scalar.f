******************************************      
* Simplest OMP task example, scalar variables 
* Liao 1/25/2010
******************************************
      program main
      include 'omp_lib.h'
      integer i

!$omp parallel
!$omp single private(i)
      print *, 'using ', omp_get_num_threads(), ' threads' 
      do i = 1, 10
!$omp task 
      print *, 'Item ', i, ' by thread ', omp_get_thread_num()
!$omp end task
      enddo  
!$omp end single
!$omp end parallel
      end 

! Rice test
!
! OFP treats an initial tab followed by any non-whitespace character as a
! continuation line with the non-whitespace character taken to be a
! continuation character.

      program initial_tab
	call f()  ! last line + this one treated as "program p all f()"
      end program

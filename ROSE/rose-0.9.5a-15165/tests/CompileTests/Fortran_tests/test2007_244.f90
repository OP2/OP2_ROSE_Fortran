subroutine example
    logical :: a(9)
    real    :: b(9)

  ! where statement with a string label accepted by gfortran, but not OFP (see test2007_88.f90)
  ! MY_WHERE: where (a) b = 0
  ! where statement with a numeric label
    where (a)
 10   b = 0
    end where

end subroutine example

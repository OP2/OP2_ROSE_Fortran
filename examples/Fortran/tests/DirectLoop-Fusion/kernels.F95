module kernels

  contains

  subroutine ker1 ( x, y, z )

    real(8) :: x(6), y(6), z(6)


    do i = 1, 6

      z(i) = z(i) + x(i) + y(i)

    end do

  end subroutine

  subroutine ker2 ( x, y, k )

    real(8) :: x(6), y(6), k(6)


    do i = 1, 6

      k(i) = k(i) + x(i) - z(i)

    end do

  end subroutine

  subroutine ker3 ( x, y, z, k )

    real(8) :: x(6), y(6), z(6), k(6)

    do i = 1, 6

      z(i) = z(i) + x(i) + y(i)

    end do

    do i = 1, 6

      k(i) = k(i) + x(i) - z(i)

    end do

  end subroutine


end module
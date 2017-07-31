program integral
use mpi

INTEGER rank, size, ierror, tag, status(MPI_STATUS_SIZE), processor, n, my_n, source
REAL (kind=8) a 
REAL (kind=8) b
REAL (kind=8) new_a
REAL (kind=8) new_b
REAL (kind=8) my_a
REAL (kind=8) my_b
REAL (kind=8) total_area
REAL (kind=8) area
REAL (kind=8) wtime

interface INTEGRATE
    function INTEGRATE(a, b)
        REAL(kind=8) :: a
        REAL(kind=8) :: b
        REAL(kind=8) :: INTEGRATE
    end function
end interface

call MPI_INIT(ierror)
call MPI_COMM_SIZE(MPI_COMM_WORLD, size, ierror)
call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierror)

source = 0
a = 0
b = 10

if (rank == 0) then
    wtime = MPI_Wtime()
end if

if (rank == 0) then
    !Send the integral range
    do processor = 1, size -1
        new_a = (((size - processor) * a) + (processor - 1) * b) / (size - 1)
        tag = 1
        call MPI_Send (new_a, 1, MPI_DOUBLE_PRECISION, processor, tag, MPI_COMM_WORLD, ierror)

        new_b = (((size - processor - 1) * a) + (processor * b)) / (size - 1)
        tag = 2
        call MPI_Send (new_b, 1, MPI_DOUBLE_PRECISION, processor, tag, MPI_COMM_WORLD, ierror)
    end do

    total_area = 0
    area = 0
else

    !Receive the integral range
    source = 0
    tag = 1
    call MPI_Recv (my_a, 1, MPI_DOUBLE_PRECISION, source, tag, MPI_COMM_WORLD, status, ierror)

    tag = 2
    call MPI_Recv (my_b, 1, MPI_DOUBLE_PRECISION, source, tag, MPI_COMM_WORLD, status, ierror)

    !Call integrate
    area = INTEGRATE(my_a, my_b)

    write ( *, '(a,i8,a,g14.6)' ) &
      '  Process ', rank, ' contributes area = ', area
end if

call MPI_Reduce(area, total_area, 1, MPI_DOUBLE_PRECISION, MPI_SUM, 0, MPI_COMM_WORLD, ierror)


if (rank == 0) then
    wtime = MPI_Wtime() - wtime
    write ( *, '(a,g24.16)' ) '  Estimate = ', total_area
    write ( *, '(a,g14.6)' ) '  Time     = ', wtime
end if

call MPI_FINALIZE(ierror)

if (rank == 0) then
    write ( *, '(a)' ) ' '
    write ( *, '(a)' ) 'test integral:'
    write ( *, '(a)' ) '  Normal end of execution.'
end if
stop
END

FUNCTION INTEGRATE(a, b)
    REAL(kind = 8) a
    REAL(kind = 8) b
    REAL(kind = 8) x
    REAL(kind = 8) y
    REAL(kind = 8) step
    REAL(kind = 8) INTEGRATE

    interface XSQUARED
        function XSQUARED(x)
            REAL(kind = 8) :: x
            REAL(kind = 8) :: XSQUARED
        end function
    end interface

    step = 0.0001
    x = a
    INTEGRATE = 0
    DO WHILE (x < b)
        x = x + step
        y = XSQUARED(x)
        INTEGRATE = INTEGRATE + (y * step)
    END DO
    RETURN
END FUNCTION


FUNCTION XSQUARED(x)
    REAL (kind=8) x
    REAL (kind=8) XSQUARED
    XSQUARED = x ** 2
    RETURN
END FUNCTION

program hello
include 'mpif.h'

INTEGER rank, size, ierror, tag, status(MPI_STATUS_SIZE), processor, n, my_n
REAL a, b, new_a, new_b, my_a, my_b, total_area, area, wtime

call MPI_INIT(ierror)
call MPI_COMM_SIZE(MPI_COMM_WORLD, size, ierror)
call MPI_COMM_RANK(MPI_COMM_WORLD, rank, ierror)

if (rank == 0) then
    my_n = n / (size - 1)
    n = (size - 1) * my_n

    a = 0
    b = 10

    wtime = MPI_Wtime()
end if

call MPI_Bcast (my_n, 1, MPI_INTEGER, source, MPI_COMM_WORLD, ierror)

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

    ! Call integate
    area = INTEGRATE(a, b)

    write ( *, '(a,i8,a,g14.6)' ) &
      '  Process ', rank, ' contributes area = ', area
end if

call MPI_Reduce (area, total_area, 1, MPI_DOUBLE_PRECISION, MPI_SUM, 0, MPI_COMM_WORLD, ierror)


if (rank == 0) then
    wtime = MPI_Wtime() - wtime
    write ( *, '(a,g24.16)' ) '  Estimate = ', total
    write ( *, '(a,g14.6)' ) '  Time     = ', wtime
end if

call MPI_FINALIZE(ierror)

if ( my_id == master ) then
    write ( *, '(a)' ) ' '
    write ( *, '(a)' ) 'QUAD_MPI:'
    write ( *, '(a)' ) '  Normal end of execution.'
end if
stop
END

FUNCTION INTEGRATE(a, b)
    REAL x,y,area,step
    step = 0.0001
    x = a
    area = 0
    DO WHILE (x < b)
        x = x + step
        y = XSQUARED(x)
        area = area + (y * step)
    END DO
    INTEGRATE = area
    RETURN
END FUNCTION


FUNCTION XSQUARED(x)
    REAL x, y
    y = x ** 2
    XSQUARED = y
    RETURN
END FUNCTION
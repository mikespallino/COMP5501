PROGRAM int_aprox

include 'mpif.h'

REAL ans, a, b
a = 0.0
b = 10
ans = INTEGRATE(a, b)
PRINT *,"Area: ", ans
END

REAL FUNCTION INTEGRATE(a, b)
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


REAL FUNCTION XSQUARED(x)
    REAL x, y
    y = x ** 2
    XSQUARED = y
    RETURN
END FUNCTION

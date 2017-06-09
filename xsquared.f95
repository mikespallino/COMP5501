PROGRAM xsquared

REAL x,y,area,step
step = 0.0001
x = 0
area = 0
DO WHILE (x < 100000)
    x = x + step
    y = XSQUARED(x)
    area = area + (y * step)
END DO
PRINT *,"Area: ", area
END


REAL FUNCTION XSQUARED(x)
REAL x
XSQUARED = x ** 2
RETURN

END
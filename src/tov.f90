PROGRAM tov



END

FUNCTION rk4_coupled(f, g, t0, f_y0, g_y0, t_max, n)
    REAL EXERTNAL f, g, f_i, g_i k1_f, k1_g, k2_f, k2_g, k3_f, k3_g, k4_f, k4_g, n, i, h, t_max, t0

    REAL dt(n+1)
    REAL df(n+1)
    REAL dg(n+1)
    h = (t_max - t0) / n
    
    i = 1
    DO i, n+1
        k1_f = f(t,         f_i,              g_i)
        k1_g = g(t,         f_i,              g_i)
        k2_f = f(t + 0.5*h, f_i + h*0.5*k1_f, g_i + h*0.5*k1_g)
        k2_g = g(t + 0.5*h, f_i + h*0.5*k1_f, g_i + h*0.5*k1_g)
        k3_f = f(t + 0.5*h, f_i + h*0.5*k2_f, g_i + h*0.5*k2_g)
        k3_g = g(t + 0.5*h, f_i + h*0.5*k2_f, g_i + h*0.5*k2_g)
        k4_f = f(t + h,     f_i + h*k3_f,     g_i + h*k3_g)
        k4_g = g(t + h,     f_i + h*k3_f,     g_i + h*k3_g)

        dt[i] = t = t0 + i * h
        df[i] = f_i = f_i + h/6 * (k1_f + 2*k2_f + 2*k3_f + k4_f)
        dg[i] = g_i = g_i + h/6 * (k1_g + 2*k2_g + 2*k3_g + k4_g)
    END DO

END FUNCTION

FUNCTION mass(r, p, m)
    REAL r, p, m, mass
    mass = BETA * (r ** 2) * (p ** (1/GAMMA))
END FUNCTION

FUNCTION tov_coupled(r, p, m)
    REAL r, p, m, tov_coupled
    tov_coupled = -1 * (ALPHA * (p ** 1/GAMMA) * m) / (r ** 2)
END FUNCTION
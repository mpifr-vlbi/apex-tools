AC_INIT([m5toneAPEX], [1.0.0], [Jan Wagner <jwagner@mpifr.de>])
AM_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE

AC_PROG_CC
AM_PROG_CC_C_O
AM_SANITY_CHECK

PKG_CHECK_MODULES(FFTW3F, fftw3f, [hasfftw=true], [hasfftw=false])
AC_SUBST(FFTW3F_CFLAGS)
AC_SUBST(FFTW3F_LIBS)
AM_CONDITIONAL(WITH_FFTW, $hasfftw)

AC_OUTPUT([ \
        Makefile \
	])



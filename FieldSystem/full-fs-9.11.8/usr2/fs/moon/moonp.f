      subroutine moonp(fjd,c1,c2,c3)
C
C     THIS SUBROUTINE COMPUTES THE COORDINATES OF THE MOON FROM
C     TRIGONOMETRIC SERIES TO AN ACCURACY OF ABOUT 1 ARCMINUTE.
C
C     FJD = JULIAN DATE OF INTEREST (IN)
C     C1,C2,C3 = COORDINATE VALUES RETURNED (OUT)
C          C1=RIGHT ASCENSION (HOURS AND DECIMAL)
C          C2=DECLINATION (DEGREES AND DECIMAL)
C          C3=GEOCENTRIC DISTANCE (EARTH RADII)
C
      implicit double precision (a-h,o-z)
      real a,b,c,d,e,f,g
C
      data rps/0.48481368110954d-5/
C
      dasin(x) = datan2(x,dsqrt(dabs(1d0-x*x)))
C
      call papvl(fjd,ct,elm,emm,eom,alm,ds,els,ems,elven)
      a=emm 
      b=alm 
      c=ds
      d=eom 
      e=els 
      f=ems 
      g=elven 
      cfac=60.40974d0 
      xadd1=elm 
      xadd2=elm 
      z=ct
      v =   +3.95580e-01*  sin(          +b          +d)
      v = v +8.20000e-02*  sin(          +b) 
      v = v +3.25700e-02*  sin(     a    -b          -d) 
      v = v +1.09200e-02*  sin(     a    +b          +d) 
      v = v +6.66000e-03*  sin(     a    -b) 
      v = v -6.44000e-03*  sin(     a    +b -2.*c    +d) 
      v = v -3.31000e-03*  sin(          +b -2.*c    +d) 
      v = v -3.04000e-03*  sin(          +b -2.*c) 
      v = v -2.40000e-03*  sin(     a    -b -2.*c    -d) 
      v = v +2.26000e-03*  sin(     a    +b) 
      v = v -1.08000e-03*  sin(     a    +b -2.*c) 
      v = v -7.90000e-04*  sin(          +b          -d) 
      v = v +7.80000e-04*  sin(          +b +2.*c    +d) 
      v = v +6.60000e-04*  sin(          +b          +d          -f )
      v = v -6.20000e-04*  sin(          +b          +d          +f )
      v = v -5.00000e-04*  sin(     a    -b -2.*c) 
      v = v +4.50000e-04*  sin( +2.*a    +b          +d)
      v = v -3.10000e-04*  sin( +2.*a    +b -2.*c    +d)
      v = v -2.70000e-04*  sin(     a    +b -2.*c    +d          +f )
      v = v -2.40000e-04*  sin(          +b -2.*c    +d          +f )
      v = v -2.10000e-04*z*sin(          +b          +d)
      v = v +1.80000e-04*  sin(          +b    -c    +d)
      v = v +1.60000e-04*  sin(          +b +2.*c)
      v = v +1.60000e-04*  sin(     a    -b          -d          -f )
      v = v -1.60000e-04*  sin( +2.*a    -b          -d)
      v = v -1.50000e-04*  sin(          +b -2.*c                +f )
      v = v -1.20000e-04*  sin(     a    -b -2.*c    -d          +f )
      v = v -1.10000e-04*  sin(     a    -b          -d          +f )
      v = v +9.00000e-05*  sin(     a    +b          +d          -f )
      v = v +9.00000e-05*  sin( +2.*a    +b)
      v = v +8.00000e-05*  sin( +2.*a    -b)
      v = v +8.00000e-05*  sin(     a    +b +2.*c    +d)
      v = v -8.00000e-05*  sin(       +3.*b -2.*c    +d)
      v = v +7.00000e-05*  sin(     a    -b +2.*c)
      v = v -7.00000e-05*  sin( +2.*a    -b -2.*c    -d)
      v = v -7.00000e-05*  sin(     a    +b          +d          +f )
      v = v -6.00000e-05*  sin(          +b    +c    +d)
      v = v +6.00000e-05*  sin(          +b -2.*c                -f )
      v = v +6.00000e-05*  sin(     a    -b          +d) 
      v = v +6.00000e-05*  sin(          +b +2.*c    +d          -f )
      v = v -5.00000e-05*  sin(     a    +b -2.*c                +f )
      v = v -4.00000e-05*  sin( +2.*a    +b -2.*c) 
      v = v +4.00000e-05*  sin(     a -3.*b          -d) 
      v = v +4.00000e-05*  sin(     a    -b                      -f )
      v = v -3.00000e-05*  sin(     a    -b                      +f )
      v = v +3.00000e-05*  sin(          +b    -c) 
      v = v +3.00000e-05*  sin(          +b -2.*c    +d          -f )
      v = v -3.00000e-05*  sin(          +b -2.*c    -d) 
      v = v +3.00000e-05*  sin(     a    +b -2.*c    +d          -f )
      v = v +3.00000e-05*  sin(          +b                      -f )
      v = v -3.00000e-05*  sin(          +b    -c    +d          -f )
      v = v -2.00000e-05*  sin(     a    -b -2.*c                +f )
      v = v -2.00000e-05*  sin(          +b                      +f )
      v = v +2.00000e-05*  sin(     a    +b    -c    +d) 
      v = v -2.00000e-05*  sin(     a    +b          -d) 
      v = v +2.00000e-05*  sin( +3.*a    +b          +d) 
      v = v -2.00000e-05*  sin( +2.*a    -b -4.*c    -d) 
      v = v +2.00000e-05*  sin(     a    -b -2.*c    -d          -f )
      v = v -2.00000e-05*z*sin(     a    -b          -d) 
      v = v -2.00000e-05*  sin(     a    -b -4.*c    -d) 
      v = v -2.00000e-05*  sin(     a    +b -4.*c) 
      v = v -2.00000e-05*  sin( +2.*a    -b -2.*c) 
      v = v +2.00000e-05*  sin(     a    +b +2.*c) 
      v = v +2.00000e-05*  sin(     a    +b                      -f )
      u           =1.d0 
      u = u -1.08280e-01*  cos(     a) 
      u = u -1.88000e-02*  cos(     a       -2.*c) 
      u = u -1.47900e-02*  cos(             +2.*c) 
      u = u +1.81000e-03*  cos( +2.*a       -2.*c) 
      u = u -1.47000e-03*  cos( +2.*a) 
      u = u -1.05000e-03*  cos(             +2.*c     -f ) 
      u = u -7.50000e-04*  cos(     a       -2.*c     +f ) 
      u = u -6.70000e-04*  cos(     a     -f ) 
      u = u +5.70000e-04*  cos(                +c) 
      u = u +5.50000e-04*  cos(     a     +f ) 
      u = u -4.60000e-04*  cos(     a       +2.*c) 
      u = u +4.10000e-04*  cos(     a -2.*b) 
      u = u +2.40000e-04*  cos(     +f ) 
      u = u +1.70000e-04*  cos(             +2.*c     +f ) 
      u = u +1.30000e-04*  cos(     a       -2.*c     -f ) 
      u = u -1.00000e-04*  cos(     a       -4.*c) 
      u = u -9.00000e-05*  cos(                +c     +f ) 
      u = u +7.00000e-05*  cos( +2.*a       -2.*c     +f ) 
      u = u +6.00000e-05*  cos( +3.*a       -2.*c) 
      u = u +6.00000e-05*  cos(       +2.*b -2.*c) 
      u = u -5.00000e-05*  cos(             +2.*c  -2.*f ) 
      u = u -5.00000e-05*  cos( +2.*a       -4.*c) 
      u = u +5.00000e-05*  cos(     a +2.*b -2.*c) 
      u = u -5.00000e-05*  cos(     a          -c) 
      u = u -4.00000e-05*  cos(     a       +2.*c     -f ) 
      u = u -4.00000e-05*  cos( +3.*a) 
      u = u -3.00000e-05*  cos(     a       -4.*c     +f ) 
      u = u -3.00000e-05*  cos( +2.*a -2.*b) 
      u = u -3.00000e-05*  cos(       +2.*b) 
      w =   +1.04780e-01*  sin(     a)
      w = w -4.10500e-02*  sin(       +2.*b       +2.*d) 
      w = w -2.13000e-02*  sin(     a       -2.*c) 
      w = w -1.77900e-02*  sin(       +2.*b          +d) 
      w = w +1.77400e-02*  sin(                      +d) 
      w = w +9.87000e-03*  sin(             +2.*c) 
      w = w -3.38000e-03*  sin(     a -2.*b       -2.*d) 
      w = w -3.09000e-03*  sin(     +f ) 
      w = w -1.90000e-03*  sin(       +2.*b) 
      w = w -1.44000e-03*  sin(     a                +d) 
      w = w -1.44000e-03*  sin(     a -2.*b          -d) 
      w = w -1.13000e-03*  sin(     a +2.*b       +2.*d) 
      w = w -9.40000e-04*  sin(     a       -2.*c     +f ) 
      w = w -9.20000e-04*  sin( +2.*a       -2.*c) 
      w = w +7.10000e-04*  sin(             +2.*c     -f ) 
      w = w +7.00000e-04*  sin( +2.*a) 
      w = w +6.70000e-04*  sin(     a +2.*b -2.*c +2.*d) 
      w = w +6.60000e-04*  sin(       +2.*b -2.*c    +d) 
      w = w -6.60000e-04*  sin(             +2.*c    +d) 
      w = w +6.10000e-04*  sin(     a     -f ) 
      w = w -5.80000e-04*  sin(                +c) 
      w = w -4.90000e-04*  sin(     a +2.*b          +d) 
      w = w -4.90000e-04*  sin(     a                -d) 
      w = w -4.20000e-04*  sin(     a     +f ) 
      w = w +3.40000e-04*  sin(       +2.*b -2.*c +2.*d) 
      w = w -2.60000e-04*  sin(       +2.*b -2.*c) 
      w = w +2.50000e-04*  sin(     a -2.*b -2.*c -2.*d) 
      w = w +2.40000e-04*  sin(     a -2.*b) 
      w = w +2.30000e-04*  sin(     a +2.*b -2.*c    +d) 
      w = w +2.30000e-04*  sin(     a       -2.*c    -d) 
      w = w +1.90000e-04*  sin(     a       +2.*c) 
      w = w +1.20000e-04*  sin(     a       -2.*c     -f ) 
      w = w +1.10000e-04*  sin(     a       -2.*c    +d) 
      w = w +1.10000e-04*  sin(     a -2.*b -2.*c    -d) 
      w = w -1.00000e-04*  sin(             +2.*c     +f ) 
      w = w +9.00000e-05*  sin(     a          -c) 
      w = w +8.00000e-05*  sin(                +c     +f ) 
      w = w -8.00000e-05*  sin(       +2.*b +2.*c +2.*d) 
      w = w -8.00000e-05*  sin(                   +2.*d) 
      w = w -7.00000e-05*  sin(       +2.*b       +2.*d     -f ) 
      w = w +6.00000e-05*  sin(       +2.*b       +2.*d     +f ) 
      w = w -5.00000e-05*  sin(     a +2.*b) 
      w = w +5.00000e-05*  sin( +3.*a) 
      w = w -5.00000e-05*  sin(     a                +16.*e      -18.*g)
      w = w -5.00000e-05*  sin( +2.*a +2.*b       +2.*d) 
      w = w +4.00000e-05*z*sin(       +2.*b       +2.*d) 
      w = w +4.00000e-05*  cos(     a               +16.*e       -18.*g)
      w = w -4.00000e-05*  sin(     a -2.*b +2.*c) 
      w = w -4.00000e-05*  sin(     a       -4.*c) 
      w = w -4.00000e-05*  sin( +3.*a       -2.*c) 
      w = w -4.00000e-05*  sin(       +2.*b +2.*c    +d) 
      w = w -4.00000e-05*  sin(             +2.*c    -d) 
      w = w -3.00000e-05*  sin(  +2.*f ) 
      w = w -3.00000e-05*  sin(     a       -2.*c  +2.*f ) 
      w = w +3.00000e-05*  sin(       +2.*b -2.*c    +d     +f ) 
      w = w -3.00000e-05*  sin(             +2.*c    +d     -f ) 
      w = w +3.00000e-05*  sin( +2.*a +2.*b -2.*c +2.*d) 
      w = w +3.00000e-05*  sin(             +2.*c  -2.*f ) 
      w = w -3.00000e-05*  sin( +2.*a       -2.*c     +f ) 
      w = w +3.00000e-05*  sin(     a +2.*b -2.*c +2.*d     +f ) 
      w = w -3.00000e-05*  sin( +2.*a       -4.*c) 
      w = w +2.00000e-05*  sin(       +2.*b -2.*c +2.*d     +f ) 
      w = w -2.00000e-05*  sin( +2.*a +2.*b          +d) 
      w = w -2.00000e-05*  sin( +2.*a                -d) 
      w = w +2.00000e-05*z*cos(     a                +16.*e      -18.*g)
      w = w +2.00000e-05*  sin(             +4.*c) 
      w = w -2.00000e-05*  sin(       +2.*b    -c +2.*d) 
      w = w -2.00000e-05*  sin(     a +2.*b -2.*c) 
      w = w -2.00000e-05*  sin( +2.*a                +d) 
      w = w -2.00000e-05*  sin( +2.*a -2.*b          -d)
      w = w +2.00000e-05*  sin(     a       +2.*c     -f ) 
      w = w +2.00000e-05*  sin( +2.*a     -f ) 
      w = w -2.00000e-05*  sin(     a       -4.*c     +f ) 
      w = w +2.00000e-05*z*sin(     a                +16.*e      -18.*g)
      w = w -2.00000e-05*  sin(     a -2.*b       -2.*d     -f ) 
      w = w +2.00000e-05*  sin( +2.*a -2.*b       -2.*d) 
      w = w -2.00000e-05*  sin(     a       +2.*c    +d) 
      w = w -2.00000e-05*  sin(     a -2.*b +2.*c    -d) 
      dist=dsqrt(u) 
      delta=dist*cfac 
      ra=((dasin(w/dsqrt(u-v**2))+xadd1)/rps)/3600.d0 
      if (ra .lt. 0.d0)ra=ra+360.d0
      ra=ra/15.d0
      dec=(dasin(v/dist)/rps)/3600.d0
      c1=ra
      c2=dec
      c3=delta
C
      return
      end

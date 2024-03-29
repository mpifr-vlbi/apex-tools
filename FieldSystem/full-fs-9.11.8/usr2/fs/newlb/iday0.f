C@IDAY0
      INTEGER FUNCTION IDAY0 ( IYEAR , MONTH )
C     Get # elapsed days in year to date  TAC 760102 
C 
C     THIRTY DAYS HATH SEPTEMBER, APRIL, JUNE AND NO WONDER . . . . .
C 
C-----IF 1<=MONTH<=12, THIS FUNCTION RETURNS THE NUMBER OF ELAPSED DAYS 
C     ON DAY NUMBER ZERO OF A GIVEN MONTH........ 
C              I.E., IF MONTH = 2 (FEB), IDAY0 = 31 ( = JAN 31) 
C     WITH LEAP YEARS ACCOUNTED FOR PROPERLY. 
C 
C-----IF MONTH <=0 OR IF MONTH >=13, IDAY0 = NUMBER OF DAYS IN IYEAR. 
C 
C     T.A.C.                                     02 JAN 1976
C     WEH  not qute 2000 IS a leap year          02 SEP 1998
C 
C-----MAXYR = MAXIMUM DAYS IN IYEAR:
      MAXYR ( IYR )   =     366 
     *     -     ( ( MOD ( IYR        ,    4 ) + 3 ) /    4 ) 
     *     -     ( ( MOD ( IYR +   99 ,  100 ) + 1 ) /  100 ) 
     *     +     ( ( MOD ( IYR +  399 ,  400 ) + 1 ) /  400 )
C 2000 is a leap year
C    *     -     ( ( MOD ( IYR + 1999 , 2000 ) + 1 ) / 2000 ) 
C                                       WHEW! 
C 
C-----TEST MONTH: 
C 
      IF ( ( MONTH .LE. 0 ) .OR. ( MONTH .GT. 12 ) ) GO TO 12 
C 
C-----RETURN DAYS TO START OF MONTH:
C 
      IDAY0      =     31 * ( MONTH - 1 ) 
     X - INT ( 2.2 + 0.4 * FLOAT ( MONTH ) + 365 - MAXYR (IYEAR) )
     X          * ( ( MONTH + 9 ) / 12 )
      RETURN
C 
C-----RETURN MAXYR: 
C 
12    IDAY0 = MAXYR ( IYEAR ) 
      RETURN
      END 

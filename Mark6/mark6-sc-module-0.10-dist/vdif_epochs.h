/*
 * $Id: vdif_epochs.h 1694 2013-12-27 20:31:27Z gbc $
 *
 * A table of UNIX clocks corresponding to the vdif epochs
 * If you want to know when the data was actually taken, you
 * need a table of leap seconds (or the UTC-TAI offset at the
 * time the DBE was initialized) and still need that correction.
 */

#ifndef vdif_epochs_h
#define vdif_epochs_h

#define NUM_VDIF_EPOCHS 64
static int vdif_epochs[NUM_VDIF_EPOCHS] = {
    /*  0@0.0== */ 946684800,
    /*  1@0.0== */ 962409600,
    /*  2@0.0== */ 978307200,
    /*  3@0.0== */ 993945600,
    /*  4@0.0== */ 1009843200,
    /*  5@0.0== */ 1025481600,
    /*  6@0.0== */ 1041379200,
    /*  7@0.0== */ 1057017600,
    /*  8@0.0== */ 1072915200,
    /*  9@0.0== */ 1088640000,
    /* 10@0.0== */ 1104537600,
    /* 11@0.0== */ 1120176000,
    /* 12@0.0== */ 1136073600,
    /* 13@0.0== */ 1151712000,
    /* 14@0.0== */ 1167609600,
    /* 15@0.0== */ 1183248000,
    /* 16@0.0== */ 1199145600,
    /* 17@0.0== */ 1214870400,
    /* 18@0.0== */ 1230768000,
    /* 19@0.0== */ 1246406400,
    /* 20@0.0== */ 1262304000,
    /* 21@0.0== */ 1277942400,
    /* 22@0.0== */ 1293840000,
    /* 23@0.0== */ 1309478400,
    /* 24@0.0== */ 1325376000,
    /* 25@0.0== */ 1341100800,
    /* 26@0.0== */ 1356998400,
    /* 27@0.0== */ 1372636800,
    /* 28@0.0== */ 1388534400,
    /* 29@0.0== */ 1404172800,
    /* 30@0.0== */ 1420070400,
    /* 31@0.0== */ 1435708800,
    /* 32@0.0== */ 1451606400,
    /* 33@0.0== */ 1467331200,
    /* 34@0.0== */ 1483228800,
    /* 35@0.0== */ 1498867200,
    /* 36@0.0== */ 1514764800,
    /* 37@0.0== */ 1530403200,
    /* 38@0.0== */ 1546300800,
    /* 39@0.0== */ 1561939200,
    /* 40@0.0== */ 1577836800,
    /* 41@0.0== */ 1593561600,
    /* 42@0.0== */ 1609459200,
    /* 43@0.0== */ 1625097600,
    /* 44@0.0== */ 1640995200,
    /* 45@0.0== */ 1656633600,
    /* 46@0.0== */ 1672531200,
    /* 47@0.0== */ 1688169600,
    /* 48@0.0== */ 1704067200,
    /* 49@0.0== */ 1719792000,
    /* 50@0.0== */ 1735689600,
    /* 51@0.0== */ 1751328000,
    /* 52@0.0== */ 1767225600,
    /* 53@0.0== */ 1782864000,
    /* 54@0.0== */ 1798761600,
    /* 55@0.0== */ 1814400000,
    /* 56@0.0== */ 1830297600,
    /* 57@0.0== */ 1846022400,
    /* 58@0.0== */ 1861920000,
    /* 59@0.0== */ 1877558400,
    /* 60@0.0== */ 1893456000,
    /* 61@0.0== */ 1909094400,
    /* 62@0.0== */ 1924992000,
    /* 63@0.0== */ 1940630400
};

#endif /* vdif_epochs_h */

/*
 * eof
 */
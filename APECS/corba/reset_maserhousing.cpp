/**********************************************************
  $Header$
***********************************************************/

#include <stdio.h>
#include <string.h>
#include "yacc_maserhousing.h"

#include "struct.h"

static Program Pdefault = {
    0,               /* status             */
    SET,             /* usage of header    */
    SET,             /* usage of long form */
};

void reset()
{
    memcpy(&program, &Pdefault, sizeof(Program));
}

/* pname, putpname

   Put/get program name for logging 
*/
#include <string.h>

static char sname[5]={' ',' ',' ',' ',' '};

void putpname(name)
char *name;
{
  memcpy(sname,name,5);
}

void pname(name)
char *name;
{
  memcpy(name,sname,5);
}


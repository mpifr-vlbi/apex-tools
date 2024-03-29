#include <string.h>

#define K4DEFAULT 10

find_delay__(nadev,ibuf,nchar,oldcmd,timnow,timlst)
char nadev[2];
char *ibuf;
int *nchar;
int *oldcmd;
double *timnow,*timlst;
{
  char *l4cmd[3]={"CH","FR","RD"};
  char *v4cmd[3]={"CH","IF","SB"};
  char buf[512];
  int delay,i,newcmd,isleep;
  /* old cmd:        CH FR RD */       
  int l4del[ ][3]={ {50,50,-1}, /* new cmd: CH */
		    {50,50,-1}, /* new cmd: FR */
		    {50,50,-1}};/* new cmd: RD */

  /* old cmd:        CH IF SB */       
  int v4del[ ][3]={ { 0, 0, 0}, /* new cmd: CH */
		    { 0, 0, 0}, /* new cmd: IF */
		    { 0, 0, 0}};/* new cmd: SB */

  strncpy(buf,ibuf,*nchar);
  buf[*nchar]=0;
  
  delay=-1;
  if(strncmp(nadev,"l4",2)==0) {
    newcmd=-1;
    for (i=0;i<sizeof(l4cmd)/sizeof(char *);i++) {
      if(NULL!=strstr(buf,l4cmd[i])) {
	newcmd=i;
	break;
      }
    }
    if(newcmd!=-1 && *oldcmd!=-1)
      delay=l4del[newcmd][*oldcmd];
    *oldcmd=newcmd;
    if(delay==-1)
      delay=K4DEFAULT;
  } else if(strncmp(nadev,"v4",2)==0) {
    newcmd=-1;
    for (i=0;i<sizeof(v4cmd)/sizeof(char *);i++) {
      if(NULL!=strstr(buf,v4cmd[i])) {
	newcmd=i;
	break;
      }
    }
    if(newcmd!=-1 && *oldcmd!=-1)
      delay=v4del[newcmd][*oldcmd];
    *oldcmd=newcmd;
    if(delay==-1)
      delay=K4DEFAULT;
  } else if (strncmp(nadev,"r1",2)==0) {
    delay=K4DEFAULT;
  } else if (strncmp(nadev,"la",2)==0) {
    delay=K4DEFAULT;
  } else if (strncmp(nadev,"lb",2)==0) {
    delay=K4DEFAULT;
  } else if (strncmp(nadev,"va",2)==0) {
    delay=K4DEFAULT;
  } else if (strncmp(nadev,"vb",2)==0) {
    delay=K4DEFAULT;
  }
 
  if(delay== -1)
    return;
/*
  printf(" timnow %lf timlst %lf timnow-timlst %lf delay %d\n",
	 *timnow,*timlst,*timnow-*timlst,delay);
*/
  if(*timnow-*timlst < delay+1){
    isleep=1+delay+.5-(*timnow-*timlst);
    rte_sleep(isleep);
/*
    printf(" isleep %d\n",isleep);
*/
  }
}

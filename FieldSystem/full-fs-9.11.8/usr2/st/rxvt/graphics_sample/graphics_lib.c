#include "rxvt_graphics.h"

FILE *infd;

long CreateWin(int x, int y, int w, int h)
{
  char line[100], *tline;
  long id;
  int n = 0;
  char c[20];

  fflush(stdout);
  printf("\033GW%d;%d;%d;%d:",x,y,w,h);
  fflush(stdout);
  n=0;
  while(n != 1)
    {
      tline = fgets(line,99,infd);
      if(tline != NULL)
	n = sscanf(line,"\033W%ld",&id);
    }
  return id;
}


void StartLine(long id, int x, int y)
{
  printf("\033GL%ld;%d;%d",id,x,y);
}

void ExtendLine(int x, int y)
{
  printf(";%d;%d",x,y);
}
void EndLine(void)
{
  printf(":");
}

void StartPoint(long id, int x, int y)
{
  printf("\033GP%ld;%d;%d",id,x,y);
}



void ExtendPoint(int x, int y)
{
  printf(";%d;%d",x,y);
}

void EndPoint(void)
{
  printf(":");
}


void ClearWindow(long id)
{
  printf("\033GC%ld:",id);
}

void QueryWin(long id, int *nfwidth, int *nfheight)
{
  int n,id1,x,y,width,height,fwidth,fheight;
  char line[100], *tline;

  printf("\033GG%ld:",id);
  fflush(stdout);
  n=0;
  while(n == 0)
    {
      tline = fgets(line,99,infd);
      if(tline != NULL)
	n = sscanf(line,"\033G%ld %ld %ld %ld %ld %ld %ld %ld %ld",&id1,
		   &x,&y,&width,&height,&fwidth,&fheight,nfwidth,nfheight);
    }
}


void StartText(long id, int x, int y, int mode, char *text)
{
  printf("\033GT%ld;%d;%d;%d;%d:%s",id,x,y,mode, strlen(text),text);
  fflush(stdout);
}

void StartFillAreas(long id,int x1, int y1, int x2, int y2)
{
  printf("\033GF%ld;%d;%d;%d;%d",id,x1,y1,x2,y2);
}

void AddFillAreas(int x1, int y1, int x2, int y2)
{
  printf(";%d;%d;%d;%d",x1,y1,x2,y2);
}

void EndFillAreas(void)
{
  printf(":");
}

void ForeColor(int color)
{
  printf("\033[%dm",color+30);
}

void DefaultRendition(void)
{
  printf("\033[0m");
}

int WaitForCarriageReturn(long *win, int *x, int *y)
{
  char line[100];
  int i;

  fgets(line,99,infd);
  line[99] = 0;
  for(i=0;i<strlen(line);i++)
    {
      if(strncmp(&line[i],"\033P",2)==0)
	{
	  sscanf(&line[i+2],"%ld;%d;%d",win,x,y);
	  return 1;
	}
      if(strncmp(&line[i],"\033R",2)==0)
	{
	  sscanf(&line[i+2],"%ld;%d;%d",win,x,y);
	  return 2;
	}
    }
  return 0;
  
}

int fno2;
static struct termios ttmode;

void InitializeGraphics(int scroll_text_up)
{
  int fno,val,i;
  char *screen_tty;
  struct winsize winsize;

  fno = fileno(stdout);
  if(!isatty(fno))
    {
      fprintf(stderr,"rxvt_graphics: stdout must be a tty! \n");
      exit(1);
    }
  screen_tty = ttyname(fno);
  ioctl(fno,TCGETS,(char *)&ttmode);
  ttmode.c_lflag &= ~ECHO;
  ioctl(fno,TCSETS,(char *)&ttmode);

  infd= fopen(screen_tty,"rw");

  fno2 = fileno(infd);
  ioctl(fno2,TCGETS,(char *)&ttmode);
  ttmode.c_lflag &= ~ECHO;
  ioctl(fno2,TCSETS,(char *)&ttmode);
  
  if(scroll_text_up)
    {
      val = ioctl(fno,TIOCGWINSZ,&winsize);
      for(i=0;i<winsize.ws_row;i++)
	fputc('\n',stdout);
      fflush(stdout);
    }

}


void CloseGraphics(void)
{
  DefaultRendition();
  fflush(stdout);
  ttmode.c_lflag |= ECHO;
  ioctl(fno2,TCSETS,(char *)&ttmode);
  fclose(infd);
}

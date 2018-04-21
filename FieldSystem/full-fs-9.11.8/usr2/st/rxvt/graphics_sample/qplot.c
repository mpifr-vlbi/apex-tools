#include <signal.h>
#include "rxvt_graphics.h"

void axis_round(float *min, float *max,float *grid_spacing);
int nice_end(int junk);

void main(int argc, char **argv)
{
  char line[256],*l,*file;
  long id,winclick;
  int Do_Start = 1,tmp;
  float x[1000000];
  float y[1000000];
  float nls[1000];
  int m,p,i,j,n,nchars,theight,twidth,xclick,yclick;
  int downx=1000,downy=1000,upx,upy;
  float xmax,xmin,ymax,ymin,xdiff,ydiff,xgrid_spacing, ygrid_spacing;
  FILE *fd;
  int dots = 0;
  char axis[100];

  ymax = xmax = -HUGE_VAL;
  ymin = xmin = HUGE_VAL;

  for(i=1;i<argc;i++)
    {
      if(strcasecmp(argv[i],"-nl") == 0)
	dots = 1;
      else if(argv[i][0] != '-')
	file = argv[i];
    }

  if(file == NULL)
    {
      fd = stdin;
      file = "Rob's QuickPlot";
    }
  else
    {
      fd = fopen(file,"r");
      if(fd == NULL)
	{
	  fprintf(stderr,"%s: Can't open file %s\n",argv[0], file);
	  exit(1);
	}
    }
  l = fgets(line,255,fd);
  m=0;
  p = 0;
  while(l != NULL)
    {
      n = sscanf(l,"%f %f",&x[m], &y[m]);
      if(n == 2)
	{
	  if(x[m] > xmax)
	    xmax = x[m];
	  if(x[m] < xmin)
	    xmin = x[m];
	  if(y[m] > ymax)
	    ymax = y[m];
	  if(y[m] < ymin)
	    ymin = y[m];
	  m++;
	}
      else
	{
	  nls[p] = m;
	  p++;
	}
      l = fgets(line,255,fd);
    }
  nls[p++] = m;
  
  if(m==0)
    return;

  signal(SIGTERM,nice_end);
  signal(SIGSTOP,nice_end);
  signal(SIGTSTP,nice_end);
  signal(SIGINT,nice_end);
  signal(SIGQUIT,nice_end);
  InitializeGraphics(1);

  n = 1;
  while(n != 0)
    {
      axis_round(&xmin, &xmax,&xgrid_spacing);
      axis_round(&ymin, &ymax,&ygrid_spacing);
      
      id = CreateWin(0,0,10000,10000);
      if(id == 0)
	{
	  fprintf(stderr,"Help id = 0\n");
	  exit(1);
	}
      /* Fill the window in black for real eye-catching graphics! */
      ForeColor(0);
      StartFillAreas(id,0,0,10000,10000);
      EndFillAreas();
      
      /* draw outline box in white */
      ForeColor(7);
      
      /* Draw outline box */
      StartLine(id,1000,1000);
      ExtendLine(1000,9000);
      ExtendLine(9000,9000);
      ExtendLine(9000,1000);
      ExtendLine(1000,1000);
      EndLine();
      
      
      /* Draw the data - either lines or dots */
      i=0;
      j=0;
      xdiff = 8000/(xmax-xmin);
      ydiff = 8000/(ymax-ymin);
      if(dots)
	{
	  n=0;
	  while(j < p)
	    {
	      ForeColor(j%6+1);
	      while(((x[i] < xmin)||(x[i] > xmax)||
		    (y[i] < ymin)||(y[i] > ymax))&&(i<nls[j]))i++;	

	      StartPoint(id,1000+(x[i]-xmin)*xdiff,9000-(y[i]-ymin)*ydiff);
	      while(((x[i] < xmin)||(x[i] > xmax)||
		     (y[i] < ymin)||(y[i] > ymax))&&(i<nls[j]))i++;	
	      
	      while(i<nls[j])
		{
		  ExtendPoint(1000+(x[i]-xmin)*xdiff,9000-(y[i]-ymin)*ydiff);
		  n++;
		  if(n>450)
		    {
		      EndPoint();
		      StartPoint(id,1000+(x[i]-xmin)*xdiff,9000-(y[i]-ymin)*ydiff);
		      n=0;
		    }
		  i++;

		  while(((x[i] < xmin)||(x[i] > xmax)||
			 (y[i] < ymin)||(y[i] > ymax))&&(i<nls[j]))i++;	
		}
	      EndPoint();
	      j++;
	      n=0;
	    }
	}
      else
	{
	  n=0;
	  /* lines */
	  while(j < p)
	    {
	      ForeColor(j%6+1);
	      while(((x[i] < xmin)||(x[i] > xmax)||
		     (y[i] < ymin)||(y[i] > ymax))&&(i<nls[j]))i++;	

	      while(i<nls[j])
		{
		  if(n == 0)
		    StartLine(id,1000+(x[i]-xmin)*xdiff,9000-(y[i]-ymin)*ydiff);
		  else
		    ExtendLine(1000+(x[i]-xmin)*xdiff,9000-(y[i]-ymin)*ydiff);
		  n++;
		  if(n>450)
		    {
		      EndLine();
		      StartLine(id,1000+(x[i]-xmin)*xdiff,9000-(y[i]-ymin)*ydiff);
		      n=1;
		    }
		  i++;
		  while(((x[i] < xmin)||(x[i] > xmax)||
			 (y[i] < ymin)||(y[i] > ymax))&&(i<nls[j]))i++;	
		}
	      if(n > 0)
		EndLine();
	      j++;
	      n=0;
	    } 
	}
      
      /* Do axis labels in black */
      ForeColor(7);
      QueryWin(id, &twidth,&theight);
      StartText(id,5000,0000,HCENTER_TEXT|TOP_TEXT, file);
      StartText(id,5000,10000,HCENTER_TEXT|BOTTOM_TEXT, "X");
      StartText(id,0000,5000,LEFT_TEXT|VCENTER_TEXT, "Y");
      sprintf(axis,"%f",ymax);
      nchars = 1000/twidth;
      
      axis[nchars] = 0;
      StartText(id,1000,1000,RIGHT_TEXT|TOP_TEXT,axis);
      sprintf(axis,"%f",ymin);
      axis[nchars] = 0;
      StartText(id,1000,9000,RIGHT_TEXT|BOTTOM_TEXT,axis);
      sprintf(axis,"%f",xmax);
      StartText(id,9000,9000,HCENTER_TEXT|TOP_TEXT,axis);
      sprintf(axis,"%f",xmin);
      StartText(id,1000,9000,HCENTER_TEXT|TOP_TEXT,axis);
      fflush(stdout);
      n=1;
      while((n != 0)&&(n != 2))
	{
	  n = WaitForCarriageReturn(&winclick,&xclick,&yclick);
	  if(n==1)
	    {
	      downx = xclick;
	      downy = yclick;
	    }
	  if(n==2)
	    {
	      upx = xclick;
	      upy = yclick;
	      if(upx < downx)
		{
		  tmp = downx ;
		  downx = upx;
		  upx = tmp;
		}
	      if(upy < downy)
		{
		  tmp = downy ;
		  downy = upy;
		  upy = tmp;
		}
	      xmin = (xmax-xmin)*(downx - 1000)/(8000)+xmin;
	      xmax = (xmax-xmin)*(upx - 1000)/(8000)+xmin;
	      ymax = ymax - (ymax-ymin)*(downy - 1000)/(8000);
	      ymin = ymax - (ymax-ymin)*(upy - 1000)/(8000);
	    }
	}
    }
  CloseGraphics();
  printf("\n");
}

void axis_round(float *min, float *max,float *grid_spacing)
{
  float diff;
  int logspacing;

  diff = *max  - *min;

  logspacing = (int)(log10((diff)/10.0)+0.5);
  *grid_spacing = pow((double)10,(double)logspacing);
  *min = (float)((int)(*min/(*grid_spacing)))* (*grid_spacing);
  *max = (float)((int)(*max/(*grid_spacing))+1)* (*grid_spacing);
}

int nice_end(int junk)
{
  CloseGraphics();
  printf("\n");
  exit(0);
}

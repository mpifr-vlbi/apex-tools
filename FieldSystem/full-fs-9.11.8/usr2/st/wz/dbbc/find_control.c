/*--------------------- find_def.c ----------------------------------------*/
/*  RTW, R. Zeitlhoefler
 * The function  'find_control()' searches 'words' in file opend with 
 *' fopen(" filename.dat", "r"); '
 * at the beginning of lines.
 * The word is defined in (char *s_search) and the next word following  in
 *  the same line is read in  (char *s_token).
 * 
 *                  
 * Anwendung: Erstellung von Control-Files in denen Parameter fuer Programme
 * definiert sind.
 *
 *
 * return Werte: 0  = ok.
 *              -1  = File open error.
 *              -2  = no control word found.
 *              -3  = control wor found but no def. behind (2. token).
*/

# include <stdio.h>
# include <string.h>
# include <stdlib.h>


# define COMMENDS "*" /*  Commends in controlfile begins with '*' */ 
# define MAXLINE 512
   /*................. 'main' for test or application  */
 
   # define CONTROLFILE "/usr2/st/control/dbbc.ctl"

/*   int main(void)  {

   unsigned long ul_Wert;
   int find_control();
   int ierr, err;
   int debug = 2;  
                
   char s_token1[15];
   char s_token2[10];
   char s_search[10];
   
    sprintf(s_search,"dbbc1"); 
   printf("   *** Testaufruf von find_control()  *** \n");

   ierr = find_control(CONTROLFILE, s_search, s_token1, s_token2, debug); 

   printf(" main: ierr= %d, token1= %s  token2= %s\n", ierr, s_token1, s_token2);

    
   return; 
  } 
*/
 
 /*------------------------------------------------------------------*/
 int find_control(char *filename,char *s_search,char *s_token1,char *s_token2,int debug) 
 /*------------------------------------------------------------------*/
{ 
  
  int ival, itmp, ilen, iret;
  long itok2;
  char  Line[255];               /* max Line length */
  char s_tok1[255], s_tok2[255];
 
  char s_null[1] ={'\0'};
  unsigned long ulLinecount=0;            /* Loopcounter */
  char *s_commend = COMMENDS;         /* Char for commend line, see top */ 
  FILE *Samplefile;
  

  ilen = strlen(s_search);
 if ( ilen <= 0) {
    if (debug)
        strcpy(s_tok1,s_null);
        strcpy(s_tok2,s_null);
        printf("find_control: error, no control word in find_control() defined\a\n");
    return -2;
 }
    
 Samplefile = fopen( filename ,"r");

 if (Samplefile == NULL) {
       strcpy(s_tok1,s_null);
       strcpy(s_tok2,s_null);
       printf("find_control: File open error, filename: %s\a\n", filename);
       
       return -1;
 }
 if (debug)
    printf("find_control: File %s is open. \n", filename);
     
     /* read lines with  fgets() */ 
    
 while ( fgets( Line, MAXLINE, Samplefile) != NULL) {
  
     ulLinecount++;        
     
   /* a comment in a conrolfile begins with '*'(see top, COMMEND='*')  
    */
    ival = sscanf(Line," %s %s %s", s_tok1, s_token1, s_token2 );
         if (debug) 
            printf("Line_ Nr: %d ival (int sscanf)=%d\n",ulLinecount,ival);

     
    if (s_tok1[0] != s_commend[0]) {
     
       /* -then no commend line */
       itmp = strncmp (s_search, s_tok1, strlen(s_search));
       if (itmp == 0)    {   /* string found */
            if (debug)      
                printf("find_control: Word %s found in line %ld\n",s_tok1, ulLinecount);
          fclose(Samplefile);      
          return 0;
       }     
    }
 }  /* end of while */
 
  if ( itmp != 0 )
      fclose(Samplefile);
      strcpy (s_tok1, s_null); 
      strcpy (s_token1,s_null);
      strcpy (s_token2,s_null);  
      printf("find_control: No control values for %s found \a\n",s_search);
      return -3; 
      
} /* end of function */      
        
















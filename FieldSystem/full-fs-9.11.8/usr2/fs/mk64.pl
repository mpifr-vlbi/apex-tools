#!/usr/bin/perl
(@file_list)= glob "*/*akefile";
$nf=$#file_list+1;
for($i=0;$i<$nf;$i++){
   open(fi,$file_list[$i]);
   $nocf=1; $noff=1;  #check if there is FF and CF
   while(<fi>){
     s/#CFLAGS/#NIX/;
     s/#FFLAGS/#NIX/;
     if(index($_,"FFLAG") > 0) {$noff=0;}
     if(index($_,"CFLAG") > 0) {$nocf=0;}
   }
   print "$file_list[$i] ====================$nocf=$noff===============\n";
   close(fi);
   $text="";
   if($nocf == 1){$text="CFLAGS= -m32\n";}
   if($noff == 1){$text=$text."FFLAGS= -m32\n";}
   open(fi,$file_list[$i]);
   while(<fi>){
      if(index($_,"FLAG") > 0) {
         if(index($_,"YFLAG") < 0){
           s/#CFLAGS/#NIX/;
           s/#FFLAGS/#NIX/;
           if(index($_,"CFLAG") >= 0){
              if(index($_,":=") < 0){
                if(index($_,"-m32") < 0){s/=/= -m32 /}
              }
           }
           if(index($_,"FFLAG") >= 0){
              if(index($_,"-m32") < 0){s/=/= -m32 /}
           }
         }
      }
      if(index($_,"cc ") > 0) {
#if no CFLAGS after cc, put in
         if(index($_,"CFLAG") < 0){
           s/\$\(OBJECTS/\$\(CFLAGS\) \$\(OBJECTS/;
         }
      }
      $text=$text.$_;
   }
   close(fi);
#reopen, rewrite
   $fname=$file_list[$i];
   open fo, ">$fname";
   print fo "$text";
}

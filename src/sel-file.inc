int code;

#include <dirent.h>
#include <stdio.h>
#include <string.h>

void sel_datafile(char *datafile) 
{
DIR *dirp;
struct dirent *dp;
char filelist[100][15]; 
int j=0;
int k=0;

dirp = opendir(".");  /* open dir, read a file name */
if(!dirp) exit(0);                 /* error opening directory, no files */
for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
  {
  if(!strcmp(dp->d_name+strlen(dp->d_name)-4,".rec")) 
    strcpy(filelist[j++],dp->d_name);
  } 
closedir (dirp);
j--; /* j is now last subscript used */ 
for (k=0; k<=j; k++) disp_str(k-20*(k/20),(k/20)*15,filelist[k],0);
k=0; disp_str(0,0,filelist[k],1);
do
  {
  code = getcode();
  switch (code)
    {
    case U_ARROW: disp_str(k-20*(k/20),(k/20)*15,filelist[k],0);
    if( k>0 ) --k;
    else k=j; disp_str(k-20*(k/20),(k/20)*15,filelist[k],1);
    break;
    case D_ARROW: disp_str(k-20*(k/20),(k/20)*15,filelist[k],0);
      if( k<j ) ++k;
      else k = 0; disp_str(k-20*(k/20),(k/20)*15,filelist[k],1);
	      break;
	 case L_ARROW: disp_str(k-20*(k/20),(k/20)*15,filelist[k],0);
	   if( k>=20 ) k-=20;
	   disp_str(k-20*(k/20),(k/20)*15,filelist[k],1);
	   break;
	 case R_ARROW: disp_str(k-20*(k/20),(k/20)*15,filelist[k],0);
	   if( k+20<=j ) k+=20;
	   disp_str(k-20*(k/20),(k/20)*15,filelist[k],1);
	      break;
    case ENTER: break;
    }
  } 
while (code!=ENTER);

strcpy(datafile,filelist[k]);
}


/* main program for C version of FLASH by Dan Kurth */
/*
Examples of builds tested:
On virtual (VirtualBox 6.1.26) Windows XP using MinGW version v10.0.0:
  gcc -I/MinGW/include/ncurses -o flash fcm.c -lncurses -L/MinGW/bin -static
On Debian GNU/Linux 11 (bullseye):
  gcc -o flash fcm.c -I/usr/include/ncurses -lncurses -ltermcap
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fcm.h"

#include <dirent.h>
#include <curses.h>

#include "cursor.inc"
#include "clear.inc"
#include "disp-str.inc"
#include "box.inc"
#include "blank.inc"
#include "getcode.inc"
#include "sel-file.inc"
#include "byebye.inc"

            /* C routines */
int  GetUser(void);              /* prompt for user name, calc hex id */
void cursor_off(void);     /* turn cursor off */
void cursor_on(void);      /* turn cursor on  */

void copyright(void);       /* display copyright message to user */
void disp_menu(void);       /* display menu to user */
void msg(int msgnum);             /* display messages to user  */
int  getcode(void);         /* get response from user  */
void action(void);         /* initiate action corresponding to request by user*/

void sel_datafile(char *datafile); /* choose a file to use for session */
void clrfile(void);               /* clear stats/scores of disk data file */
void cntfile(void);           /* cnt right/wrong in whole file */
int  rfile(void);           /* read data file into memory (some or all records)*/
void update_file(void);    /* update disk record stats to match those in memory   */

void myflash(int myflash_choice);    /* show questions, prompt for answers & keep stats */
void cnt_cards(void);
void clear_cardmem(void);             /* clear all cards from memory */
void get_rand_card(int rand_parm);  /* get a card, any card */
void honor_system(void);            /* user tells computer if answer was right */
void typein_letter(void);  /* user has to type in letter for correct answer */
void multiple_choice(void);  /* have user pick from multiple answers */
void init_multiple_choice(void);   /* get random answers, question */
void disp_multiple_choice(void);   /* show possible answers            */
void pick_multiple_choice(void);   /* get answer for multiple choice */
void pick(int);                   /*                                */
void disp_stats(void);             /* show session & file stats */
void disp_stats_legend(void); /* show legend for session & file stats */
void go_byebye(void);   /* clear screen, cleanup unix curses stuff */
int getcode(void); /* returns an integer for key pressed */

void clrscr() { clear(); refresh(); }

          /* assembly language routines (dos) or in c for unix */
void clr_scr(int attribute);
void disp_str(int row, int column, char strtext[], int attrib);
void blanks(int row1, int column1, int row2, int column2, int attribute);
void draw_box(unsigned char row1, unsigned char column1, unsigned char row2, unsigned char column2, unsigned char attribute);

char qbuffer[500];
char abuffer[500];

struct myflashcard
    {
    char *question;
    char *answer;
    char ans_stat;
    int usrstat;
    int tries_session;
    struct myflashcard *ptrprev;
    struct myflashcard *ptrnext;
    long int disk_fptr;
    };
struct myflashcard *ptrfirst, *ptrthis, *ptrnew, *ptrlast, *ptrtemp;
struct myflashcard *store();
struct myflashcard *testptr[10];

struct
   {
   int total;
   int correctFT;
   } filecnt;

struct
   {
   int total;
   int correct;
   int correctFT;
   } memcnt;

char *str[] =
     { "select file     ", /* main menu */
       "flash cards     ",
       "clear statistics",
       "quit            ", /* myflash choices menu */
       "multiple choice ",
       "honor system    ",
       "pick a letter   ",
       "quit            ", };

int usrid;                /* unique single bit number to mark file usrstat */
char datafile[13] = "NoName";     /* name of file to use as datafile */
int honor=TRUE;     /* honor system enabler, command line parm /h sets TRUE */
/* was FALSE but tiresome, maybe remove it, but keep for now as option */
int firstrow = 0;         /* first row of str to display as menu */
int lastrow = 3;          /* last row of str to display as menu */
int pos = 0;              /* which row of menu highlighted */
int sel = 0;              /* which multiple choice answer highlighted */
int code;                 /* keyboard input */

char strnum[7];
int max_recs = 300;
int score = 0;

main(int argc, char *argv[])
   {
   int arg;   /* used in for loop involving command line args only */

initscr();
cbreak();
noecho();
keypad(stdscr,TRUE);
leaveok(stdscr,TRUE);
usrid=1;

   clr_scr(0);
   ptrfirst = ptrlast = NULL;
   cursor_off();
   for (arg=1; arg<argc; arg++)   /* get command line arguments */
     {
     /* if (strncmp(argv[arg],"/h",2)==0) honor=TRUE; */ /* enable honor system */
     if (strncmp(argv[arg],"-m",2)==0)
       {
       max_recs=atoi(argv[arg+1]); sprintf(strnum,"%d",max_recs); arg++;
       /* disp_str(15,15,strnum,1);  getch(); */
       }
     else strcpy(datafile,argv[arg]);             /* file name to use */
     }
   if(strncmp(datafile,"NoName",6)!=0) cntfile();
   msg(0);

   srand(1);

   while (TRUE)
      {
      copyright();
      disp_str(0,48,"Data File:",0); disp_str(0,59,datafile,1);
      disp_menu();
      draw_box(2,47,lastrow-firstrow+4,78,0);
      do
      {
      code = getcode();
      switch (code)
      {
      case U_ARROW: disp_str(pos-firstrow+ver_offset,hor_offset,str[pos],0);
	if( pos>firstrow ) --pos;
        else pos = lastrow;
        disp_str(pos-firstrow+ver_offset,hor_offset,str[pos],1);
        break;
      case D_ARROW: disp_str(pos-firstrow+ver_offset,hor_offset,str[pos],0);
        if( pos<lastrow ) ++pos;
        else pos = firstrow;
        disp_str(pos-firstrow+ver_offset,hor_offset,str[pos],1);
        break;
      case ENTER: action(); code = ENTER; break;
      }
      }
      while (code!=ENTER);
      }
   }


void disp_menu()
{
int row;
for (row=firstrow; row<=lastrow; row++)
   {
   if (row==pos) disp_str(row-firstrow+ver_offset,hor_offset,str[row],1);
   else disp_str(row-firstrow+ver_offset,hor_offset,str[row],0);
   }
}

void msg(int msgnum)
{
char *msgptr[] = {
"Up-Arrow=Scroll Up  Down-Arrow=Scroll Down ENTER=Make Selection",
"Q-Menu    INS-Add Card    DEL-Delete Card    -Prev    -Next   PgUp-Up 20    PgDn-Down 20    HOME-1st    END-Last ",
"Enter questions and answers, To return to view/edit press <enter> w/out input ",
"Picture answer to yourself, then press any key to see if you were right ",
"Left Arrow=Answer was Wrong, Right Arrow=Answer was Right, Q=Quit ",
"Press any key to continue ",
"ENTER-Make Selection  Up-Arrow=Scroll Up  Down-Arrow=Scroll Down ",
"Enter letter that matches correct answer ",
"Press any key to continue, or q to quit ", };
     /*  some lines above continue that way ----->>>  */

blanks(22,0,23,79,0);
disp_str(22,0,msgptr[msgnum],0);
}


void action()
{
switch(pos)
   {
   case 0: clr_scr(0); sel_datafile(&datafile[0]); cntfile(); break;
   case 1: if (honor) { clr_scr(0); firstrow=4; lastrow=7; pos=4; break; }
        else { myflash(0); break; }             /* multiple choice  */
   case 2: clrfile(); break;                      /* clear users stats */
   case 3: go_byebye();                             /* quit */
   case 4: myflash(0); break;                         /* multiple choice */
   case 5: myflash(1); break;                           /* honor system */
   case 6: myflash(2); break;       /* type in letter that matches answer */
   case 7: firstrow=0; lastrow=3; pos=3; break;         /* to main menu */
   }
clr_scr(0); msg(0);
}

/******************************************************************/

int rfile()
   {
   FILE *fptr; int j=0, Ncount=0, Ycount=0; char ch;
   int hexbuffer;
   ptrtemp=NULL;
   if( (fptr=fopen(datafile,"r"))==NULL )
      {
      disp_str(2,0,"Can't open file",0);
      disp_str(2,16,datafile,0);
      getch(); return(2);
      }
   clear_cardmem(); /* test here, so can switch between mult & honor */
   ptrfirst = (struct myflashcard *) malloc(sizeof(struct myflashcard));
   ptrfirst->question=NULL; ptrfirst->answer=NULL;
   if(!ptrfirst)
      {
      disp_str(1,0,"Out of memory at first card",0);
      fclose(fptr); getch(); return(2);
      }
   ptrthis=ptrfirst;
/*
   while( (fscanf(fptr,"%x ",&hexbuffer)) != EOF  &&  Ncount<NUM_RECS)
 */
  while( (fscanf(fptr,"%x ",&hexbuffer)) != EOF  &&  Ncount<max_recs)
      {
      ptrthis->usrstat=hexbuffer;
      if(usrid==(hexbuffer&usrid)) ptrthis->ans_stat='Y';
      else ptrthis->ans_stat='N';
      if(usrid==0) ptrthis->ans_stat='N';
      if(ptrthis->ans_stat == 'N')
      {
      ptrthis->tries_session=0;
      ptrthis->disk_fptr = ftell(fptr)-3;
      j=0;
      while(((ch=getc(fptr))!='=') && j<Q_SIZE) /* cut off if too long */
         {
            qbuffer[j]=ch; j++;
         }
         qbuffer[j]='\0';
         /* printw("qbuffer %s\n",qbuffer); refresh();  */
	 ptrthis->question=(char *) malloc(strlen(qbuffer)+1);
         if(ptrthis->question==NULL)
           {
           disp_str(1,0,"malloc failed for question: ",0);
           disp_str(1,28,qbuffer,0); /* refresh(); */ getch(); go_byebye();
           }
         strncpy(ptrthis->question,qbuffer,strlen(qbuffer));
	 ptrthis->question[j]='\0';
         /* printw("question %s\n",ptrthis->question); refresh(); */

      if(ch!='=')while((ch=getc(fptr))!='=');  /* if cut off move past excess to the start of next read */

         while(TRUE) /* skip spaces and newlines preceding start of answer */
	   {
           ch=getc(fptr);
           if((ch!='\n') && (ch!=' ')) break; /* if newline or space loop again */
           }
         ungetc(ch,fptr);

      j=0;
      while(((ch=getc(fptr))!='}') && j<A_SIZE)
         {
         if (ch==EOF)
            {
            fclose(fptr);
            clrscr();
	    printf("\n File read error at line %d",Ncount+Ycount+1);
            printf("\n Premature EOF");
            printf("\n Press any key to continue"); getch();
            go_byebye();
            }
            abuffer[j]=ch; j++;
         }
      abuffer[j]='\0';
         /* printw("abuffer %s\n",abuffer); refresh();  */
         ptrthis->answer=(char *) malloc(strlen(abuffer)+1);
         if(ptrthis->answer==NULL)
           {
           disp_str(1,0,"malloc failed for answer: ",0);
	disp_str(1,28,abuffer,0); /* refresh(); */ getch(); go_byebye();
           }
         strncpy(ptrthis->answer,abuffer,strlen(abuffer));
         ptrthis->answer[j]='\0';
       /*  printw("answer %s\n",ptrthis->answer); refresh(); */
      if(ch!='}')while((ch=getc(fptr))!='}')
         {
	 if (ch==EOF)
            {
            fclose(fptr);
            clrscr();
            printf("\n File read error at line %d",Ncount+Ycount+1);
            printf("\n Premature EOF");
	    printf("\n Press any key to continue"); getch();
            go_byebye();
            }
         }
      Ncount++;
      ptrthis->ptrnext=(struct myflashcard *) malloc(sizeof(struct myflashcard));
         if(!ptrthis->ptrnext)
	   {
           disp_str(1,0,"Out of memory at card",0);
           fclose(fptr); getch(); return(2);
           }
         ptrthis->ptrnext->question=NULL; ptrthis->ptrnext->answer=NULL;
      if(ptrthis->ptrnext)
	 {
         ptrthis->ptrprev = ptrtemp;
         ptrtemp = ptrthis;
         ptrthis = ptrthis->ptrnext;
         }
      else
         {
	 disp_str(1,0,"Out of memory ",1); getch(); return(2);
         }
      }
      else if (ptrthis->ans_stat == 'Y')
      {
      while((ch=getc(fptr)) != '}' );
      Ycount++;
      }
      else
      {
      fclose(fptr);
      clrscr();
      printf("\n File read error at line %d",Ncount+Ycount+1);
      printf("\n Press any key to continue"); getch();
      go_byebye();
      }
      } /* end while */
   if(Ncount==0)
      {
      disp_str(1,1,"There were no unmatched cards in the file selected",0);
      disp_str(2,1,"To use file first select CLEAR STATISTICS in menu",0);
      disp_str(4,1,"Press any key to continue",0);
      getch();
      fclose(fptr);
      if(ptrfirst->question) free(ptrfirst->question); ptrfirst->question=NULL;
      if(ptrfirst->answer) free(ptrfirst->answer); ptrfirst->answer=NULL;
      free(ptrfirst);
      ptrthis=ptrfirst=NULL;
      return(2);
      }
   else
      {
      ptrtemp->ptrnext = NULL;
      ptrlast = ptrtemp;
      ptrfirst->ptrprev = NULL;
      fclose(fptr);
      }
   return(0);
   }

void cntfile()
   {
   FILE *fptr; char ch; int hexbuffer;

   filecnt.correctFT=0; filecnt.total=0;
   if( (fptr=fopen(datafile,"r"))==NULL )
      {
      disp_str(2,0,"Can't open file",0);
      disp_str(2,16,datafile,0);
      getch(); go_byebye();
      }
   while( (fscanf(fptr,"%x ",&hexbuffer)) != EOF)
      {
      filecnt.total++;
      if(usrid==(hexbuffer&usrid)) filecnt.correctFT++;
      while((ch=getc(fptr))!='}')
      {
      if (ch==EOF)
	 {
         fclose(fptr);
         clrscr();
         printf("\n File read error at line %d",filecnt.total);
         printf("\n Premature EOF");
         printf("\n Press any key to continue"); getch();
         go_byebye();
	 }
      }
      }
   fclose(fptr);
   }

void update_file()
   {
   long int offset;
   FILE *fptr;
   if (ptrfirst == (struct myflashcard *)NULL )
      { printf("\nCan't write empty list.\n"); getch(); return; }
   if( (fptr=fopen(datafile,"r+"))==NULL )
      { printf("\nCan't access disk drive\n"); getch(); return; }
   ptrthis = ptrfirst;
   while(ptrthis)
      {
      if ((ptrthis->ans_stat=='Y') && (ptrthis->tries_session==1))
      {
      offset = ptrthis->disk_fptr;
      fseek(fptr, offset, 0);
      fprintf(fptr,"%2X",((ptrthis->usrstat)|usrid));
      }
      ptrthis = ptrthis->ptrnext;
      }
   fclose(fptr);
   }

/******************************************************************/
void myflash(int myflash_choice)
   {
   clr_scr(0);
   if (rfile()==2) return;
   score=0; cnt_cards();
   if(memcnt.total==memcnt.correct)
      {
      disp_str(1,0,"There are no unmatched cards in memory ",1);
      msg(5);
      getch(); return;
      }
   disp_stats_legend(); disp_stats();
   while((memcnt.total > memcnt.correct) && (code!=ESC))
      {
      get_rand_card(0);
      blanks(4,27,4,32,0); sprintf(strnum,"%d",ptrthis->tries_session); disp_str(4,27,strnum,0);
      if (myflash_choice == 0) multiple_choice();
      else if (myflash_choice == 1) honor_system();
      else typein_letter();
      if (ptrthis->ans_stat=='Y')
      {
      memcnt.correct++;
      blanks(3,27,3,32,0); sprintf(strnum,"%d",memcnt.total-memcnt.correct); disp_str(3,27,strnum,0);
      if (ptrthis->tries_session==1)
	 {
	 memcnt.correctFT++; filecnt.correctFT++;
	 blanks(3,67,4,72,0);
	 sprintf(strnum,"%d",filecnt.correctFT); disp_str(3,67,strnum,0);
	 sprintf(strnum,"%d",filecnt.total-filecnt.correctFT); disp_str(4,67,strnum,0);
	 score++;
	 blanks(0,67,0,72,0);
         sprintf(strnum,"%d",score); disp_str(0,67,strnum,0);
         }
      }
      }
   clr_scr(0);
/* if (memcnt.total==memcnt.correct) */
   if(TRUE)
      {
/*
      disp_str(10,26," ALL CARDS MATCHED FOR THIS SESSION ",1);
*/
   if(myflash_choice==0) disp_str(12,31,"Multiple Choice",0);
   else if(myflash_choice==1) disp_str(12,31,"Honor System",0);
   else disp_str(12,31,"Pick a Letter",0);

   disp_str(13,31,"Score: ",0);
   sprintf(strnum,"%d",score); disp_str(13,44,strnum,0);
   score=100*score/memcnt.total;
   disp_str(14,31,"Percentile: ",0);
   sprintf(strnum,"%d",score); disp_str(14,44,strnum,0);
   msg(5);getch();
   clr_scr(0);
      }
   update_file();
   /* clear_cardmem(); */
   }

void multiple_choice()
{
msg(6);
blanks(6,0,12,79,0);
disp_str(6,0,ptrthis->question,0);
init_multiple_choice();
disp_multiple_choice();
do
   {
   pick_multiple_choice();
   }
while (code!=ENTER && code!=ESC);
}

void pick_multiple_choice()
   {
   int firstrow = 0;
   int lastrow = 6;
   code = getcode();
   switch (code)
      {
      case U_ARROW: disp_str(14+sel,0,testptr[sel]->answer,0);
        if( sel>firstrow ) --sel;
        else sel = lastrow;
        disp_str(14+sel,0,testptr[sel]->answer,1);
           break;
      case D_ARROW: disp_str(14+sel,0,testptr[sel]->answer,0);
           if( sel<lastrow ) ++sel;
           else sel = firstrow;
        disp_str(14+sel,0,testptr[sel]->answer,1);
        break;
      case ENTER:   pick(sel); break;
      case ESC:     break;
      }
   }

void init_multiple_choice()
{
int j;
ptrtemp = ptrthis;
for (j=0; j<7; j++)
   {
   get_rand_card(1);
   testptr[j] = ptrthis;
   }
ptrthis = ptrtemp;

#ifdef DOS
j = random(7);
#else
j = rand() % 7;
#endif

testptr[j] = ptrthis;
}

void pick(int j)
{
if ((testptr[j]==ptrthis) || strcmp(testptr[j]->answer,ptrthis->answer)==0)
   {
   ptrthis->ans_stat = 'Y';
   }
else
   {
   disp_str(9,0,ptrthis->answer,0);
   msg(5);
   getch(); /* wait for key press to continue */
   }
msg(6);
ptrthis->tries_session++;
}

void disp_multiple_choice()
{
int j;
for (j=0; j<7; j++)
   {
   if (j==sel)
      disp_str(14+j,0,testptr[j]->answer,1);
   else
      disp_str(14+j,0,testptr[j]->answer,0);
   blanks(14+j,0+strlen(testptr[j]->answer),14+j,79,0);
   }
}

void cnt_cards()
   {
   memcnt.total=memcnt.correct=memcnt.correctFT=0;
   ptrtemp = ptrthis;
   ptrthis = ptrfirst;
   while(ptrthis)
      {
      memcnt.total++;
      if(ptrthis->ans_stat == 'Y') memcnt.correct++;
      ptrthis = ptrthis->ptrnext;
      }
   ptrthis = ptrtemp;
   }

void get_rand_card(int rand_parm)
   {
   int i = 0;
   int rand_num;

   rand_num = rand() % memcnt.total;

   ptrthis = ptrfirst;
   while(i < rand_num)
      {
      ptrthis = ptrthis->ptrnext;
      i++;
      }
   while((ptrthis->ans_stat == 'Y') && (rand_parm == 0))
      {
      if(ptrthis->ptrnext)
      {
      ptrthis = ptrthis->ptrnext;
      }
      else ptrthis = ptrfirst;
      }
   }

void typein_letter()
   {
   char ch; int j=0;
   disp_str(6,0,ptrthis->question,0);
   msg(7);
   ch=getch();
   if(ch==ptrthis->answer[0]) ptrthis->ans_stat = 'Y';
   else
     {
     disp_str(14,0,ptrthis->answer,0);
     msg(8);
     code = getcode();
     switch (code)
       {
       case ESC: break;
       }
     }
   blanks(6,0,23,79,0);
   ptrthis->tries_session++;
   }


void honor_system()
   {
   disp_str(6,0,ptrthis->question,0);
   msg(3);
   getch();
   disp_str(14,0,ptrthis->answer,0);
   msg(4);
   code = getcode();
   switch (code)
      {
      case L_ARROW: break;
      case R_ARROW: ptrthis->ans_stat = 'Y'; break;
      case ESC: break;
      }
   blanks(6,0,23,79,0);
   ptrthis->tries_session++;
   }

void disp_stats_legend()
{
disp_str(0,40,"Score this session:",0);
disp_str(2,0,"Total cards this session:",0);
disp_str(3,0,"Remaining this session:",0);
disp_str(4,0,"Times tried this session:",0);
disp_str(2,40,"Total # of cards in file:",0);
disp_str(3,40,"Cards marked as correct:",0);
disp_str(4,40,"Cards not marked:",0);
}



void disp_stats()
   {
   sprintf(strnum,"%d",score); disp_str(0,67,strnum,0);
   sprintf(strnum,"%d",memcnt.total); disp_str(2,27,strnum,0);
   sprintf(strnum,"%d",memcnt.total-memcnt.correct); disp_str(3,27,strnum,0);
/*
   sprintf(strnum,"%d",ptrthis->tries_session); disp_str(4,27,strnum,0);
*/
   sprintf(strnum,"%d",filecnt.total); disp_str(2,67,strnum,0);
   sprintf(strnum,"%d",filecnt.correctFT); disp_str(3,67,strnum,0);
   sprintf(strnum,"%d",filecnt.total-filecnt.correctFT); disp_str(4,67,strnum,0);
   }

void clrfile()
   {
   FILE *fptr1; FILE *fptr2; char ch; long int offset;
   int hexbuffer;
   if( (fptr1=fopen(datafile,"r+"))==NULL )
      { printf("\nCan't access disk drive\n"); getch(); return; }
   if( (fptr2=fopen(datafile,"r+"))==NULL )
      { printf("\nCan't access disk drive\n"); getch(); return; }
   fprintf(fptr2,"%2X",0); /* needed? */
   while( (fscanf(fptr1,"%x ",&hexbuffer)) != EOF)
      {
      if(usrid==(hexbuffer&usrid))
      {
      offset=ftell(fptr1)-3;
      fseek(fptr2,offset,0);
      fprintf(fptr2,"%2X",hexbuffer-usrid);
      }
      while(getc(fptr1)!='}');    /* get to next line */
      }
   filecnt.correctFT=0;
   fclose(fptr1); fclose(fptr2);
   }




void copyright()
/*
Coded copyright notice to avoid removal by user message is found by adding
two values to obtain third which is numeric for the char desired the
copyright notice will also show up in a dump due to the array cpr[] but if
modified there that will not affect the copyright notice which is printed on
the screen.
Added to this is a routine to check for alterations to the notice, if any
are found the program will terminate */
{
int k, k1, k2;
char *cpr[] = { " FLASH by Dan Kurth ", };
int cpr1[] = { 19,13,63,52,37,11,70,109,1,20,28,46,14,7,25,38,68,54,66,97 };
int cpr2[] = { 93,19,82,40,21,12,47,1,18,65,4,79,5,36,25,11,59,49,30,12 };
int cpr3[] = { 10,19,14,15,17,13,12,16,4,9,8,2,6,0,5,11,18,3,1,7 };
int cpr4[] = { 8,19,4,12,13,9,5,7,6,15,1,17,16,18,0,3,11,10,2,14 };

disp_str(0,0,cpr[0],1);
for (k=0; k<strlen(cpr[0]); k++)    /* check for alterations to copyright */
   {
   k1 = 0;
   while (cpr3[k1] != k) k1++;
   k2 = 0;
   while (cpr4[k2] != k) k2++;

   if (*(cpr[0]+k) != (cpr1[k1]+cpr2[k2]))
      {
      printf("\nAlteration to copyright notice in program file");
      getch(); go_byebye();
      }
   }
}

int GetUser()
/* prompt for user name, look it up or add to list,
SETS USRID
return 1 success, 0 on lack thereof*/
{
FILE *fptr; char sbuffer[80]; char ch;
int found=0; int j=0; int uidexp=0; char username[25]; int done=0;
if( (fptr=fopen("userlist.dta","a+"))==NULL ) /* create it if not here */
   {
   printf("\nCan't open userlist.dta for append\n"); getch();
   go_byebye();
   }
else fclose(fptr);
if( (fptr=fopen("userlist.dta","r+"))==NULL )
   {
   printf("\nCan't open userlist.dta for read/write\n"); getch(); go_byebye();
   }
for(j=0; j<79; j++) sbuffer[j]=' '; sbuffer[j]='\0';
for(j=0; j<24; j++) username[j]=' '; username[j]='\0';
while (username[0]=='\0' || username[0]==' ')
   {
   clrscr();
   printf("\nA name is required to record your work.");
   printf("\nIt should be unique, eg full name or a psuedonym like RainMan.");
   printf("\nIf two people use the same name, eg 'Dan', they will affect");
   printf("\neach others work.");
   printf("\n\nIf the name you enter is already in use (hopefully by you)");
   printf("\nFLASH will proceed, otherwise you will be asked if you wish");
   printf("\nto add it to the list of users.");
   printf("\n\nPlease enter name: ");
   gets(sbuffer);
   strncpy(username,sbuffer,24);username[24]='\0';
   }
for(j=0; j<79; j++) sbuffer[j]=' '; sbuffer[j]='\0';
/* the following returns the offset of the user from 0 to 7 to use as an
exponent in getting the value of one of the 8 bits to represent that user,
the offset is incremented each time a next user is found. Because of where
the strncmp is placed if there is a match it will be done before the /n for
that line is reached so the next user will not be found */

j=0;
while(!done)
   {
   ch=getc(fptr);
   switch(ch)
      {
      case EOF:  if(j>0)  putc('\n',fptr);
           done=TRUE; break;
      case 26:   if(j>0)  putc('\n',fptr);
           done=TRUE; break;
      case '\n': j=0; uidexp++; break;
      default:   sbuffer[j++]=ch;
           if(j>79)
              {
              printf("\nError: name in user list too long");
                    go_byebye();
              }
           else sbuffer[j]='\0';
      }
   if(strncmp(sbuffer,username,24)==0) { found=1; done=TRUE; }
   if (uidexp>7) done=TRUE;
   }
if(found) usrid=1<<uidexp;
else if (uidexp>7)
   {
   printf("\nSorry, 8 users already, cannot add more");
   printf("\nLog in as a guest? (does not record work) y/N "); ch=getch();
   printf("%c",ch);
   if (ch=='y' || ch== 'Y') usrid=0;
   else { fclose(fptr); go_byebye(); }
   }
else
   {
   printf("\nAdd \"%s\" to list of users? (y/n) ",username);
   ch=getch(); printf("%c",ch);
   if (ch=='y' || ch== 'Y')
      {
      usrid=1<<uidexp;
      fprintf(fptr,"%s\n",username); getch();
      printf("\n\nUser name \"%s\" added to list of users",username);
      printf("\nPlease use this same name for all future sessions");
      getch();
      }
   else { fclose(fptr); return(0); }
   }
fclose(fptr); return(1);
}

void clear_cardmem()
{
   while(ptrfirst)
      {
      ptrthis = ptrfirst->ptrnext;
      if(ptrfirst->question) free(ptrfirst->question); ptrfirst->question=NULL;
      if(ptrfirst->answer) free(ptrfirst->answer); ptrfirst->answer=NULL;
      free(ptrfirst);
      ptrfirst = ptrthis;
      }
}

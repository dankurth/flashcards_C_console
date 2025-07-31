/* main program for C version of FLASH by Dan Kurth */
/*
Examples of builds tested:
On virtual (VirtualBox 6.1.26) Windows XP using MinGW version v10.0.0:
  gcc -I/MinGW/include/ncurses -o flash fcm.c -lncurses -L/MinGW/bin -static
On Debian GNU/Linux:
  // gcc -o flash fcm.c -I/usr/include/ncurses -lncurses -ltermcap
  // does not display characters such as Él properly using mvprintw

  gcc -o flash fcm.c -I/usr/include/ncursesw/ncursesw -lncursesw -ltermcap
  // displays characters such as Él properly using mvprintw,
  // even though still passing (char *) not (wchar_t *)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "fcm.h"

#include <dirent.h>
#include <curses.h>
#include <locale.h>

#include "disp-str.inc"
#include "blank.inc"
#include "sel-file.inc"
#include "byebye.inc"

/* C routines */
void disp_menu(void); /* display menu to user */
void msg(int msgnum); /* display messages to user  */
void action(void);    /* initiate action corresponding to request by user*/

void sel_datafile(char *datafile); /* choose a file to use for session */
void clrfile(void);                /* clear stats/scores of disk data file */
void cntfile(void);                /* cnt right/wrong in whole file */
int rfile(void);                   /* read data file into memory (some or all records)*/
void update_file(void);            /* update disk record stats to match those in memory   */

void myflash(void); /* show questions, prompt for answers & keep stats */
void cnt_cards(void);
void clear_cardmem(void);          /* clear all cards from memory */
void get_rand_card(int rand_parm); /* get a card, any card */
void honor_system(void);           /* user tells computer if answer was right */
void pick(int);                    /*                                */
void disp_stats(void);             /* show session & file stats */
void disp_stats_legend(void);      /* show legend for session & file stats */
void go_byebye(void);              /* clear screen, cleanup unix curses stuff */

int count_char(char *str, char ch);
int size_after_last_char(char *str, char ch);

void clrscr()
{
   clear();
   refresh();
}
void disp_str(int row, int column, char strtext[], int attrib);
void blanks(int row1, int column1, int row2, int column2, int attribute);

char qbuffer[MAX_FIELD_LENGTH];
char abuffer[MAX_FIELD_LENGTH];

struct myflashcard
{
   char *question;
   char *answer;
   int answered_correctly;
   int tries_session;
   struct myflashcard *ptrprev;
   struct myflashcard *ptrnext;
   long int disk_fptr;
};
struct myflashcard *ptrfirst, *ptrthis, *ptrnew, *ptrlast, *ptrtemp;

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

char *str[] = {
    "select file     ", // main menu
    "flash cards     ",
    "clear statistics",
    "quit            "};

char datafile[13] = "NoName"; /* name of file to use as datafile */
int firstrow = 0;             /* first row of str to display as menu */
int lastrow = 3;              /* last row of str to display as menu */
int pos = 0;                  /* which row of menu highlighted */
int code;                     /* keyboard input */

char strnum[4];
int max_recs = 100;
int maxRow = 22; // will be modified once stdscr is initiated

int main(int argc, char *argv[])
{
   setlocale(LC_ALL, "");
   initscr();
   maxRow = getmaxy(stdscr) - 1;
   cbreak();
   noecho();
   keypad(stdscr, TRUE);
   leaveok(stdscr, TRUE);
   clrscr();
   ptrfirst = ptrlast = NULL;
   curs_set(0); // turn that annoying blinking cursor off for the duration
   if (argc > 1)
   {
      strcpy(datafile, argv[1]); // file name to use
   }
   if (strncmp(datafile, "NoName", 6) != 0)
   {
      cntfile();
   }
   msg(0);
   srand(1);

   while (TRUE)
   {
      disp_str(0, 0, " FLASH by Dan Kurth ", 1);
      disp_str(0, 48, "Data File:", 0);
      disp_str(0, 59, datafile, 1);
      disp_menu();
      do
      {
         code = getch();
         switch (code)
         {
         case KEY_UP:
            disp_str(pos - firstrow + ver_offset, hor_offset, str[pos], 0);
            if (pos > firstrow)
               --pos;
            else
               pos = lastrow;
            disp_str(pos - firstrow + ver_offset, hor_offset, str[pos], 1);
            break;
         case KEY_DOWN:
            disp_str(pos - firstrow + ver_offset, hor_offset, str[pos], 0);
            if (pos < lastrow)
               ++pos;
            else
               pos = firstrow;
            disp_str(pos - firstrow + ver_offset, hor_offset, str[pos], 1);
            break;
         case ENTER:
            action();
            code = ENTER;
            break;
         }
      } while (code != ENTER);
   }
   return 0;
}

void disp_menu()
{
   int row;
   for (row = firstrow; row <= lastrow; row++)
   {
      if (row == pos)
         disp_str(row - firstrow + ver_offset, hor_offset, str[row], 1);
      else
         disp_str(row - firstrow + ver_offset, hor_offset, str[row], 0);
   }
}

void msg(int msgnum)
{
   char *msgptr[] = {
       "Up-Arrow=Scroll Up  Down-Arrow=Scroll Down ENTER=Make Selection",
       "Q-Menu    INS-Add Card    DEL-Delete Card    -Prev    -Next   PgUp-Up 20    PgDn-Down 20    HOME-1st    END-Last ",
       "Enter questions and answers, To return to view/edit press <enter> w/out input ",
       "Right Arrow=View Answer      q=Quit",
       "Left Arrow=View Question     Up Arrow=Correct     Down Arrow=Wrong",
       "Press any key to continue "};

   blanks(maxRow, 0, maxRow + 1, 79, 0);
   disp_str(maxRow, 0, msgptr[msgnum], 0);
}

void action()
{
   switch (pos)
   {
   case 0: // select file
      clrscr();
      sel_datafile(&datafile[0]);
      cntfile();
      break;
   case 1: // flash cards
      myflash();
      break;
   case 2: // clear statistics
      clrfile();
      break;
   case 3: // quit program
      go_byebye();
   }
   clrscr();
   msg(0);
}

int rfile()
{
   FILE *fptr;
   int Ncount = 0, Ycount = 0;
   // char ch;
   long int number = -1; // 0 if not yet learned, 1 if learned
   ptrtemp = NULL;

   char *field = malloc(MAX_FIELD_LENGTH);
   int field_index = 0;
   int in_quotes = 0;
   int ch;
   int row = 1;
   int column = 1;
   // long int number;
   long int line_file_start_position;
   // char qbuffer[MAX_FIELD_LENGTH];
   // char abuffer[MAX_FIELD_LENGTH];

   if ((fptr = fopen(datafile, "r")) == NULL)
   {
      disp_str(2, 0, "Can't open file", 0);
      disp_str(2, 16, datafile, 0);
      getch();
      return (2);
   }
   clear_cardmem(); // test here, so can switch between options
   ptrfirst = (struct myflashcard *)malloc(sizeof(struct myflashcard));
   ptrfirst->question = NULL;
   ptrfirst->answer = NULL;
   if (!ptrfirst)
   {
      disp_str(1, 0, "Out of memory at first card", 0);
      fclose(fptr);
      getch();
      return (2);
   }
   ptrthis = ptrfirst;

   while (((ch = fgetc(fptr)) != EOF) && (Ncount < max_recs))
   {
      if (ch == '"')
      {
         in_quotes = !in_quotes; // Toggle the in_quotes state
         continue;
      }

      if (ch == ',' && !in_quotes)
      {
         field[field_index] = '\0'; // Null-terminate the field
         switch (column)
         {
         case 1:
            char *endptr = NULL;
            number = strtol(field, &endptr, 10);
            if (endptr == field)
            {
               printf("process error at row %d\n", row);
               getch();
               endwin();
               exit(EXIT_FAILURE);
            }
            line_file_start_position = ftell(fptr) - 1 - field_index;
            break;
         case 2:
            strncpy(qbuffer, field, strlen(field));
            qbuffer[strlen(field)] = '\0';
            break;
         default:
         }

         column++;
         field_index = 0; // Reset for the next field
         continue;
      }

      // Handle newlines
      if (ch == '\n')
      {
         if (in_quotes)
         {
            // If we're inside quotes, treat newline as part of the field
            if (field_index < MAX_FIELD_LENGTH - 1)
               field[field_index++] = ch;
            continue;
         }
         else
         {
            // End of line and not in quotes
            if (field_index > 0)
            {
               field[field_index] = '\0'; // Null-terminate the last field
               switch (column)
               {
               case 3:
                  strncpy(abuffer, field, strlen(field));
                  abuffer[strlen(field)] = '\0';
                  break;
               default:
               }
               if (number)
                  Ycount++;
               else
               {
                  ptrthis->answered_correctly = number;
                  ptrthis->tries_session = 0;
                  ptrthis->disk_fptr = line_file_start_position;

                  ptrthis->question = (char *)malloc(strlen(qbuffer) + 1);
                  if (ptrthis->question == NULL)
                  {
                     disp_str(1, 0, "malloc failed for question: ", 0);
                     disp_str(1, 28, qbuffer, 0); /* refresh(); */
                     getch();
                     go_byebye();
                  }
                  strncpy(ptrthis->question, qbuffer, strlen(qbuffer));
                  ptrthis->question[strlen(qbuffer)] = '\0';

                  ptrthis->answer = (char *)malloc(strlen(abuffer) + 1);
                  if (ptrthis->answer == NULL)
                  {
                     disp_str(1, 0, "malloc failed for answer: ", 0);
                     disp_str(1, 28, abuffer, 0);
                     getch();
                     go_byebye();
                  }
                  strncpy(ptrthis->answer, abuffer, strlen(abuffer));
                  ptrthis->answer[strlen(abuffer)] = '\0';

                  Ncount++;
                  ptrthis->ptrnext = (struct myflashcard *)malloc(sizeof(struct myflashcard));
                  if (!ptrthis->ptrnext)
                  {
                     disp_str(1, 0, "Out of memory at card", 0);
                     fclose(fptr);
                     getch();
                     return (2);
                  }
                  ptrthis->ptrnext->question = NULL;
                  ptrthis->ptrnext->answer = NULL;
                  if (ptrthis->ptrnext)
                  {
                     ptrthis->ptrprev = ptrtemp;
                     ptrtemp = ptrthis;
                     ptrthis = ptrthis->ptrnext;
                  }
                  else
                  {
                     disp_str(1, 0, "Out of memory ", 1);
                     getch();
                     return (2);
                  }
               }
               row++;
               column = 1;
               field_index = 0; // Reset for the next field
            }
         }
         continue; // Continue to the next character
      }

      // Add character to the field
      if (field_index < MAX_FIELD_LENGTH - 1)
      {
         field[field_index++] = ch; // Add character to the field
      }
   }

   // Handle the last field if it exists
   if (field_index > 0)
   {
      field[field_index] = '\0'; // Null-terminate the last field
      printf("Field: %s\n", field);
   }

   free(field);

   if (Ncount == 0)
   {
      disp_str(1, 1, "There were no unmatched cards in the file selected", 0);
      disp_str(2, 1, "To use file first select CLEAR STATISTICS in menu", 0);
      disp_str(4, 1, "Press any key to continue", 0);
      getch();
      fclose(fptr);
      if (ptrfirst->question)
         free(ptrfirst->question);
      ptrfirst->question = NULL;
      if (ptrfirst->answer)
         free(ptrfirst->answer);
      ptrfirst->answer = NULL;
      free(ptrfirst);
      ptrthis = ptrfirst = NULL;
      return (2);
   }
   else
   {
      ptrtemp->ptrnext = NULL;
      ptrlast = ptrtemp;
      ptrfirst->ptrprev = NULL;
      fclose(fptr);
   }
   return (0);
}

void cntfile()
{
   FILE *fptr;
   int in_quotes = 0;
   int c; // current char
   int column = 1;
   int b; // prior char

   filecnt.correctFT = 0;
   filecnt.total = 0;
   if ((fptr = fopen(datafile, "r")) == NULL)
   {
      disp_str(2, 0, "Can't open file", 0);
      disp_str(2, 16, datafile, 0);
      getch();
      go_byebye();
   }

   while ((c = fgetc(fptr)) != EOF)
   {
      if (c == '"')
      {
         in_quotes = !in_quotes;
         continue;
      }

      if (c == ',' && !in_quotes)
      {
         if (column == 1)
         {
            if (b != '0')
            {
               filecnt.correctFT++;
            }
            filecnt.total++;
         }
         column++;
         continue;
      }

      if (c == '\n')
      {
         if (!in_quotes)
            column = 1;
         continue;
      }

      b = c;
   }

   fclose(fptr);
}

void update_file()
{
   long int offset;
   FILE *fptr;
   if (ptrfirst == (struct myflashcard *)NULL)
   {
      printf("\nCan't write empty list.\n");
      getch();
      return;
   }
   if ((fptr = fopen(datafile, "r+")) == NULL)
   {
      printf("\nCan't access disk drive\n");
      getch();
      return;
   }
   ptrthis = ptrfirst;
   while (ptrthis)
   {
      if ((ptrthis->answered_correctly) && (ptrthis->tries_session == 1))
      {
         offset = ptrthis->disk_fptr;
         fseek(fptr, offset, 0);
         fprintf(fptr, "%d", 1);
      }
      ptrthis = ptrthis->ptrnext;
   }
   fclose(fptr);
}

void myflash()
{
   clrscr();
   if (rfile() == 2)
      return;
   cnt_cards();
   if (memcnt.total == memcnt.correct)
   {
      disp_str(1, 0, "There are no unmatched cards in memory ", 1);
      msg(5);
      getch();
      return;
   }
   disp_stats_legend();
   disp_stats();
   while ((memcnt.total > memcnt.correct) && (code != QUIT))
   {
      get_rand_card(0);
      blanks(4, 27, 4, 32, 0);
      sprintf(strnum, "%d", ptrthis->tries_session);
      disp_str(4, 27, strnum, 0);
      honor_system();
      if (ptrthis->answered_correctly)
      {
         memcnt.correct++;
         blanks(3, 27, 3, 32, 0);
         sprintf(strnum, "%d", memcnt.total - memcnt.correct);
         disp_str(3, 27, strnum, 0);
         if (ptrthis->tries_session == 1)
         {
            memcnt.correctFT++;
            filecnt.correctFT++;
            blanks(3, 67, 4, 72, 0);
            sprintf(strnum, "%d", filecnt.correctFT);
            disp_str(3, 67, strnum, 0);
            sprintf(strnum, "%d", filecnt.total - filecnt.correctFT);
            disp_str(4, 67, strnum, 0);
         }
      }
   }
   clrscr();
   update_file();
}

void cnt_cards()
{
   memcnt.total = memcnt.correct = memcnt.correctFT = 0;
   ptrtemp = ptrthis;
   ptrthis = ptrfirst;
   while (ptrthis)
   {
      memcnt.total++;
      if (ptrthis->answered_correctly)
         memcnt.correct++;
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
   while (i < rand_num)
   {
      ptrthis = ptrthis->ptrnext;
      i++;
   }
   while ((ptrthis->answered_correctly) && (rand_parm == 0))
   {
      if (ptrthis->ptrnext)
      {
         ptrthis = ptrthis->ptrnext;
      }
      else
         ptrthis = ptrfirst;
   }
}

void honor_system()
{
   do
   {
      disp_str(6, 0, ptrthis->question, 0);
      msg(3);
      int validInput = 0;
      do
      {
         code = getch();
         switch (code)
         {
         case KEY_RIGHT: // show answer
            validInput = 1;
            break;
         case QUIT: // quit without affecting stats
            return;
         }
      } while (!validInput);
      blanks(6, 0, 23, 99, 0); // clear question

      disp_str(6, 0, ptrthis->answer, 0);
      msg(4);
      validInput = 0;
      do
      {
         code = getch();
         switch (code)
         {
         case KEY_DOWN: // got it wrong
            ptrthis->tries_session++;
            validInput = 1;
            break;
         case KEY_UP: // got it right
            ptrthis->tries_session++;
            ptrthis->answered_correctly = 1;
            validInput = 1;
            break;
         case KEY_LEFT:
            validInput = 1;
         }
      } while (!validInput);
      blanks(6, 0, 23, 99, 0); // clear answer
   } while (code == KEY_LEFT);
}

// how many characters ch are in string str?
int count_char(char *str, char ch)
{
   int count = 0;
   for (int i = 0; i < strlen(str); i++)
   {
      if (ch == str[i])
         count++;
   }
   return count;
}

// what is the size of the string str from end back to character ch (or back to start of str if no char ch)?
int size_after_last_char(char *str, char ch)
{
   int size = 0;
   for (int i = strlen(str) - 1; i >= 0; i--)
   {
      if (ch == str[i])
         break;
      size++;
   }
   return size;
}

void disp_stats_legend()
{
   disp_str(2, 0, "Total cards this session:", 0);
   disp_str(3, 0, "Remaining this session:", 0);
   disp_str(4, 0, "Times tried this session:", 0);
   disp_str(2, 40, "Total # of cards in file:", 0);
   disp_str(3, 40, "Got right the first time:", 0);
   disp_str(4, 40, "Still need some practice:", 0);
}

void disp_stats()
{
   sprintf(strnum, "%d", memcnt.total);
   disp_str(2, 27, strnum, 0);
   sprintf(strnum, "%d", memcnt.total - memcnt.correct);
   disp_str(3, 27, strnum, 0);
   sprintf(strnum, "%d", filecnt.total);
   disp_str(2, 67, strnum, 0);
   sprintf(strnum, "%d", filecnt.correctFT);
   disp_str(3, 67, strnum, 0);
   sprintf(strnum, "%d", filecnt.total - filecnt.correctFT);
   disp_str(4, 67, strnum, 0);
}

void clrfile()
{

   FILE *file = fopen(datafile, "r+");
    if (!file)
    {
        perror("Unable to open file");
        getch();
        return;
    }

    int in_quotes = 0;
    int c; // current char
    int column = 1;
    int b; // prior char

    while ((c = fgetc(file)) != EOF)
    {
        if (c == '"')
        {
            in_quotes = !in_quotes;
            continue;
        }

        if (c == ',' && !in_quotes)
        {
            if (column == 1 && b != '0')
            {
                long int current_position = ftell(file);
                fseek(file, current_position - 2, 0);
                fprintf(file, "%d", 0);
                fseek(file, current_position, 0);
            }
            column++;
            continue;
        }

        if (c == '\n')
        {
            if (!in_quotes)
                column = 1;
            continue;
        }

        b = c;
    }

    fclose(file);
    filecnt.correctFT = 0;

}

void clear_cardmem()
{
   while (ptrfirst)
   {
      ptrthis = ptrfirst->ptrnext;
      if (ptrfirst->question)
         free(ptrfirst->question);
      ptrfirst->question = NULL;
      if (ptrfirst->answer)
         free(ptrfirst->answer);
      ptrfirst->answer = NULL;
      free(ptrfirst);
      ptrfirst = ptrthis;
   }
}

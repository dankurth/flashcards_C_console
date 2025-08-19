/* main program for C version of FLASH by Dan Kurth */
/*
Examples of builds tested:
On virtual (VirtualBox 6.1.26) Windows XP using MinGW version v10.0.0:
  gcc -I/MinGW/include/ncurses -o flash fcm.c -lncurses -L/MinGW/bin -static
On Debian GNU/Linux:
  // gcc -o flash fcm.c -I/usr/include/ncurses -lncurses -ltermcap -Wall -g
  // does not display characters such as Él properly using mvprintw

  gcc -o flash fcm.c -I/usr/include/ncursesw/ncursesw -lncursesw -ltermcap -Wall -g
  // displays characters such as Él properly using mvprintw,
  // even though still passing (char *) not (wchar_t *)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <curses.h>
#include <locale.h>
#include <ctype.h>
#include <stdbool.h>

#include "fcm.h"
#include "display.c"
#include "sel-file.c"

/* C routines */
void disp_menu(void); /* display menu to user */
void msg(int msgnum); /* display messages to user  */
void action(void);    /* initiate action corresponding to request by user*/

int sel_datafile(char datafile[], int lastRow); /* choose a file to use for session */
void clrfile(void);                             /* clear stats/scores of disk data file */
int cntfile(void);                              /* cnt right/wrong in whole file */
int rfile(void);                                /* read data file into memory (some or all records)*/
void update_file(void);                         /* update disk record stats to match those in memory   */

void myflash(void); /* show questions, prompt for answers & keep stats */
void cnt_cards(void);
void clear_cardmem(void);          /* clear all cards from memory */
void get_rand_card(int rand_parm); /* get a card, any card */
void honor_system(void);           /* user tells computer if answer was right */
void pick(int);                    /*                                */
void disp_stats(void);             /* show session & file stats */
void disp_stats_legend(void);      /* show legend for session & file stats */
bool isEmptyOrSpaces(const char str[]);

void clrscr()
{
   clear();
   refresh();
}

char qbuffer[MAX_FIELD_LENGTH]; // question
char abuffer[MAX_FIELD_LENGTH]; // answer
char ibuffer[MAX_FIELD_LENGTH]; // instructions (if any)

struct myflashcard
{
   int answered_correctly; // 0 or 1
   char *question;
   char *answer;
   char *instructions; // optional, may be NULL
   long int disk_fptr; // start of corresponding row in datafile, used to update file
   struct myflashcard *ptrnext;
};
struct myflashcard *ptrfirst = NULL;
struct myflashcard *ptrthis = NULL;
struct myflashcard *ptrtemp = NULL;
struct myflashcard *ptrprev = NULL;

struct
{
   int total;
   int correct;
} filecnt;

struct
{
   int total;
   int correct;
} memcnt;

char *str[] = {
    "select file     ", // main menu
    "flash cards     ",
    "clear statistics",
    "quit            "};

char datafile[256] = "NoName"; /* name of file to use as datafile */
int firstMenuRow = 0;          /* first row of str to display as menu */
int lastMenuRow = 3;           /* last row of str to display as menu */
int pos = 0;                   /* which row of menu highlighted */
int code;                      /* keyboard input */

char strnum[4];
int max_recs = 100; // limit per session in rfile, does not affect file stats shown to user
int lastRow;        // set after stdscr is initiated, last row of display for curses
int quit = false;   // flag set in menu to quit the program gracefully

int main(int argc, char *argv[])
{
   setlocale(LC_ALL, "");
   initscr();
   lastRow = getmaxy(stdscr) - 1;
   cbreak();
   noecho();
   keypad(stdscr, TRUE);
   leaveok(stdscr, TRUE);
   clrscr();
   curs_set(0); // turn that annoying blinking cursor off for the duration
   if (argc > 1)
   {
      strcpy(datafile, argv[1]); // file name to use
   }
   msg(0);
   srand(1);

   while (!quit)
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
            disp_str(pos - firstMenuRow + ver_offset, hor_offset, str[pos], 0);
            if (pos > firstMenuRow)
               --pos;
            else
               pos = lastMenuRow;
            disp_str(pos - firstMenuRow + ver_offset, hor_offset, str[pos], 1);
            break;
         case KEY_DOWN:
            disp_str(pos - firstMenuRow + ver_offset, hor_offset, str[pos], 0);
            if (pos < lastMenuRow)
               ++pos;
            else
               pos = firstMenuRow;
            disp_str(pos - firstMenuRow + ver_offset, hor_offset, str[pos], 1);
            break;
         case ENTER:
            action();
            code = ENTER;
            break;
         }
      } while (code != ENTER);
   }
   clear();
   refresh();
   endwin();
   printf("Flash by Dan Kurth ended normally\n");
   return 0;
}

void disp_menu()
{
   int row;
   for (row = firstMenuRow; row <= lastMenuRow; row++)
   {
      if (row == pos)
         disp_str(row - firstMenuRow + ver_offset, hor_offset, str[row], 1);
      else
         disp_str(row - firstMenuRow + ver_offset, hor_offset, str[row], 0);
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

   blanks(lastRow, 0, lastRow + 1, 79, 0); // clear bottom of screen only
   disp_str(lastRow, 0, msgptr[msgnum], 0);
}

void action()
{
   switch (pos)
   {
   case 0: // select file
      clrscr();
      sel_datafile(datafile, lastRow);
      break;
   case 1:            // flash cards
      if (!cntfile()) // if no errors
         myflash();
      break;
   case 2: // clear statistics
      clrfile();
      break;
   case 3: // quit program
      quit = 1;
      return;
   }
   clrscr();
   msg(0);
}

int rfile()
{
   FILE *fptr;
   int Ncount = 0, Ycount = 0;
   long int number = -1; // 0 if not yet learned, 1 if learned

   char csvfield[MAX_FIELD_LENGTH];
   int field_index = 0;
   int in_quotes = 0;
   int ch;
   int row = 1;
   int column = 1;
   long int line_file_start_position;
   qbuffer[0] = '\0';
   abuffer[0] = '\0';
   ibuffer[0] = '\0';

   if ((fptr = fopen(datafile, "r")) == NULL)
   {
      em("rfile: Cannot open file");
      return 2;
   }
   clear_cardmem();

   while (((ch = fgetc(fptr)) != EOF) && (Ncount < max_recs))
   {
      if (ch == '"')
      {
         in_quotes = !in_quotes; // Toggle the in_quotes state
         continue;
      }

      if (ch == ',' && !in_quotes)
      {
         csvfield[field_index] = '\0'; // Null-terminate the csvfield
         switch (column)
         {
         case 1: // the marker for learned or not learned, a number (0 or 1)
            char *endptr = NULL;
            number = strtol(csvfield, &endptr, 10);
            if (endptr == csvfield)
            {
               char buffer[80];
               sprintf(buffer, "rfile: Invalid number in col 1 row %d\n", row);
               char *msg = buffer;
               em(msg);
               return 2;
            }
            line_file_start_position = ftell(fptr) - 1 - field_index;
            break;
         case 2: // the question
            strncpy(qbuffer, csvfield, strlen(csvfield));
            qbuffer[strlen(csvfield)] = '\0';
            if (isEmptyOrSpaces(qbuffer))
            {
               char buffer[80];
               sprintf(buffer, "rfile: blank or empty question at row %d\n", row);
               char *msg = buffer;
               em(msg);
               return 2;
            }
            break;
         case 3: // the answer (if delimiter comma due to optional instructions in column 4)
            strncpy(abuffer, csvfield, strlen(csvfield));
            abuffer[strlen(csvfield)] = '\0';
            if (isEmptyOrSpaces(abuffer))
            {
               char buffer[80];
               sprintf(buffer, "rfile: blank or empty answer at row %d\n", row);
               char *msg = buffer;
               em(msg);
               return 2;
            }
            break;
         default: // 4th column is last used but as last is not terminated by comma
         }
         column++;
         field_index = 0; // Reset for the next csvfield
         continue;
      }

      // Handle newlines
      if (ch == '\n')
      {
         if (in_quotes)
         {
            // If we're inside quotes, treat newline as part of the csvfield
            if (field_index < MAX_FIELD_LENGTH - 1)
               csvfield[field_index++] = ch;
            continue;
         }
         else
         {
            csvfield[field_index] = '\0'; // Null-terminate the last csvfield
            switch (column)
            {
            case 1: // digit followed by newline
            case 2: // question followed by newline
               char buffer[80];
               sprintf(buffer, "rfile: non-delimited column on row %d\n", row);
               char *msg = buffer;
               em(msg);
               return 2;
               break;
            case 3: // answer followed by newline (so....no optional instructions)
               strncpy(abuffer, csvfield, strlen(csvfield));
               abuffer[strlen(csvfield)] = '\0';
               if (isEmptyOrSpaces(abuffer))
               {
                  char buffer[80];
                  sprintf(buffer, "rfile: blank or empty answer at row %d\n", row);
                  char *msg = buffer;
                  em(msg);
                  return 2;
               }
               break;
            case 4: // optional instructions
               strncpy(ibuffer, csvfield, strlen(csvfield));
               ibuffer[strlen(csvfield)] = '\0';
               break;
            default:
            }
            if (number)
               Ycount++;
            else // not marked as answered correctly yet, so add it to session quiz list
            {
               ptrthis = (struct myflashcard *)malloc(sizeof(struct myflashcard));
               if (ptrthis == NULL)
               {
                  char buffer[80];
                  sprintf(buffer, "rfile: malloc failed for ptrthis at Ncount %d\n", Ncount);
                  char *msg = buffer;
                  em(msg);
                  return 2;
               }
               else
               {
                  ptrthis->answered_correctly = number;
                  ptrthis->disk_fptr = line_file_start_position;

                  ptrthis->question = (char *)malloc(strlen(qbuffer) + 1);
                  if (ptrthis->question == NULL)
                  {
                     char buffer[80];
                     sprintf(buffer, "rfile: malloc failed for question on row %d\n", row);
                     char *msg = buffer;
                     em(msg);
                     return 2;
                  }
                  strncpy(ptrthis->question, qbuffer, strlen(qbuffer));
                  ptrthis->question[strlen(qbuffer)] = '\0';
                  qbuffer[0] = '\0';

                  ptrthis->answer = (char *)malloc(strlen(abuffer) + 1);
                  if (ptrthis->answer == NULL)
                  {
                     char buffer[80];
                     sprintf(buffer, "rfile: malloc failed for answer on row %d\n", row);
                     char *msg = buffer;
                     em(msg);
                     return 2;
                  }
                  strncpy(ptrthis->answer, abuffer, strlen(abuffer));
                  ptrthis->answer[strlen(abuffer)] = '\0';
                  abuffer[0] = '\0';

                  if (isEmptyOrSpaces(ibuffer))
                  {
                     ptrthis->instructions = NULL;
                     ibuffer[0] = '\0';
                  }
                  else
                  {
                     ptrthis->instructions = (char *)malloc(strlen(ibuffer) + 1);
                     if (ptrthis->instructions == NULL)
                     {
                        char buffer[80];
                        sprintf(buffer, "rfile: malloc failed for instructions on row %d\n", row);
                        char *msg = buffer;
                        em(msg);
                        return 2;
                     }
                     strncpy(ptrthis->instructions, ibuffer, strlen(ibuffer));
                     ptrthis->instructions[strlen(ibuffer)] = '\0';
                  }

                  Ncount++;

                  ptrthis->ptrnext = NULL; // no card after this card yet

                  if (ptrfirst == NULL) // there can be only one
                     ptrfirst = ptrthis;

                  if (ptrprev != NULL)           // if there was a previous card
                     ptrprev->ptrnext = ptrthis; // next card of previous card is ... this card

                  ptrprev = ptrthis; // on next round the previous card will be ... this card
               }
            }
            row++;
            column = 1;
            field_index = 0; // Reset for the next csvfield
         }
         continue; // Continue to the next character
      }

      // not a start or end quote, nor a working (terminating) comma, nor a working newline
      // so must be a content character for the current column/field
      if (field_index < MAX_FIELD_LENGTH - 1)
      { // add character to the csvfield, within limits
         csvfield[field_index++] = ch;
      }
   }

   if (Ncount == 0)
   {
      em("rfile: There were no unmatched cards in the file selected");
      return 2;
   }

   fclose(fptr);
   return 0;
}

// count total and correct within file
int cntfile()
{
   FILE *fptr;
   int in_quotes = 0;
   int c; // current char
   int column = 1;
   int b; // prior char

   filecnt.correct = 0;
   filecnt.total = 0;
   if ((fptr = fopen(datafile, "r")) == NULL)
   {
      em("cntfile: Cannot open file");
      return 1;
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
               filecnt.correct++;
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
   return 0;
}

void update_file()
{
   long int offset;
   FILE *fptr;
   if (ptrfirst == NULL)
   {
      em("update_file: empty list");
      return;
   }
   if ((fptr = fopen(datafile, "r+")) == NULL)
   {
      em("update_file: failed to open file");
      return;
   }
   ptrthis = ptrfirst;
   while (ptrthis)
   {
      if (ptrthis->answered_correctly)
      {
         offset = ptrthis->disk_fptr;
         fseek(fptr, offset, 0);
         fprintf(fptr, "%d", 1); // mark as answered correctly
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
   cnt_cards(); // count total and correct in memory
   if (memcnt.total == memcnt.correct)
   {
      em("myflash: There are no unmatched cards in memory");
      return;
   }
   disp_stats_legend();
   disp_stats();
   while ((memcnt.total > memcnt.correct) && (code != QUIT))
   {
      get_rand_card(0);
      honor_system();
      if (ptrthis->answered_correctly)
      {
         memcnt.correct++;
         filecnt.correct++;
         blanks(3, 56, 4, 72, 0);
         sprintf(strnum, "%d", filecnt.correct);
         disp_str(3, 56, strnum, 0);
         sprintf(strnum, "%d", filecnt.total - filecnt.correct);
         disp_str(4, 56, strnum, 0);
      }
   }
   clrscr();
   update_file();
}

void cnt_cards() // count total and correct in memory
{
   memcnt.total = memcnt.correct = 0;
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
      if (ptrthis->instructions != NULL)
         disp_str(6, 0, ptrthis->instructions, 0);
      disp_str(8, 0, ptrthis->question, 0);
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
      blanks(6, 0, 23, 99, 0); // clear question and any related instructions

      disp_str(8, 0, ptrthis->answer, 0);
      msg(4);
      validInput = 0;
      do
      {
         code = getch();
         switch (code)
         {
         case KEY_DOWN: // got it wrong
            validInput = 1;
            break;
         case KEY_UP: // got it right
            ptrthis->answered_correctly = 1;
            validInput = 1;
            break;
         case KEY_LEFT:
            validInput = 1;
         }
      } while (!validInput);
      blanks(8, 0, 23, 99, 0); // clear answer
   } while (code == KEY_LEFT);
}

void disp_stats_legend()
{
   disp_str(2, 40, "        Cards:", 0); // # of cards IN FILE
   disp_str(3, 40, "      Correct:", 0);
   disp_str(4, 40, "    Remaining:", 0);
}

void disp_stats() // of cards in file, cards in memory are subset of those loaded for session
{
   sprintf(strnum, "%d", filecnt.total);
   disp_str(2, 56, strnum, 0);
   sprintf(strnum, "%d", filecnt.correct);
   disp_str(3, 56, strnum, 0);
   sprintf(strnum, "%d", filecnt.total - filecnt.correct);
   disp_str(4, 56, strnum, 0);
}

void clrfile() // set digit in column 1 to zero
{

   FILE *file = fopen(datafile, "r+");
   if (!file)
   {
      em("clrfile: Unable to open file");
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
   filecnt.correct = 0;
}

void clear_cardmem()
{
   while (ptrfirst != NULL)
   {
      if (ptrfirst->ptrnext != NULL)
      {
         ptrtemp = ptrfirst;
         ptrfirst = ptrfirst->ptrnext;

         if (ptrtemp->question != NULL)
         {
            free(ptrtemp->question);
            ptrtemp->question = NULL;
         }
         if (ptrtemp->answer != NULL)
         {
            free(ptrtemp->answer);
            ptrtemp->answer = NULL;
         }
         free(ptrtemp);
         ptrtemp = NULL;
      }
      else
      {
         if (ptrfirst->question != NULL)
         {
            free(ptrfirst->question);
            ptrfirst->question = NULL;
         }
         if (ptrfirst->answer != NULL)
         {
            free(ptrfirst->answer);
            ptrfirst->answer = NULL;
         }
         free(ptrfirst);
         ptrfirst = NULL;
      }
   }
}

bool isEmptyOrSpaces(const char str[])
{
   for (int i = 0; str[i] != '\0'; i++)
   {
      // If find a non-whitespace character, return false
      if (!isspace((unsigned char)str[i]))
      {
         return false;
      }
   }
   // If only found whitespace or the string is empty, return true
   return true;
}

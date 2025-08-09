
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "fcm.h"

char fileList[30][MAX_FIELD_LENGTH]; // up to 30 filenames, need more?
int lastRow = 15;                    // index of last row to display
int lastFile = 0;                    // index of last file in fileList
int t = 0;                           // top, index of item in fileList on visible on first row
int s = 0;                           // selected, index of row to highlight
int i = 0;                           // re-usable, general purpose

void display_fileList(void)
{
  clear();
  i = t;
  for (int row = 0; row <= lastRow; row++)
  {
    if (i > lastFile)
      break;
    if (row == s)
    {
      disp_str(row, 0, fileList[i++], 1);
    }
    else
    {
      disp_str(row, 0, fileList[i++], 0);
    }
  }
}

void sel_datafile(char *datafile)
{
  int code;
  DIR *dirp;
  struct dirent *dp;

  dirp = opendir("."); /* open dir, read a file name */
  if (!dirp)
    exit(0); /* error opening directory, no files */
  for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
  {
    if (!strcmp(dp->d_name + strlen(dp->d_name) - 4, ".csv"))
      strcpy(fileList[i++], dp->d_name);
  }
  closedir(dirp);
  if (!i)
    return; // no suitable files
  lastFile = i - 1;

  display_fileList();

  do
  {
    code = getch();
    switch (code)
    {
    case KEY_UP:
      if (s == 0)
      {
        if (t > 0)
          t--;
      }
      else
      {
        s--;
      }
      display_fileList();
      break;
    case KEY_DOWN:
      if (s == lastRow)
      {
        if (lastFile > (t + lastRow))
          t++;
      }
      else
      {
        s++;
      }
      display_fileList();
      break;
    case ENTER:
      break;
    }
  } while (code != ENTER);

  strcpy(datafile, fileList[s]);
}

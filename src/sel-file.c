
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include "fcm.h"

int lastRow = 15; // maximum screen row index to use, might use less
int lastFile = 0; // fileList index for the last filename in it
int t = 0;        // top, index of filename shown in first row of screen
int s = 0;        // selected row, index of row selected

/**
 * Displays subset of filenames from fileList in rows.
 * Screen rows are as used by ncurses, numbered from top starting at zero.
 * Filename at fileList[t] goes into first row (row[0]), fileList[t+1] into row[1],
 * until either last row is filled or fileList has no more filenames,
 * whichever comes first.
 * The filename which is on the selected row (first row by default) is highlighted,
 * all others are shown in normal font.
 */
void display_fileList(char **files)
{
  clear();
  int i = t;
  for (int row = 0; row <= lastRow; row++)
  {
    if (i > lastFile)
      break;
    if (row == s)
    {
      disp_str(row, 0, files[i++], 1);
    }
    else
    {
      disp_str(row, 0, files[i++], 0);
    }
  }
}

void sel_datafile(char datafile[])
{
  int code;
  DIR *dirp;
  struct dirent *dp;
  char **fileList;
  int c;

  dirp = opendir("."); /* open dir, read a file name */
  if (!dirp)
    return; /* error opening directory, no files */

  c=0;
  while ((dp = readdir(dirp)) != NULL)
  {
    if (!strcmp(dp->d_name + strlen(dp->d_name) - 4, ".csv"))
      c++; // number of suitable files
  }
  closedir(dirp);
  if (!c)
    return; // no suitable files
  fileList = (char **)malloc(c * sizeof(char *));
  if (fileList == NULL)
    return; // failed to allocate

  int i = 0;
  dirp = opendir("."); // open dir again (how else would I reset dirp?)
  while ((dp = readdir(dirp)) != NULL)
  {
    if (!strcmp(dp->d_name + strlen(dp->d_name) - 4, ".csv"))
    {
      fileList[i] = (char *)malloc(((strlen(dp->d_name)) * sizeof(char)) + 1);
      if (fileList[i] == NULL) return;
      strcpy(fileList[i++], dp->d_name); // copy d_name including string terminator
    }
  }
  closedir(dirp);
  lastFile = i - 1;

  display_fileList(fileList);

  /** As user scrolls down list of displayed filenames each is highlighted in turn.
   * If user scrolls down below bottom row the list of displayed filenames is moved up by one
   * to show and select next filename from fileList (at which point the first displayed filename
   * is moved up and replaced by the next at top of screen in frrst row).
   */
  do
  {
    code = getch();
    switch (code)
    {
    case KEY_UP: // user pressed up arrow
      if (s > 0)
      {
        s--;
        display_fileList(fileList);
      }
      else if (t > 0)
      {
        t--;
        display_fileList(fileList);
      }
      break;
    case KEY_DOWN: // user pressed down arrow
      if (s < (lastFile - t))
      {
        if (s < lastRow)
          s++;
        else
          t++;
        display_fileList(fileList);
      }
      break;
    case ENTER:
      break;
    }
  } while (code != ENTER);

  strcpy(datafile, fileList[s]); // use the filename selected

  for (i=0; i<c; i++) 
  {
    free(fileList[i]);
  }
  free(fileList);
}

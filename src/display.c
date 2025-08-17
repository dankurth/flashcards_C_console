
void disp_str(int row, int column, char strtext[], int attrib)
{
  if (attrib == 1)
  {
    standout();
    /*  mvaddstr(row,column,strtext); */
    mvprintw(row, column, "%s", strtext);
    standend();
  }
  else
  {
    /*  mvaddstr(row,column,strtext); */
    mvprintw(row, column, "%s", strtext);
  }
  move(23, 79);
  refresh();
}


void blanks(int row1, int column1, int row2, int column2, int attribute)
{
  int y, x;
  for (y = row1; y <= row2; y++)
  {
    for (x = column1; x <= column2; x++)
      mvaddch(y, x, ' ');
  }
  refresh();
}

/**
 * usage:
 *   em("sample error"); return 1;
 * or
 *   char buffer[40];
 *   sprintf(buffer, "sample error: i %d j %d", i, j);
 *   char *msg = buffer;
 *   em(msg); return 1;
 */
void em (char *msg)
{
  clear();
  disp_str(0, 1, msg, 0);
  disp_str(2, 1, "Press any key to continue", 0);
  getch();
}





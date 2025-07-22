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

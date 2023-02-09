
void disp_str(int row,int column,char strtext[],int attrib)
{
if(attrib==1) 
  {
  standout();
/*  mvaddstr(row,column,strtext); */
  mvprintw(row,column,"%s",strtext); 
  standend();
  }
else 
  {
/*  mvaddstr(row,column,strtext); */
  mvprintw(row,column,"%s",strtext); 
  }
move(23,79); 
refresh(); 
}

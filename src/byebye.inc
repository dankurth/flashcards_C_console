void go_byebye(void)
{
#ifdef DOS
clrscr();
cursor_on();
#else
clear();
refresh();
endwin();
#endif
printf("Flash by Dan Kurth ended normally\n");
exit(0);
}

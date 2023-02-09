
#ifdef DOS
void cursor_off(void)
{
union REGS regs;
regs.h.ch = STOPBIT;
regs.h.ah = CURSIZE;
int86(VIDEO, &regs, &regs);
}

void cursor_on(void)
{
union REGS regs;
regs.h.ch = 0x06;   /* color screen, not monochrome */
regs.h.cl = 0x07;
regs.h.ah = CURSIZE;
int86(VIDEO, &regs, &regs);
}
#else

void cursor_off(void)
{
int j;
j=1;
}

void cursor_on(void)
{
int j;
j=1;
}
#endif


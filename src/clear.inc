
#ifdef DOS
#pragma inline
/*
 int 10h version of _clr_scr.c, slower to setup but faster if clearing whole
screen, almost as fast as dos clrscr but in addition puts out selected color
*/




void clr_scr(int attribute)
{
asm  push ax
asm  push bx
asm  push cx
asm  push dx

asm     mov bh,1fh
asm     mov ax,attribute
asm     cmp al,0
asm  jz rev_vid
asm  mov bh,71h
rev_vid:
asm  mov ch,0
asm  mov cl,0
asm  mov dh,24
asm  mov dl,79
asm  mov ah,6
asm  mov al,0
asm  int 10h

asm  pop dx
asm  pop cx
asm  pop bx
asm  pop ax
}

#else

void clr_scr(int attribute)
{
/* uses curses.h */
clear();
refresh();
}
#endif

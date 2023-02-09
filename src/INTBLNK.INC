#pragma inline
void blanks(int row1,int column1,int row2,int column2,int attribute)
{
asm  mov cx,row2             /* calc total rows */
asm  sub cx,row1
asm  add cx,1                /* total number of rows to blank */
asm  mov dx,column1            /* use dl to pass first column */
asm  mov dh,row1               /* uses dh to pass first row */
asm  mov ax,column2        /* calc total columns */
asm  sub ax,column1
asm  add ax,1                /* total columns to blank in each row */
asm  mov bh,al       /* use bh to pass total # of columns */
asm     mov bl,1fh             /* set up attribute */
asm     mov ax,attribute
asm     cmp al,0
asm  jz the_loop
asm  mov bl,71h          /* use bl to pass attribute */
asm  mov al,' '          /* use al to pass char */
the_loop:
asm  call _blank_line
asm  inc dh           /* uses dh to pass current row to blank */
asm  loop the_loop
}

void blank_line()
/*  receives
al:char
dh:row
dl:column1
bl:attrib
bh:#columns
*/
{
asm  push bx
asm  push cx
asm  mov cl,bh          /* set for column length passed in bh */
asm  mov ch,0
put_out:
/*   set cursor()          row in dh, column1 in dl */
asm  push ax
asm  push bx
asm  mov bh,0
asm  mov ah,2
asm  int 10h
asm  pop bx
asm  pop ax
/*   write chars()      attrib in bl, char in al, column length in cx */
asm     push ax
asm     push bx
asm  mov ah,9
asm  mov bh,0
asm  int 10h
asm  pop bx
asm  pop ax
exit:
asm  pop cx
asm  pop bx
}

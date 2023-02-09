#pragma inline

void draw_box(unsigned char row1,unsigned char column1,unsigned char row2,\
unsigned char column2,unsigned char attribute)
{
set_attrib:
asm  mov bl,1fh          /* set up attribute */
asm  mov al,attribute
asm  cmp al,0
asm  jz  vid
asm  mov bl,71h          /* use bl to pass attribute */
vid:
asm  mov bh,0             /* set vid page for ints */

horiz_lines:
asm  mov cx,column2        /* last column */
asm  sub cx,column1        /* last column minus first column = # columns */
asm  mov al,0c4h          /* char '-' */
asm  mov dl,column1           /* start at first column */
asm  mov dh,row1           /* row to start at (top row) */
asm  mov ah,2           /* int 10, service 2 */
asm  int 10h           /* set cursor */
asm  mov ah,9           /* int 10, service 9 */
asm  int 10h          /* write char */
asm  mov dh,row2         /* row to start at (bottom row), all else the same */
asm  mov ah,2
asm  int 10h           /* set cursor */
asm  mov ah,9
asm  int 10h          /* write char */

vertical_lines:
asm  mov cx,row2          /* calc total rows enclosed by box */
asm  sub cx,row1
asm  sub cx,1
asm  mov dh,row1             /* top row of box (horiz line & corners) */
asm  add dh,1                /* sides start one down from top line */
asm  mov al,0B3h                /* char '³' */
the_loop:                /* do left & right side time thru loop */
asm  push cx           /* stash # of rows to be done */
asm  mov cx,1          /* one char at a time */
asm  mov dl,column1         /* left side of box */
asm  mov ah,2
asm  int 10h           /* set cursor */
asm  mov ah,9
asm  int 10h          /* write char */
asm  mov dl,column2          /* right side of box */
asm  mov ah,2
asm  int 10h           /* set cursor */
asm  mov ah,9
asm  int 10h          /* write char */
asm  inc dh                  /* next row down */
asm  pop cx              /* retrieve # of rows to be done */
asm  loop the_loop          /* controlled by cx, decremented each loop */

corners:
asm  mov cx,1              /* # of columns, 1 for corners */
upper_left:
asm  mov dl,column1
asm  mov dh,row1
asm  mov al,0DAh              /* char 'Ú' */
asm  mov ah,2
asm  int 10h           /* set cursor */
asm  mov ah,9
asm  int 10h          /* write char */
upper_right:
asm  mov dl,column2
asm  mov dh,row1
asm  mov al,0BFh              /* char '¿' */
asm  mov ah,2
asm  int 10h           /* set cursor */
asm  mov ah,9
asm  int 10h          /* write char */
lower_left:
asm  mov dl,column1
asm  mov dh,row2
asm  mov al,0C0h                 /* char 'À' */
asm  mov ah,2
asm  int 10h           /* set cursor */
asm  mov ah,9
asm  int 10h          /* write char */
lower_right:
asm  mov dl,column2
asm  mov dh,row2
asm  mov al,0D9h                   /* char 'Ù' */
asm  mov ah,2
asm  int 10h           /* set cursor */
asm  mov ah,9
asm  int 10h          /* write char */
}

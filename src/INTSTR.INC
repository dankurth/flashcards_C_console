#pragma inline

void disp_str(int row,int column,char strtext[],int attrib)
{
asm     mov bl,1fh             /* set attribute to normal video */
asm     mov ax,attrib         /* get set to compare passed parm */
asm     cmp al,0            /* see if user selected normal attribute */
asm     jz vid            /* if so then we're ready to set it */
asm     mov bl,71h      /* else we'll change the attrib to reverse video */
vid:                     /*  set up other parms for int's */
asm     mov bh,0          /* set video page */
asm     mov cx,1          /* # words for int 10 service 9 goes to cx */
asm     mov ax,row        /* row goes to dh */
asm     mov dh,al
asm	mov ax,column     /* column goes to dl */
asm 	mov dl,al
asm     mov di,strtext     /* di will put to each char one by one */
psloop:                        /* spit 'em out */
asm     mov al,byte ptr [di]  /* store one char */
asm     cmp al,0              /* are we at end of string? */
asm     je  exit             /* if so then quit */
asm     mov ah,2               /* service 2 */
asm     int 10h
asm     mov ah,9              /* service 9 */
asm     int 10h
asm	inc dl                /* set to next column */
asm     inc di	              /* set to next char */
asm     jmp psloop
exit:
;}

/* bios service to set cursor position (int 10, service 2)
ah: 2
bh: video page number
dh: cursor row
dl: cursor column
   bios service to write char & attribute (int 10, service 9)
ah: 9
al: ascii character code
bh: video page number
bl: video attribute of character in al
cx: number of character/attribute words to display


	      */


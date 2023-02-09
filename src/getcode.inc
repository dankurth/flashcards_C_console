int getcode(void)
{
   int key;
   key = getch();
   if(key==KEY_LEFT) return(L_ARROW);
   if(key==KEY_RIGHT) return(R_ARROW);
   if(key==KEY_UP) return(U_ARROW);
   if(key==KEY_DOWN) return(D_ARROW);
   if((key==KEY_ENTER) || (key==10)) return(ENTER);
   if(key==113) return(ESC);
}


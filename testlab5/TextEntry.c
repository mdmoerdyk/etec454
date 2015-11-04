/********************
Mark Moerdyk
First modification: 2/18/13
Last modification: 2/22/13
**************************/

#include "includes.h"

#define DC1 (INT8U)0x11
#define DC2 (INT8U)0x12
#define DC3 (INT8U)0x13
#define DC4 (INT8U)0x14
#define TEXT_CHAR 16 

void TypeText(void);

typedef enum {WAITFORPRESS, WAITFORONESEC, CLEARROW}TEXTSTATES;



void TypeText(void)
{
    TEXTSTATES curstate = WAITFORPRESS;
    INT8U keypress = 0;
    INT8U err;
    INT8U charrow = 2;
    INT8U text_done = FALSE;
    INT8U charcol = 1;  // 16
    INT8U char_value[TEXT_CHAR];
    INT8U first_value;
    INT8U second_value;
    INT8U displayed_characters[10][6] = {
    { '1','.',',',0x27,'?','!'},
    {'2','A','B','C',0,0},
    {'3','D','E','F',0,0},
    {'4','G','H','I',0,0},
    {'5','J','K','L',0,0},
    {'6','M','N','O',0,0},
    {'7','P','Q','R','S',0},
    {'8','T','U','V',0,0},
    {'9','W','X','Y','Z',0},
    {'0','_',0x7F,0,0,0}};
    
    
    
    
    while(text_done == FALSE)
    {
        switch(curstate)
        {
            case(WAITFORPRESS):
                 while((keypress != '0') && (keypress != '1') && (keypress != '2') 
                     && (keypress != '3') && (keypress != '4') && (keypress != '5')
                     && (keypress != '6') && (keypress != '7')&& (keypress != '8')
                     && (keypress != '9')&& (keypress != DC2)&& (keypress != DC3)
                     && (keypress != DC1))
                {
                    keypress = KeyPend(0, &err);
                }
                first_value = keypress - 48;
                LcdDispChar(2,1,CLOCK_LAYER,keypress);
                if( keypress == DC3)
                {
                    text_done = TRUE;
                }
                else
                {
                    curstate = WAITFORPRESS;
                    keypress = 0;
                }
                break;
            case(WAITFORONESEC):
                keypress = KeyPend(1000, &err);
            default:
            break;
                             
            
        }    
    }
    
    
   
} 


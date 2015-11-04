/********************************************************************
* LCDKeyTerm.c - A simple keypad demonstration program. 
* It reads a keypad by using the key.c module and outputs the key
* pressed to the LCD display. 
*
* Todd Morton, 01/15/2010 
*********************************************************************
* Include Project Master Header File
********************************************************************/
#include "includes.h"
#define SLICE_PER 10
#define LINELENGTH 16
#define NUMLINES 2
#define DC1 (INT8U)0x11
#define DC2 (INT8U)0x12
#define DC3 (INT8U)0x13
#define DC4 (INT8U)0x14

/********************************************************************
* Global variable and function definitions
*************************************************************************/
void DispKeyTask(void);
void WaitForSlice(void);    /* Time slicer */
/************************************************************************/
void main(void) {
    
    OCDlyInit();
    ENABLE_INT();         /* Enable interrupts to use OCDelay()   */
    KeyInit();
    LcdInit();
    LcdClrDisp();	
    LcdMoveCursor(1,1);
    
    FOREVER(){
        WaitForSlice();
        KeyTask();
        DispKeyTask();
    }
}
/*************************************************************************
* WaitForSlice() - Time slicer. Uses OCDelay module for a time slice
*                  period of SLICE_PER.
* Modules: OCDelay
* Member: GetmSCnt()
*************************************************************************/
void WaitForSlice(void){

    static INT16U LastTime;
    static INT8U TSInit = TRUE;

    if(TSInit){   /* Initialize LastTime first time through */
        LastTime = GetmSCnt();
        TSInit = FALSE;
    }else{        /* wait for next time slice */
        while((GetmSCnt() - LastTime) < SLICE_PER){} 
        LastTime += SLICE_PER;           /* set up for next time slice */
    }
}
/*************************************************************************/

void DispKeyTask(void){
    INT8U key;
    static INT8U currentcol = 1;
    static INT8U currentlin = 1;
    
    /* Control cursor */
    if((currentcol > 16) && (currentlin == 1)){
        currentcol = 1;
        currentlin = 2;
        LcdClrLine(2);
    }else if ((currentcol > 16) && (currentlin == 2)){
        currentcol = 1;
        currentlin = 1;
        LcdClrLine(1);
    }else{} /* Do nothing */
    
    /* Check for keypress and display */
    key = GetKey();
    switch(key){
    case 0:
        break;
    case DC1: /* Define action for DC1 key */
        LcdDispChar('A');
        currentcol++;
        break;
    case DC2: /* Define action for DC2 key */
        LcdDispChar('B');
        currentcol++;
        break;
    case DC3: /* Define action for DC3 key */
        LcdDispChar('C');
        currentcol++;
        break;
    case DC4: /* Define action for DC4 key */
        LcdDispChar('D');
        currentcol++;
        break;
    default:
        LcdDispChar(key);
        currentcol++;
    }
}
/*******************************************************************/


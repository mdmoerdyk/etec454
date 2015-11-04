/********************
Mark Moerdyk
First modification: 2/14/13
Last modification: 2/22/13
**************************/

#include "includes.h"

#define DC1 (INT8U)0x11
#define DC2 (INT8U)0x12
#define DC3 (INT8U)0x13
#define DC4 (INT8U)0x14

/*************************************************************************
* Public Event Definitions
*************************************************************************/

/*************************************************************************
* Task Function Prototypes. 
*   - Private if in the same module as startup task. Otherwise public.
*************************************************************************/
static void StartTask(void *p_arg);
static void UITask(void *p_arg);
static void TimeDispTask(void *p_arg);
void SetTheTime(void);

/*************************************************************************
* Allocate task stack space.
*************************************************************************/
OS_STK  StartTaskStk[STARTTASK_STK_SIZE]; 
OS_STK  UITaskStk[UITASK_STK_SIZE];
OS_STK  TimeDispTaskStk[TIMEDISPTASK_STK_SIZE];
/************************************************
*Global Variables
*************************************************/

 typedef enum {INITIALSETUP, TENHRPLACE, ONEHRPLACE, TENMINPLACE, ONEMINPLACE,
               TENSECPLACE, ONESECPLACE, VALUEGOESTHROUGH, BACKTOORIGINAL
              }CLKSTATES;
/*************************************************************************
* main()
Includes: Initialize OS, Key, and LCD
Creates start task
*************************************************************************/
void main(void) 
{
    DBUG_PORT = 0x00;      //Initialize Debug bits
    DBUG_PORT_DIR = DB_OUTS;
    
    OSInit();                       /* Initialize uC/OS-II              */
    KeyInit();
    LcdInit(TRUE,TRUE,FALSE);
    
    (void)OSTaskCreate(StartTask,         /* Create Startup Task        */
                (void *)0,
                (void *)&StartTaskStk[STARTTASK_STK_SIZE],
                STARTTASK_PRIO);
    
    OSStart();                      /* Start multitasking               */
}

/*************************************************************************
* STARTUP TASK - Prints out checksum and waits for c press. When C is pressed,
* starts LCD and Demo Task, then deletes itself
* Functions included: CalcChkSum, LcdDispStrg, DisplayCheckSum
* Creates: LCDDemoTask and DemoCntrlTask
*************************************************************************/
static void StartTask(void *p_arg) 
{    
    (void)p_arg;                          /* Avoid compiler warning     */
    OSTickInit();
    LcdDispClear(1);	
    
    
    DBUG_PORT |= PP7;
    
    TimeInit();
    (void)OSTaskCreate(UITask,         /* Create UITask   */
                (void *)0,
                (void *)&UITaskStk[UITASK_STK_SIZE],
                UITASK_PRIO);
    (void)OSTaskCreate(TimeDispTask,         /* Create TimeDispTask  */
                (void *)0,
                (void *)&TimeDispTaskStk[TIMEDISPTASK_STK_SIZE],
                TIMEDISPTASK_PRIO);
    
    DBUG_PORT &= ~PP7;
    (void)OSTaskDel(STARTTASK_PRIO);
    FOREVER()
    {
      //do nothing
    }
}
/*****************************************************************
*UITask - Task that waits for a keypress. If the # key is press, then jumps to
SetTheTime function. Else, waits for the # press*/
static void UITask(void *p_arg)
{
    INT8U keypress = 0;
    INT8U key=0;
    INT8U err;
    
    (void)p_arg;
    FOREVER()
    {
        DBUG_PORT &= ~PP6;
        keypress = KeyPend(key, &err);
        DBUG_PORT |= PP6;
        if(keypress == '#')
        {
             DBUG_PORT |= PP6;
            SetTheTime();
            DBUG_PORT &= ~PP6;
        }
        else
        {   
        }

    }
}
/****************************************************
TimeDispTask - Takes the value of TimeOfClock, and displays it on the LCD
Functions: TimeGet, LCD
*****************************************************/
static void TimeDispTask(void *p_arg)
{
    TIME displaytime;
    (void)p_arg;
    
    FOREVER()
    {
        DBUG_PORT |= PP4;
        TimeGet(&displaytime);
        LcdDispTime(1,9,CLOCK_LAYER,displaytime.hr,displaytime.min,displaytime.sec);
        DBUG_PORT &= ~PP4;
          
    }
}
/****************************************************************
SetTheTime - Goes through each of the six different places that can be 
programmed for time, and when done, sets the time of the programmed value 
equal to the TimeOfDay time. If C is pressed, nothing happens.
Functions: TimeSet(), OSTaskSuspend(), OSTaskResume()
*****************************************************************/
void SetTheTime(void)
{ 
    LcdDispChar(1,1,CLOCK_LAYER,'4');    /*
    TIME changetime;
    CLKSTATES curstate = INITIALSETUP;
    INT8U err;
    INT8U keypress = 0;
    INT8U key;
    INT8U hrtenval = 0x00;
    INT8U hroneval = 0x00;
    INT8U mintenval = 0x00;
    INT8U minoneval = 0x00;
    INT8U sectenval = 0x00;
    INT8U seconeval = 0x00;
    INT8U finishset =0x00;
    INT8U tenhrset = FALSE;
    INT8U onehrset = FALSE;
    INT8U tenminset = FALSE;
    INT8U oneminset = FALSE;
    INT8U tensecset = FALSE;
    INT8U onesecset = FALSE;
    INT8U remainder;
    INT8U hourset = FALSE;
    INT8U onepressed = FALSE;
    INT8U zeropressed = FALSE;
    
    OSTaskSuspend(TIMEDISPTASK_PRIO);
    TimeGet(&changetime);
    LcdMoveCursor(1,5);
    
    keypress = KeyPend(key, &err);
    
    while((finishset != 0x01) && (finishset != 0x02))
    {
      
        switch(curstate)
        {
            case(INITIALSETUP):
           
                LcdMoveCursor(1,5);
                LcdCursor(TRUE,TRUE);
                curstate = TENHRPLACE;
                break;
           
            case(TENHRPLACE):
           
                keypress = KeyPend(0, &err);
                while((keypress != '1') && (keypress != '0')&& (keypress != DC3)
                      && (keypress != DC1))
                {
                    keypress = KeyPend(0, &err);              
                }//do nothing
                if(keypress == '1')
                {
                    if ((changetime.hr <=0x09)&& (changetime.hr > 0x02) 
                       && (hourset == FALSE))
                    {
                        curstate = TENHRPLACE;
                    }
                    else if(hroneval <= 0x02)
                    {
                        LcdDispChar('1');
                        hrtenval = 0x0A;
                        tenhrset = TRUE;
                        hourset = TRUE;
                        onepressed = TRUE;
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        curstate = TENHRPLACE;
                    }
                }
                else if(keypress == DC1)
                {
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == DC3)
                {
                    curstate = BACKTOORIGINAL;
                }

                else
                {
                    if ((hroneval == 0x00) && (onehrset == TRUE))
                    {
                        curstate = TENHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('0');
                        hrtenval = 0x00;
                        tenhrset = TRUE;
                        hourset = TRUE;
                        zeropressed = TRUE; 
                        curstate = ONEHRPLACE;
                    }
                }
                break;
           
            case(ONEHRPLACE):
           
                LcdMoveCursor(1,6);
                keypress = KeyPend(0, &err);
                while((keypress != '0') && (keypress != '1') && (keypress != '2') 
                     && (keypress != '3') && (keypress != '4') && (keypress != '5')
                     && (keypress != '6') && (keypress != '7')&& (keypress != '8')
                     && (keypress != '9')&& (keypress != DC2)&& (keypress != DC3)
                     && (keypress != DC1))
                {
                    keypress = KeyPend(0, &err);
                }
                if(keypress == '0')
                {
                    if (hrtenval == 0x00)
                    {
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('0');
                        hroneval = 0x00;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 
                }
                else if(keypress == '1')
                {
                    LcdDispChar('1');
                    hroneval = 0x01;
                    onehrset = TRUE;
                    curstate = TENMINPLACE;
                }
                else if(keypress == '2')
                {
                    LcdDispChar('2');
                    hroneval = 0x02;
                    onehrset = TRUE;
                    curstate = TENMINPLACE;
                }
                else if (keypress == '3')
                {
                    if (hrtenval == 0x0A)
                    {
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('3');
                        hroneval = 0x03;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 

                }
                else if (keypress == '4')
                {
                    if (hrtenval == 0x0A)
                    {
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('4');
                        hroneval = 0x04;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 

                }
                else if (keypress == '5')
                {
                    if (hrtenval == 0x0A)
                    {
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('5');
                        hroneval = 0x05;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 

                }
                else if (keypress == '6')
                {
                    if (hrtenval == 0x0A)
                    {
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('6');
                        hroneval = 0x06;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 

                }
                else if (keypress == '7')
                {
                    if (hrtenval == 0x0A)
                    {
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('7');
                        hroneval = 0x07;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 

                }
                else if (keypress == '8')
                {
                    if (hrtenval == 0x0A)
                    {
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('8');
                        hroneval = 0x08;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 

                }
                else if (keypress == '9')
                {
                    if (hrtenval == 0x0A)
                    {
                        curstate = ONEHRPLACE;
                    }
                    else
                    {
                        LcdDispChar('9');
                        hroneval = 0x09;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 

                }

                else if(keypress == DC1)
                {
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == DC3)
                {
                    curstate = BACKTOORIGINAL;
                }
                else
                {
                    LcdBSpace();
                    curstate = TENHRPLACE;
                }
                OSTimeDly(100);
                break;
           
            case(TENMINPLACE):
           
                LcdMoveCursor(1,8);
                keypress = KeyPend(0, &err);
                while((keypress != '0') && (keypress != '1') && (keypress != '2')
                     && (keypress != '3') && (keypress != '4') && (keypress != '5')
                     && (keypress != DC2 )&& (keypress != DC3)
                      && (keypress != DC1))
                {
                    keypress = KeyPend(0, &err);    
                }
                if(keypress == '0')
                {
                    LcdDispChar('0');
                    mintenval = 0x00;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE; 
                }
                else if(keypress == '1')
                {
                    LcdDispChar('1');
                    mintenval = 0x0A;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == '2')
                {
                    LcdDispChar('2');
                    mintenval = 0x14;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == '3')
                {
                    LcdDispChar('3');
                    mintenval = 0x1E;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == '4')
                {
                    LcdDispChar('4');
                    mintenval = 0x28;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == '5')
                {
                    LcdDispChar('5');
                    mintenval = 0x32;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == DC1)
                {
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == DC3)
                {
                    curstate = BACKTOORIGINAL;
                }
                else
                {
                    curstate = ONEHRPLACE;
                }             
                break;
           
            case(ONEMINPLACE):
           
                LcdMoveCursor(1,9);
                keypress = KeyPend(0, &err);
                while((keypress != '0') && (keypress != '1') && (keypress != '2')
                     && (keypress != '3') && (keypress != '4') && (keypress != '5')
                     && (keypress != '6') && (keypress != '7')&& (keypress != '8')
                     && (keypress != '9')&& (keypress != DC2 )&& (keypress != DC3)
                     && (keypress != DC1))
                {
                    keypress = KeyPend(0, &err);   
                }
                if(keypress == '0')
                {
                    LcdDispChar('0');
                    minoneval = 0x00;
                    oneminset = TRUE;
                    curstate = TENSECPLACE; 
                }
                else if(keypress == '1')
                {
                    LcdDispChar('1');
                    minoneval = 0x01;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '2')
                {
                    LcdDispChar('2');
                    minoneval = 0x02;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '3')
                {
                    LcdDispChar('3');
                    minoneval = 0x03;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '4')
                {
                    LcdDispChar('4');
                    minoneval = 0x04;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '5')
                {
                    LcdDispChar('5');
                    minoneval = 0x05;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '6')
                {
                    LcdDispChar('6');
                    minoneval = 0x06;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '7')
                {
                    LcdDispChar('7');
                    minoneval = 0x07;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '8')
                {
                    LcdDispChar('8');
                    minoneval = 0x08;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '9')
                {
                    LcdDispChar('9');
                    minoneval = 0x09;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == DC1)
                {
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == DC3)
                {
                    curstate = BACKTOORIGINAL;
                }
                else
                {
                    curstate = TENMINPLACE;
                }
                break;
           
            case(TENSECPLACE):
           
                LcdMoveCursor(1,11);
                keypress = KeyPend(0, &err);
                while((keypress != '0') && (keypress != '1') && (keypress != '2')
                     && (keypress != '3') && (keypress != '4') && (keypress != '5')
                     && (keypress != DC2 )&& (keypress != DC3)
                      && (keypress != DC1))
                {
                    keypress = KeyPend(0, &err);  
                }
                if(keypress == '0')
                {
                    LcdDispChar('0');
                    sectenval = 0x00;
                    tensecset = TRUE;
                    curstate = ONESECPLACE; 
                }
                else if(keypress == '1')
                {
                    LcdDispChar('1');
                    sectenval = 0x0A;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == '2')
                {
                    LcdDispChar('2');
                    sectenval = 0x14;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == '3')
                {
                    LcdDispChar('3');
                    sectenval = 0x1E;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == '4')
                {
                    LcdDispChar('4');
                    sectenval = 0x28;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == '5')
                {
                    LcdDispChar('5');
                    sectenval = 0x32;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == DC1)
                {
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == DC3)
                {
                    curstate = BACKTOORIGINAL;
                }
                else
                {
                    curstate = ONEMINPLACE;
                }              
                break;
             
            case(ONESECPLACE):
           
                LcdMoveCursor(1,12);
                keypress = KeyPend(0, &err);
                while((keypress != '0') && (keypress != '1') && (keypress != '2')
                     && (keypress != '3') && (keypress != '4') && (keypress != '5')
                     && (keypress != '6') && (keypress != '7')&& (keypress != '8')
                     && (keypress != '9')&& (keypress != DC2 )&& (keypress != DC3)
                      && (keypress != DC1))
                {
                    keypress = KeyPend(0, &err);
                    
                }
                if(keypress == '0')
                {
                    LcdDispChar('0');
                    seconeval = 0x00;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH; 
                }
                else if(keypress == '1')
                {
                    LcdDispChar('1');
                    seconeval = 0x01;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '2')
                {
                    LcdDispChar('2');
                    seconeval = 0x02;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '3')
                {
                    LcdDispChar('3');
                    onesecset = TRUE;
                    seconeval = 0x03;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '4')
                {
                    LcdDispChar('4');
                    seconeval = 0x04;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '5')
                {
                    LcdDispChar('5');
                    seconeval = 0x05;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '6')
                {
                    LcdDispChar('6');
                    seconeval = 0x06;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '7')
                {
                    LcdDispChar('7');
                    seconeval = 0x07;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '8')
                {
                    LcdDispChar('8');
                    seconeval = 0x08;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '9')
                {
                    LcdDispChar('9');
                    seconeval = 0x09;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == DC1)
                {
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == DC3)
                {
                    curstate = BACKTOORIGINAL;
                }
                else
                {
                    curstate = TENSECPLACE;
                }
                break;
           
            case(VALUEGOESTHROUGH):
           
                finishset = 0x01;
                break;
            
            case(BACKTOORIGINAL):
                finishset = 0x02;
                break;
               
            default:
            break;          
        }
        
    }
    //sends the value and puts it in TimeSet() 
    if(finishset == 0x01)
    {
        if(onesecset == TRUE)
        {
            changetime.hr = hrtenval + hroneval;
            changetime.min = mintenval + minoneval;
            changetime.sec = sectenval + seconeval;  
        }
        else if((tensecset == TRUE) && (onesecset == FALSE))
        {
            changetime.hr = hrtenval + hroneval;
            changetime.min = mintenval + minoneval;
            if( changetime.sec >= 0x32)
            {
                remainder = changetime.sec - 0x32;
            }
            else if(changetime.sec >= 0x28)
            {
                remainder = changetime.sec - 0x28;
            }
            else if(changetime.sec >= 0x1E)
            {
                remainder = changetime.sec - 0x1E;
            }
            else if(changetime.sec >=0x14)
            {
                remainder = changetime.sec - 0x14;
            }
            else if(changetime.sec >= 0x0A)
            {
                remainder = changetime.sec - 0x0A;
            }
            else
            {
                remainder = changetime.sec;
            }
            changetime.sec = sectenval + remainder;
        }
        else if((oneminset == TRUE) && (tensecset == FALSE))
        {
            changetime.hr = hrtenval + hroneval;
            changetime.min = mintenval + minoneval;
        }
        else if((tenminset == TRUE) && (oneminset == FALSE))
        {
            changetime.hr = hrtenval + hroneval;
            if( changetime.min >= 0x32)
            {
                remainder = changetime.min - 0x32;
            }
            else if(changetime.min >= 0x28)
            {
                remainder = changetime.min - 0x28;
            }
            else if(changetime.min >= 0x1E)
            {
                remainder = changetime.min - 0x1E;
            }
            else if(changetime.min >=0x14)
            {
                remainder = changetime.min - 0x14;
            }
            else if(changetime.min >= 0x0A)
            {
                remainder = changetime.min - 0x0A;
            }
            else
            {
                remainder = changetime.min;
            }
            changetime.min = mintenval+ remainder;
          
        }
        else if((onehrset == TRUE) && (tenminset == FALSE))
        {
            changetime.hr = hrtenval + hroneval;
        }
        else if((tenhrset == TRUE) && (onehrset == FALSE) && (zeropressed == TRUE))
        {
            remainder = changetime.hr - 0x0A; 
            changetime.hr = remainder + hrtenval;
        }
        else if((tenhrset == TRUE) && (onehrset == FALSE) && (onepressed == TRUE))
        {
            changetime.hr = changetime.hr + hrtenval;
        }
        else
        {
        }
        TimeSet(&changetime);
    }
    else
    {
    }//nothing
    LcdMoveCursor(1,5);
    LcdCursor(FALSE,FALSE);
    OSTaskResume(TIMEDISPTASK_PRIO);
    */
} 
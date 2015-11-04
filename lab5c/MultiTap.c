/********************
Mark Moerdyk
First modification: 3/09/13
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


/*************************************************************************
* Allocate task stack space.
*************************************************************************/
OS_STK  StartTaskStk[STARTTASK_STK_SIZE]; 
OS_STK  UITaskStk[UITASK_STK_SIZE];
OS_STK  TimeDispTaskStk[TIMEDISPTASK_STK_SIZE];
/************************************************
*Global Variables
*************************************************/

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
    LcdInit();
    
    KeyInit();
    
    
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
    //OSTickInit();
   	
    
    DBUG_PORT |= PP7;
    
     //LcdInit(TRUE,TRUE,FALSE);
     TimeInit();
    (void)OSTaskCreate(UITask,         /* Create UITask   */
                (void *)0,
                (void *)&UITaskStk[UITASK_STK_SIZE],
                UITASK_PRIO);
    (void)OSTaskCreate(TimeDispTask,         /* Create TimeDispTask  */
                (void *)0,
                (void *)&TimeDispTaskStk[TIMEDISPTASK_STK_SIZE],
                TIMEDISPTASK_PRIO);
     //LcdDispChar(1,1,CLOCK_LAYER,'2');
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
    INT8U key;
    INT8U err;
    
    (void)p_arg;
    FOREVER()
    {
        //LcdDispChar(1,2,CLOCK_LAYER,'6');
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
    //LcdDispChar(1,2,CLOCK_LAYER,'6');
    FOREVER()
    {
        DBUG_PORT |= PP4;
        TimeGet(&displaytime);
        //LcdDispTime(1,9,CLOCK_LAYER,displaytime.hr,displaytime.min,displaytime.sec);
        LcdDispTime(displaytime.hr, displaytime.min,displaytime.sec);
        DBUG_PORT &= ~PP4;
          
    }
}




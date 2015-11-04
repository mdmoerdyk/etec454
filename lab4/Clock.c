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

OS_EVENT *SecFlag;
/*************************************************************************
* Task Function Prototypes. 
*   - Private if in the same module as startup task. Otherwise public.
*************************************************************************/
static void ClockTask(void *p_arg);
void TimeGet(TIME *ltime);
void TimeSet(TIME *ltime);
void TimeInit(void);
void ClockTimerFnct(void *ptmr, void *callback_arg);

/*************************************************************************
* Allocate task stack space.
*************************************************************************/
OS_EVENT *ClockMutexKey;
OS_TMR *ClockTimer;
OS_STK  ClockTaskStk[CLOCKTASK_STK_SIZE]; 
/************************************************
*Global Variables
*************************************************/

static TIME TimeOfDay;

/**************************************************
*Clocktask - counts the number of clock cycles like a 12 hour clock
* uses a secflag in order to count a 1 second cycle
Initialize: TimeOfDay
*******************************************************/
static void ClockTask(void *p_arg)
{
    INT8U error;
    INT8U key;
    INT8U keypress;
    INT8U err;
    (void)p_arg;                               
    FOREVER()
    {
        DBUG_PORT &= ~PP3;
        OSSemPend(SecFlag, 0 ,&err);
        DBUG_PORT |= PP3;
        TimeOfDay.sec = TimeOfDay.sec + 1;
      
        if(TimeOfDay.sec > 0x3B && TimeOfDay.min < 0x3B && TimeOfDay.hr < 0x0D)
        {
            TimeOfDay.sec = 0x00;
            TimeOfDay.min++;
        }
        else if(TimeOfDay.sec > 0x3B && TimeOfDay.min > 0x3B && TimeOfDay.hr < 0x0D)
        {
            TimeOfDay.sec = 0x00;
            TimeOfDay.min = 0x00;
            TimeOfDay.hr++;
        }
        else if (TimeOfDay.sec > 0x3B && TimeOfDay.min == 0x3B 
                 && TimeOfDay.hr == 0x0C)
        {
            TimeOfDay.sec = 0x00;
            TimeOfDay.min = 0x00;
            TimeOfDay.hr = 0x01;
        }
        else
        {
        }        
    }
}
/**********************************************************
TimeInit - function that initializes timer, mutex, and clock task.
* sets values to timeOfDay if reset button is hit
Creates: ClockTimer, ClockMutexKey, SecFlag
*********************************************************8*/
void TimeInit(void)
{ 
    
   
    INT8U err;
    TimeOfDay.hr = 0x0C;
    TimeOfDay.min = 0x00;
    TimeOfDay.sec = 0x00;
    LcdDispTime(TimeOfDay.hr,TimeOfDay.min,TimeOfDay.sec);
    
    ClockTimer = OSTmrCreate(0,
                             10,
                             OS_TMR_OPT_PERIODIC,
                             ClockTimerFnct,
                             (void *)0,
                             "Clock Timer ",
                             &err);
    OSTmrStart(ClockTimer, &err);
    
    ClockMutexKey = (CLOCK_PIP, &err);
    
    
    (void)OSTaskCreate(ClockTask,           
                (void *)0,
                (void *)&ClockTaskStk[CLOCKTASK_STK_SIZE],
                CLOCKTASK_PRIO); 
    
    SecFlag = OSSemCreate(0);
}
 /******************************************************
 TimeSet - takes the programmed time of the clock, and 
 * sets it to the TimeofDay
 Passes in: ltime
 Passes out: nothing
 *******************************************************/
void TimeSet(TIME *ltime)
{
    INT8U err;
    OSMutexPend(ClockMutexKey,0,&err);
    TimeOfDay = *ltime;
    OSMutexPost(ClockMutexKey);
}
/******************************************************
TimeGet- sets the value of TimeofDay to ltime to be displyed
on the LCD
Passes in: nothing
Passes out : TimeOfDay
*******************************************************/
void TimeGet(TIME *ltime)
{
    INT8U err;
    OSMutexPend(ClockMutexKey,0,&err);
    *ltime = TimeOfDay ;
    OSMutexPost(ClockMutexKey);
}
/***************************************************
* ClockTimerFnct - Gets called from the OSTmrCreate to 
* post the SecFlag created in the TimeInit function
*****************************************************/
void ClockTimerFnct(void *ptmr, void *callback_arg)
{
    OSSemPost(SecFlag);  
} 
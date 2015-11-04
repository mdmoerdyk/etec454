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
static void TransmitTask(void *p_arg);
static void ReceiveTask(void *p_arg);
void sci_open(void);
void sci_open_int(void);
void sci_write(INT8U character);
INT8U sci_read(void);
/*************************************************************************
* Allocate task stack space.
*************************************************************************/
OS_STK  StartTaskStk[STARTTASK_STK_SIZE]; 
OS_STK  UITaskStk[UITASK_STK_SIZE];
OS_STK  TimeDispTaskStk[TIMEDISPTASK_STK_SIZE];
OS_STK  TransmitTaskStk[TRANSMITTASK_STK_SIZE];
OS_STK  ReceiveTaskStk[RECEIVETASK_STK_SIZE];
/************************************************
*Global Variables
*************************************************/
OS_EVENT *TransmitFlag;
OS_EVENT *ReceiveFlag;
INT8U ReceivedString[25];
INT8U ReceiveVar;
INT8U AddNum1;
INT8U AddNum2;
INT8U AddNum3;
INT8U InTran = FALSE;
/*************************************************************************
* main()
Includes: Initialize OS, Key, and LCD
Creates start task
*************************************************************************/
void main(void) 
{
    DBUG_PORT = 0x00;      //Initialize Debug bits
    DBUG_PORT_DIR = DB_OUTS;
    
    OSInit();
  
    TransmitFlag = OSSemCreate(0);
    ReceiveFlag = OSSemCreate(0);
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
   	    
    DBUG_PORT |= PP7;
           
    KeyInit();     
    TimeInit();                          
    LcdInit(TRUE,TRUE,FALSE);
    SetTheTime();//for reset purposes
    (void)OSTaskCreate(UITask,         /* Create UITask   */
                (void *)0,
                (void *)&UITaskStk[UITASK_STK_SIZE],
                UITASK_PRIO);
    (void)OSTaskCreate(TimeDispTask,         /* Create TimeDispTask  */
                (void *)0,
                (void *)&TimeDispTaskStk[TIMEDISPTASK_STK_SIZE],
                TIMEDISPTASK_PRIO);
    (void)OSTaskCreate(TransmitTask,         /* Create UITask   */
                (void *)0,
                (void *)&TransmitTaskStk[TRANSMITTASK_STK_SIZE],
                TRANSMIT_PRIO);
    (void)OSTaskCreate(ReceiveTask,         /* Create UITask   */
                (void *)0,
                (void *)&ReceiveTaskStk[RECEIVETASK_STK_SIZE],
               RECEIVE_PRIO);
         
    DBUG_PORT &= ~PP7;
    (void)OSTaskDel(STARTTASK_PRIO);
    
    FOREVER()
    {
    }
}
/*****************************************************************
*UITask - Task that waits for a keypress. If the # key is press, then jumps
to SetTheTime function. Else, waits for the # press*/
static void UITask(void *p_arg)
{
    INT8U keypress = 0;
    INT8U err;
    INT8U sendmessage;
    INT8U dbutton = TRUE;
    TIME displaytime;
    
    (void)p_arg;
    FOREVER()
    {
        DBUG_PORT &= ~PP6;
        keypress = KeyPend(0, &err);
        DBUG_PORT |= PP6;
                
        if(keypress == '#')
        {
            DBUG_PORT |= PP6;
            SetTheTime();
            DBUG_PORT &= ~PP6;
        }
        else if (keypress == DC1)
        {
            DBUG_PORT |= PP6;
            InTran = TRUE; //if message received throw up symbol 
            LcdShowLayer(CLOCK_LAYER);
            LcdHideLayer(DBUTTON_LAYER);
            LcdHideLayer(DISPLAY_LAYER);
            TypeText();
            InTran = FALSE;//out of transmit phase
            LcdShowLayer(DISPLAY_LAYER);
            TransmitCheck(&sendmessage);
            if(sendmessage == TRUE)
            {
                OSSemPost(TransmitFlag);//Allows transmit to start
            }
            else
            {
            }
            sendmessage = FALSE;
            DBUG_PORT &= ~PP6;   
        }
        else if (keypress == DC4)
        {
            DBUG_PORT |= PP6;
            if(dbutton == TRUE)
            {
            
                LcdShowLayer(DBUTTON_LAYER);
                LcdHideLayer(CLOCK_LAYER);
                LcdDispChar(1,1,DBUTTON_LAYER,AddNum1);
                LcdDispChar(1,2,DBUTTON_LAYER,AddNum2);
                LcdDispChar(1,3,DBUTTON_LAYER,AddNum3);
                DispTimeStamp();//displays when message was received
                dbutton = FALSE; 
            }
            else
            {
                dbutton = TRUE;
                LcdHideLayer(DBUTTON_LAYER);
                LcdShowLayer(CLOCK_LAYER);
            }
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
        OSTimeDly(100);
        TimeGet(&displaytime);
        LcdDispTime(1,9,CLOCK_LAYER,displaytime.hr,displaytime.min,
                    displaytime.sec);
        DBUG_PORT &= ~PP4;
          
    }
}
/********************************************
TransmitTask - pends on the Transmit flag, when flag is posted
transmitts message typed.
uses: sci_write, sci_open, MessageCheckSum();
*********************************************/
static void TransmitTask(void *p_arg)
{
    INT8U err;
    INT8U var = 'M';
    INT8U sourcea = '1';
    INT8U sourceb = '1';
    INT8U sourcec = '7';
    INT8U send_message[16];
    INT8U counter = 0x00;
    INT8U checksum[2];
    (void)p_arg;
    sci_open();

    FOREVER()
    {
        OSSemPend(TransmitFlag, 0, &err);
        DBUG_PORT |= PP1;
        sci_write(var);
        sci_write(sourcea);
        sci_write(sourceb);
        sci_write(sourcec);
        GetMessage(&send_message);
        while(counter != 0x10)
        {
            sci_write(send_message[counter]);        
            counter++;
        }
        counter = 0x00;
        MessageCheckSum(&checksum,sourcea,sourceb,sourcec);
        sci_write(checksum[0]);
        sci_write(checksum[1]);
        DBUG_PORT &= ~PP1;
        
    }
      
}
/********************************************
 ReceiveTask- pends on the interupt service routine, when condition
 is filled, fills up array, then checks if message is good. If good,
 post message, else CS ERROR
 Uses:sci_open_int,sci_read,ReceivedCheckSum
*********************************************/
static void ReceiveTask(void *p_arg)
{
    TIME get_time;
    INT8U input=0;
    INT8U err;
    INT8U transfer = 0x00;
    INT8U taken_message[16];
    INT8U sent_checksum[2];
    INT8U real_checksum[2];
    INT8U fill_array = FALSE;
    
    (void)p_arg;
    sci_open_int();

    FOREVER()
    {     
        while(input != 22)
        {
            OSSemPend(ReceiveFlag, 0, &err);
            if(ReceiveVar == 'M')//only m
            {
                fill_array = TRUE;
            }
            else{}
            if(fill_array == TRUE)
            {
                ReceivedString[input] = ReceiveVar;
                input++;
            }
            else{}
            
        }
        DBUG_PORT |= PP0;
        fill_array = FALSE;
        LcdDispClear(DISPLAY_LAYER);
        if (InTran == TRUE)
        {
            LcdDispChar(1,1,MESSAGE_LAYER,'!');
        }
        else{}
        input = 0x04;
        AddNum1 = ReceivedString[1];
        AddNum2 = ReceivedString[2];
        AddNum3 = ReceivedString[3];
       
        while(transfer != 16)
        {
            taken_message[transfer] = ReceivedString[input];
            transfer++;
            input++;
        }
        transfer = 0x00;
        sent_checksum[0] = ReceivedString[20];
        sent_checksum[1] = ReceivedString[21];
        ReceivedCheckSum(&real_checksum,&taken_message,AddNum1,AddNum2,
                         AddNum3);
        if((real_checksum[1] == sent_checksum[1]) && 
          (real_checksum[0] == sent_checksum[0]))
        {
            while(transfer != 16)
            {
                if(taken_message[transfer] != 0)
                {
                    LcdDispChar(2,(transfer + 1),DISPLAY_LAYER,
                                taken_message[transfer]);
                
                }
                else
                {
                    LcdDispChar(2,(transfer + 1),DISPLAY_LAYER,' ');
                }
                transfer++;
            }   
        }
        else
        {
            LcdDispChar(2,1,DISPLAY_LAYER,'C');
            LcdDispChar(2,2,DISPLAY_LAYER,'S');
            LcdDispChar(2,3,DISPLAY_LAYER,' ');
            LcdDispChar(2,4,DISPLAY_LAYER,'E');
            LcdDispChar(2,5,DISPLAY_LAYER,'r');
            LcdDispChar(2,6,DISPLAY_LAYER,'r');
            LcdDispChar(2,7,DISPLAY_LAYER,'o');
            LcdDispChar(2,8,DISPLAY_LAYER,'r');
        }
        transfer = 0x00;
        input = 0x00;
        GetReceiveTime();
        DBUG_PORT &= ~PP0;        
    }
}
//interupt waits for a read from the port
ISR OCOIsr(void)
{
    OS_ISR_ENTER();
    ReceiveVar = sci_read();
    OSSemPost(ReceiveFlag);
    OSIntExit();  
}
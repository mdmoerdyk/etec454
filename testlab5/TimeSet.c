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


void SetTheTime(void);



typedef enum {INITIALSETUP, TENHRPLACE, ONEHRPLACE, TENMINPLACE, ONEMINPLACE,
               TENSECPLACE, ONESECPLACE, VALUEGOESTHROUGH, BACKTOORIGINAL
             }CLKSTATES;
             
/****************************************************************
SetTheTime - Goes through each of the six different places that can be 
programmed for time, and when done, sets the time of the programmed value 
equal to the TimeOfDay time. If C is pressed, nothing happens.
Functions: TimeSet(), OSTaskSuspend(), OSTaskResume()
*****************************************************************/
void SetTheTime(void)
{
    /*TIME changetime;
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
    LcdDispTime(1,5,2,changetime.hr,changetime.min,changetime.sec);
    
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
                        LcdDispChar(1,5,2,'1');
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
                        LcdDispChar(1,5,2,'0');
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
                        LcdDispChar(1,6,2,'0');
                        hroneval = 0x00;
                        onehrset = TRUE;
                        curstate = TENMINPLACE;
                    } 
                }
                else if(keypress == '1')
                {
                    LcdDispChar(1,6,2,'1');
                    hroneval = 0x01;
                    onehrset = TRUE;
                    curstate = TENMINPLACE;
                }
                else if(keypress == '2')
                {
                    LcdDispChar(1,6,2,'2');
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
                        LcdDispChar(1,6,2,'3');
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
                        LcdDispChar(1,6,2,'4');
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
                        LcdDispChar(1,6,2,'5');
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
                        LcdDispChar(1,6,2,'6');
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
                        LcdDispChar(1,6,2,'7');
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
                        LcdDispChar(1,6,2,'8');
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
                        LcdDispChar(1,6,2,'9');
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
                    LcdDispChar(1,8,2,'0');
                    mintenval = 0x00;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE; 
                }
                else if(keypress == '1')
                {
                    LcdDispChar(1,8,2,'1');
                    mintenval = 0x0A;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == '2')
                {
                    LcdDispChar(1,8,2,'2');
                    mintenval = 0x14;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == '3')
                {
                    LcdDispChar(1,8,2,'3');
                    mintenval = 0x1E;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == '4')
                {
                    LcdDispChar(1,8,2,'4');
                    mintenval = 0x28;
                    tenminset = TRUE;
                    curstate = ONEMINPLACE;
                }
                else if(keypress == '5')
                {
                    LcdDispChar(1,8,2,'5');
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
                    LcdDispChar(1,9,2,'0');
                    minoneval = 0x00;
                    oneminset = TRUE;
                    curstate = TENSECPLACE; 
                }
                else if(keypress == '1')
                {
                    LcdDispChar(1,9,2,'1');
                    minoneval = 0x01;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '2')
                {
                    LcdDispChar(1,9,2,'2');
                    minoneval = 0x02;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '3')
                {
                    LcdDispChar(1,9,2,'3');
                    minoneval = 0x03;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '4')
                {
                    LcdDispChar(1,9,2,'4');
                    minoneval = 0x04;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '5')
                {
                    LcdDispChar(1,9,2,'5');
                    minoneval = 0x05;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '6')
                {
                    LcdDispChar(1,9,2,'6');
                    minoneval = 0x06;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '7')
                {
                    LcdDispChar(1,9,2,'7');
                    minoneval = 0x07;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '8')
                {
                    LcdDispChar(1,9,2,'8');
                    minoneval = 0x08;
                    oneminset = TRUE;
                    curstate = TENSECPLACE;
                }
                else if(keypress == '9')
                {
                    LcdDispChar(1,9,2,'9');
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
                    LcdDispChar(1,11,1,'0');
                    sectenval = 0x00;
                    tensecset = TRUE;
                    curstate = ONESECPLACE; 
                }
                else if(keypress == '1')
                {
                    LcdDispChar(1,11,2,'1');
                    sectenval = 0x0A;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == '2')
                {
                    LcdDispChar(1,11,2,'2');
                    sectenval = 0x14;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == '3')
                {
                    LcdDispChar(1,11,2,'3');
                    sectenval = 0x1E;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == '4')
                {
                    LcdDispChar(1,11,2,'4');
                    sectenval = 0x28;
                    tensecset = TRUE;
                    curstate = ONESECPLACE;
                }
                else if(keypress == '5')
                {
                    LcdDispChar(1,11,2,'5');
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
                    LcdDispChar(1,12,2,'0');
                    seconeval = 0x00;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH; 
                }
                else if(keypress == '1')
                {
                    LcdDispChar(1,12,2,'1');
                    seconeval = 0x01;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '2')
                {
                    LcdDispChar(1,12,2,'2');
                    seconeval = 0x02;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '3')
                {
                    LcdDispChar(1,12,2,'3');
                    onesecset = TRUE;
                    seconeval = 0x03;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '4')
                {
                    LcdDispChar(1,12,2,'4');
                    seconeval = 0x04;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '5')
                {
                    LcdDispChar(1,12,2,'5');
                    seconeval = 0x05;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '6')
                {
                    LcdDispChar(1,12,2,'6');
                    seconeval = 0x06;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '7')
                {
                    LcdDispChar(1,12,2,'7');
                    seconeval = 0x07;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '8')
                {
                    LcdDispChar(1,12,2,'8');
                    seconeval = 0x08;
                    onesecset = TRUE;
                    curstate = VALUEGOESTHROUGH;
                }
                else if(keypress == '9')
                {
                    LcdDispChar(1,12,2,'9');
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


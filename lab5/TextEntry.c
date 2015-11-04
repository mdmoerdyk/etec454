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
void GetMessage(INT8U *message);
void MessageCheckSum(INT8U *message , INT8U sourcea, INT8U sourceb,
                     INT8U sourcec);
void TransmitCheck(INT8U *message);
void ReceivedCheckSum(INT8U *message, INT8U *rec_mess, INT8U sourcea, 
                      INT8U sourceb,INT8U sourcec);
void DispTimeStamp(void);
void GetReceiveTime(void);

typedef enum {WAITFORPRESS, WAITFORONESEC,ANOTHERBUTTONPRESSED, CLEARROW,
              BACKSPACE, PLACEVALUE, ENDTASK, IGNORECASE}TEXTSTATES;

INT8U Displayed_Characters[10][6] = {
    {'1','.',',',0x27,'?','!'},
    {'2','A','B','C',0,0},
    {'3','D','E','F',0,0},
    {'4','G','H','I',0,0},
    {'5','J','K','L',0,0},
    {'6','M','N','O',0,0},
    {'7','P','Q','R','S',0},
    {'8','T','U','V',0,0},
    {'9','W','X','Y','Z',0},
    {'0','_',0x7F,0,0,0}};
INT8U Final_Message[16];
INT8U Initial_Message[16];
TIME Message_Time;
INT8U Message_Status;
TIME Receive_Time;
/************************************************
TypeText- Lets user type up to 16 characters on LCD. When done
if A is pressed, sends message through, else, nothing
Uses Casestates for different sections
************************************************/
void TypeText(void)
{
    
    TEXTSTATES curstate = WAITFORPRESS;
    INT8U keypress = 0;
    INT8U err;
    INT8U charrow = 2;
    INT8U text_done = 0x02;
    INT8U charcol = 1; 
    INT8U char_value[TEXT_CHAR];
    INT8U first_value;
    INT8U second_value;
    INT8U cursor_val = 1;
    INT8U y_char = 0x00 ;
    INT8U y = 0x00;
    INT8U message_pos = 0x00;
        
    Message_Status = FALSE;   
    LcdCursor(TRUE,TRUE);
    while(message_pos != 0x10)
    {
        Initial_Message[message_pos] = 0;
        message_pos++;  
    } 
    message_pos = 0x00;
    
    while(text_done != 0x03)
    {
        switch(curstate)
        {
            case(WAITFORPRESS)://waits for a key press
            keypress = 0;
            LcdMoveCursor(2,cursor_val);
            while((keypress != '0') && (keypress != '1') && 
                    (keypress != '2') && (keypress != '3') 
                    && (keypress != '4') && (keypress != '5')
                    && (keypress != '6') && (keypress != '7')
                    && (keypress != '8')&& (keypress != '9')
                    && (keypress != DC2)&& (keypress != DC3)
                    && (keypress != DC1))
                {
                    keypress = KeyPend(0, &err);
                }
                first_value = keypress - 48;
                LcdDispChar(2,cursor_val,MESSAGE_LAYER,keypress);
                if( keypress == DC3)
                {
                    curstate = BACKSPACE;
                }
                else if(keypress == DC1)
                {
                    curstate = ENDTASK;
                }
                else
                {
                    curstate = WAITFORONESEC;
                    keypress = 0;
                }
                if(first_value == 0)
                {
                    second_value = 9; 
                }
                else
                {
                    second_value = first_value - 1;
                    
                }
                break;
/***********************************************************************/            
            case(WAITFORONESEC): // waits for press within one second
            LcdMoveCursor(2,cursor_val);
            keypress = KeyPend(1000,&err);
            if(err == OS_ERR_TIMEOUT)
            {       
                curstate = PLACEVALUE;  
            }
            else if(err == OS_ERR_NONE)
            {
                if(keypress == (first_value + 48)) 
                {
                    y = y + 1; //increments y axis of double array
                    if( y == 6)
                    {
                        y= 0;
                    }
                    else if(Displayed_Characters[second_value][y] == 0)
                    {
                        y= 0;
                    }
                    else{}
                    LcdDispChar(2,cursor_val,MESSAGE_LAYER, 
                                Displayed_Characters[second_value][y]);
                    curstate = WAITFORONESEC; 
                    }
                    else
                    {
                        curstate = ANOTHERBUTTONPRESSED;
                    } 

                }
                else
                {}
                break;
/**********************************************/
            case(ANOTHERBUTTONPRESSED): //another button w/ one second
            if(Displayed_Characters[second_value][y] == '_')
            {        
                Initial_Message[cursor_val-1] = ' ';
                LcdDispChar( 2,cursor_val, MESSAGE_LAYER, ' ');
                cursor_val++;
                if(cursor_val == 0x11)
                {
                     curstate = IGNORECASE;
                }
                else
                {
                    curstate = WAITFORONESEC;
                }
                LcdDispChar( 2,cursor_val, MESSAGE_LAYER, keypress);
                
                first_value = keypress - 48; // set it back to zero
                keypress = 0;
                if(first_value == 0)
                {
                    second_value = 9; 
                }
                else
                {
                    second_value = first_value - 1;    
                }
                y =0x00;

            }
            else if(Displayed_Characters[second_value][y] == 0x7F)//<- char
            {        
                curstate = CLEARROW;
            }
            else if(keypress == DC3)// if C is pressed
            {
                cursor_val++;
                curstate = BACKSPACE;
            }
            else if(keypress == DC1)//if A is pressed
            {
                Initial_Message[cursor_val-1] = 
                    Displayed_Characters[second_value][y];
                curstate = ENDTASK;
            }
            else
            {      
                Initial_Message[cursor_val-1] = 
                    Displayed_Characters[second_value][y];
                cursor_val++;
                if(cursor_val == 0x11)
                {
                    curstate = IGNORECASE; 
                }
                else
                {
                    curstate = WAITFORONESEC;
                }
                LcdDispChar( 2,cursor_val, MESSAGE_LAYER, keypress);
                
                first_value = keypress - 48; // set it back to zero
                keypress = 0;
                if(first_value == 0)
                {
                    second_value = 9; 
                }
                else
                {
                    second_value = first_value - 1;
                }
                y =0x00;
            }
            break;
/************************************************************************/               
            case(PLACEVALUE): //sets value to cursor position
            if(Displayed_Characters[second_value][y] == '_')
            {
                Initial_Message[cursor_val] = ' ';
                LcdDispChar(2,cursor_val,MESSAGE_LAYER, ' ');
                Initial_Message[cursor_val-1] = ' ';
                cursor_val++;
                if(cursor_val == 0x11)
                {
                    curstate = IGNORECASE; 
                }
                else
                {
                    curstate = WAITFORPRESS;
                }
                
            }
            else if(Displayed_Characters[second_value][y] == 0x7F)
            {        
                curstate = CLEARROW;                          
            }
            else
            {
                Initial_Message[cursor_val - 1] = 
                    Displayed_Characters[second_value][y];
                cursor_val++;
                if(cursor_val == 0x11)
                {
                    curstate = IGNORECASE; 
                }
                else
                {
                    curstate = WAITFORPRESS;
                }            }    
            break;
/*************************************************************************/
            case(CLEARROW):// clears all rows and starts back at zero
            while(cursor_val != 0x00)
            {
                Initial_Message[cursor_val-1] = 0;
                cursor_val--;  
            }
                first_value = 0x00;
                second_value = 0x00;
                cursor_val = 1;
                y_char = 0x00 ;
                y = 0x00;
                message_pos = 0x00;
                LcdDispClear(MESSAGE_LAYER);
                curstate = WAITFORPRESS;
            break; 
/***********************************************************************/
            case(IGNORECASE):// if all 16 spots are filled
            keypress = KeyPend(0, &err);
            if(keypress == DC1)
            {
                curstate = ENDTASK;
            }
            else if(keypress == DC3)
            {
                curstate = BACKSPACE;
            }
            else
            {
                curstate = IGNORECASE;
            }
            break;
/*************************************************************/            
            case(BACKSPACE)://clears spot and moves cursor back one
            if(cursor_val == 0x01)
            {
                text_done = 0x03;
            }
            else
            {
                Initial_Message[cursor_val-1] = 0;
                cursor_val--;
                LcdMoveCursor(2,cursor_val);
                curstate = WAITFORPRESS;
            }
            break;
/**********************************************************************/
            case(ENDTASK)://A is pressed and sends message to final value
            message_pos = 0x00;
            while(message_pos != 0x10)
            {
                Final_Message[message_pos] = Initial_Message[message_pos];
                message_pos++;  
            }
            text_done = 0x03;
            TimeGet(&Message_Time);
            message_pos = 0x00;
            Message_Status = TRUE;
            break;   
            
            default:
            break;   
        }    
    }
    
    LcdCursor(FALSE,FALSE);
    LcdDispClear(MESSAGE_LAYER);
}
 
/*********************************************************
GetMessage-takes message of TypedText and stores in into Transmitter 
message so it can be transmitted
********************************************************/ 
void GetMessage(INT8U *message)
{
    INT8U counter = 0x00;
    while (counter != 0x10)
    {
        message[counter] = Final_Message[counter];
        counter++;
    }   
}
/**********************************************************
MessageCheckSum - calculates transmitted checksum, and stores values into 
two different bytes for transmission
Stores in *message
**********************************************************/
void MessageCheckSum(INT8U *message, INT8U sourcea, INT8U sourceb,
                     INT8U sourcec)
{
    INT8U TotalSum = 0x00;
    INT8U counter = 0x00;
    INT8U HighBit;
    INT8U LowBit;
   
    while(counter != 0x10)
    {
        TotalSum = TotalSum + Final_Message[counter];
        counter++;    
    }
    TotalSum = TotalSum + sourcea + sourceb + sourcec;    
    HighBit = (TotalSum >> 4); //high 8 bits of the 16 bit interger
    LowBit = (TotalSum & 0x0F);
    
    if((HighBit >=0) && (HighBit <= 9))
    {
        message[0] = HighBit + 48;
    }
    else
    {
        message[0] = HighBit + 55;
    }
    if((LowBit >=0) && (LowBit <= 9))
    {
        message[1] = LowBit + 48;
    }
    else
    {
        message[1] = LowBit + 55;
    }
}
/**********************************
TransmitCheck- sees if the A button is pressed
if A is presses allows TransmitFlag to post
***********************************/
void TransmitCheck(INT8U *message)
{
    if(Message_Status == TRUE)
    {
        message[0] = Message_Status;
    }
    else
    {}
}
/************************************
ReceivedCheckedSum- calculate checksum of received task
then store that checksum in *message.
**************************************/
void ReceivedCheckSum(INT8U *message, INT8U *rec_mess, INT8U sourcea, 
                      INT8U sourceb,INT8U sourcec)
{
    INT8U TotalSum = 0x00;
    INT8U counter = 0x00;
    INT8U HighBit;
    INT8U LowBit;
   
    while(counter != 0x10)
    {
        TotalSum = TotalSum + rec_mess[counter];
        counter++;    
    }
    TotalSum = TotalSum + sourcea + sourceb + sourcec;    
    HighBit = (TotalSum >> 4); //high 8 bits of the 16 bit interger
    LowBit = (TotalSum & 0x0F);
    
    if((HighBit >=0) && (HighBit <= 9))
    {
        message[0] = HighBit + 48;
    }
    else
    {
        message[0] = HighBit + 55;
    }
    if((LowBit >=0) && (LowBit <= 9))
    {
        message[1] = LowBit + 48;
    }
    else
    {
        message[1] = LowBit + 55;
    }


}
/********************************
GetReceiveTime- Gets the Received Time
*********************************/
void GetReceiveTime(void)
{
    TimeGet(&Receive_Time);
}
/*********************************
DispTimeStamp- Displays Time Stamp in DBUTTON_LAYER
**********************************/
void DispTimeStamp(void)
{
    LcdDispTime(1,9,DBUTTON_LAYER,Receive_Time.hr,Receive_Time.min,
                Receive_Time.sec);
}
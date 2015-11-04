/********************************************************************
* Lcd.c
* 
* A set of general purpose LCD utilities. This module should not be
* with a preemptive kernel without protection of the shared LCD.
*  
* Controller: 9s12dp256 PORTA and PORTK0-2 port 
* 
* Author: Andrew Pace
* Date: 2/6/99 
* Class: ET 454 
*  
* Revised: Todd Morton, 04/14/2000 
*          1. Revised to use OCDelay
*          2. Revised to use PORTA and DLC port as wired on target
*             boards.
*          3. Added LcdDispDecByte(), LcdDispTime(), LcdCursor()
*
* Revised: Todd Morton, 02/20/2001
*          1. Revised to use crude software delays to shorten
*             blocking as much as practical.
*          2. Added documentation to Lcd.h.
*          3. Made dummy params in prototypes the same as in
*             function defintions.
*          4. Change LcdClrLine() to require a 2 be entered for 
*             line 2. Before, any number besides 1 would clear line2
*          5. Changed all UBYTE's to INT8U's.
*
* Revised: Todd Morton, 01/30/2003
*          1. Added LcdBSpace() and LcdFSpace().
*
* Revised: Todd Morton, 02/09/2005
*          1. Added extra delay in LcdInit() for powerup.
********************************************************************
* Master Include File  
*******************************************************************/
#include "includes.h"

/*******************************************************************
* LCD Target System Configuration 
*******************************************************************/
#define LCD_RS              0x01
#define LCD_R_WN            0x02
#define LCD_E               0x04
#define LCD_DBUS_PORT       PORTA
#define LCD_DBUS_PORT_DIR   DDRA
#define LCD_CTRL_PORT       PORTK
#define LCD_CTRL_PORT_DIR   DDRK
#define LCD_DBUS_PORT_DIR_OUT 0xFF
#define LCD_CTRL_PORT_DIR_OUT 0x07

/********************************************************************
* LCD Configuration                                                 *
********************************************************************/
#define LCD_DAT_INIT  0x38    /*Data length: 8 bit. Font: 5x7 dots.*/
#define LCD_DIS_INIT  0x0C    /*Display: on. Cursor: off.          */
#define LCD_CLR_CMD   0x01    /*Clear display and move cursor home */
#define LCD_SHIFT_CUR 0x06    /*Increments cursor addr after write.*/

#define LCD_LINE1_ADDR 0x80   /* Display address for line1 column1 */
#define LCD_LINE2_ADDR 0xC0   /* Display address for line2 column1 */
#define LCD_BS_CMD     0x10   /* Move cursor left one space */
#define LCD_FS_CMD     0x14   /* Move cursor right one space */

/********************************************************************
* Macros
********************************************************************/

/********************************************************************
* Public Function prototypes
********************************************************************/
void LcdWrCmd(INT8U cmd);
void LcdInit(void);
void LcdClrDisp(void);
void LcdClrLine(INT8U line);
void LcdDispChar(INT8U c);
void LcdDispByte(INT8U *b);
void LcdDispStrg(INT8U *s);
void LcdMoveCursor(INT8U row, INT8U col);
void LcdDispDecByte(INT8U *b, INT8U lz);
void LcdDispTime(INT8U hrs, INT8U mins, INT8U secs);
void LcdCursor(INT8U on, INT8U blink);
void LcdBSpace(void);
void LcdFSpace(void);

/* Private */
static void LcdDly500ns(void);
static void LcdDly40us(void);
static void LcdDlyms(INT8U ms);

/********************************************************************
* Function Definitions
*********************************************************************
** LcdWrCmd(INT8U cmd) - Public
*
*  FILENAME: LCD.c
*
*  PARAMETERS: cmd - Command to be sent to the LCD
*
*  DESCRIPTION: Sends a command write sequence to the LCD
*
*  RETURNS: None
********************************************************************/
void LcdWrCmd(INT8U cmd) {
      LCD_CTRL_PORT &= ~LCD_RS;
      LCD_DBUS_PORT = cmd;
      LCD_CTRL_PORT |= LCD_E;
      LcdDly500ns();
      LCD_CTRL_PORT &= ~LCD_E;
      LCD_CTRL_PORT |= LCD_RS;
      LcdDly40us();
}

/********************************************************************
** LcdInit
*
*  FILENAME: LCD.c
*
*  PARAMETERS: None
*
*  DESCRIPTION: Initializes LCD ports to outputs and sends LCD
*               initialization sequence.
*
*  RETURNS: None
*
********************************************************************/
void LcdInit(void) {
   
   LCD_CTRL_PORT_DIR = LCD_CTRL_PORT_DIR | LCD_CTRL_PORT_DIR_OUT;
   LCD_DBUS_PORT_DIR = LCD_DBUS_PORT_DIR | LCD_DBUS_PORT_DIR_OUT;
   LCD_CTRL_PORT &= ~LCD_R_WN; /*This module assume write only LCD */
   LCD_CTRL_PORT |= LCD_RS;    /*Data select unless in LcdWrCmd()  */

   LcdDlyms(10);  /* LCD requires 15ms delay at powerup. 9S12 too fast */ 
   LcdDlyms(5); 
   LcdWrCmd(LCD_DAT_INIT);
   LcdDlyms(4); 
  
   LcdWrCmd(LCD_DAT_INIT);
   LcdDlyms(4); 
  
   LcdWrCmd(LCD_DAT_INIT);
   LcdDlyms(4);

   LcdWrCmd(LCD_DIS_INIT);
   LcdDlyms(1);

   LcdWrCmd(LCD_CLR_CMD);
   LcdDlyms(1); 
   
   LcdWrCmd(LCD_SHIFT_CUR);
   LcdDlyms(1); 
} 

/********************************************************************
** LcdDispChar(INT8U c) - Public
*
*  FILENAME: LCD.c
*
*  PARAMETERS: c - ASCII character to be sent to the LCD
*
*  DESCRIPTION: Displays a character at current LCD address. Assumes
*               that the LCD port is configured for a data write.
*
*  RETURNS: None
********************************************************************/
void LcdDispChar(INT8U c) {
      LCD_DBUS_PORT = c;
      LCD_CTRL_PORT |= LCD_E;
      LcdDly500ns();
      LCD_CTRL_PORT &= ~LCD_E;
      LcdDly40us();
}

/********************************************************************
** LcdClrDisp
*
*  FILENAME: LCD.c
*
*  PARAMETERS: None
*
*  DESCRIPTION: Clears the LCD display and returns the cursor to
*               row1, col1.
*
*  RETURNS: None
********************************************************************/
void LcdClrDisp(void) {

   LcdWrCmd(LCD_CLR_CMD);
   LcdDlyms(2);
}

/********************************************************************
** LcdClrLine
*
*  FILENAME: LCD.c
*
*  PARAMETERS: line - Line to be cleared (1 or 2).
*
*  DESCRIPTION: Writes spaces to every location in a line and then 
*               returns the cursor to column 1 of that line.
*
*  RETURNS: None
********************************************************************/
void LcdClrLine (INT8U line) {
  
   INT8U BeginLocation; 
   INT8U i;

   if(line == 1){ 
      BeginLocation = LCD_LINE1_ADDR;  
   }else if(line == 2){ 
      BeginLocation = LCD_LINE2_ADDR;
   }else{
      return;
   }
   LcdWrCmd(BeginLocation);
   for(i = 0x0; i <= 0xF; i++) {
      LcdDispChar(' ');
   }
   LcdWrCmd(BeginLocation);
}
 
/********************************************************************
** LcdDispByte(INT8U *b)
*
*  FILENAME: LCD.c
*
*  PARAMETERS: *b - pointer to the byte to be displayed.
*
*  DESCRIPTION: Displays the byte pointed to by b in hex.
*
*  RETURNS: None
********************************************************************/
void LcdDispByte(INT8U *b) {

   INT8U upnib, lonib;

   lonib = *b & 0x0F;
   upnib = (*b & 0xF0)>>4;

   if(lonib > 9) 
      lonib = lonib + 0x37;
   else
      lonib = lonib + 0x30;

   if(upnib > 9)
      upnib = upnib + 0x37;
   else
      upnib = upnib + 0x30; 

   LcdDispChar(upnib);
   LcdDispChar(lonib);
}   

/********************************************************************
** LcdDispStrg(INT8U *s)
*
*  FILENAME: LCD.c
*
*  PARAMETERS: *s - pointer to the NULL terminated string to be 
*                   displayed.
*
*  DESCRIPTION: Displays the string pointed to by s.
*
*  RETURNS: None
********************************************************************/
void LcdDispStrg(INT8U *s) {

   INT8U *sptr = s;

   while(*sptr != 0x00) {
      LcdDispChar(*sptr);
      sptr++;
   }
}

/********************************************************************
** LcdMoveCursor(INT8U row, INT8U col)
*
*  FILENAME: LCD.c
*
*  PARAMETERS: row - Destination row (1 or 2).
*              col - Destination column (1 - 16).
*
*  DESCRIPTION: Moves the cursor to [row,col].
*
*  RETURNS: None
********************************************************************/
void LcdMoveCursor(INT8U row, INT8U col) {

   if(row == 1) {
      LcdWrCmd(LCD_LINE1_ADDR + col - 1);
   } else {
      LcdWrCmd(LCD_LINE2_ADDR + col - 1);
   }
}

/********************************************************************
** LcdDispTime(INT8U hrs, INT8U min, INT8U sec)
*
*  FILENAME: LCD.c
*
*  PARAMETERS: row - Destination row (1 or 2).
*              col - Destination column (1 - 16).
*
*  DESCRIPTION: Displays the time in HH:MM:SS format. 
*               First converts to decimal.
*
*  RETURNS: None
********************************************************************/
void LcdDispTime(INT8U hrs, INT8U min, INT8U sec) {

  INT8U tens, ones;

   ones = (hrs % 10) + '0';
   hrs = hrs / 10;
   tens = (hrs % 10) + '0';

   LcdDispChar(tens);
   LcdDispChar(ones);
   LcdDispChar(':');
   
   ones = (min % 10) + '0';
   min = min / 10;
   tens = (min % 10) + '0';

   LcdDispChar(tens);
   LcdDispChar(ones);
   LcdDispChar(':');
   
   ones = (sec % 10) + '0';
   sec = sec / 10;
   tens = (sec % 10) + '0';

   LcdDispChar(tens);
   LcdDispChar(ones);
   
}

/********************************************************************
** LcdDispDecByte(INT8U *b, INT8U lz)
*
*  FILENAME: LCD.c
*
*  PARAMETERS: *b - Pointer to the byte to be displayed.
*              lz - (Binary)Display leading zeros if TRUE.
*                   Delete leading zeros if FALSE.
*
*  DESCRIPTION: Displays the byte pointed to by b in decimal. 
*               Deletes leading zeros if lz is zero. Digits are
*               right justified if leading zeros are deleted.
*
*  RETURNS: None
********************************************************************/
void LcdDispDecByte(INT8U *b, INT8U lz) {

   INT8U bin = *b;
   INT8U huns, tens, ones;
   
   ones = (bin % 10) + '0';   /* Convert to decimal digits        */
   bin    = bin / 10;
   tens = (bin % 10) + '0';
   huns = bin / 10 + '0';

   if((huns == '0') && (!lz)){
       LcdDispChar(' ');
   }else{
       lz = TRUE;
       LcdDispChar(huns);
   }
   if((tens == '0') && (!lz)){ 
       LcdDispChar(' ');
   }else{
       LcdDispChar(tens);
   }
   LcdDispChar(ones);
}

/********************************************************************
** LcdCursor(INT8U on, INT8U blink)
*
*  FILENAME: LCD.c
*
*  PARAMETERS: on - (Binary)Turn cursor on if TRUE, off if FALSE.
*              blink - (Binary)Cursor blinks if TRUE.
*
*  DESCRIPTION: Changes LCD cursor state.
*
*  RETURNS: None
********************************************************************/
void LcdCursor(INT8U on, INT8U blink) {

    INT8U curcmd = 0x0C;
    
    if(on){
        curcmd |= 0x02;
    }
    if(blink) {
        curcmd |= 0x01;
    }
    LcdWrCmd(curcmd);
}

/********************************************************************
** LcdDly500ns(void)
*
*  FILENAME: LCD.c
*
*  DESCRIPTION: Delays, at least, 500ns. 9 cycles bsr/jsr and rts
*               plus 4 cycles nop. 
*               Designed for 24MHz or 25MHz clock.
*
*  RETURNS: None
********************************************************************/
static void LcdDly500ns(void){
    __asm {
        nop
        nop
        nop
        nop
    }
}

/********************************************************************
** LcdDly40us(void)
*
*  FILENAME: LCD.c
*
*  DESCRIPTION: Clock frequency independent because it uses

*               LcdDly500ns.
*
*  RETURNS: None
********************************************************************/
static void LcdDly40us(void){
    INT8U cnt;
    for(cnt=80;cnt > 0;cnt--){
        LcdDly500ns();
    }
}

/********************************************************************
** LcdDlyms(INT8U ms)
*
*  FILENAME: LCD.c
*
*  DESCRIPTION: Delays, at least, ms milliseconds. Maximum 10ms.
*
*  RETURNS: None
********************************************************************/
static void LcdDlyms(INT8U ms){
    INT8U cnt;
    for(cnt=ms*25;cnt > 0;cnt--){
        LcdDly40us();
    }
}

/********************************************************************
** LcdBSpace
*
*  FILENAME: LCD.c
*
*  PARAMETERS: None
*
*  DESCRIPTION: Moves cursor back one space.
*
*  RETURNS: None
********************************************************************/
void LcdBSpace(void) {

   LcdWrCmd(LCD_BS_CMD);
}

/********************************************************************
** LcdFSpace
*
*  FILENAME: LCD.c
*
*  PARAMETERS: None
*
*  DESCRIPTION: Moves cursor right one space.
*
*  RETURNS: None
********************************************************************/
void LcdFSpace(void) {

   LcdWrCmd(LCD_FS_CMD);
}

/********************************************************************/

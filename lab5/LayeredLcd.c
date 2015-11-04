/*************************************************************************
* LayeredLcd.c - A MicroC/OS driver for for a Hitachi-type LCD Display   *
*                                                                        *
*                This LCD driver implements the concept of layers.       *
*                This allows asynchronous application tasks to write to  *
*                a single LCD display without interfering with each      *
*                other.                                                  *
*                                                                        *
*                This driver requires hardware for read access because   *
*                it uses the LCD busy flag, BF.                          *
*                                                                        *
*                Requires the following be defined in app_cfg.h:         *
*                   LCD_LAYERS_KEY_PIP                                   *
*                   LCDTASK_PRIO                                         *
*                   LCDTASK_STK_SIZE                                     *
*                                                                        *
*                It is derived from the work of Matthew Cohn, 2/26/2008  *
*                                                                        *
* Todd Morton, 02/26/2013, First Revised Release                         *
*************************************************************************/

/*************************************************************************
  Include Master Header File
*************************************************************************/
#include "includes.h"
/*************************************************************************
  Public Functions
*************************************************************************/
//void LcdInit(INT8U dl, INT8U n, INT8U f);
void LcdDispChar(INT8U row,INT8U col,INT8U layer,INT8U c);
void LcdDispString(INT8U row,INT8U col,INT8U layer,const INT8U *string);
void LcdDispTime(INT8U row,INT8U col,INT8U layer,INT8U hrs,INT8U mins,
                 INT8U secs);
void LcdDispByte(INT8U row,INT8U col,INT8U layer,INT8U byte);
void LcdDispDecByte(INT8U row,INT8U col,INT8U layer,INT8U byte,
                    INT8U lzeros);
void LcdDispClear(INT8U layer);
void LcdDispClrLine(INT8U row, INT8U layer);

void LcdMoveCursor(INT8U row, INT8U col);
void LcdCursor(INT8U on, INT8U blink,INT8U layer);
void LcdBSpace(void);

/*************************************************************************
  Local Defines
*************************************************************************/
#define LCD_DBUS_PORT PORTA
#define LCD_DBUS_DDR  DDRA
#define LCD_CTRL_PORT PORTK
#define LCD_CTRL_DDR  DDRK

// LCD Configuration
#define LCD_NUM_ROWS   2
#define LCD_NUM_COLS   16

// LCD Control Bus
#define LCD_ENABLE     0x04
#define LCD_RS         0x01

// LCD Data Bus
#define LCD_BF         0x80
#define LCD_CLEAR_BYTE 0x20    //SPACE is set as the transparent character

#define DDR_OUT 0xFF
#define DDR_IN 0x00

#define LCD_LINE1_ADDR 0x80   /* Display address for line1 column1 */
#define LCD_LINE2_ADDR 0xC0   /* Display address for line2 column1 */


// LCD layer and buffer typdedef
typedef struct {
    INT8U lcd_char[LCD_NUM_ROWS][LCD_NUM_COLS];
    INT8U hidden;
} LCD_BUFFER;

/*************************************************************************
  Private Local Functions
*************************************************************************/
static void LcdDlyus(INT16U us);
static void LcdDly500ns(void);
static void LcdClockOn(void);
static void LcdClockOff(void);
static void LcdWrite(INT16U data);
static void LcdClear(LCD_BUFFER *buffer);
static void LcdDly40us(void);

static void LcdFlattenLayers(LCD_BUFFER *dest_buffer,
                             LCD_BUFFER *src_layers);
static void LcdWriteBuffer(LCD_BUFFER *buffer);

/*************************************************************************
  MicroC/OS Resources
*************************************************************************/
static void LayeredLcdTask(void *p_arg);                    
static OS_EVENT *lcdLayersKey;
static OS_EVENT *lcdModifiedFlag;    /* Posted when the LCD is modified */
static OS_STK  LcdTaskStk[LCDTASK_STK_SIZE];

/*************************************************************************
  Global Variables
*************************************************************************/
// Stored Constants
static const INT8U lcdRowAddress[LCD_NUM_ROWS] = {0x00, 0x40};

// Static Globals
static LCD_BUFFER lcdBuffer;
static LCD_BUFFER lcdPreviousBuffer;
static LCD_BUFFER LcdLayers[LCD_NUM_LAYERS];

INT8U row_value;
INT8U colum_value;

/*************************************************************************
  LCD Command Macros
*************************************************************************/
/*                                                    R R D D D D D D D D
                                                      / S B B B B B B B B
                                                      W   7 6 5 4 3 2 1 0
*/
// Clear Display                                      0 0 0 0 0 0 0 0 0 1
#define LCD_CLR_DISP()         (0x0001)      
// Return Home                                        0 0 0 0 0 0 0 0 1 *
#define LCD_CUR_HOME()         (0x0002)      
// Entry Mode Set                                     0 0 0 0 0 0 0 1 ids
#define LCD_ENTRY_MODE(id, s)  (0x0004                       \
                                | ((INT16U)id ? 0x0002 : 0)  \
                                | ((INT16U)s  ? 0x0001 : 0))
// Display ON/OFF Control                             0 0 0 0 0 0 1 d c b
#define LCD_ON_OFF(d, c, b)    (0x0008                       \
                                | ((INT16U)d  ? 0x0004 : 0)  \
                                | ((INT16U)c  ? 0x0002 : 0)  \
                                | ((INT16U)b  ? 0x0001 : 0))
// Cursor or Display Shift                            0 0 0 0 0 1 scrl* *
#define LCD_SHIFT(sc, rl)      (0x0010                       \
                                | ((INT16U)sc ? 0x0008 : 0)  \
                                | ((INT16U)rl ? 0x0004 : 0))
// Function Set                                       0 0 0 0 1 dln f * *
#define LCD_FUNCTION(dl, n, f) (0x0020                       \
                                | ((INT16U)dl ? 0x0010 : 0)  \
                                | ((INT16U)n  ? 0x0008 : 0)  \
                                | ((INT16U)f  ? 0x0004 : 0))
// Set CG RAM Address                                 0 0 0 1 ----acg-----
#define LCD_CG_RAM(acg)        (0x0040                       \
                                | ((INT16U)agc  & 0x003F))
// Set DD RAM Address                                 0 0 1 -----add------
#define LCD_DD_RAM(add)        (0x0080                       \
                                | ((INT16U)add  & 0x007F))
// Read Busy Flag & Address                           1 0 ---READ DATA----
#define LCD_READ_BF()          (0x0200)
// Write Data to CG or DD RAM                         0 1 ------data------
#define LCD_WRITE(data)        (0x0100                       \
                                | ((INT16U)data & 0x00FF))
                                                                       
/*************************************************************************
  LayeredLcdTask() - Handles writing to the LCD module      (Private Task)   
  
        When writing to the LCD, will block and poll for as long as the
        LCD Busy Flag LCD_BF is asserted.  This is worst-case 1.3ms, but
        will be much lower if not every character on the screen is
        changing.
*************************************************************************/
static void LayeredLcdTask(void *p_arg) {
    INT8U err;
    
    // Avoid compiler warning
    (void)p_arg;
    
    while(1) {
    
        // Wait for an lcd layer to be modified
        DBUG_PORT &= ~DBUG_LCDTASK;
        OSSemPend(lcdModifiedFlag, 0, &err);
        DBUG_PORT |= DBUG_LCDTASK;
        
        LcdFlattenLayers(&lcdBuffer, (LCD_BUFFER *)&LcdLayers);
        LcdWriteBuffer(&lcdBuffer);
    }

}


/*************************************************************************
  LcdDispClear() - Clears a layer                                 (Public)   

                   Pends on the lcdLayersKey mutex
                   Posts the lcdModifiedFlag semaphore
*************************************************************************/
void LcdDispClear(INT8U layer) {
    INT8U err;
    LCD_BUFFER *llayer = &LcdLayers[layer];

    OSMutexPend(lcdLayersKey, 0, &err);

    LcdClear(llayer);

    (void)OSMutexPost(lcdLayersKey);
    
    // We have modified a layer
    (void)OSSemPost(lcdModifiedFlag);
}


/*************************************************************************
  LcdDispClrLine() - Clears a line of a layer                     (Public)   

                     Pends on the lcdLayersKey mutex
                     Posts the lcdModifiedFlag semaphore
*************************************************************************/
void LcdDispClrLine(INT8U row, INT8U layer) {
    INT8U err, col;
    LCD_BUFFER *llayer = &LcdLayers[layer];
    
    OSMutexPend(lcdLayersKey, 0, &err);
    
    // For each column...
    for(col = 0; col < LCD_NUM_COLS; col++) {

        // Clear the character at that position
        llayer->lcd_char[row-1][col] = LCD_CLEAR_BYTE;
    }
    
    (void)OSMutexPost(lcdLayersKey);
    
    // We have modified a layer
    (void)OSSemPost(lcdModifiedFlag);
}


/*************************************************************************
  LcdDispString() - Writes a null terminated string to a layer    (Public)

                    Pends on the lcdLayersKey mutex
                    Posts the lcdModifiedFlag semaphore
*************************************************************************/
void LcdDispString(INT8U row,
                   INT8U col,
                   INT8U layer,
                   const INT8U *string) {

    INT8U cnt, err, row_index, col_index;
    LCD_BUFFER *llayer = &LcdLayers[layer];

    row_index = row - 1;
    col_index = col - 1;
    
    OSMutexPend(lcdLayersKey, 0, &err);
    
    // Iterate through the string until we reach a null
    for(cnt = 0; string[cnt] != 0x00; cnt++) {
    
        if((col_index+cnt) < LCD_NUM_COLS){ // not at end of row
            // Copy from the passed paramater to the layer
            llayer->lcd_char[row_index][col_index+cnt] = string[cnt];
        }else{ //outside buffer
        }
    }
    
    (void)OSMutexPost(lcdLayersKey);
    
    // We have modified a layer
    (void)OSSemPost(lcdModifiedFlag);
}



/*************************************************************************
  LcdDispChar() - Writes a character to a layer                   (Public)

                  Pends on the lcdLayersKey mutex
                  Posts the lcdModifiedFlag semaphore
*************************************************************************/
void LcdDispChar(INT8U row,
                 INT8U col,
                 INT8U layer,
                 INT8U character) {
    INT8U err, row_index, col_index;
    LCD_BUFFER *llayer = &LcdLayers[layer];

    row_index = row - 1;
    col_index = col - 1;
    
    if(col_index < LCD_NUM_COLS){
        OSMutexPend(lcdLayersKey, 0, &err);
    
        // Copy from the passed paramater to the layer
        llayer->lcd_char[row_index][col_index] = character;
    
        (void)OSMutexPost(lcdLayersKey);
    
        // We have modified a layer
        (void)OSSemPost(lcdModifiedFlag);
    }else{ //outside layer
    }
}



/*************************************************************************
  LcdDispByte - Writes the ASCII representation of a byte to a    (Public)
                layer in hex

                Pends on the lcdLayersKey mutex
                Posts the lcdModifiedFlag semaphore
*************************************************************************/
void LcdDispByte(INT8U row, INT8U col, INT8U layer, INT8U byte) {
    INT8U err, row_index, col_index;
    LCD_BUFFER *llayer = &LcdLayers[layer];
    
    // Convert row / col index 1 to index 0
    row_index = row - 1;
    col_index = col - 1;
    
    if(col < LCD_NUM_COLS){
        OSMutexPend(lcdLayersKey, 0, &err);

        llayer->lcd_char[row_index][col_index+0] = (byte >> 4);   // MSB
        llayer->lcd_char[row_index][col_index+1] = (byte & 0x0F); // LSB
    
        // Convert MSB to ASCII character
        llayer->lcd_char[row_index][col_index+0] +=
            (llayer->lcd_char[row_index][col_index+0] <= 9 ? '0' : 'A' - 10);

        // Convert LSB to ASCII character
        llayer->lcd_char[row_index][col_index+1] +=
            (llayer->lcd_char[row_index][col_index+1] <= 9 ? '0' : 'A' - 10);


        (void)OSMutexPost(lcdLayersKey);

        // We have modified a layer
        (void)OSSemPost(lcdModifiedFlag);
    }else{ //outside layer
    }
}


/*************************************************************************
  LcdDispDecByte - Writes the ASCII representation of a byte to a (Public)
                   layer in decimal

                   Pends on the lcdLayersKey mutex
                   Posts the lcdModifiedFlag semaphore
*************************************************************************/
void LcdDispDecByte(INT8U row,
                    INT8U col,
                    INT8U layer,
                    INT8U byte,
                    INT8U lzeros) {
    
    INT8U err, row_index, col_index, hunds, tens, ones;
    LCD_BUFFER *llayer = &LcdLayers[layer];
    
    if((col + 1) < LCD_NUM_COLS){
        // Convert row / col index 1 to index 0
        row_index = row - 1;
        col_index = col - 1;
    
        hunds = byte / 100;
        tens = (byte / 10) % 10;
        ones = byte % 10;
    
        OSMutexPend(lcdLayersKey, 0, &err);

        if(lzeros == 1 || hunds > 0) {
            llayer->lcd_char[row_index][col_index+0] = hunds; // Hundreds
            llayer->lcd_char[row_index][col_index+0] += '0';  //  --> ASCII
        }

        if(lzeros == 1 || hunds > 0 || tens > 0) {
            llayer->lcd_char[row_index][col_index+1] = tens;  // Tens
            llayer->lcd_char[row_index][col_index+1] += '0';  //  --> ASCII
        }
    
        llayer->lcd_char[row_index][col_index+2] = ones;      // Ones
        llayer->lcd_char[row_index][col_index+2] += '0';      //  --> ASCII
        

        (void)OSMutexPost(lcdLayersKey);

        // We have modified a layer
        (void)OSSemPost(lcdModifiedFlag);
    }else{ //outside layer
    }
}

/*************************************************************************
  LcdDispTime - Writes a time to a layer                          (Public)

                Pends on the lcdLayersKey mutex
                Posts the lcdModifiedFlag semaphore
*************************************************************************/
void LcdDispTime(INT8U row,
                 INT8U col,
                  INT8U layer,
                 INT8U hrs,
                 INT8U mins,
                 INT8U secs) {
    INT8U err, row_index, col_index;
    LCD_BUFFER *llayer = &LcdLayers[layer];

    if((col + 6) < LCD_NUM_COLS){
        // Convert row / col index 1 to index 0
        row_index = row - 1;
        col_index = col - 1;

    
        OSMutexPend(lcdLayersKey, 0, &err);
    

        llayer->lcd_char[row_index][col_index+0] = hrs / 10 + '0';
        llayer->lcd_char[row_index][col_index+1] = hrs % 10 + '0';

        llayer->lcd_char[row_index][col_index+2] = ':';

        llayer->lcd_char[row_index][col_index+3] = mins / 10 + '0';
        llayer->lcd_char[row_index][col_index+4] = mins % 10 + '0';

        llayer->lcd_char[row_index][col_index+5] = ':';

        llayer->lcd_char[row_index][col_index+6] = secs / 10 + '0';
        llayer->lcd_char[row_index][col_index+7] = secs % 10 + '0';
    
           
        (void)OSMutexPost(lcdLayersKey);
    
        // We have modified a layer
        (void)OSSemPost(lcdModifiedFlag);
    }else{ //outside layer
    }
}


/*************************************************************************
  LcdInit() - Initializes the LCD                                 (Public)

        Initializes the LCD hardware, sets up our semaphores/mutexes/task,
        clears all of our buffers and layers.  This needs to be run before
        any other function that accesses the LCD.
*************************************************************************/
void LcdInit(INT8U dl, INT8U n, INT8U f) {
    INT8U err, layer_cnt;

    // Create mutex key, modified semaphore, and task
    lcdLayersKey = OSMutexCreate(LCD_LAYERS_KEY_PIP, &err);
    lcdModifiedFlag = OSSemCreate(0);
    (void)OSTaskCreate(LayeredLcdTask,
         (void *)0,
         (void *)&LcdTaskStk[LCDTASK_STK_SIZE],
         LCDTASK_PRIO);

    // Perform LCD hardware initialization
    LcdDlyus(15000);
    LcdWrite(LCD_FUNCTION(dl, n, f));
    LcdDlyus(4100);
    LcdWrite(LCD_FUNCTION(dl, n, f));
    LcdDlyus(100);
    LcdWrite(LCD_FUNCTION(dl, n, f));
    LcdDlyus(40);
    LcdWrite(LCD_FUNCTION(dl, n, f));
    LcdDlyus(40);
    LcdWrite(LCD_ENTRY_MODE(1, 0)); // Increment, no shift
    LcdDlyus(40);
    LcdWrite(LCD_ON_OFF(1, 0, 0));  // LCD on, cursor off, blink off
    LcdDlyus(40);
    LcdWrite(LCD_CLR_DISP());       // Clear display
    LcdDlyus(1640);
    LcdWrite(LCD_DD_RAM(0x0000));   // Reset cursor
    LcdDlyus(40);
    
    
    // Clear all of our layers
    for(layer_cnt = 0; layer_cnt < LCD_NUM_LAYERS; layer_cnt++) {
        LcdClear(&LcdLayers[layer_cnt]);
    }
    
    // Clear the current buffer
    // and the previous buffer
    LcdClear(&lcdBuffer);
    LcdClear(&lcdPreviousBuffer);
}


/*************************************************************************
  LcdFlattenLayers() - Combines *src_layers onto *dest_buffer    (Private)

        The src_layer with the lowest index will be on the bottom, the
        src_layer with the highest index will be on the top.  Treats the
        character defined as LCD_CLEAR_BYTE as a transparent byte.

                       Pends on the lcdLayersKey mutex
*************************************************************************/
void LcdFlattenLayers(LCD_BUFFER *dest_buffer,
                             LCD_BUFFER *src_layers) {
    
    INT8U err, layer, row, col, current_char;
    
    // Clear the destination buffer
    LcdClear(dest_buffer);

    DBUG_PORT &= ~DBUG_LCDTASK;
    OSMutexPend(lcdLayersKey, 0, &err);
    DBUG_PORT |= DBUG_LCDTASK;
    
    // For each layer...
    for(layer = 0; layer < LCD_NUM_LAYERS; layer++) {

        // If that layer is not hidden...
        if((src_layers+layer)->hidden == 0) {
            // For each row...
            for(row = 0; row < LCD_NUM_ROWS; row++) {
                // For each column...
                for(col = 0; col < LCD_NUM_COLS; col++) {
                    current_char = (src_layers+layer)->lcd_char[row][col];
                
                    // If the source layer is not null
                    if(current_char != LCD_CLEAR_BYTE) {
                        // Copy from the source layer to the buffer
                        dest_buffer->lcd_char[row][col] = current_char;
                    }else{ //Do nothing - transparent
                    }
                    
                } // column
            } // row
        }else{ //Do nothing - layer is hidden
        }
    } // layer
    
    (void)OSMutexPost(lcdLayersKey);

}


/*************************************************************************
  LcdWriteBuffer() - Sends an LCD_BUFFER buffer to LcdWrite()    (Private)
  
        The previous buffer lcdPreviousBuffer is a global variable
        containing a copy of the actual contents of the LCD module.  By 
        using the lcdPreviousBuffer and repos_flag, we are able to only
        write bytes that have changed.
                                                           
                     Blocks for as long as LcdWrite() blocks              
*************************************************************************/
void LcdWriteBuffer(LCD_BUFFER *buffer) {
    INT8U row, col, repos_flag;
    
    // For each row...
    for(row = 0; row < LCD_NUM_ROWS; row++) {
    
        // Set our cursor to the beginning of the row
        LcdWrite(LCD_DD_RAM(lcdRowAddress[row]));
        repos_flag = 0;
        
        // For each column...
        for(col = 0; col < LCD_NUM_COLS; col++) {

            // If the character at the current position has changed...
            if(lcdPreviousBuffer.lcd_char[row][col]
                != buffer->lcd_char[row][col]) {
                
                // If we need to reposition, do that now
                if(repos_flag == 1) {
                    LcdWrite(LCD_DD_RAM(lcdRowAddress[row] + col));
                    repos_flag = 0;
                }
            
                // Write the character to the LCD
                LcdWrite(LCD_WRITE(buffer->lcd_char[row][col]));
             
                // And update the previous buffer
                lcdPreviousBuffer.lcd_char[row][col] =
                    buffer->lcd_char[row][col];
            } else {
                // Otherwise we don't write the character... but we need
                //   to set the reposition flag
                
                repos_flag = 1;
            }
            
            
        }
    }
    LcdMoveCursor(row_value,colum_value);
}

/*************************************************************************
  LcdWrite() - Writes a command (both data and control busses)   (Private)
               to the LCD.
               
               Blocks while the LCD Busy Flag LCD_BF is asserted
*************************************************************************/
void LcdWrite(INT16U data) {
    INT8U lcd_dbus_in;
    
    // Change direction to read LCD Busy Flag
    LCD_DBUS_DDR = DDR_IN;
    LCD_CTRL_DDR = (0x07 & DDR_OUT);

    // Send "Read Busy Flag" command
    LCD_CTRL_PORT = (INT8U)(LCD_READ_BF() >> 8); // Ctrl bus gets MSB
    
    LcdClockOn();
    lcd_dbus_in = LCD_DBUS_PORT & LCD_BF;
    LcdClockOff();
    
    // While the LCD is busy...
    while(lcd_dbus_in == LCD_BF)
    {
        // Poll LCD Busy Flag
        LcdClockOn();
        lcd_dbus_in = LCD_DBUS_PORT & LCD_BF;
        LcdClockOff();
    }
    
    // Write command to LCD
    LCD_DBUS_DDR = DDR_OUT;
    LCD_CTRL_DDR = (0x07 & DDR_OUT);
    
    LCD_CTRL_PORT = (INT8U)(data >> 8);  // Ctrl bus gets MSB
    LCD_DBUS_PORT = (INT8U)(data);       // Data bus gets LSB
    
    LcdClockOn();
    LcdClockOff();
}


/*************************************************************************
  LcdClockOn() - Turns on the enable bit and waits the duration  (Private)
                 of Enable pulse width High Level (PW_EH)
                 
                 LCD Enable rising edge
                 
              ** CONTAINS uPROCESSOR SPECIFIC TIMING ASSEMBLY CODE **
*************************************************************************/
void LcdClockOn() {
    // Pull up Enable port
    LCD_CTRL_PORT |= LCD_ENABLE;

    // Hold LCD Enable up for PW_EH (230ns).  Putting this in a function
    //   would cause unnecisary and unavoidable delay.
    __asm {
        nop
        nop
    }
}


/*************************************************************************
  LcdClockOff() - Turns off the enable bit of the LCD            (Private)
                          
                  LCD Enable falling edge
*************************************************************************/
void LcdClockOff() {
    // Push down Enable port
    LCD_CTRL_PORT &= ~LCD_ENABLE;
}


/*************************************************************************
  LcdClear() - Clears a buffer or layer                          (Private)
*************************************************************************/
void LcdClear(LCD_BUFFER *buffer) {
    INT8U row, col;
    
    // For each row...
    for(row = 0; row < LCD_NUM_ROWS; row++) {
        // For each column...
        for(col = 0; col < LCD_NUM_COLS; col++) {

            // Clear the character at that position
            buffer->lcd_char[row][col] = LCD_CLEAR_BYTE;

        }
    }
    
}


/*************************************************************************
  LcdDlyus() - Blocks for the passed number of microseconds      (Private)
*************************************************************************/
void LcdDlyus(INT16U us) {
    INT16U cnt;
    
    for(cnt = 0; cnt <= us; cnt++) {
        LcdDly500ns();
        LcdDly500ns();
    }

}


/*************************************************************************
  LcdDly500ns(void)                                              (Private)


  DESCRIPTION: Delays, at least, 500ns. 9 cycles bsr/jsr and rts
               plus 4 cycles nop. 
               Designed for 24MHz or 25MHz clock.

  RETURNS: None

  Adapted and modified from LCD.C by Andrew Pace, Todd Morton
*************************************************************************/
void LcdDly500ns(void) {
    __asm {
        nop
        nop
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
************************************************************************/
void LcdMoveCursor(INT8U row, INT8U col) {
     
    row_value = row;
    colum_value = col;   
    if(row == 1) {
      LcdWrite(LCD_LINE1_ADDR + col - 1);
   } else {
      LcdWrite(LCD_LINE2_ADDR + col - 1);
   }
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
    LcdWrite(curcmd);
}

static void LcdDly40us(void){
    INT8U cnt;
    for(cnt=80;cnt > 0;cnt--){
        LcdDly500ns();
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

    LCD_CTRL_PORT &= ~LCD_RS;
    LCD_DBUS_PORT = 0x10;
    LCD_CTRL_PORT |= LCD_ENABLE;
    LcdDly500ns();
    LCD_CTRL_PORT &= ~LCD_ENABLE;
    LCD_CTRL_PORT |= LCD_RS;
    LcdDly40us(); 

}
void LcdHideLayer(INT8U layer)
{
    INT8U err;
    LCD_BUFFER *llayer = &LcdLayers[layer];
    llayer->hidden = 1;
    
           
}
void LcdShowLayer(INT8U layer)
{
    INT8U err;
    LCD_BUFFER *llayer = &LcdLayers[layer];
    llayer->hidden = 0;
}




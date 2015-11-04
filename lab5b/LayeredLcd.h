/*************************************************************************
* LayeredLcd.c - A MicroC/OS driver for for the Seiko LCD Display        *
*                                                                        *
*                This LCD driver implements the concept of layers.       *
*                This allows asynchronous application tasks to write to  *
*                a single LCD display without interfering with each      *
*                other.                                                  *
*                                                                        *
*                This driver requires hardware for read access because   *
*                it uses the LCD busy flag, BF.                          *
*                                                                        *
*                It is derived from the work of Matthew Cohn, 2/26/2008  *
*                                                                        *
* Todd Morton, 02/26/2013, First Release                                 *
*************************************************************************/

/*************************************************************************
* LCD Layers - Define all layer values here                              *
*              Range from 0 to (LCD_NUM_LAYERS - 1)                      *
*              Arranged from largest number on top, down to 0 on bottom. *
*************************************************************************/
#define LCD_NUM_LAYERS 1

#define CLOCK_LAYER    0

/*************************************************************************
  Public Functions
*************************************************************************/

extern void LcdInit(INT8U dl, INT8U n, INT8U f);

extern void LcdDispChar(INT8U row,INT8U col,INT8U layer,INT8U c);

extern void LcdDispString(INT8U row,INT8U col,INT8U layer,
                          const INT8U *string);
                          
extern void LcdDispTime(INT8U row,INT8U col,INT8U layer,
                        INT8U hrs,INT8U mins,INT8U secs);
                        
extern void LcdDispByte(INT8U row,INT8U col,INT8U layer,INT8U byte);
                        
extern void LcdDispDecByte(INT8U row,INT8U col,INT8U layer,
                           INT8U byte,INT8U lzeros);
                        
extern void LcdDispClear(INT8U layer);

extern void LcdDispClrLine(INT8U row, INT8U layer);

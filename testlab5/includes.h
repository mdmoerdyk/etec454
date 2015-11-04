/*************************************************************************
* includes.h - Master header file template for star12 projects.
*              This version is for ETec454 Lab1 using the Metrowerks 
*              compiler. 
*
* Todd Morton 01/05/05
* Modifided by Mark Moerdyk 2/14/13 - 2/22/13
**************************************************************************
* General type definitions 
*************************************************************************/
typedef unsigned char   INT8U;
typedef signed char     INT8S;
typedef unsigned short  INT16U;
typedef signed short    INT16S;
typedef unsigned long   INT32U;
typedef signed long     INT32S;
typedef struct{
    INT8U hr;
    INT8U min;
    INT8U sec;
    }TIME;

#define ISR __interrupt void

/*************************************************************************
* General Defined Constants 
*************************************************************************/
#define FALSE    0
#define TRUE     1

/*************************************************************************
* General defined macros 
*************************************************************************/
#define FOREVER()       while(1)
#define TRAP()          while(1){}

/*************************************************************************
* MCU specific definitions
*************************************************************************/
#include "9S12dp512.h"           /* CODE 9S12DP256 register defines     */
#define  SWI()          asm swi;
#define  ENABLE_INT()   asm cli;
#define  DISABLE_INT()  asm sei;

/*************************************************************************
* Project Constant and Macro Definitions
*************************************************************************/
#define DBUG_PORT_DIR DDRP           /* Initialize debug port      */
#define DBUG_PORT PTP
#define PP7 0x80
#define PP6 0x40
#define PP5 0x20
#define PP4 0x10
#define PP3 0x08
#define DB_OUTS 0xFC
#define  DBUG_LCDTASK 0x04;
/*************************************************************************
* System Header Files 
*************************************************************************/

/*************************************************************************
* Module Header Files or Declarations 
*************************************************************************/
#include <ucos_ii.h>
#include "UcosKey.h"
#include "Clock.h"
#include "TextEntry.h"
#include "TimeSet.h"
#include "LayeredLcd.h"
//#include "LCD.h"

/************************************************************************/


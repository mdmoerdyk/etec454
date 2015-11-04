/*************************************************************************
* includes.h - Master header file template for star12 projects.
*              This version is for ETec454 Lab1 using the Metrowerks 
*              compiler. 
*
* Todd Morton 01/05/05

*Modified by Mark Moerdyk
**************************************************************************
* General type definitions 
*************************************************************************/
typedef unsigned char   INT8U;
typedef signed char     INT8S;
typedef unsigned short  INT16U;
typedef signed short    INT16S;
typedef unsigned long   INT32U;
typedef signed long     INT32S;

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
* Move the comment to use the project MCU, or enter a new MCU header.
*************************************************************************/
#include "9S12dp512.h"           /* CODE 9S12DP512 register defines     */
//#include "9S12dg128.h"           /* CODE 9S12DG128 register defines     */
#include "Key.h"
#include "LCD.h"
#include "OCDelay.h"

#define  SWI()          asm swi;
#define  ENABLE_INT()   asm cli;
#define  DISABLE_INT()  asm sei;

/*************************************************************************
* Project Constant and Macro Definitions
*************************************************************************/

/*************************************************************************
* System Header Files 
*************************************************************************/

/*************************************************************************
* Module Header Files or Declarations 
*************************************************************************/
/************************************************************************/


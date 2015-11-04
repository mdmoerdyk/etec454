/********************************************************************
* OCDelay.c - A delay module based on Output Compare 0
* 7/29/99 Todd Morton
* 02/01/02 Todd Morton
*    - Fixed OCDelay() to handle overruns
*
* 02/04/08 Todd Morton
*    - Modified for 9S12Dx
*********************************************************************
* Project master header file
********************************************************************/
#include "includes.h"

/********************************************************************
* Public Resources
********************************************************************/
void OCDlyInit(void);           /* OCDelay Initialization Routine  */
void OCDelay(INT16U ms);        /* OCDelay Function                */
INT16U GetmSCnt(void);          /* Read the current mS count       */

/********************************************************************
* Private Resources
********************************************************************/
ISR OC0Isr(void);               /* OC0 interrupt service routine   */
static volatile INT16U OCmSCnt; /* 1ms counter variable            */

/********************************************************************
* Module Defines
********************************************************************/
#define E_PER_MS 24000           /* E clock cycles per 1ms         */
#define C0F 0x01
#define OL0 0x01
#define C0 0x01
#define TEN 0x80
#define TC0_NUM 23

/********************************************************************
* OCDelay Function
*    - Public
*    - Delays 'ms' milliseconds
*    - Accuracy +/- 1 mS
********************************************************************/
void OCDelay(INT16U ms){
    INT16U start_cnt;
    start_cnt = OCmSCnt;
    while((OCmSCnt - start_cnt) < ms){}  /* wait for ms to pass    */
}

/********************************************************************
* OCDlyInit() - Initialization routine for OCDelay()
*    MCU: 9S12Dx
*    - Sets TC0 for an output compare.
********************************************************************/
void OCDlyInit(void){
    TSCR1 |= TEN;                       /* Enable timer            */
    TIOS |= C0;                         /* Set Channel 0 to OC     */
    TCTL2 = OL0|(TCTL2&0xFC);           /*Toggle OC0 pin for debug */
    TIE |= C0F;                         /*Enable OC0 interrupt     */
    TFLG1 = C0F;                        /*Clear Channel 0 flag     */
}

/********************************************************************
* GetmSCnt() - Read the current mS counter value. (Public)
********************************************************************/
INT16U GetmSCnt(void) {
    return OCmSCnt;
}

/********************************************************************
* OC0Isr() - OC0 Service Routine. (Private)
*    MCU: 9S12Dx
*    - Requires TC0 be set for an output compare (OC0Init()).
*    - setup for a 1ms periodic interrupt.
*********************************************************************/
ISR OC0Isr(void){
     TC0 = TC0 + E_PER_MS;              /* Interrupt in 1ms later   */
     TFLG1 = C0F;                       /* Clear Channel 0 flag     */
     OCmSCnt++;                         /* Increment 1mS counter    */
}
/********************************************************************/

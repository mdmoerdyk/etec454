/*******************************************************************
* OCDelay.h - Project Header file for the Output Compare - based
*             delay routine. 
*
* Todd Morton 7/28/99
*********************************************************************
* Public Function Prototypes 
********************************************************************/
extern void OCDelay(INT16U ms); /* Blocking delay routine.         */
                                /* The parameter is the number of  */
                                /* ms to delay.                    */
                                
extern void OCDlyInit(void);    /* OCDelay initialization. This    */
                                /* function must be called before  */
                                /* OCDelay() can be called.        */

extern INT16U GetmSCnt(void);   /* Read the current mS count       */
 
/*******************************************************************/

/********************************************************************
* KeyUcos.h - The header file for the keypad module, KeyUcos.c
* 02/15/2006 Todd Morton
*********************************************************************
* Public Resources
********************************************************************/
extern INT8U KeyPend(INT16U tout, INT8U *err); /* Pend on key press*/
                             /* tout - semaphore timeout           */
                             /* *err - destination of err code     */
                             /* Error codes are identical to a semaphore */

extern void KeyInit(void);             /* Keypad Initialization    */


/*************************************************************************
* 9S12DP256Start.c
* Startup code - Designed for Codewarrior programs that run in the small 
*               memmory model. 
*
* MCU: 9S12DP256
* Compiler: Metrowerks HC12 v2.0
* 05/02/04 Todd Morton
* 02/03/05 Todd Morton - Revised to trap instead of restarting.
*************************************************************************/
#include "includes.h"

/*************************************************************************
* Types for linker generated startup data
*************************************************************************/
typedef struct _Range {
    INT8U *beg;
    INT16U size;
} _Range ;

typedef struct _Copy {
    INT16U size;
    INT8U * dest;
}_Copy;

typedef void (*_PFunc)(void);

extern struct _tagStartup {
    INT8U flags;
    _PFunc main;
    INT16U stackOffset;
    INT16U nofZeroOuts;
    _Range *pZeroOut;
    _Copy *toCopyDownBeg;
} _startupData;

/* This stuff doesn't work. I will wait till later to fix it. Until
 * then I'll use defines. So, if the memory map changes, this has
 * to change in addition to the *.prm file.
 * Todd Morton 05/05/04
 *
 */
//extern INT16U __SEG_START_IO[];
//const INT16U IOStart = (INT16U)__SEG_START_IO;
//extern INT16U __SEG_START_DEFAULT_RAM[];
//const INT16U RAMStart = (INT16U)__SEG_START_DEFAULT_RAM;
//extern INT16U __SEG_START_EE[];
//const INT16U EEStart = (INT16U)__SEG_START_EE;
#define IOStart (INT16U)0x0000  /* Start address of the IO registers. */
#define RAMStart (INT16U)0x1000 /* Start address of the RAM. */
#define EEStart (INT16U)0x0400  /* Start address of the EEPROM. */

/*************************************************************************
* Function Prototypes
*************************************************************************/
void _Startup (void);       /* Entry point - Stored in RESET vector. */
static void Init(void);

/*************************************************************************
* Instance on data structure for linker generated startup data.
*************************************************************************/
struct _tagStartup _startupData;

#pragma NO_FRAME    /* Make sure Compiler doesn't generate code. All assembly */
#pragma NO_ENTRY
#pragma NO_EXIT
#pragma MESSAGE DISABLE C12053
void _Startup(void){
        if (!(_startupData.flags & 0x02)) { /*If stack is specified */
            __asm lds _startupData.stackOffset;
        }
        Init();
        (*_startupData.main)();
        TRAP();
}

static void Init(void) {

    /*************************************************************************
    ; MCU Write-once Register Initialization and COP config/reset
    ;*************************************************************************
    ; Phase-Lock Loop Initialization for System Clock
    ;  - Set for 24MHz (SYNR = 2, REFDV = 1)
    ;************************************************************************/
    SYNR = 0x02;
    REFDV = 0x01;
    while((CRGFLG & 0x08) == 0){}
    CLKSEL |= 0x80;
    /* Look into ECLKDIV requirements before trying to program EEPROM. */

    /*************************************************************************
    * Recommended initialization for all standalone systems
    *************************************************************************/
    INITRG = (INT8U)(IOStart>>8); /* Set Register location */
                                          /* (from linker cmd file) */
    MODE = 0x80;            /* Protect from mode change(Norm Single-Chip) */
    MISC = 0x0D;            /* Protect flash from being turned off */
    INITRM = (INT8U)((RAMStart>>8)|0x01);        /* Set RAM loacation */
    INITEE = (INT8U)((EEStart>>8)|0x01);     /* Set EEPROM location */
                                            /* (from linker cmd file) */
    //EPROT = 0xFE;                         /* Lock EEPROM Protection */
                                            /* (from linker cmd file) */
    /*************************************************************************
    * Recommended initialization for most standalone systems
    *************************************************************************/
    PEAR = 0x00;            /* PortE always GPIO w/ E-clk */

    /*************************************************************************
    * Clear and initialize RAM. Uses startup data structure filled by the
    *                           linker.
    *************************************************************************/
    __asm {        
        ZeroOut:
                    ldx _startupData.pZeroOut
                    ldy _startupData.nofZeroOuts
                    beq CopyDown

        NextZeroOut: pshy
                    ldy 2,X+
                    ldd 2,X+
        NextWord:   clr 1,Y+
                    dbne D,NextWord
                    puly
                    dey
                    bne NextZeroOut

        CopyDown:
                    ldx _startupData.toCopyDownBeg
        NextBlock:
                    ldd 2,X+
                    beq funcInits
                    ldy 2,X+

        Copy:
                    movb 1,X+,1,Y+
                    dbne D,Copy
                    bra NextBlock
        funcInits:
    }
}

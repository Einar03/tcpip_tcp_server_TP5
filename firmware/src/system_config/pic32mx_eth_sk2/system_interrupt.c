/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "system/common/sys_common.h"
#include "app.h"
#include "appgen.h"
#include "system_definitions.h"
#include "GesPec12.h"
#include "GesS9.h"
#include "Generateur.h"


#define INIT_TIME 2999  //Init time in [ms]
#define MACHINE_CYCLE 10  //Cycle for execution sequence in [ms]

// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************

//==============================================================================
//                          Timers Statics
//==============================================================================
void __ISR(_TIMER_1_VECTOR, ipl3AUTO) IntHandlerDrvTmrInstance1(void)
{
    static uint16_t Timer1Counter = 0;
    static bool FlagInit = 0;
    
    //LED_0Toggle();
    // =================================
    //           Initialisation
    // =================================
    if(FlagInit == 0)
    {
        if(Timer1Counter <= INIT_TIME)
        {
          Timer1Counter++;
        }
        else
        {
          Timer1Counter = 0;
          FlagInit = 1;
        }
    }
    // =================================
    //           Execution
    // =================================
    else
    {
        
        if(Timer1Counter < (MACHINE_CYCLE-1))
        {
          Timer1Counter++;
        }
        else
        {
          Timer1Counter = 0;
          APPGEN_UpdateState(APPGEN_STATE_SERVICE_TASKS);
        }
        // Si TCP connecté, arrêter la lecture des boutons 
        if(GetTCPFlagState() == false)
        {
            ScanPec12(PEC12_A, PEC12_B, PEC12_PB);
            ScanS9(S_OK);
        }
    }
    //LED1_W = !LED1_R;
    //BSP_LEDOff(BSP_LED_0);
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_1);
}
void __ISR(_TIMER_3_VECTOR, ipl7AUTO) IntHandlerDrvTmrInstance2(void)
{
    //LED0_W = 1;
    GENSIG_Execute();
    //LED0_W = 0;
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_3);
}


void __ISR(_TIMER_2_VECTOR, ipl1AUTO) IntHandlerDrvTmrInstance0(void)
{
    DRV_TMR_Tasks(sysObj.drvTmr0);
}
 void __ISR(_USB_1_VECTOR, ipl1AUTO) _IntHandlerUSBInstance0(void)
{
    DRV_USBFS_Tasks_ISR(sysObj.drvUSBObject);
}

void __ISR(_ETH_VECTOR, ipl5AUTO) _IntHandler_ETHMAC(void)
{
    DRV_ETHMAC_Tasks_ISR((SYS_MODULE_OBJ)0);
}

/* This function is used by ETHMAC driver */
bool SYS_INT_SourceRestore(INT_SOURCE src, int level)
{
    if(level)
    {
        SYS_INT_SourceEnable(src);
    }

    return level;
}

/*******************************************************************************
 End of File
*/

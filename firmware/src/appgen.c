/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    appgen.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

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

#include "appgen.h"
#include "Mc32DriverLcd.h"
#include "Mc32gestSpiDac.h"
#include "DefMenuGen.h"
#include "MenuGen.h"
#include "GesPec12.h"
#include "GesS9.h"
#include "Generateur.h"
#include "Mc32gest_SerComm.h"
#include "Mc32gestI2cSeeprom.h"
// Include pour les drivers des timers static
#include "driver/tmr/drv_tmr_static.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APPGEN_DATA appgenData;

S_ParamGen LocalParamGen;
S_ParamGen RemoteParamGen;
S_ParamGen CheckUpdateParamGen;

S_ParamGen NewParamGen; // Pour récuperation des nouveaux paramètres
static bool TCPConnected = false;     // Flag pour l'état du TCP
static bool SaveData = false; // Flag pour la sauvegarde en mode remote
// Tableu pour la réception des données de l'USB (app.c))
uint8_t ReceiveMessageString[30] = "!S=CF=1122A=33445O=+5566WP=0#";
bool flag_IP = false;

IPV4_ADDR  ipAddr;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APPGEN_Initialize ( void )

  Remarks:
    See prototype in appgen.h.
 */

void APPGEN_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    APPGEN_UpdateState(APPGEN_STATE_INIT);
    // Initialisation des Timers
    DRV_TMR0_Initialize();
    DRV_TMR1_Initialize();
    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APPGEN_Tasks ( void )

  Remarks:
    See prototype in appgen.h.
 */

void APPGEN_Tasks ( void )
{
    // Variables locales
    // Tableau pour sauvegarder le message à envoyer
    uint8_t MessageString[30] = "!S=0F=0000A=00000O=+0000WP=1#";
    // Variables
    static uint16_t count = 0;
    static bool RemoteSave = false;
    
    /* Check the application's current state. */
    switch ( appgenData.state )
    {
        /* Application's initial state. */
        case APPGEN_STATE_INIT:
        {
            lcd_init();
            lcd_init();
            lcd_bl_on();
//            printf_lcd("TP5 IpGen 2023");
//            lcd_gotoxy(1,2);
//            printf_lcd("Einar Farinas");
            
            // Inititalisation du SPI DAC
            SPI_InitLTC2604();

            // Initialisation PEC12
            Pec12Init();

            // Initialisation S9
            S9Init();
            
            // Init RTCC
            I2C_InitMCP79411();
           
            // Initialisation du generateur
            GENSIG_Initialize(&LocalParamGen);

            // Initialisation du menu
            MENU_Initialize(&LocalParamGen);
            
            // Synchronisation des parametres remote avec locaux
            RemoteParamGen = LocalParamGen;
            
            // Demarrage du generateur de fonction
            GENSIG_UpdateSignal(&LocalParamGen);
            APPGEN_UpdateState(APPGEN_WAIT);
            
            // Active les timers
            DRV_TMR0_Start();
            DRV_TMR1_Start();
            break;
        }

        case APPGEN_STATE_SERVICE_TASKS:
        {
            LED_0Toggle();
            // Si des nouvelles donnees ont ete recues
            if (appgenData.newDataReceived == true)
            {
                appgenData.newDataReceived = false;
                // Decodage des donnees recues
                GetMessage(ReceiveMessageString, &RemoteParamGen, &SaveData);
                if(SaveData == true)
                {
                    RemoteParamGen.Magic = MAGIC;
                    I2C_WriteSEEPROM(&RemoteParamGen, 0x00, 16);
                    RemoteSave = true;
                    
                }
                SendMessage(MessageString, &RemoteParamGen, &SaveData);
            }
            if(RemoteSave == true)
            {
                count++;
                if(count == 1)
                {
                    MENU_DemandeSave();
                }
                if(count == 300)
                {
                    count = 0;
                    RemoteSave = false;
                }
            }
            // Détection de nouvelle adresse IP
            else if (flag_IP == true)
            {         
               //Variables
               count++;
               
               // Affichage de l'adresse IP
               if(count == 1)
               {
                   // Mise en forme LCD pour l'adresse IP
                   lcd_bl_on();
                   lcd_ClearLine(1);
                   lcd_gotoxy(1,2);
                   printf_lcd("   Adr. IP          ");
                   lcd_gotoxy(1,3);
                   printf_lcd(" IP:%03d.%03d.%03d.%03d ", ipAddr.v[0], ipAddr.v[1], ipAddr.v[2], ipAddr.v[3]); 
                   lcd_ClearLine(4);
               }
               if(count == 500)
               {
                    // Remise à zéro du flag IP
                    flag_IP = false;
                    count = 0;
               }
            }
            else
            {
                // Execution du menu en local ou remote
                // ====================================
                // Si USB connecte => mode remote
                if(TCPConnected == true)
                {
                    MENU_Execute(&RemoteParamGen, REMOTE);
                    NewParamGen = RemoteParamGen;
                }
                // Si non mode local
                else
                {
                    MENU_Execute(&LocalParamGen, LOCAL);
                    NewParamGen = LocalParamGen;
                }
            }
            
            
            // Execution du générateur avec les valeurs locales ou remote
            // Si les données sont différentes, mettre à jour le signal de sortie
            if((CheckUpdateParamGen.Forme!=NewParamGen.Forme)||
              (CheckUpdateParamGen.Amplitude!=NewParamGen.Amplitude)||
              (CheckUpdateParamGen.Offset!=NewParamGen.Offset))
            {
                GENSIG_UpdateSignal(&NewParamGen);
            }
            if(CheckUpdateParamGen.Frequence!=NewParamGen.Frequence)
            {
                GENSIG_UpdatePeriode(&NewParamGen);
            }
            // Sauvegarde des parametres precedents
            CheckUpdateParamGen = NewParamGen;
            
            APPGEN_UpdateState(APPGEN_WAIT);
            
            break;
        }
        
        case APPGEN_WAIT:
        {
            // Rien faire
            break;
        }
        /* TODO: implement your application state machine.*/
        

        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}
void APPGEN_ReadDatasFromTCPBuffer(uint8_t *SerialDatas)
{
    // Copie du buffer de l'USB dans le tableu de données
    strncpy((char*)ReceiveMessageString, (char*)SerialDatas, 29);
    // Set flag pour indique la réception d'une nouvelle donnée
    appgenData.newDataReceived = true;
}

void APPGEN_UpdateState(APPGEN_STATES NewState)
{
    appgenData.state = NewState;
}

void SetTCPFlag(void)
{
    TCPConnected = true;
}
void ResetTCPFlag(void)
{
    TCPConnected = false;
}
bool GetTCPFlagState(void)
{
    return TCPConnected;
}

void SetIP_Flag(void)
{
    flag_IP = true;
}
void ResetIP_Flag(void)
{
    flag_IP = false;
}



// Pour copier le message de l'adresse IP (tableau de app.c)
void Update_IP(uint8_t *adresse)
{
    // Variables locales
    
    strncpy((char*)ipAddr.v, (char*)adresse, 4);
}
/*******************************************************************************
 End of File
 */

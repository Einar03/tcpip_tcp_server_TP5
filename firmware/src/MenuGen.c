// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du générateur
// Traitement cyclique à 10 ms

#include <stdint.h>                   
#include <stdbool.h>
#include "MenuGen.h"
#include "Mc32gestI2cSeeprom.h"
#include "appgen.h"

S_ParamGen ParamDisplay; //= {SignalSinus, 100, 500, 0};

E_Menu_State menuState = Main_Menu;
E_Save_Menu_State saveMenu = FALSE;

// Initialisation du menu et des paramètres
void MENU_Initialize(S_ParamGen *pParam)
{
    lcd_bl_on();
	printf_lcd(" TP5 IpGen 2022-23");           
    lcd_gotoxy(1,2);
    printf_lcd("   Einar & Taulant");
    lcd_gotoxy(1,4);
    if(pParam -> Magic != MAGIC)
    {
        printf_lcd("    MEMORY ERROR ");
    }
    else
    {
        printf_lcd("    Param charged");
    }
    ParamDisplay.Forme = pParam -> Forme;
    ParamDisplay.Frequence = pParam -> Frequence;
    ParamDisplay.Amplitude = pParam -> Amplitude;
    ParamDisplay.Offset = pParam -> Offset;
}


// Execution du menu, appel cyclique depuis l'application
void MENU_Execute(S_ParamGen *pParam, bool local)
{
    static uint8_t position = 0;
    static uint8_t selected = FALSE;   //The option is selected or not
    // static uint8_t saveMenu = FALSE;
    char textSignal[4][11]={"Sine Wave  ", "Triangle   ", "Saw Tooth  ", "Square Wave"};
    int lcdPosition;
    static uint8_t saveMenuCounter = 0;
    static char cursor[4] = {BLANK, BLANK, BLANK, BLANK};
    uint8_t i;
    
    
    if(!local)
    {
        menuState = Remote_Menu;
    }
    
    // =========================================================================
    //Manage backlight
    if(Pec12.NoActivity == TRUE)
    {
        lcd_bl_off();
    }
    else
    {
        lcd_bl_on();
    }
    // =========================================================================
    //                           Gestion du menu
    // =========================================================================
    switch(menuState)
    {
        // ===========================================================
        //                      Menu principal
        // ===========================================================
        case Main_Menu:
        {
            // ===========================================
            //      Position du curseur avec l'encodeur
            // ===========================================
            // Incrementation du curseur (Descendre le curseur)
            // si il n'est pas dans la derniere ligne
            if(Pec12.Inc == TRUE && position < 3)
            {
                cursor[position + 1] = cursor[position];
                cursor[position]=BLANK;
                position++;
                Pec12ClearPlus();
            }
            // Reset le flag si le curseur est dans la derniere ligne
            else
            {
                Pec12ClearPlus();
            }
            // Decrementation du curseur (monter le curseur)
            // si il n'est pas dans la premiere ligne
            if(Pec12.Dec == TRUE && position > 0)
            {
                cursor[position - 1] = cursor[position];
                cursor[position]=BLANK;
                position--;
                Pec12ClearMinus();
            }
            // Reset le flag si le curseur est dans la premiere ligne
            else
            {
                Pec12ClearMinus();
            }
            // ===========================================
            //            Bouton de selection
            // ===========================================
            if(Pec12.OK == TRUE && selected == FALSE)
            {
                selected = TRUE;
                Pec12ClearOK();
            }
            
            // ===========================================
            //            Bouton de sauvegarde
            // ===========================================
            if(S9.LNG == TRUE && selected == FALSE)
            {
                S9ClearLNG();
                saveMenu = TRUE;
            }
            
            if(saveMenu == TRUE)
            {
                menuState = Save_Menu;
            }

            if(selected == TRUE)
            {
                cursor[position] = SELECTED;
                menuState = position;
               
                
                //ParamDisplay = ParamGen;
            }
            else
            {
                cursor[position] = NAVIGATION;
            }

            break;
        }
        // ===========================================================
        //              Modification de la forme du signal
        // ===========================================================
        case Wave_Menu:
        {
            // ===========================================
            //         Changement du signal affiche
            //  Singaux : 
            //  Sinus, triangle, dent de scie et carre
            // ===========================================
            if(Pec12.Inc == TRUE)
            {
                ParamDisplay.Forme++;
                if(ParamDisplay.Forme > 3){ParamDisplay.Forme = 0;}
                Pec12ClearPlus();
            }
            else if(Pec12.Dec == TRUE)
            {
                if(ParamDisplay.Forme == 0){ParamDisplay.Forme = 3;}
                else{ParamDisplay.Forme--;}        
                Pec12ClearMinus();
            }
            // ===========================================
            //       Sauvegarde du signal choisi
            //       et retour au menu principal
            // ===========================================
            if(Pec12.OK == TRUE)
            {
                pParam -> Forme = ParamDisplay.Forme;
                menuState = Main_Menu;
                selected = FALSE;
                Pec12ClearOK();
            }
            // ===========================================
            //       Retour au menu principal
            //       sans sauvegarde du signal choisi
            // ===========================================
            else if(Pec12.ESC == TRUE)
            {
                ParamDisplay.Forme = pParam -> Forme;
                menuState = Main_Menu;
                selected = FALSE;
                Pec12ClearESC();
            }
            break;
        }
        // ===========================================================
        //              Modification de la frequence
        // ===========================================================
        case Frequency_Menu:
        {
            // ===========================================
            //     Changement de la frequence affichee
            //     min = 20Hz   Max = 2kHz
            // ===========================================
            if(Pec12.Inc == TRUE)
            {
                ParamDisplay.Frequence += FREQUENCY_INC;
                if(ParamDisplay.Frequence > MAX_FREQUENCY){ParamDisplay.Frequence = MIN_FREQUENCY;}
                Pec12ClearPlus();
            }
            else if(Pec12.Dec == TRUE)
            {
                ParamDisplay.Frequence -= FREQUENCY_INC;
                if(ParamDisplay.Frequence < MIN_FREQUENCY){ParamDisplay.Frequence = MAX_FREQUENCY;}
                Pec12ClearMinus();
            }
            // ===========================================
            //       Sauvegarde de la frequence choisie
            //       et retour au menu principal
            // ===========================================
            if(Pec12.OK == TRUE)
            {
                pParam -> Frequence = ParamDisplay.Frequence;
                menuState = Main_Menu;
                selected = FALSE;
                Pec12ClearOK();
            }
            // ===========================================
            //       Retour au menu principal
            //    sans sauvegarde de la frequence choisie
            // ===========================================
            else if(Pec12.ESC == TRUE)
            {
                ParamDisplay.Frequence = pParam -> Frequence;
                menuState = Main_Menu;
                selected = FALSE;
                Pec12ClearESC();
            }
            break;
        }
        // ===========================================================
        //              Modification de l'amplitude
        // ===========================================================
        case Amplitude_Menu:
        {
            // ===========================================
            //     Changement de l'amplitude affichee
            //     min = 0 mV   Max = 10'000 mV
            // ===========================================
            if(Pec12.Inc == TRUE)
            {
                ParamDisplay.Amplitude += AMPLITUDE_INC;
                if(ParamDisplay.Amplitude > MAX_AMPLITUDE){ParamDisplay.Amplitude = MIN_AMPLITUDE;}
                Pec12ClearPlus();
            }
            else if(Pec12.Dec == TRUE)
            {
                ParamDisplay.Amplitude -= AMPLITUDE_INC;
                if(ParamDisplay.Amplitude < MIN_AMPLITUDE){ParamDisplay.Amplitude = MAX_AMPLITUDE;}
                Pec12ClearMinus();
            }
            // ===========================================
            //       Sauvegarde de l'amplitude choisie
            //       et retour au menu principal
            // ===========================================
            if(Pec12.OK == TRUE)
            {
                pParam -> Amplitude = ParamDisplay.Amplitude;
                menuState = Main_Menu;
                selected = FALSE;
                Pec12ClearOK();
            }
            // ===========================================
            //       Retour au menu principal
            //    sans sauvegarde de l'amplitude choisie
            // ===========================================
            else if(Pec12.ESC == TRUE)
            {
                ParamDisplay.Amplitude = pParam -> Amplitude;
                menuState = Main_Menu;
                selected = FALSE;
                Pec12ClearESC();
            }
            break;
        }
        // ===========================================================
        //              Modification de l'offset
        // ===========================================================
        case Offset_Menu:
        {
            // ===========================================
            //     Changement de l'offset affiche
            //     min = -5'000 mV   Max = +5'000 mV
            // ===========================================
            if(Pec12.Inc == TRUE && ParamDisplay.Offset < MAX_OFFSET)
            {
                ParamDisplay.Offset += OFFSET_INC;
                Pec12ClearPlus();
            }
            else if(Pec12.Dec == TRUE && ParamDisplay.Offset > MIN_OFFSET)
            {
                ParamDisplay.Offset -= OFFSET_INC;
                Pec12ClearMinus();
            }
            // ===========================================
            //       Sauvegarde de l'offset choisi
            //       et retour au menu principal
            // ===========================================
            if(Pec12.OK == TRUE)
            {
                pParam -> Offset = ParamDisplay.Offset;
                menuState = Main_Menu;
                selected = FALSE;
                Pec12ClearOK();
            }
            // ===========================================
            //       Retour au menu principal
            //    sans sauvegarde de l'offset choisi
            // ===========================================
            else if(Pec12.ESC == TRUE)
            {
                ParamDisplay.Offset = pParam -> Offset;
                menuState = Main_Menu;
                selected = FALSE;
                Pec12ClearESC();
            }
            break;  
        }
        // ===========================================================
        //                      Menu de sauvegarde
        // =========================================================== 
        case Save_Menu:
        {
            // Si appui long sur S9, sauvegarde des parametres
            // dans la memoire du uC
            if(S9.LNG == TRUE)
            {
                pParam -> Magic = MAGIC;
                I2C_WriteSEEPROM((uint8_t*)pParam, 0x00, 16);
                saveMenu = SAVED;
                S9ClearLNG();
            }
            // Si un autre bouton est appuye
            // annuler la sauvergarde des parametres
            else if((Pec12.Inc||Pec12.Dec||Pec12.OK||Pec12.ESC) == 1)
            {
                saveMenu = CANCELED;
            }
            // Si sauvegade ou pas des parametres 
            // retour au menu principal
            if((saveMenu == SAVED)||(saveMenu == CANCELED))
            {
                saveMenuCounter++;
                if(saveMenuCounter>=CONFIRM_TIME)
                {
                    saveMenuCounter = 0;
                    saveMenu = FALSE;
                    menuState = Main_Menu;
                    lcd_ClearLine(1);
                    lcd_ClearLine(2);
                    lcd_ClearLine(3);
                    lcd_ClearLine(4);
                    Pec12ClearInactivity();
                }
            }
            break;
        }
        case Remote_Menu:
        {
            // Changement du symbole du curseur pour le mode remote
            for(i=0; i<4; i++)
            {
                cursor[i] = REMOTE_SYMBOL;
            }
            if(local)
            {
                // Reset du symbole du curseur par defaut
                for(i=0; i<4; i++)
                {
                    cursor[i] = BLANK;
                }
                menuState = Main_Menu;
                saveMenu = FALSE;
            }
            ParamDisplay.Forme = pParam -> Forme;
            ParamDisplay.Frequence = pParam -> Frequence;
            ParamDisplay.Amplitude = pParam -> Amplitude;
            ParamDisplay.Offset = pParam -> Offset;
            break;
        }
        default:
        break;        
    }
    // =========================================================================
    //                     Mise a jour des parametres affichees
    // =========================================================================
    switch(saveMenu)
    {
        case Save:
        {
            lcd_bl_on();
            lcd_ClearLine(1);
            lcd_gotoxy(1,2);
            printf_lcd(" Save Parameters  ? ");
            lcd_gotoxy(1,3);
            printf_lcd("    (long press)    ");
            lcd_ClearLine(4);
            break;
        }
        case Saved:
        {
            MENU_DemandeSave();
            break;
        }
        case Cancelled:
        {
            lcd_bl_on();
            lcd_ClearLine(1);
            lcd_gotoxy(1,2);
            printf_lcd(" Operation Canceled ");
            lcd_ClearLine(3);
            lcd_ClearLine(4);
            break;
        }
        default:
        {
            lcd_gotoxy(1,1);
            printf_lcd("%cShape %c ", cursor[0], SEPARATOR);
            for (lcdPosition = 0; lcdPosition < 11; lcdPosition++)
            {
                printf_lcd("%c", textSignal[ParamDisplay.Forme][lcdPosition]);
            }
            lcd_gotoxy(1,2);
            printf_lcd("%cFrequ [Hz] %c %-4d", cursor[1], SEPARATOR, ParamDisplay.Frequence);
            lcd_gotoxy(1,3);
            printf_lcd("%cAmpli [mV] %c %-5d", cursor[2], SEPARATOR, ParamDisplay.Amplitude);
            lcd_gotoxy(1,4);
            printf_lcd("%cOffset [mV] %c %-5d", cursor[3], SEPARATOR, ParamDisplay.Offset);
            break;
        }
    }
}

void MENU_DemandeSave(void)
{
    lcd_bl_on();
    lcd_ClearLine(1);
    lcd_gotoxy(1,2);
    printf_lcd("#Parameters Saved# ");
    lcd_ClearLine(3);
    lcd_ClearLine(4);
}
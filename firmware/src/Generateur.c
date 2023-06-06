// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion  du générateur

// Prévu pour signal de 40 echantillons

// Migration sur PIC32 30.04.2014 C. Huber


#include "Generateur.h"
#include "DefMenuGen.h"
#include "Mc32gestSpiDac.h"
#include "math.h"
#include "driver/tmr/drv_tmr_static.h"
#include "Mc32gestI2cSeeprom.h"

// Variables globales
uint16_t SignalValues[MAX_ECH] = {0};

// Initialisation du  générateur
void  GENSIG_Initialize(S_ParamGen *pParam)
{
    // Variables locales
    //Recup val mémoire
    I2C_ReadSEEPROM((uint8_t*)pParam, 0x00, 16);
    
    if(pParam->Magic != MAGIC)
    {
        // Fréquence par défaut = 20
        pParam -> Frequence = INIT_FREQ;
        // Forme du signal par défaut = Sinus
        pParam -> Forme = INIT_FORM;
        // Amplitude par défaut = 0;
        pParam -> Amplitude = INIT_AMPLITUDE;
        // Offset par défaut = 0
        pParam -> Offset = INIT_OFFSET;
    }
}
  

// Mise à jour de la periode d'échantillonage
void  GENSIG_UpdatePeriode(S_ParamGen *pParam)
{
    // Variable locale
    uint16_t Val_Periode = 0;
    
    // Calcul valeur du prescaler du timer 3 pour la frequence choisie
    //                                    F_SYS            
    // Val periode = -------------------------------------------------------   - 1
    //                (Fréquence * Nombre d'echantillons * prescaler Timer3)
    Val_Periode = (float)F_SYS/(float)(pParam -> Frequence * MAX_ECH * PRESCALER) - 1;
    // Mise de la valeur du prescaler
    PLIB_TMR_Period16BitSet(TMR_ID_3, Val_Periode);
}

// Mise à jour du signal (forme, amplitude, offset)
void  GENSIG_UpdateSignal(S_ParamGen *pParam)
{
    // Variables locales
    uint8_t i = 0;              // pour la boucle for
    // 
    int16_t DAC_Amplitude = ((float)pParam -> Amplitude * (float)DEFAULT_OFFSET) / 10000.5;
    int16_t DAC_Offset = ((float)pParam -> Offset * (float)DEFAULT_OFFSET) / 10000.5;
    float pointValue = ((float)DEFAULT_OFFSET - (float)DAC_Amplitude);
    // Variable de 32bit pour le calcul de l'offset et savoir si il y eu un overflow
    int32_t CalculOffset = 0;
    
    // Calcul des points du signal choisi sans offset
    for(i=0; i<=MAX_ECH; i++)
    {
        switch(pParam -> Forme)
        {
            case SignalSinus:
                SignalValues[i] = ((float)DAC_Amplitude * sin((float)2 * M_PI * i/(float)MAX_ECH) + 0.5) + DEFAULT_OFFSET;
                //SignalValues[i] = 65535;
                break;
            case SignalTriangle:
                SignalValues[i] = pointValue + 0.5;
                if(i < (MAX_ECH / 2))
                {
                    pointValue = pointValue + ((float)DAC_Amplitude * (float)2 + (float)1) / (float)(MAX_ECH/2.0);
                }
                else
                {
                    pointValue = pointValue - ((float)DAC_Amplitude * (float)2 + (float)1) / (float)(MAX_ECH/2.0);
                }
                break;
            case SignalDentDeScie:
                SignalValues[i] = pointValue + 0.5;
                pointValue = pointValue + ((float)DAC_Amplitude * (float)2 + (float)1) / (float)(MAX_ECH - 1);
                break;
            case SignalCarre:
                if(i < (MAX_ECH / 2))
                {
                    SignalValues[i] = DAC_Amplitude + 1 + DEFAULT_OFFSET;;
                }
                else
                {
                    SignalValues[i] = -DAC_Amplitude + DEFAULT_OFFSET;
                }
                break;
            default :
                break;
        }
    }
    // Ajout de l'offset au signal plus fixation des limites max et min
    for(i=0; i<=MAX_ECH; i++)
    {
        CalculOffset = (int32_t)SignalValues[i] - (int32_t)DAC_Offset;
        if(CalculOffset > MAX_VAL)
        {
            CalculOffset = MAX_VAL;
        }
        else if(CalculOffset < MIN_VAL)
        {
            CalculOffset = MIN_VAL;
        }
        SignalValues[i] = (uint16_t)CalculOffset;
    }
}


// Execution du générateur
// Fonction appelée dans Int timer3 (cycle variable variable)

// Version provisoire pour test du DAC à modifier
void  GENSIG_Execute(void)
{
    LED7_W = 1;
    // Compteur pour la selection et envoie des points
    static uint16_t EchNb = 0;
    // Envoie des points du signal au DAC avec le SPI
    SPI_WriteToDac(0, SignalValues[EchNb]);      // sur canal 0
    // Comptage de 0 à 99 avec EchNb
    //EchNb++;
    EchNb = (EchNb + 1) % MAX_ECH;
    LED7_W = 0;
}

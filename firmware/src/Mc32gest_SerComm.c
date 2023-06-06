// Mc32Gest_SerComm.C
// fonction d'�mission et de r�ception des message
// transmis en USB CDC
// Canevas TP4 SLO2 2015-2015


#include "app.h"
#include "Mc32gest_SerComm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


// Fonction de reception  d'un  message
// Met � jour les param�tres du generateur a partir du message recu
// Format du message
//  !S=TF=2000A=10000O=+5000W=0#
//  !S=TF=2000A=10000O=-5000W=1#


void GetMessage(uint8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo)
{
    // Variable locales
    char *PtValue;
    char CharValue[5];
    
    // Controle le debut '!' et la fin du message '#'
    // avant de recuperer les donnees du message
    if((USBReadBuffer[0] == '!') && (USBReadBuffer[27] == '#'))
    {
        // Forme du signal
        // Pour chercher le texte "S=" dans la chaine de caract�res
        // Retourne la position du d�but du texte
        // Exemple pour "S=" :
        // !S=TF=2000A=10000O=-5000W=1#
        // retourne la valeur 1
        PtValue = strstr((char*)USBReadBuffer, "S=");
        // Incr�menter le pointeur pour se placer au caract�re de la forme
        PtValue += 2;
        // R�cuperation d'un caract�re (parametre 3) de la forme dans CharValue
        strncpy(CharValue, PtValue, 1);
        
        // Switch en fonction de la forme r�cuper�e
        // et sauvegarde dans la structure
        switch(CharValue[0])
        {
            case 'S':
                pParam->Forme = SignalSinus;
                break;
            case 'T':
                pParam->Forme = SignalTriangle;
                break;
            case 'D':
                pParam->Forme = SignalDentDeScie;
                break;
            case 'C':
                pParam->Forme = SignalCarre;
                break;
            default:
                break;
        }
        // Sauvegarde de la valeur de la fr�quence
        pParam->Frequence = SearchAndGetValue((char*)USBReadBuffer, "F=", 4);
        
        // Sauvegarde de la valeur de l'amplitude
        pParam->Amplitude = SearchAndGetValue((char*)USBReadBuffer, "A=", 5);
        
        // Sauvegarde de la valeur de l'offset
        pParam->Offset = SearchAndGetValue((char*)USBReadBuffer, "O=", 5);
        
        // Recup�ration de la sauvegarde
        PtValue = strstr((char*)USBReadBuffer, "W=");
        PtValue += 3;
        // conversion de la valeur de sauvegarde en int 
        // Si �gal 0 save � false sinon true
        if((atoi(PtValue)) == 0)
        {
            *SaveTodo = false;
        }
        else
        {
            *SaveTodo = true;
        }
    }
}
int16_t SearchAndGetValue(char *USBReadBuffer, const char *TextToSearch , uint8_t NbCharToGet)
{
    // Variable locales
    char *PtValue;
    char CharValue[5];
    // Pour chercher le texte "S=" dans la chaine de caract�res
    // Retourne la position du d�but du texte
    // Exemple pour "S=" :
    // !S=TF=2000A=10000O=-5000WP=1#
    // retourne la valeur 1
    PtValue = strstr((char*)USBReadBuffer, TextToSearch);
    
    // Incr�menter le pointeur pour se placer au caract�re de la valeur
    PtValue = PtValue + (sizeof(TextToSearch)-2);
    // R�cuperation des caract�res (parametre 3) de la forme dans CharValue
    strncpy(CharValue, PtValue, NbCharToGet);
    // retourne la valeur en int de la valeur en string
    return atoi(CharValue);
}

// GetMessage
// Fonction d'envoi d'un  message
// Rempli le tampon d'�mission pour USB en fonction des param�tres du g�n�rateur
// Format du message
// !S=TF=2000A=10000O=+5000D=25WP=0#
// !S=TF=2000A=10000O=+5000D=25WP=1#    // ack sauvegarde
void SendMessage(uint8_t *USBSendBuffer, S_ParamGen *pParam, bool *Saved)
{
    // Varaibles locales
    char CharValue[6] = "00000";
    
    // Conversion Forme signal en caractere et �criture dans le buffer
    switch(pParam->Forme)
    {
        case SignalSinus:
        {
            USBSendBuffer[3] = 'S';
            break;
        }
        case SignalTriangle:
        {
            USBSendBuffer[3] = 'T';
            break;
        }
        case SignalDentDeScie:
        {
            USBSendBuffer[3] = 'D';
            break;
        }
        case SignalCarre:
        {
            USBSendBuffer[3] = 'C';
            break;
        }
        default:
        {
            USBSendBuffer[3] = '0';
            break;
        }
        
    }
    // Conversion de la frequence en string et ecriture dans el buffer
    sprintf(CharValue, "%04d", pParam->Frequence);
    WriteMessageValue(6,4,USBSendBuffer,CharValue);
    // Convertion de l'amplitude en string et ecriture dans el buffer
    sprintf(CharValue, "%05d", pParam->Amplitude);
    WriteMessageValue(12,5,USBSendBuffer,CharValue);
    // Convertion de l'offset en string et ecriture dans el buffer
    sprintf(CharValue, "%+05d", pParam->Offset);
    WriteMessageValue(19,5,USBSendBuffer,CharValue);
    
    if(*Saved == true)
    {
        USBSendBuffer[27] = '1';
    }
    else
    {
        USBSendBuffer[27] = '0';
    }
    // Mettre � jour le tableu d'envoi dans app.c pour l'envoyer avec l'USB
    Update_Message(USBSendBuffer);
    // Forcer l'�tat de l'USB en mode �criture
//    APP_UpdateState(APP_STATE_SCHEDULE_WRITE);
    // Autoriser l'�criture dans le USB
    SetSendFlag();
    
    
} // SendMessage

void WriteMessageValue(uint8_t Index, uint8_t ValSize, uint8_t *Message, char *Value)
{
    uint8_t i = 0;
    
    for(i = 0; i < ValSize; i++)
    {
        Message[i+Index] = Value[i];
    }
}


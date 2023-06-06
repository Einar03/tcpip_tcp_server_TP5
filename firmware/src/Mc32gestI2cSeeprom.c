//--------------------------------------------------------
// Mc32gestI2cEEprom.C
//--------------------------------------------------------
// Gestion I2C de la SEEPROM du MCP79411 (Solution exercice)
//	Description :	Fonctions pour EEPROM MCP79411
//
//	Auteur 		: 	C. HUBER
//      Date            :       26.05.2014
//	Version		:	V1.0
//	Compilateur	:	XC32 V1.31
// Modifications :
//
/*--------------------------------------------------------*/



#include "Mc32gestI2cSeeprom.h"
#include "Mc32_I2cUtilCCS.h"


// Définition pour MCP79411
#define MCP79411_EEPROM_R    0xAF         // MCP79411 address for read
#define MCP79411_EEPROM_W    0xAE         // MCP79411 address for write
// La EEPROM du 79411 est de 1 Kbits donc 128 octets
#define MCP79411_EEPROM_BEG   0x00         // addr. début EEPROM
#define MCP79411_EEPROM_END   0x7F         // addr. fin EEPROM

// Definitions du bus (pour mesures)
// #define I2C-SCK  SCL2/RA2      PORTAbits.RA2   pin 58
// #define I2C-SDA  SDa2/RA3      PORTAbits.RA3   pin 59




// Initialisation de la communication I2C et du MCP79411
// ------------------------------------------------

void I2C_InitMCP79411(void)
{
   bool Fast = true;
   i2c_init( Fast );
} //end I2C_InitMCP79411

// Ecriture d'un bloc dans l'EEPROM du MCP79411 
void I2C_WriteSEEPROM(void *SrcData, uint32_t EEpromAddr, uint16_t NbBytes)
{
    // Variables locales
    // ======================
    // Pointeur de 8 bits pour envoyer chaque 8 bits à l'EEPROM de la RTCC
    uint8_t *ptByteData = SrcData;
    uint32_t AdresseCnt = EEpromAddr;
    uint8_t i = 0;
    bool ack;
   
    // Start 
    do
    {
        i2c_start();
        // Ecriture du byte de controle de la RTCC en mode W
        ack = i2c_write(MCP79411_EEPROM_W);        
    }while(!ack); // Attente d'un ACK
    
    do
    {
        ack = i2c_write(EEpromAddr); // Adresse de début de la mémoire
    }while(!ack);

    // enovie le nombre de bytes
    for(i=0; i<NbBytes;i++)
    {
        do
        {
            ack = i2c_write(*ptByteData); // Envoie du byte
        }while(!ack);  
        // Incrémetation du pointeur de datas pour sélecionnner le prochain byte
        ptByteData++;
        AdresseCnt++;
        // Si dernier byte de la page
        if((AdresseCnt % 8) == 0)
        {
            // Faire un stop
            i2c_stop();
            // Faire un nouveau start avec la première adresse de la page suivante
            do
            {
                i2c_start();
                ack = i2c_write(MCP79411_EEPROM_W);        
            }while(!ack);
            do
            {
                ack = i2c_write(AdresseCnt); // Adresse dans la mémoire
            }while(!ack);
        }
    }
    i2c_stop(); // I2C stop    
} // end I2C_WriteSEEPROM

// Lecture d'un bloc dans l'EEPROM du MCP79411
void I2C_ReadSEEPROM(void *DstData, uint32_t EEpromAddr, uint16_t NbBytes)
{ 
    // Variables locales
    // ======================
    // Pointeur de 8 bits pour envoyer chaque 8 bits à l'EEPROM de la RTCC
    uint8_t *ptByteData = DstData;
    uint8_t AdresseCnt = 0;
    uint8_t i = 0;
    
    bool ack;
    
    AdresseCnt = EEpromAddr;
    
    // Start
    do{
        i2c_start();
        // Mode écriture
        ack = i2c_write(MCP79411_EEPROM_W);        
    }while(!ack);
    // Adresse de debut de lecture
    do{
        ack = i2c_write(EEpromAddr);
    }while(!ack);
    // Restart
    do{
        i2c_reStart();
        // Mode lecture
        ack = i2c_write(MCP79411_EEPROM_R);
    }while(!ack);
    
    for ( i = 0 ; i < NbBytes; i++ )
    {
        AdresseCnt++;
        // Si dernier byte envoie d'un Non ack
        if (i == (NbBytes - 1))
        {
            *ptByteData = i2c_read(0); //NO ACK  
        }
        // Si dernier byte de la page
        else if((AdresseCnt % 8) == 0)
        {
            // Faire un no ACK
            *ptByteData = i2c_read(0); //NO ACK 
            // Faire un stop
            i2c_stop();
            // Faire un nouveau start avec la première adresse de la page suivante
            do{
                i2c_start();
                ack = i2c_write(MCP79411_EEPROM_W);        
            }while(!ack);
            do{
                ack = i2c_write(AdresseCnt);
            }while(!ack);
            // Restart
            do{
                i2c_reStart();
                ack = i2c_write(MCP79411_EEPROM_R);
            }while(!ack);
        }
        // Ecriture d'un byte, envoi d'un ACK
        else
        {
            *ptByteData = i2c_read(1); // ACK
        }
        ptByteData++;
    }
    i2c_stop();
} // end I2C_ReadSEEPROM
   





 




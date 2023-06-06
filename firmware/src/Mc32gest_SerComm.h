#ifndef Mc32Gest_SERCOMM_H
#define Mc32Gest_SERCOMM_H
/*--------------------------------------------------------*/
// Mc32Gest_SerComm.h
/*--------------------------------------------------------*/
//	Description :	emission et reception specialisee
//			pour TP4 2015-2016
//
//	Auteur 		: 	C. HUBER
//
//	Version		:	V1.2
//	Compilateur	:	XC32 V1.40 + Harmony 1.06
//
/*--------------------------------------------------------*/

#include <stdint.h>
#include "DefMenuGen.h"

/*--------------------------------------------------------*/
// Prototypes des fonctions 
/*--------------------------------------------------------*/

void SendMessage(uint8_t *USBSendBuffer, S_ParamGen *pParam, bool *Saved);

int16_t SearchAndGetValue(char *USBReadBuffer, const char *, uint8_t NbCharToGet);

void GetMessage(uint8_t *USBReadBuffer, S_ParamGen *pParam, bool *SaveTodo);

void WriteMessageValue(uint8_t Index, uint8_t ValSize, uint8_t *Message, char *Value);


#endif

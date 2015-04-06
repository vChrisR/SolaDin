/*
  Soladin.cpp - Library for comunicating to a Soladin 600.
  Created by  teding,  November , 2011.
  Released into the public domain.

  Modified by Christiaan Roeleveld, April 2015. To make it compatible with Arduino for ESP8266

*/



  #if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #else
  #include "WProgram.h"
  #endif
//#include "WProgram.h"
#include "Soladin.h"




void Soladin::begin(Stream *theSerial){
     _serial = theSerial;
}
// set TxBuf to 0, because not all bytes will be write conCat()
Soladin::Soladin() {					// constructor, 
    for (int i=0 ; i < 9 ; i++) {
        TxBuf[i] = 0x00 ;}            
 }       

// this api will call the soladin 
bool Soladin::query(uint8_t Cmdo){
	conCat(Cmdo,null,TxBuf);
	return(txRx(Cmdo));
}

// the overload version that takes the day arg, for de historical data
bool Soladin::query(uint8_t Cmdo,uint8_t _day){  // overload
	if ((Cmdo == HSD) && ( _day >= 0) && ( _day <= 9)) {
		conCat(Cmdo, _day, TxBuf);
	return(txRx(Cmdo));
	}
}

// send the commando, and process the result
bool Soladin::txRx(uint8_t Cmdo) {
	sndBuf(9,TxBuf);
	delay(dly);   // some time to get the Rx buffer filled
	int res = PolRxBuf(Cmdo); //  poll the rx buffer
	if (res == 0) {
		switch( Cmdo) {  // process the incomming info to variables
			case FWI:
				FW_deCode(RxBuf);  
			break;
			case DVS:
				DS_deCode(RxBuf); 
			break;
			case HSD:
				HD_deCode(RxBuf); 
			break;
			case RMP:
				MP_deCode(RxBuf); 
			break;
			}
		return(true);	
		}
	else  return(false);	
	//RxError = res;
}  

void Soladin::conCat(uint8_t _cmd, uint8_t _act, char *OutBuf)  {
	if (_cmd == PRB) {
		OutBuf[0] = null ; } // Destination Address
	else {
		OutBuf[0] = dest ; } // Destination Address
		OutBuf[4] = _cmd ;   // Comando
	if (_cmd == ZMP) {
		OutBuf[5] = Ac_zmp ; }  // action reset max power
	else {
		OutBuf[5] = _act ;  } // no action or Day @ histroical Data
	OutBuf[8] = 0x00;
	for (int i=0 ; i<8; i++)  {
		OutBuf[8] = OutBuf[8] + OutBuf[i] ;  //Checksum
		}
}

// Send out txbuffer
void Soladin::sndBuf(int size, char *OutBuf){
	for (int i=0; i <size ; i++){
		_serial->write(OutBuf[i]);
	}
}

// poll Rxbuffer and check for valid response
int Soladin::PolRxBuf(uint8_t Cmd){
    int i = 0 ;
    int error = 0 ; //OK
    uint32_t start = millis();
    while ((millis()- start) < TimeOut ){
		if (_serial->available() == 0) {
			error = 3 ;  //time out          
			}	
		else {
			while (_serial->available() > 0 &&  i <= 31 ){ 
				RxBuf[i] = _serial->read();
				i++ ;
				}   
			if (byte(RxBuf[2]) != dest) {    
				error = 1;     // not from slave 
				}  
			else if (byte(RxBuf[4]) != byte(Cmd) ) {
				error = 2 ;  //error 2; Wrong command response
				}  
			else error = 0;  // must be ok then	
			break;
			}
		}
	RxLgth = i ;
	RxError = error ;
	return error ;
}    
    

void Soladin::DS_deCode(char *InBuf) {
    Flag = word(InBuf[7],InBuf[6]);
    PVvolt = word(InBuf[9],InBuf[8]);
    PVamp = word(InBuf[11],InBuf[10]);
    Gridfreq = word(InBuf[13],InBuf[12]);
    Gridvolt = word(InBuf[15],InBuf[14]);
    Gridpower = word(InBuf[19],InBuf[18]); 
    Totalpower  = ((unsigned long)InBuf[22] & 0xFF) << 16;
    Totalpower += ((unsigned long)InBuf[21] & 0xFF) << 8;
    Totalpower += ((unsigned long)InBuf[20] & 0xFF) << 0;
    DeviceTemp = InBuf[23];
    TotalOperaTime  = ((unsigned long)InBuf[27] & 0xFF) << 24;
    TotalOperaTime += ((unsigned long)InBuf[26] & 0xFF) << 16;
    TotalOperaTime += ((unsigned long)InBuf[25] & 0xFF) << 8;
    TotalOperaTime += ((unsigned long)InBuf[24] & 0xFF) << 0;
} 

void Soladin::FW_deCode(char *InBuf) {
    FW_ID = InBuf[13];
    FW_version = word(InBuf[16],InBuf[15]);
    FW_date = word(InBuf[18],InBuf[17]);
}

void Soladin::MP_deCode(char *InBuf) {
    MaxPower = word(InBuf[25],InBuf[24]);
}

void Soladin::HD_deCode(char *InBuf) {
    DailyOpTm = InBuf[5];
    Gridoutput = word(InBuf[7],InBuf[6]);
}

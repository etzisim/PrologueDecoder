#include "Arduino.h"
#include <PrologueDecoder.h>

const char SYNC_KEY[] = {1,0,0,1,1,0,0,1};

enum {IDLE,SYNCING,SYNCED};

PrologueDecoder::PrologueDecoder(){
	reset();
	detected = false;
}

void PrologueDecoder::pulse(word width, bool high){

	switch (state){
	
	case IDLE:
		{
			if (high && ( (width > 330) && (width < 530))){
				// Might be the start of a sync bit
				state = SYNCING;
			}
		}
	case SYNCING:
		{
			if (!high && ( (width > 7500) && (width < 10000))){
				state = SYNCED;
			} else {
				reset();
			}
		}
	case SYNCED:
		{
			if ( high && !(halftime%2) ){
				// High part of bit
				if  ( (width < 330) || (width > 530) ){
					reset();
				}

			} else if ( !high && (halftime%2) ){
				// Low part of bit
				if ( (width > 1500) && (width < 2500)){
					// Short low: "0"
					bits[bitN++] = 0;
				} else if ( (width > 3500) && (width < 4500)){
					// Long low: "1"
					bits[bitN++] = 1;
				}else{
					/* Don't worry about it... */
				}

			} else {
				reset();
			}

			if (bitN == 36){
				detected = true;
				decode();
				reset();
			}

			halftime++;

		break;

		}
	}
}

bool PrologueDecoder::hasDetected(){
	if (detected){
		detected = false;
		return true;
	}
	return false;
}

Data PrologueDecoder::getData(){
	return data;
}

unsigned long binaryNumber(byte b[], int size){
	unsigned long num = 0;
	for (int i = 0; i < size; i++){
		num = 2*num + b[i];
	}

	return num;
}

void PrologueDecoder::decode(){

	data.ID = binaryNumber(bits, 4);
	data.rollingCode = binaryNumber(bits+4, 8);
	data.channel = binaryNumber(bits+14, 2);
	data.temp = (bits[16]*(-2)+1.0)*binaryNumber(bits+17, 11)/10.0;
}


void PrologueDecoder::reset(){
	state = IDLE;
	bitN = 0;
	halftime = 0;
	for (int i = 0; i < 36; i++){
		bits[i] = -1;
	}
}

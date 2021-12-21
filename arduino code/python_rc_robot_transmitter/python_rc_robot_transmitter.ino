#include "SPI.h"
#include "NRFLite.h"

const static uint8_t RADIO_ID = 1;             // Our radio's id.
const static uint8_t DESTINATION_RADIO_ID = 0; // Id of the radio we will transmit to.
const static uint8_t PIN_RADIO_CE = 7;
const static uint8_t PIN_RADIO_CSN = 8;

struct RadioPacket {
    uint8_t FromRadioId;
    uint8_t dir;
    uint8_t velocity;
};

NRFLite _radio;
RadioPacket _radioData;

void setup(){
    Serial.begin(115200);
    
    if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN)){
        Serial.println("Cannot communicate with radio");
        while (1);
    }
    _radioData.FromRadioId = RADIO_ID;
}

uint8_t dir = 0;
uint8_t velocity = 0;
void loop(){
  if(Serial.available() > 0){
    uint8_t data = Serial.read();
    Serial.println(dir);
    dir = data & 0x07;
    Serial.println(dir);
    velocity = (data & 120) >> 3;
    Serial.println(velocity);
    _radioData.dir = dir;
    _radioData.velocity = velocity;
    if (_radio.send(DESTINATION_RADIO_ID, &_radioData, sizeof(_radioData))){
        Serial.println("Success");
    }
    else{
        Serial.println("Failed");
    }
  }
}

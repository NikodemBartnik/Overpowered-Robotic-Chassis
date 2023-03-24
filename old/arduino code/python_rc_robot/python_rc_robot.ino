#include <SPI.h>
#include <mcp2515.h>
#include "SPI.h"
#include "NRFLite.h"

const static uint8_t RADIO_ID = 0;       // Our radio's id.  The transmitter will send to this id.
const static uint8_t PIN_RADIO_CE = 7;
const static uint8_t PIN_RADIO_CSN = 8;

 #define P_MIN -12.5f
 #define P_MAX 12.5f
 #define V_MIN -50.0f
 #define V_MAX 50.0f
 #define KP_MIN 0.0f
 #define KP_MAX 500.0f
 #define KD_MIN 0.0f
 #define KD_MAX 5.0f
 #define T_MIN -10.0f
 #define T_MAX 10.0f

struct can_frame canMsg;
MCP2515 mcp2515(10);


struct RadioPacket {
    uint8_t FromRadioId;
    uint8_t dir;
    uint8_t velocity;

};

NRFLite _radio;
RadioPacket _radioData;
void setup() {
  Serial.begin(115200);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_1000KBPS);
  mcp2515.setNormalMode();
  setMotorZero(1);
  setMotorZero(2);
  enterMotorMode(1);
  enterMotorMode(2);

  if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN)){
        Serial.println("Cannot communicate with radio");
        while (1);
    }
}
uint8_t last_dir = 0;
uint8_t dir = 0;
uint8_t velocity = 0;
void loop() {

  GetDataFromRadio();

  if(dir != last_dir){
    Serial.println(dir);
    Serial.println(velocity);
    if(dir == 1){
      sendToMotor(1, 0, -velocity*6.28, 0, 1, 0);
      sendToMotor(2, 0, velocity*6.28, 0, 1, 0);
    }
    if(dir == 2){
      sendToMotor(1, 0, velocity*6.28, 0, 1, 0);
      sendToMotor(2, 0, velocity*6.28, 0, 1, 0);
    }
    if(dir == 3){
      sendToMotor(1, 0, -velocity*6.28, 0, 1, 0);
      sendToMotor(2, 0, -velocity*6.28, 0, 1, 0);
    }
    if(dir == 4){
      sendToMotor(1, 0, velocity*6.28, 0, 1, 0);
      sendToMotor(2, 0, -velocity*6.28, 0, 1, 0);
    }
    if(dir == 0){
      sendToMotor(1, 0, 0, 0, 1, 0);
      sendToMotor(2, 0, 0, 0, 1, 0);
    }
  last_dir = dir;
  }
}

void enterMotorMode(int mot_id){
  struct can_frame cf1;
  cf1.can_id  = mot_id;
  cf1.can_dlc = 8;
  cf1.data[0] = 0xFF;
  cf1.data[1] = 0xFF;
  cf1.data[2] = 0xFF;
  cf1.data[3] = 0xFF;
  cf1.data[4] = 0xFF;
  cf1.data[5] = 0xFF;
  cf1.data[6] = 0xFF;
  cf1.data[7] = 0xFC;
  mcp2515.sendMessage(&cf1);
}

void setMotorZero(int mot_id){
  struct can_frame cf2;
  cf2.can_id  = mot_id;
  cf2.can_dlc = 8;
  cf2.data[0] = 0xFF;
  cf2.data[1] = 0xFF;
  cf2.data[2] = 0xFF;
  cf2.data[3] = 0xFF;
  cf2.data[4] = 0xFF;
  cf2.data[5] = 0xFF;
  cf2.data[6] = 0xFF;
  cf2.data[7] = 0xFE;
  mcp2515.sendMessage(&cf2);
}

void sendToMotor(int mot_id, float pos, float vel, float kp, float kd, float torq){
  struct can_frame cf;

  unsigned int con_pos = float_to_uint(constrain(pos, P_MIN, P_MAX), P_MIN, P_MAX, 16);
  unsigned int con_vel = float_to_uint(constrain(vel, V_MIN, V_MAX), V_MIN, V_MAX, 12);
  unsigned int con_kp = float_to_uint(constrain(kp, KP_MIN, KP_MAX), KP_MIN, KP_MAX, 12);
  unsigned int con_kd = float_to_uint(constrain(kd, KD_MIN, KD_MAX), KD_MIN, KD_MAX, 12);
  unsigned int con_torq = float_to_uint(constrain(torq, T_MIN, T_MAX), T_MIN, T_MAX, 12);
  cf.can_id  = mot_id;
  cf.can_dlc = 8;
  cf.data[0] = con_pos>>8;
  cf.data[1] = con_pos & 0xFF;
  cf.data[2] = con_vel >> 4;
  cf.data[3] = ((con_vel&0xF)<<4) | (con_kp>>8);
  cf.data[4] = con_kp&0xFF;
  cf.data[5] = con_kd>>4;
  cf.data[6] = ((con_kd&0xF)<<4) | (con_torq>>8);
  cf.data[7] = con_torq&0xFF;
  mcp2515.sendMessage(&cf);
}


//Function by Ben Katz: 
//https://os.mbed.com/users/benkatz/code/CanMasterTest//file/d24fd64d1fcb/math_ops.cpp/
int float_to_uint(float x, float x_min, float x_max, int bits){
    // Converts a float to an unsigned int, given range and number of bits 
    float span = x_max - x_min;
    float offset = x_min;
    unsigned int pgg = 0;
    if(bits == 12){
      pgg = (unsigned int) ((x-offset)*4095.0/span);
    }else if(bits == 16){
      pgg = (unsigned int) ((x-offset)*65535.0/span);
    }
    return pgg;
}

void GetDataFromRadio(){
  while (_radio.hasData()){
        _radio.readData(&_radioData);
        dir = _radioData.dir;
        velocity = _radioData.velocity;
    }
}

#include <SPI.h>
#include <mcp2515.h>

 #define P_MIN -12.5f
 #define P_MAX 12.5f
 #define V_MIN -45.0f
 #define V_MAX 45.0f
 #define KP_MIN 0.0f
 #define KP_MAX 500.0f
 #define KD_MIN 0.0f
 #define KD_MAX 5.0f
 #define T_MIN -10.0f
 #define T_MAX 10.0f

struct can_frame canMsg;
struct can_frame turnOff;
MCP2515 mcp2515(10);
String command; 
float serial_motor_values[5];


void setup() {

  turnOff.can_id  = 0x001;
  turnOff.can_dlc = 8;
  turnOff.data[0] = 0xFF;
  turnOff.data[1] = 0xFF;
  turnOff.data[2] = 0xFF;
  turnOff.data[3] = 0xFF;
  turnOff.data[4] = 0xFF;
  turnOff.data[5] = 0xFF;
  turnOff.data[6] = 0xFF;
  turnOff.data[7] = 0xFD;
  
  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_1000KBPS);
  mcp2515.setNormalMode();
  motorZero(1);
  motorZero(2);
  motorMode(1);
  motorMode(2);
  //sendToMotor(0,0,0,0,0);
  //sendToMotor(0.05f, 0.0f, 100.0f, 0.5f, 0.0f);
}

float motor_position = 0;


void loop() {
  char buf[50];
  if(Serial.available() > 0){
    command = Serial.readString();
    command.trim();
    command.toCharArray(buf, 50);
    char *i;
     serial_motor_values[0] = atof(strtok_r(buf, ",", &i));
    for(int a = 0; a < 4; a++){
      serial_motor_values[a+1] = atof(strtok_r(NULL, ",", &i));
    }
  }

  sendToMotor(1, serial_motor_values[0], -serial_motor_values[1], serial_motor_values[2], serial_motor_values[3], serial_motor_values[4]);
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print(canMsg.data[0], HEX); // print ID
    Serial.print(" "); 
   // Serial.print(canMsg.can_dlc, HEX); // print DLC
   // Serial.print(" ");

    Serial.print(uint_to_float((canMsg.data[1] << 8) | canMsg.data[2], P_MIN, P_MAX, 16));
    Serial.print(",");
    Serial.print(uint_to_float((canMsg.data[3] << 4) | (canMsg.data[4]>>4), V_MIN, V_MAX, 12));
    Serial.print(",");
    Serial.print(uint_to_float(((canMsg.data[4] &0xF)<<8) | canMsg.data[5], P_MIN, P_MAX, 12));
    Serial.println();      
   
  }
  sendToMotor(2, serial_motor_values[0], serial_motor_values[1], serial_motor_values[2], serial_motor_values[3], serial_motor_values[4]);


if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print(canMsg.data[0], HEX); // print ID
    Serial.print(" "); 
   // Serial.print(canMsg.can_dlc, HEX); // print DLC
   // Serial.print(" ");

    Serial.print(uint_to_float((canMsg.data[1] << 8) | canMsg.data[2], P_MIN, P_MAX, 16));
    Serial.print(",");
    Serial.print(uint_to_float((canMsg.data[3] << 4) | (canMsg.data[4]>>4), V_MIN, V_MAX, 12));
    Serial.print(",");
    Serial.print(uint_to_float(((canMsg.data[4] &0xF)<<8) | canMsg.data[5], P_MIN, P_MAX, 12));
    Serial.println();      
   
  }
  delay(25);
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

void motorMode(int mot_id){
  struct can_frame cf;
  cf.can_id  = mot_id;
  cf.can_dlc = 8;
  cf.data[0] = 0xFF;
  cf.data[1] = 0xFF;
  cf.data[2] = 0xFF;
  cf.data[3] = 0xFF;
  cf.data[4] = 0xFF;
  cf.data[5] = 0xFF;
  cf.data[6] = 0xFF;
  cf.data[7] = 0xFC;
  mcp2515.sendMessage(&cf);
}

void motorZero(int mot_id){
  struct can_frame cf;
  cf.can_id  = mot_id;
  cf.can_dlc = 8;
  cf.data[0] = 0xFF;
  cf.data[1] = 0xFF;
  cf.data[2] = 0xFF;
  cf.data[3] = 0xFF;
  cf.data[4] = 0xFF;
  cf.data[5] = 0xFF;
  cf.data[6] = 0xFF;
  cf.data[7] = 0xFE;
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

float uint_to_float(unsigned int x_int, float x_min, float x_max, int bits){
  float span = x_max - x_min;
  float offset = x_min;
  float pgg = 0;
  if(bits == 12){
    pgg = ((float)x_int)*span/4095.0 + offset;
  }else if(bits == 16){
    pgg = ((float)x_int)*span/65535.0 + offset;
  }
  return pgg;
}

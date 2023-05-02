#include <SPI.h>
#include <mcp2515.h>

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


void setup() {

  
  while (!Serial);
  Serial.begin(115200);
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_1000KBPS);
  mcp2515.setNormalMode();
  enterMotorMode(1);
  zeroMotor(1);
  enterMotorMode(2);
  zeroMotor(2);
  delay(500);
  Serial.println("test1");
}

void loop() {
  readCommand();
  readFromMotor();
}

void enterMotorMode(int mot_id){
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

void zeroMotor(int mot_id){
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


void readCommand(){
  if(Serial.available() > 0){
    String command = Serial.readStringUntil('\n');
    int breakPosition = command.indexOf(" ");
    float speedA = NULL;
    float speedB = NULL;

    String firstLetter = command.substring(0, 1);
    if(firstLetter.equals("M")){
      Serial.println("Mode selection");
      String commandNumber = command.substring(1, 2);
      if(commandNumber.equals("1")){
        
      }else if(commandNumber.equals("2")){

      }
      
    }else if(firstLetter.equals("A")){
      speedA = (command.substring(1, breakPosition)).toFloat();
      Serial.print("Speed A: ");
      Serial.print(speedA);
      Serial.println("RPM");
      if(speedA == 0){
      sendToMotor(1, 0, speedA, 0, 0, 0);  
      }else{
      sendToMotor(1, 0, speedA, 0, 10, 0);
      }
      if(command.substring(breakPosition+1, breakPosition+2).equals("B")){
      speedB = (command.substring(breakPosition+2, command.length())).toFloat();
      Serial.print("Speed B: ");
      Serial.print(speedB);
      Serial.println("RPM");
      if(speedA == 0){
      sendToMotor(2, 0, speedB, 0, 0, 0);  
      }else{
      sendToMotor(2, 0, -speedB, 0, 10, 0);
      }
    }
    }
  }
}


void readFromMotor(){
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
}

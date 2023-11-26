#include "esp32_can.h"            // https://github.com/collin80/esp32_can AND https://github.com/collin80/can_common

#define CANPID_SPEED        0x0D
#define CAN_REQST_ID        0x7DF 
#define CAN_REPLY_ID        0x7E8

uint16_t speed;


void setup() {
  Serial.begin(115200);
  Serial.println("Boot");

  pinMode(27, OUTPUT); // THE CAN LIBRARY HAS THIS PIN FOR INTERRUPT FOR CAN1 (UNSUSED HERE) INPUT WITHOUT PULLUP, FORCE TO OUTPUT INSTEAD TO PREVENT ERRONEOUS INTERRUPTS.

  CAN1.begin(CAN_BPS_500K);
  CAN1.watchFor(CAN_REPLY_ID);
  CAN1.setCallback(0, callback);
}

void loop() {
  requestCar(CANPID_SPEED);
  Serial.println(speed);
  delay(200);
}



void requestCar(uint8_t pid) {  
  CAN_FRAME outgoing;
  outgoing.id = CAN_REQST_ID;
  outgoing.length = 8;
  outgoing.extended = 0;
  outgoing.rtr = 0;
  outgoing.data.uint8[0] = 0x02;  
  outgoing.data.uint8[1] = 0x01;  
  outgoing.data.uint8[2] = pid; 
  outgoing.data.uint8[3] = 0x00;
  outgoing.data.uint8[4] = 0x00;  
  outgoing.data.uint8[5] = 0x00;  
  outgoing.data.uint8[6] = 0x00;  
  outgoing.data.uint8[7] = 0x00;  
  CAN1.sendFrame(outgoing);
}


void callback(CAN_FRAME *from_car) {
  if (from_car->data.uint8[2]==CANPID_SPEED) {
    speed = from_car->data.uint8[3];
  }
}
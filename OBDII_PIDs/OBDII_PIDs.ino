/*************************************************************************************************
    OBD-II_PIDs TEST CODE
    LOOVEE @ JUN24, 2017

    Query
    send id: 0x7df
      dta: 0x02, 0x01, PID_CODE, 0, 0, 0, 0, 0

    Response
    From id: 0x7E9 or 0x7EA or 0x7EB
      dta: len, 0x41, PID_CODE, byte0, byte1(option), byte2(option), byte3(option), byte4(option)

    https://en.wikipedia.org/wiki/OBD-II_PIDs

    Input a PID, then you will get reponse from vehicle, the input should be end with '\n'
***************************************************************************************************/
#include <SPI.h>
#include "mcp_can.h"

/*SAMD core*/
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 2;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

#define PID_ENGIN_PRM       0x0C
#define PID_VEHICLE_SPEED   0x0D
#define PID_COOLANT_TEMP    0x05

#define CAN_ID_PID          0x7DF

unsigned char PID_INPUT;
unsigned char getPid    = 0;

uint16_t rpm;
uint16_t speed;
uint16_t temperature;

long long lastPrintTime = 0;

void set_mask_filt() {
    CAN.init_Mask(0, 0, 0x7FC);
    CAN.init_Mask(1, 0, 0x7FC);

    CAN.init_Filt(0, 0, 0x7E8);
    CAN.init_Filt(1, 0, 0x7E8);

    CAN.init_Filt(2, 0, 0x7E8);
    CAN.init_Filt(3, 0, 0x7E8);
    CAN.init_Filt(4, 0, 0x7E8);
    CAN.init_Filt(5, 0, 0x7E8);
}

void sendPid(unsigned char __pid) {
    unsigned char tmp[8] = {0x02, 0x01, __pid, 0, 0, 0, 0, 0};
    CAN.sendMsgBuf(CAN_ID_PID, 0, 8, tmp);
}

void setup() {
    SERIAL.begin(115200);
    while (CAN_OK != CAN.begin(CAN_500KBPS, MCP_8MHz)) {  // init can bus : baudrate = 500k
        SERIAL.println("CAN BUS Shield init fail");
        SERIAL.println(" Init CAN BUS Shield again");
        delay(100);
    }
    SERIAL.println("CAN BUS Shield init ok!");
    set_mask_filt();
}


void loop() {
    taskCanRecv();

    delay(50);
    sendPid(PID_ENGIN_PRM);
    delay(50);
    sendPid(PID_VEHICLE_SPEED);
    delay(50);
    sendPid(PID_COOLANT_TEMP);

    if (millis() - lastPrintTime > 300) {
      SERIAL.print(rpm);
      SERIAL.print("; ");
      SERIAL.print(speed);
      SERIAL.print("; ");
      SERIAL.println(temperature);
      lastPrintTime = millis();
    }
}

void taskCanRecv() {
    unsigned char len = 0;
    unsigned char buf[8];

    while (CAN_MSGAVAIL == CAN.checkReceive()) {                // check if get data
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        if (buf[2]==PID_ENGIN_PRM) {
          uint8_t rpmOBDH = buf[3];
          uint8_t rpmOBDL = buf[4];
          rpm = (uint16_t) ((256*rpmOBDH) + rpmOBDL)/(float)4;
        }
        else if (buf[2]==PID_VEHICLE_SPEED) {
          speed = buf[3];
        }
        else if (buf[2]==PID_COOLANT_TEMP) {
          temperature = buf[3] - 40;
        }
    }
}
// END FILE
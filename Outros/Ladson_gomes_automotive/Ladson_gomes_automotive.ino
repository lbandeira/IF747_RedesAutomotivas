/************************************************************************************************* 
  Automotive Networks assignment code by Ladson 03/2019
  Based on the following:
  
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

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

#define PID_ENGIN_RPM       0x0C
#define PID_VEHICLE_SPEED   0x0D
#define PID_COOLANT_TEMP    0x05
#define PID_FUEL_PRESSURE   0x0A
#define PID_SHORT_TERM_FUEL_TRIM  0x06 

#define CAN_ID_PID          0x7DF
unsigned char PID_array[5] = {PID_ENGIN_RPM, PID_VEHICLE_SPEED, PID_COOLANT_TEMP, PID_FUEL_PRESSURE, PID_SHORT_TERM_FUEL_TRIM};
unsigned char PID_INPUT;
unsigned char getPid    = 0;
unsigned char isExt     = 0;

void set_mask_filt()
{
    /*
     * set mask, set both the mask to 0x3ff
     */
    CAN.init_Mask(0, 0, 0x7FC);
    CAN.init_Mask(1, 0, 0x7FC);

    /*
     * set filter, we can receive id from 0x04 ~ 0x09
     */
    CAN.init_Filt(0, 0, 0x7E8);                 
    CAN.init_Filt(1, 0, 0x7E8);

    CAN.init_Filt(2, 0, 0x7E8);
    CAN.init_Filt(3, 0, 0x7E8);
    CAN.init_Filt(4, 0, 0x7E8); 
    CAN.init_Filt(5, 0, 0x7E8);
}


void sendPid(unsigned char __pid)
{
    unsigned char tmp[8] = {0x02, 0x01, __pid, 0, 0, 0, 0, 0};
    Serial.print("SEND PID: 0x");
    Serial.println(__pid, HEX);
    CAN.sendMsgBuf(CAN_ID_PID, 0, 8, tmp);
}

int translate(unsigned char pid, unsigned char A, unsigned char B){
  int ret = 0;
  switch(pid){
    case(PID_ENGIN_RPM):
      ret = (256*A + B)/4;
      Serial.print("Engine RPM: ");
      break;
      
    case(PID_VEHICLE_SPEED):
        ret = A;
        Serial.print("Vehicle Speed: ");
      break;
    case(PID_COOLANT_TEMP):
        ret = A -40;
        Serial.print("Coolant Temperature: ");
      break;
    case(PID_FUEL_PRESSURE):
        ret = 3*A;
        Serial.print("Fuel Pressure: ");
      break;
    case(PID_SHORT_TERM_FUEL_TRIM):
        ret = (100/128)*A -100;
        Serial.print("Short Term Fuel Trim 1: ");
      break;
     default:
        ret = 0;
        break;
  }
  Serial.println(ret);
  return ret;
}

void setup()
{
    Serial.begin(115200);
    while (CAN_OK != CAN.begin(CAN_500KBPS))    // init can bus : baudrate = 500k
    {
        Serial.println("CAN BUS Shield init fail");
        Serial.println(" Init CAN BUS Shield again");
        delay(100);
    }
    Serial.println("CAN BUS Shield init ok!");
    set_mask_filt();
}


void loop()
{
    for(int i = 0; i < 5; i++){
      sendPid(PID_array[i]);
      taskCanRecv(PID_array[i]);
    }
    delay(1000);

}

void taskCanRecv(unsigned char pid)
{
    unsigned char len = 0;
    unsigned char buf[8];

    if((CAN_MSGAVAIL == CAN.checkReceive()))                   // check if get data
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        Serial.println("\r\n------------------------------------------------------------------");
        Serial.print("Get Data From id: 0x");
        Serial.println(CAN.getCanId(), HEX);
        translate(pid, buf[3],buf[4]);
        
        Serial.println();
    }
}

// END FILE

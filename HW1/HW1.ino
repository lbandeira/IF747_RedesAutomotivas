#include <SPI.h>
#include "mcp_can.h"

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

#define PID_ENGIN_RPM       0x0C
#define PID_VEHICLE_SPEED   0x0D
#define PID_COOLANT_TEMP    0x05
#define PID_FUEL_STATUS     0x03
#define PID_FUEL_PRESSURE   0x0A

#define CAN_ID_PID          0x7DF

unsigned char PID_INPUT;
unsigned char getPid    = 0;

int PIDs[] = {0x0C, 0x0D, 0x05, 0x03, 0x0A};

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
    int c;
    for(c = 0; c < 5; c++){
      sendPid(PIDs[c]);
      PID_INPUT = 0;
      taskCanRecv();
      delay(5000);
    }

}

void taskCanRecv()
{
    unsigned char len = 0;
    unsigned char buf[8];
    int res;
    if(CAN_MSGAVAIL == CAN.checkReceive())                   // check if get data
    {
        CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

        Serial.println("\r\n------------------------------------------------------------------");
        Serial.print("Get Data From id: 0x");
        Serial.println(CAN.getCanId(), HEX);
        for(int i = 0; i<len; i++)    // print the data
        {
            Serial.print("0x");
            Serial.print(buf[i], HEX);
            Serial.print("\t");
        }
        Serial.println();

        switch(buf[2]){               //translate the data
          case 0x0C:
          //  (256*A + B)/4
          res = (256*buf[3] + buf[4])/4;
          break;
          case 0x0D:
          //  A
          res = buf[3];
          break;
          case 0x05:
          //  A-40
          res = buf[3] - 40;
          break;
          case 0x03:
          // Bit encoded
          case 0x0A:
          //  3*A
          res = 3*buf[3];
          break;
          }
          Serial.println(res);

    }
}

// END FILE

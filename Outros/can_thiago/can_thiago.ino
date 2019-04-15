
#include <SPI.h>
#include "mcp_can.h"

const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

#define PID_ENGIN_PRM       0x0C
#define PID_VEHICLE_SPEED   0x0D
#define PID_COOLANT_TEMP    0x05

#define CAN_ID_PID          0x7DF

unsigned char PID_INPUT;
unsigned char getPid    = 0;
unsigned char len = 0;
unsigned char buf[8];
long unsigned int rxId;
unsigned long rcvTime;

long unsigned int txID = 0x1881ABBA; // this format is an identifier, the most significant digit is never greater than one
unsigned char stmp[8] = {0x0E, 0x00, 0xFF, 0x22, 0xE9, 0xFA, 0xDD, 0x51};

void set_mask_filt()
{
    
    CAN.init_Mask(0, 0, 0x7FC);
    CAN.init_Mask(1, 0, 0x7FC);

    
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
    taskCanRecv();
    taskDbg();
    
    if(getPid)          // GET A PID
    {
        getPid = 0;
        sendPid(PID_INPUT);
        PID_INPUT = 0;
    }
}

void taskCanRecv()
{
    unsigned char len = 0;
    unsigned char buf[8];

     if(CAN_MSGAVAIL == CAN.checkReceive())            // check if data is coming
    {
        rcvTime = millis();
        CAN.readMsgBuf(&len, buf);    //read data,  len: data length, buf: data buffer

        rxId= CAN.getCanId();        //getting the ID of the incoming message

        Serial.print(rcvTime);
        Serial.print("\t\t");
        Serial.print("0x");
        Serial.print(rxId, HEX);    //printing the ID in its standard form, HEX
        Serial.print("\t");

        for(int i = 0; i<len; i++)    //looping on the incoming data to print them
        {
            if(buf[i] > 15){
              Serial.print("0x");
              Serial.print(buf[i], HEX);    
            }
          else{
              Serial.print("0x0");
              Serial.print(buf[i], HEX);
          }  
            
            Serial.print("\t");            
        }
        Serial.println();
    }
}

void taskDbg()
{
Serial.println("In loop");
    
    CAN.sendMsgBuf(txID,1, 8, stmp);    
    delay(25);    // send data every 25mS
}

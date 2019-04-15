// Nome: Gabriela Alves (gar)

#include <mcp_can.h>
#include <SPI.h>
#include <stdint.h>

// the cs pin of the version after v1.1 is default to D9
// v0.9b and v1.0 is default D10
const int SPI_CS_PIN = 9;

MCP_CAN CAN(SPI_CS_PIN);                                    // Set CS pin

#define PID_ENGIN_PRM       0x0C
#define PID_VEHICLE_SPEED   0x0D
#define PID_COOLANT_TEMP    0x05
#define PID_INTAKE          0x0B
#define PID_FUEL_PRESSURE   0x0A

unsigned char len = 0;
unsigned char buf[8];
char str[20];
unsigned char car_speed;
int16_t engine_rpm;
unsigned char tempor;
unsigned char coolant_temp;
unsigned char fuel_pressure;
unsigned char intake;


void send_message(unsigned char pid_code) {
  unsigned char send_tmp[8] = {0x02, 0x01, pid_code, 0, 0, 0, 0, 0};
  Serial.print("PID: 0x");
  Serial.println(pid_code, HEX);
  CAN.sendMsgBuf(0x7DF, 0, 8, send_tmp); //for the diagnostics case, the system must send the 0x7DF ID
  //CAN.sendMsgBuf(INT8U id, INT8U ext, INT8U len, data_buf);
  delay(100);
}

void receive_message() {
  unsigned char check = '1';
  Serial.println("Waiting the response");
  while(check){
    if (CAN_MSGAVAIL == CAN.checkReceive()) {                 // check if get data
      CAN.readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf
      Serial.println("\r\n------------------------------------------------------------------");
      Serial.print("Get Data From id: 0x");
      Serial.println(CAN.getCanId(), HEX);
      for (int i = 0; i < len; i++) { // print the data
        Serial.print("0x");
        Serial.print(buf[i], HEX);
        Serial.print("\t");
      }
      
      Serial.println();
      check = 0;
    }
  }
}

void setup()
{
  Serial.begin(115200);

  while (CAN_OK != CAN.begin(CAN_500KBPS))              // init can bus : baudrate = 500k
  {
    Serial.println("CAN BUS Shield init fail");
    Serial.println(" Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");

  /*
     set mask, set both the mask to 0x3ff
  */
  CAN.init_Mask(0, 0, 0x3ff);                         // there are 2 mask in mcp2515, you need to set both of them
  CAN.init_Mask(1, 0, 0x3ff);

  /*
     set filter, we can receive id from 0x04 ~ 0x09
  */
  CAN.init_Filt(0, 0, 0x7E8);                          // there are 6 filter in mcp2515
  CAN.init_Filt(1, 0, 0x7E8);                          // there are 6 filter in mcp2515

  CAN.init_Filt(2, 0, 0x7E8);                          // there are 6 filter in mcp2515
  CAN.init_Filt(3, 0, 0x7E8);                          // there are 6 filter in mcp2515
  CAN.init_Filt(4, 0, 0x7E8);                          // there are 6 filter in mcp2515
  CAN.init_Filt(5, 0, 0x7E8);                          // there are 6 filter in mcp2515

  /*
     Explanation:
     Ref:https://www.redalyc.org/jatsRepo/4115/411556006003/html/index.html
     For the diagnostics case, the system must send the 0x7DF ID
     and the expected responses must arrive with 0x7E8 and 0x7E9 IDs,
     corresponding to the most important modules in the system (ISO 15765-4).
  */
}


void loop()
{
  Serial.println("Choose: (Enter the number)");
  Serial.println("1. Speed");
  Serial.println("2. RPM");
  Serial.println("3. Engine Coolant Temperature");
  Serial.println("4. Fuel Pressure");
  Serial.println("5. Intake Manifold Absolute Pressure");
  unsigned char check = 1;
  char option = '0';
  while (check) {
    option = Serial.read();
    if (isDigit(option)) {
      check = 0;
    }
  }

  Serial.println(option);

  switch (option) {
    case '1':
      send_message(PID_VEHICLE_SPEED);
      receive_message();
      car_speed = buf[0];
      Serial.print("SPEED: ");
      Serial.print(car_speed);
      Serial.println(" km/h");
      break;

    case '2':
      send_message(PID_ENGIN_PRM);
      receive_message();
      engine_rpm = buf[1];
      engine_rpm = engine_rpm << 8;
      tempor = buf[2] & 0x3F;
      engine_rpm = engine_rpm + tempor;
      Serial.print("RPM = ");
      Serial.println(engine_rpm);
      break;

    case '3':
      send_message(PID_COOLANT_TEMP);
      receive_message();
      coolant_temp = buf[0];
      coolant_temp = coolant_temp - 40;
      Serial.print("ENGINE COOLANT TEMPERATURE: ");
      Serial.print(coolant_temp);
      Serial.println(" °X");
      break;

    case '4':
      send_message(PID_FUEL_PRESSURE);
      receive_message();
      fuel_pressure = buf[0];
      fuel_pressure = 3 * fuel_pressure;
      Serial.print("FUEL PRESSURE: ");
      Serial.print(fuel_pressure);
      Serial.println(" °X");
      break;


    case '5':
      send_message(PID_INTAKE);
      receive_message();
      intake = buf[0];
      Serial.print("INTAKE MANIFOLD ABSOLUTE PRESSURE: ");
      Serial.print(intake);
      Serial.println(" °X");
      break;


    default:
      Serial.println("Choose an option");
  }
  memset(buf, 0, sizeof(buf));
}

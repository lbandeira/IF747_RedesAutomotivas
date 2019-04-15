#include <SPI.h>
#include "mcp_can.h"

#define PID_ENGINE_LOAD 0x04
#define PID_ENGINE_COOLANT_TEMPERATURE 0x05
#define PID_FUEL_PRESSURE 0x0A
#define PID_ENGINE_RPM 0x0C
#define PID_VEHICLE_SPEED 0x0D

// CAN ID's que devem ser testados (descomentar o que vai usar)
#define CAN_ID_REQUEST 0x7DF
// #define CAN_ID_REQUEST 0x7E0
// #define CAN_ID_REQUEST 0x7E1
// #define CAN_ID_REQUEST 0x7E2
// #define CAN_ID_REQUEST 0x7E3

#define CAN_ID_RESPONSE 0x7E8
// #define CAN_ID_RESPONSE 0x7E9
// #define CAN_ID_RESPONSE 0x7EA
// #define CAN_ID_RESPONSE 0x7EB

// Indica se ha resposta pendente
uint8_t flag_rcv = 0;

// Indica o tamanho da mensagem recebida
uint8_t msg_len = 0;

// Buffer para as mensagens (8 bytes)
uint8_t data[8];

// Variavel utilizada para identificar opcao no menu
uint8_t option;

const uint8_t pids[5] = {
  PID_ENGINE_LOAD,
  PID_ENGINE_COOLANT_TEMPERATURE,
  PID_FUEL_PRESSURE,
  PID_ENGINE_RPM,
  PID_VEHICLE_SPEED
};

// Porta utilizada
const int SPI_CS_PIN = 9;

// Instanciando mcp can
MCP_CAN CAN(SPI_CS_PIN);

// Funcao que decodifica e printa os dados de acordo com o pid
void decode_data(uint8_t *data) {
  double decoded_data = 0;
  uint8_t pid = data[2];
  char buf[51];

Serial.println("----------DEBUG------------");
for (int i = 0; i < 8; i++) {
  Serial.print(data[i], HEX);
  Serial.print(" ");
}
Serial.println();
  
  // Dependendo do pid o calculo a ser feito difere, assim como a unidade
  switch (pid) {
    case PID_ENGINE_LOAD:
      decoded_data = ((uint16_t)data[3]) / 2.55;
      sprintf(buf, "Engine load: %.2f%%\n", decoded_data);
      break;
    
    case PID_ENGINE_COOLANT_TEMPERATURE:
      decoded_data = data[3] - 40;
      sprintf(buf, "Engine coolant temperature: %.0f °C\n", decoded_data);
      break;

    case PID_FUEL_PRESSURE:
      decoded_data = 3 * data[3];
      sprintf(buf, "Fuel pressure: %.2f kPa\n", decoded_data);
      break;

    case PID_ENGINE_RPM:
      decoded_data = ((256 * (uint16_t)data[3]) + (uint16_t)data[4]) / 4.0;
      sprintf(buf, "Engine RPM: %.2f rpm\n", decoded_data);
      break;

    case PID_VEHICLE_SPEED:
      decoded_data = data[3];
      sprintf(buf, "Vehicle speed: %.0f km/h\n", decoded_data);
      break;

    default:
      return;
  }

  // Printa mensagem com os dados decodificados de acordo com o pid
  Serial.println(decoded_data);
  Serial.println(buf);
  flag_rcv = 0;
}

void read_data() {
  if (CAN_MSGAVAIL == CAN.checkReceive()) {
    CAN.readMsgBuf(&msg_len, data);

    unsigned long canId = CAN.getCanId();
    if (CAN_ID_RESPONSE == canId) {
      decode_data(data);
    }
  }
}

void print_menu() {
  Serial.println("====================================================================================");
  Serial.println("Choose an option:");
  Serial.println("0 - Engine Load");
  Serial.println("1 - Engine Coolant Temperature");
  Serial.println("2 - Fuel Pressure");
  Serial.println("3 - Intake Manifold Absolute Pressure");
  Serial.println("4 - Vehicle Speed");
  Serial.println("====================================================================================");

  while (true) {
    while (!Serial.available());
    option = Serial.read() - '0';

    if (option >= 0 && option <= 4) break;
    
    Serial.println(option);
    Serial.println("Invalid option! Choose again:");
  }
}

void send_msg() {
  uint8_t pid = pids[option];
  /*
    | Additional bytes | mode | pid code | not used... |
  */
  uint8_t payload[8] = {2, 1, pid, 0, 0, 0, 0, 0};

  CAN.sendMsgBuf(CAN_ID_REQUEST, 0, sizeof(payload), payload);
}

void setup() {
  Serial.begin(115200);

  // Inicializando o can bus
  while (CAN_OK != CAN.begin(CAN_500KBPS)) {
    Serial.println("CAN BUS Shield init fail");
    Serial.println("Init CAN BUS Shield again");
    delay(100);
  }
  Serial.println("CAN BUS Shield init ok!");
}

void loop() {
  if (!flag_rcv) {
    print_menu();
    send_msg();
    flag_rcv = 1;
  } else {
    read_data();
  }
}

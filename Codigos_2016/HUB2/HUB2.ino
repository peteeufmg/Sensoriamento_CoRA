//Bibliotecas NRF24
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

//---------------------------------DEFINIÇÕES----------------------------------------------
//Serial
#define BAUDRATE            9600

//LEDs
#define LED_1               4
#define LED_2               5
#define LED_3               6
#define LED_4               7
#define LED_5               8

//Pinos do nRF24L01 - UNO 
#define PINO_CE             9 
#define PINO_CS             10 

//Parâmetros de operação do nRF24L01
#define DATA_RATE           RF24_250KBPS
#define POWER_AMPLIFICATION RF24_PA_MAX
#define CANAL               70
#define CRC_SIZE            RF24_CRC_16
#define RETRY_DELAY         15
#define RETRY_COUNT         15

//Endereços da Rede sem Fio
#define MESTRE              00
#define HUB1                01
#define HUB2                02
#define CP0                 011
#define CP1                 021
#define CP2                 031
#define CP3                 041
#define CP4                 051
#define CP5                 012
#define CP6                 022
#define CP7                 032
#define CP8                 042
#define CP9                 052

//---------------------------------CONFIGURAÇÃO DO HUB--------------------------------------
//Variáveis da Rede
uint16_t this_node = HUB2; //Endereço deste Nó
uint16_t to_s = MESTRE; //Endereço para envio da mensagem do status dos sensores
#define ini  8
#define fim  ini + 4
int i = ini; //Iterador

//--------------------------------VARIÁVEIS GLOBAIS----------------------------------------
//Vetor de Endereços
const uint16_t node_address_set[13] = { MESTRE, HUB1, HUB2, CP0, CP1, CP2, CP3, CP4, CP5, CP6, CP7, CP8, CP9};

//Instância do Wireless
RF24 radio(PINO_CE, PINO_CS);
RF24Network network(radio);

//Variáveis de Checagem
bool status_ultrassonicos[5];
const unsigned long interval = 125; //Intervalo de checagem do ultrassônico
unsigned long last_time_sent = 0;
unsigned long now = 0;
int time_counter_1 = 0;
int time_counter_2 = 0;
int time_counter_3 = 0;
int time_counter_4 = 0;
int time_counter_5 = 0;
//--------------------------------VARIÁVEIS GLOBAIS----------------------------------------

void setup() {
  //Inicialização dos LEDs
  pinMode(LED_1, OUTPUT);
  digitalWrite(LED_1, LOW);
  pinMode(LED_2, OUTPUT);
  digitalWrite(LED_2, LOW);
  pinMode(LED_3, OUTPUT);
  digitalWrite(LED_3, LOW);
  pinMode(LED_4, OUTPUT);
  digitalWrite(LED_4, LOW);
  pinMode(LED_5, OUTPUT);
  digitalWrite(LED_5, LOW);
  
  //Inicialização do Rádio e da Rede
  SPI.begin();
  radio.begin();
  radio.setDataRate(DATA_RATE);
  radio.setPALevel(POWER_AMPLIFICATION);
  radio.setRetries(RETRY_DELAY,RETRY_COUNT);
  radio.setCRCLength(CRC_SIZE);
  network.begin(/*channel*/ CANAL, /*node address*/ this_node );
  
  //Inicialização do Serial
  Serial.begin(BAUDRATE);
  Serial.println("DEPURACAO HUB2"); // Depuração
  
  status_ultrassonicos[0] = false;
  status_ultrassonicos[1] = false;
  status_ultrassonicos[2] = false;
  status_ultrassonicos[3] = false;
  status_ultrassonicos[4] = false;
  
  delay(500);
}

void loop() {
  //Atualiza a Rede
  network.update();
  while ( network.available() ){
    RF24NetworkHeader header;
    network.peek(header);
      switch (header.type){
        case 'C': 
          InformStatus(header); 
          break;
        
        case 'S': 
          UpdateStatus(header); 
          break;
          
        default:  
          Serial.println("*** ATENÇÃO *** Tipo de Mensagem Desconhecido");
          network.read(header,0,0);
          break;
      };
    }
  
  
  //Solicitar Status
  now = millis();
  if ( now - last_time_sent >= interval ){
    last_time_sent = now; 
    
    sendCMessage(node_address_set[i]);
    
    i++;
    if(i > fim){
      i = ini;}
      
      
    time_counter_1++;
    time_counter_2++;
    time_counter_3++;
    time_counter_4++;
    time_counter_5++;
    
    if(time_counter_1>=5)
    {
      status_ultrassonicos[0] = false;
      digitalWrite(LED_1, LOW);
    }
    if(time_counter_2>=5)
    {
      status_ultrassonicos[1] = false;
      digitalWrite(LED_2, LOW);
    }
    if(time_counter_3>=5)
    {
      status_ultrassonicos[2] = false;
      digitalWrite(LED_3, LOW);
    }
    if(time_counter_4>=5)
    {
      status_ultrassonicos[3] = false;
      digitalWrite(LED_4, LOW);
    }
    if(time_counter_5>=5)
    {
      status_ultrassonicos[4] = false;
      digitalWrite(LED_5, LOW);
    }
  }
  
}

//Subrotida de enviar mensagem de Checagem
bool sendCMessage(uint16_t to){
  RF24NetworkHeader header(/*para o nó*/ to, /*tipo de mensagem*/ 'C' /*Checagem*/);
  bool c = true;  
  //Serial.println(c); //Depuração
  return network.write(header, &c, sizeof(c));
}

//Subrotida de enviar mensagem de Status
bool sendSMessage(uint16_t to){
  RF24NetworkHeader header(/*para o nó*/ to, /*tipo de mensagem*/ 'S' /*Status*/);  
  //*
  Serial.print("HUB2: ");
  Serial.print(status_ultrassonicos[0]);
  Serial.print("   ");
  Serial.print(status_ultrassonicos[1]);
  Serial.print("   ");
  Serial.print(status_ultrassonicos[2]);
  Serial.print("   ");
  Serial.print(status_ultrassonicos[3]);
  Serial.print("   ");
  Serial.println(status_ultrassonicos[4]); //Depuração
  
  return network.write(header, &status_ultrassonicos, sizeof(status_ultrassonicos));
}

//Subrotina de recepção de mensagem
void InformStatus(RF24NetworkHeader& header){
  bool message;
  
  network.read(header, &message, sizeof(message));
  uint16_t from = header.from_node;
  
  if(message)
  {
    sendSMessage(to_s);
  }
}

//Subrotina de atualização de status dos sensores
void UpdateStatus(RF24NetworkHeader& header){
  bool message;
  
  network.read(header, &message, sizeof(message));
  uint16_t from = header.from_node;
  
  if(message)
  {
    switch(from){
      case CP5:
        status_ultrassonicos[0] = true;
        time_counter_1 = 0;
        digitalWrite(LED_1, HIGH);
        break;
        
      case CP6:
        status_ultrassonicos[1] = true;
        time_counter_2 = 0;
        digitalWrite(LED_2, HIGH);
        break;
      
      case CP7:
        status_ultrassonicos[2] = true;
        time_counter_3 = 0;
        digitalWrite(LED_3, HIGH);
        break;
      
      case CP8:
        status_ultrassonicos[3] = true;
        time_counter_4 = 0;
        digitalWrite(LED_4, HIGH);
        break;

      case CP9:
        status_ultrassonicos[4] = true;
        time_counter_5 = 0;
        digitalWrite(LED_5, HIGH);
        break;
    }
  }
}

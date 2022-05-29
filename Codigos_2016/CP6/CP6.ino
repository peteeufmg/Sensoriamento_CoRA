//Bibliotecas NRF24
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

//Biblioteca Sensor Ultrasônico HCSR04
#include <Ultrasonic.h>

//---------------------------------DEFINIÇÕES----------------------------------------------
//Serial
#define BAUDRATE            9600

//Ultrassônico
#define PINO_TRIGGER        3
#define PINO_ECHO           2

//LEDs
#define LED_VERDE           8
#define LED_VERMELHO        6
#define LED_AMARELO         7

//Distâncias válidas para captação do sensor (cm)
#define DIST_MIN            4
#define DIST_MAX            47
#define DIST_DESATIVADO     53    

// Delay para medida da distância pelo ultrassônico
#define DELAY               250

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

//---------------------------------CONFIGURAÇÃO DO NÓ--------------------------------------
//Variáveis da Rede
uint16_t this_node = CP6; //Endereço deste Nó
uint16_t to_p = MESTRE; //Endereço para envio da mensagem de passagem
uint16_t to_c = HUB2; //Endereço para envio da mensagem de checagem

//--------------------------------VARIÁVEIS GLOBAIS----------------------------------------
//Vetor de Endereços
const uint16_t node_address_set[13] = { MESTRE, HUB1, HUB2, CP0, CP1, CP2, CP3, CP4, CP5, CP6, CP7, CP8, CP9};

//Instância do Wireless
RF24 radio(PINO_CE, PINO_CS);
RF24Network network(radio);

//Variáveis do Sensor Ultrassônico
Ultrasonic ultrasonic(PINO_TRIGGER, PINO_ECHO); //Instância do ultrassônico
long tempo;     //Tempo de recepção da onda sonora enviada pelo sensor
long distancia; //Distância calculada
int sensorTriggered = 0; //Indica disparo do sensor
const unsigned long interval = 50; //Intervalo de checagem do ultrassônico
unsigned long last_time_sent = 0;
unsigned long now = 0;
int tries = 0;

void setup() {
  //Inicialização dos LEDs
  pinMode(LED_VERDE, OUTPUT);
  digitalWrite(LED_VERDE, LOW);
  pinMode(LED_AMARELO, OUTPUT);
  digitalWrite(LED_AMARELO, LOW);
  pinMode(LED_VERMELHO, OUTPUT);
  digitalWrite(LED_VERMELHO, LOW);
  
  //Inicialização do Rádio e da Rede
  SPI.begin();
  radio.begin();
  radio.setDataRate(DATA_RATE);
  radio.setPALevel(POWER_AMPLIFICATION);
  radio.setRetries(RETRY_DELAY,RETRY_COUNT);
  radio.setCRCLength(CRC_SIZE);
  network.begin(/*channel*/ CANAL, /*node address*/ this_node );
  
  //Inicialização do Serial
  //Serial.begin(BAUDRATE);
  //Serial.println("DEPURACAO ULTRASSONICO"); // Depuração
  
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
        default:  
          //Serial.println("*** ATENÇÃO *** Tipo de Mensagem Desconhecido");
          network.read(header,0,0);
          break;
      };
    }
  
  
  //Sensor Ultrassônico
  now = millis();
  if ( now - last_time_sent >= interval ){
    last_time_sent = now; 
    
    sensorTriggered = monitorarSensor();
    //LEDs
    if(sensorTriggered == 0) {
      digitalWrite(LED_VERMELHO, HIGH);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_AMARELO, LOW);
      //Serial.println("SENSOR DESATIVADO..."); // Depuração
    } else if(sensorTriggered == 1) {
      digitalWrite(LED_VERMELHO, LOW);
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(LED_AMARELO, LOW);
      //Serial.println("SENSOR ATIVADO...");   // Depuração
      
      //Enviando mensagem ao Mestre
      while((!sendPMessage(to_p))&&(tries <= 15)){delay(10);tries++;}
      tries = 0;
      
    } else {
      digitalWrite(LED_VERMELHO, LOW);
      digitalWrite(LED_VERDE, LOW);
      digitalWrite(LED_AMARELO, HIGH);
      //Serial.println("MEDIDA INVALIDA...");   // Depuração
    }
  }
  
  
}

//Subrotina do Sensor Ultrassônico
int monitorarSensor(void) {
  noInterrupts(); //Desabilita interrupções para não afetar medição
  tempo = ultrasonic.timing();
  interrupts(); //Habilita as interrupções novamente
  distancia = ultrasonic.convert(tempo,Ultrasonic::CM);  
  //Serial.print("Dist: ");
  //Serial.print(distancia);
  //Serial.println(" cm");
  if(distancia > DIST_MAX && distancia <= DIST_DESATIVADO) {
    return 0; // Sensor Desativado
  } else if(distancia >= DIST_MIN && distancia <= DIST_MAX) {
    return 1; // Sensor Ativado
  } else {
    return 2; // Medida Inválida
  }
}

//Subrotida de enviar mensagem de Passagem
bool sendPMessage(uint16_t to){
  RF24NetworkHeader header(/*para o nó*/ to, /*tipo de mensagem*/ 'P' /*Passagem*/);
  bool p = true;  
  //Serial.println(p); //Depuração
  return network.write(header, &p, sizeof(p));
}

//Subrotida de enviar mensagem de Checagem
bool sendSMessage(uint16_t to){
  RF24NetworkHeader header(/*para o nó*/ to, /*tipo de mensagem*/ 'S' /*Status*/);
  bool c = true;  
  //Serial.println(c); //Depuração
  return network.write(header, &c, sizeof(c));
}

//Subrotina de recepção de mensagem
void InformStatus(RF24NetworkHeader& header){
  //char message[DATA_SIZE];
  bool message;
  
  network.read(header, &message, sizeof(message));
  uint16_t from = header.from_node;
  
  if(message)
  {
    sendSMessage(to_c);
  }
}

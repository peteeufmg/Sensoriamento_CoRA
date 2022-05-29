// Bibliotecas NRF24
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

//---------------------------------DEFINIÇÕES----------------------------------------------
// Serial
#define BAUDRATE 9600

// Pino de leitura do sensor IR
#define PINO_TRIGGER 3

// LEDs
#define LED_VERDE 8
#define LED_VERMELHO 6
#define LED_AMARELO 7

// Pinos do nRF24L01 - UNO
#define PINO_CE 9
#define PINO_CS 10

// Parâmetros de operação do nRF24L01
#define DATA_RATE RF24_250KBPS
#define POWER_AMPLIFICATION RF24_PA_MAX
#define CANAL 70
#define CRC_SIZE RF24_CRC_16
#define RETRY_DELAY 15
#define RETRY_COUNT 15

// Endereços da Rede sem Fio
#define MESTRE 00
#define HUB1 01
#define HUB2 02
#define CP0 011
#define CP1 021
#define CP2 031
#define CP3 041
#define CP4 051
#define CP5 012
#define CP6 022
#define CP7 032
#define CP8 042
#define CP9 052

//---------------------------------CONFIGURAÇÃO DO NÓ--------------------------------------
// Variáveis da Rede
uint16_t this_node = CP8; // Endereço deste Nó
uint16_t to_p = MESTRE;   // Endereço para envio da mensagem de passagem
uint16_t to_c = HUB2;     // Endereço para envio da mensagem de checagem

//--------------------------------VARIÁVEIS GLOBAIS----------------------------------------
// Vetor de Endereços
const uint16_t node_address_set[13] = {MESTRE, HUB1, HUB2, CP0, CP1, CP2, CP3, CP4, CP5, CP6, CP7, CP8, CP9};

// Instância do Wireless
RF24 radio(PINO_CE, PINO_CS);
RF24Network network(radio);

// Variáveis do Sensor IR
int sensorTriggered = 0; // Indica disparo do sensor
int tries = 0;

// Subrotina de enviar mensagem de Passagem
bool sendPMessage(uint16_t to)
{
  RF24NetworkHeader header(/*para o nó*/ to, /*tipo de mensagem*/ 'P' /*Passagem*/);
  bool p = true;
  // Serial.println(p); //Depuração
  return network.write(header, &p, sizeof(p));
}

// Subrotina de enviar mensagem de Checagem
bool sendSMessage(uint16_t to)
{
  RF24NetworkHeader header(/*para o nó*/ to, /*tipo de mensagem*/ 'S' /*Status*/);
  bool c = true;
  // Serial.println(c); //Depuração
  return network.write(header, &c, sizeof(c));
}

// Subrotina de recepção de mensagem
void InformStatus(RF24NetworkHeader &header)
{
  // char message[DATA_SIZE];
  bool message;

  network.read(header, &message, sizeof(message));
  uint16_t from = header.from_node;

  if (message)
  {
    sendSMessage(to_c);
  }
}

void sensorINT()
{

  // Sensor Infravermelho
  int sensor = digitalRead(PINO_TRIGGER);

  // LEDs
  if (sensor == 0)
  {
    sensorTriggered = 1;
  }

  else
  {
    sensorTriggered = 0;
  }

  return;
}

void setup()
{
  // Inicialização dos LEDs
  pinMode(LED_VERDE, OUTPUT);
  digitalWrite(LED_VERDE, LOW);
  pinMode(LED_AMARELO, OUTPUT);
  digitalWrite(LED_AMARELO, LOW);
  pinMode(LED_VERMELHO, OUTPUT);
  digitalWrite(LED_VERMELHO, LOW);

  // Inicialização do Rádio e da Rede
  SPI.begin();
  radio.begin();
  radio.setDataRate(DATA_RATE);
  radio.setPALevel(POWER_AMPLIFICATION);
  radio.setRetries(RETRY_DELAY, RETRY_COUNT);
  radio.setCRCLength(CRC_SIZE);
  network.begin(/*channel*/ CANAL, /*node address*/ this_node);

  attachInterrupt(digitalPinToInterrupt(3), sensorINT, CHANGE);

  // Inicialização do Serial
  // Serial.begin(BAUDRATE);
  delay(500);
}

void loop()
{
  Serial.println(sensorTriggered);

  // Atualiza a Rede
  network.update();
  while (network.available())
  {
    RF24NetworkHeader header;
    network.peek(header);
    switch (header.type)
    {
    case 'C':
      InformStatus(header);
      break;
    default:
      // Serial.println("*** ATENÇÃO *** Tipo de Mensagem Desconhecido");
      network.read(header, 0, 0);
      break;
    };
  }

  if (sensorTriggered)
  {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);

    // Enviando mensagem ao Mestre
    // for (int i = 0; i <= 10; i++) //Serial.println("Teste");
    while ((!sendPMessage(to_p)) && (tries <= 15))
    {
      delay(10);
      tries++;
    }
    tries = 0;
  }

  else
  {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
  }
}
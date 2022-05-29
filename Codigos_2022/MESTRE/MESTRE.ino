// Bibliotecas NRF24
#include <avr/pgmspace.h>
#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>

//---------------------------------DEFINIÇÕES----------------------------------------------
// Serial
#define BAUDRATE 9600

// LEDs
#define LED_1 8
#define LED_2 6

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

//---------------------------------CONFIGURAÇÃO DO MESTRE--------------------------------------
// Variáveis da Rede
uint16_t this_node = MESTRE; // Endereço deste Nó
uint16_t to_c1 = HUB1;       // Endereço para solicitar a mensagem do status dos sensores
uint16_t to_c2 = HUB2;       // Endereço para solicitar a mensagem do status dos sensores

//--------------------------------VARIÁVEIS GLOBAIS----------------------------------------
// Vetor de Endereços
const uint16_t node_address_set[13] = {MESTRE, HUB1, HUB2, CP0, CP1, CP2, CP3, CP4, CP5, CP6, CP7, CP8, CP9};

// Instância do Wireless
RF24 radio(PINO_CE, PINO_CS);
RF24Network network(radio);

// Variáveis de Checagem
bool status_sensores1[5];
bool status_sensores2[5];
const unsigned long interval = 500; // Intervalo de checagem dos sensores
unsigned long last_time_sent = 0;
unsigned long now = 0;
bool unsync = false;
int time_count = 0;

// Variáveis Serial
int crc;
char m[20], r;
String s;
int ack = 0;
int c0 = 0, c1 = 0, c2 = 0, c3 = 0, c4 = 0, c5 = 0, c6 = 0, c7 = 0, c8 = 0, c9 = 0;

//--------------------------------FIM VARIÁVEIS GLOBAIS------------------------------------

// Subrotina de enviar mensagem de Checagem
bool sendCMessage(uint16_t to)
{
  RF24NetworkHeader header(/*para o nó*/ to, /*tipo de mensagem*/ 'C' /*Checagem*/);
  bool c = true;
  // Serial.println(c); //Depuração
  return network.write(header, &c, sizeof(c));
}

// Subrotina de enviar mensagem de Status
void SensorTriggered(RF24NetworkHeader &header)
{
  uint16_t from = header.from_node;
  bool message;

  switch (from)
  {
  case CP0:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c0 = 1;
    }
    // Serial.println("CP0"); //Depuração
    break;

  case CP1:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c1 = 1;
    }
    // Serial.println("CP1"); //Depuração
    break;

  case CP2:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c2 = 1;
    }
    // Serial.println("CP2"); //Depuração
    break;

  case CP3:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c3 = 1;
    }
    // Serial.println("CP3"); //Depuração
    break;

  case CP4:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c4 = 1;
    }
    // Serial.println("CP4"); //Depuração
    break;

  case CP5:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c5 = 1;
    }
    // Serial.println("CP5"); //Depuração
    break;

  case CP6:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c6 = 1;
    }
    // Serial.println("CP6"); //Depuração
    break;

  case CP7:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c7 = 1;
    }
    // Serial.println("CP7"); //Depuração
    break;

  case CP8:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c8 = 1;
    }
    // Serial.println("CP8"); //Depuração
    break;

  case CP9:
    network.read(header, &message, sizeof(message));
    if (message)
    {
      c9 = 1;
    }
    // Serial.println("CP9"); //Depuração
    break;
  }
}

// Subrotina de recepção de mensagem
void InformStatus()
{
  int CRC = 5;

  Serial.print('1');
  Serial.print('C');
  Serial.print(status_sensores1[0]);
  Serial.print(status_sensores1[1]);
  Serial.print(status_sensores1[2]);
  Serial.print(status_sensores1[3]);
  Serial.print(status_sensores1[4]);
  Serial.print(status_sensores2[0]);
  Serial.print(status_sensores2[1]);
  Serial.print(status_sensores2[2]);
  Serial.print(status_sensores2[3]);
  Serial.print(status_sensores2[4]);

  if (status_sensores1[0])
  {
    CRC = CRC + 1;
  }
  if (status_sensores1[1])
  {
    CRC = CRC + 1;
  }
  if (status_sensores1[2])
  {
    CRC = CRC + 1;
  }
  if (status_sensores1[3])
  {
    CRC = CRC + 1;
  }
  if (status_sensores1[4])
  {
    CRC = CRC + 1;
  }
  if (status_sensores2[0])
  {
    CRC = CRC + 1;
  }
  if (status_sensores2[1])
  {
    CRC = CRC + 1;
  }
  if (status_sensores2[2])
  {
    CRC = CRC + 1;
  }
  if (status_sensores2[3])
  {
    CRC = CRC + 1;
  }
  if (status_sensores2[4])
  {
    CRC = CRC + 1;
  }

  if (CRC == 5)
  {
    Serial.println("05");
  }
  if (CRC == 6)
  {
    Serial.println("06");
  }
  if (CRC == 7)
  {
    Serial.println("07");
  }
  if (CRC == 8)
  {
    Serial.println("08");
  }
  if (CRC == 9)
  {
    Serial.println("09");
  }
  if (CRC == 10)
  {
    Serial.println("10");
  }
  if (CRC == 11)
  {
    Serial.println("11");
  }
  if (CRC == 12)
  {
    Serial.println("12");
  }
  if (CRC == 13)
  {
    Serial.println("13");
  }
  if (CRC == 14)
  {
    Serial.println("14");
  }
  if (CRC == 15)
  {
    Serial.println("15");
  }
  if (CRC == 16)
  {
    Serial.println("16");
  }
}

// Subrotina de atualização de status dos sensores
void UpdateStatus(RF24NetworkHeader &header)
{
  // char message[DATA_SIZE];
  uint16_t from = header.from_node;

  switch (from)
  {
  case HUB1:
    network.read(header, &status_sensores1, sizeof(status_sensores1));
    break;

  case HUB2:
    network.read(header, &status_sensores2, sizeof(status_sensores2));
    break;
  }
}

// Subrotina de comunicação do disparo de sensores com o PC
void NotifyPC()
{
  unsigned int i;
  if (c0 == 1)
  {
    Serial.println("1P002");
    delay(10);
    i = 0;
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P002");
        delay(10);
      }
      i = 0;
    }
    ack = 0;
  }
  c0 = 0;

  if (c1 == 1)
  {
    Serial.println("1P103");
    delay(10);
    i = 0;
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P103");
        delay(10);
      }
      i = 0;
    }
    ack = 0;
  }
  c1 = 0;

  if (c2 == 1)
  {
    Serial.println("1P204");
    delay(10);
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P204");
        delay(100);
      }
      i = 0;
    }
    ack = 0;
  }
  c2 = 0;

  if (c3 == 1)
  {
    Serial.println("1P305");
    delay(10);
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P305");
        delay(100);
      }
      i = 0;
    }
    ack = 0;
  }
  c3 = 0;

  if (c4 == 1)
  {
    Serial.println("1P406");
    delay(10);
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P406");
        delay(10);
      }
      i = 0;
    }
    ack = 0;
  }
  c4 = 0;

  if (c5 == 1)
  {
    Serial.println("1P507");
    delay(10);
    i = 0;
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P507");
        delay(10);
      }
      i = 0;
    }
    ack = 0;
  }
  c5 = 0;

  if (c6 == 1)
  {
    Serial.println("1P608");
    delay(10);
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P608");
        delay(100);
      }
      i = 0;
    }
    ack = 0;
  }
  c6 = 0;

  if (c7 == 1)
  {
    Serial.println("1P709");
    delay(10);
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P709");
        delay(100);
      }
      i = 0;
    }
    ack = 0;
  }
  c7 = 0;

  if (c8 == 1)
  {
    Serial.println("1P810");
    delay(10);
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P810");
        delay(10);
      }
      i = 0;
    }
    ack = 0;
  }
  c8 = 0;

  if (c9 == 1)
  {
    Serial.println("1P911");
    delay(10);
    while (ack == 0)
    {
      while (Serial.available() <= 0)
      {
      }
      r = Serial.read();
      while (r != '\n')
      {
        m[i] = r;
        i = i + 1;
        while (Serial.available() <= 0)
        {
        }
        r = Serial.read();
      }
      m[i] = r;

      if ((m[1] == 'A') && (m[4] == '2'))
      {
        ack = 1;
      }
      else
      {
        Serial.println("1P911");
        delay(10);
      }
      i = 0;
    }
    ack = 0;
  }
  c9 = 0;
}

void setup()
{
  // Inicialização do Rádio e da Rede
  SPI.begin();
  radio.begin();
  radio.setDataRate(DATA_RATE);
  radio.setPALevel(POWER_AMPLIFICATION);
  radio.setRetries(RETRY_DELAY, RETRY_COUNT);
  radio.setCRCLength(CRC_SIZE);
  network.begin(/*channel*/ CANAL, /*node address*/ this_node);

  // Inicialização do Serial
  Serial.begin(BAUDRATE);
  Serial.println("DEPURACAO MESTRE"); // Depuração

  delay(500);
}

void loop()
{
  // Atualiza a Rede
  network.update();
  while (network.available())
  {
    RF24NetworkHeader header;
    network.peek(header);
    switch (header.type)
    {
    case 'P':
      SensorTriggered(header);
      break;

    case 'S':
      UpdateStatus(header);
      break;

    default:
      Serial.println("*** ATENÇÃO *** Tipo de Mensagem Desconhecido");
      network.read(header, 0, 0);
      break;
    }
  }

  // Solicitar Status
  now = millis();
  if (now - last_time_sent >= interval)
  {
    last_time_sent = now;

    if (unsync)
    {
      sendCMessage(HUB1);
      unsync = false;
    }
    else
    {
      sendCMessage(HUB2);
      unsync = true;
    }

    InformStatus();
  }

  // Enviar mensagem de disparo de sensores
  NotifyPC();
}

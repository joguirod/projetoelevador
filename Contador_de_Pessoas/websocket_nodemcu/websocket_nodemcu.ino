// Projeto feito pelo EletroPET para contar a quantidade de pessoas em uma sala
// e medir a temperatura e umidade da regiao

//--------------- Dados da rede wifi -----------------------------
#include <ESP8266WiFi.h>

const char *ssid = "VIVOFIBRA-1C12";
const char *password = "33d7f81c12";
//--------------------- Contador de pessoas ----------------------
#define Sensor1 D6
#define Sensor2 D7

int qtdPessoas = 0;
int sensor1 = 0, sensor2 = 0;
int incremento = 0, decremento = 0;
unsigned long currentTime = 0;
unsigned long initialTime = 0;
//----------------------------------------------------------------

//------------------- Para pegar o tempo atual --------------------
#include <NTPClient.h>
#include <WiFiUdp.h>

const long utcOffsetInSeconds = -10800;

char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);
//----------------------------------------------------------------

//-------- Responsavel pela comunicacao com websocket ------------
#include <WebSocketsClient.h>

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  String dia_da_semana = daysOfTheWeek[timeClient.getDay()];
  String hora = timeClient.getFormattedTime();

  String dados = dia_da_semana + " - " + hora + "," + qtdPessoas;

  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[WSc] Desconectado!\n");
    break;
  case WStype_CONNECTED:
    Serial.printf("[WSc] Conectado ao websocket\n");
    break;

  case WStype_TEXT:
    webSocket.sendTXT(dados);
    Serial.println(dados);
    break;
  }

  delay(300);
}
//----------------------------------------------------------------
//------------------ Funcao contador de pessoas ------------------
void contaPessoas()
{
  currentTime = millis();

  // limpa as variaveis
  if ((currentTime - initialTime) >= 10000)
  {
    sensor1 = 0;
    sensor2 = 0;
    incremento = 0;
    decremento = 0;
    initialTime = millis();
  }

  if (digitalRead(Sensor1) == 0)
  {
    sensor1 = 1;
    if (sensor2 == 0)
    {
      incremento = 1;
    }
  }

  if (digitalRead(Sensor2) == 0)
  {
    sensor2 = 1;
    if (sensor1 == 0)
    {
      incremento = 0;
      decremento = 1;
    }
  }

  if (sensor1 == 1 && sensor2 == 1)
  {
    while (1)
    {
      if (incremento == 1 && digitalRead(Sensor2) == 0 && digitalRead(Sensor1) == 1)
      {
        qtdPessoas++;
        break;
      }
      else if (decremento == 1 && digitalRead(Sensor1) == 0 && digitalRead(Sensor2) == 1)
      {
        if (qtdPessoas == 0)
        {
          qtdPessoas = 0;
          break;
        }
        else
        {
          qtdPessoas--;
          break;
        }
      }
      else if ((incremento == 1 && digitalRead(Sensor1) == 1 && digitalRead(Sensor2) == 1) || (decremento == 1 && digitalRead(Sensor1) == 1 && digitalRead(Sensor2) == 1))
      {
        break;
      }
    }

    webSocket.loop();

    incremento = 0;
    decremento = 0;
    sensor1 = 0;
    sensor2 = 0;

    delay(350);
  }
}
//----------------------------------------------------------------

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.print("Conectando ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado!");
  Serial.println("Endereco IP: ");
  Serial.println(WiFi.localIP());

  delay(500);
  dht.begin();

  delay(1000);

  initialTime = millis();
  pinMode(Sensor1, INPUT);
  pinMode(Sensor2, INPUT);

  // endereco do servidor, porta e URL
  //webSocket.begin("contador-de-pessoas.herokuapp.com", 80, "/");
  webSocket.begin("192.168.15.24", 4000, "/"); // se for rodar localhost

  // event handler
  webSocket.onEvent(webSocketEvent);

  // tenta a cada 2 segundos se a conexao falhar
  webSocket.setReconnectInterval(2000);

  timeClient.begin();
}

// fica atualizando a hora com a internet e verifica se algum dado foi alterado
void loop()
{
  timeClient.update();
  contaPessoas();
}

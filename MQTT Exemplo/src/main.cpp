#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
/*
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1
*/

#define relay 0

const char* ssid = "myssid";
const char* password = "password";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

const char* mqtt_publicar = "meu/topico/resposta";
const char* mqtt_receber = "meu/topico/comando";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print(F("Conectando-se em "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    ESP.wdtFeed();
    delay(500);
    Serial.print(F("."));
  }

  randomSeed(micros());

  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  Serial.print(WiFi.localIP());  
  Serial.println(F("MAC ADDRESS "));
  Serial.print(WiFi.macAddress());
  Serial.println(F(""));  
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(F("Mensagem recebida ["));
  Serial.print(topic);
  Serial.print(F("]/"));

  String payloadMessage = "";
  for (unsigned int i = 0; i < length; i++) {
    payloadMessage +=(char)payload[i];
  }
  Serial.println(payloadMessage);
  Serial.println();

  if(String(topic).equals(mqtt_receber)){    

    // Se receber um comando ON ou 1
    if(payloadMessage.equals("ON")||payloadMessage.equals("1")){

      // muda o estado do relay
      digitalWrite(relay, HIGH);
      Serial.println(F("Comando de Ligar"));

      // Publica na resposta o tipo de mensagem recebida...
      client.publish(mqtt_publicar, "Comando de Ligar");

      // Grava o valor 1 no endereco 0 da eeprom
      EEPROM.write(0, 1);
      EEPROM.commit();
   
    }else if (payloadMessage.equals("OFF")||payloadMessage.equals("0")){

      // muda o estado do relay
      digitalWrite(relay, LOW);
      Serial.println(F("Comando de Desligar"));

      // Publica na resposta o tipo de mensagem recebida...
      client.publish(mqtt_publicar, "Comando de Desligar");

      // Grava o valor 0 no endereco 0 da eeprom
      EEPROM.write(0, 0);
      EEPROM.commit();
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  ESP.wdtFeed();            // Alimenta o WatchDog
  while (!client.connected()) {
    Serial.print(F("Tentando se conectar com o MQTT..."));
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println(F("conectado!"));
      Serial.println(F("Aguardando por comandos...")); 
      // Uma vez conectado, ira publicar a mensagem de conectado
      client.publish(mqtt_publicar, "Conectado");
      // ... and resubscribe
      client.subscribe(mqtt_receber);
    } else {
      Serial.print(F("falhou, rc="));
      Serial.print(client.state());
      Serial.println(F(" tentar novamente em 5 segundos!"));
      ESP.wdtFeed();            // Alimenta o WatchDog
      // Wait 5 seconds before retrying
      delay(5000);
      ESP.wdtFeed();            // Alimenta o WatchDog
    }
  }
}

void setup() {
  ESP.wdtFeed();            // Alimenta o WatchDog
  EEPROM.begin(1);            // Quantidade de bytes para acessar memoria, nesse caso é 1 apenas
  value = EEPROM.read(0);
  if ( value != 1){           // Se houver sujeira no endereco da memoria, seta valor como 0
    value = 0;
    EEPROM.write(0, 0);       // grava na memoria o valor 0 que será usado na saida relay...
    EEPROM.commit();
  }
  pinMode(relay, OUTPUT);
  digitalWrite(relay, value); 

  ESP.wdtFeed();            // Alimenta o WatchDog
  delay(2000);                // Um delay para dar tempo de ver abrir a serial para visualizar as mensagens printadas

  Serial.begin(9600);
  Serial.println(F("Exemplo MQTT"));  

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  ESP.wdtFeed();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

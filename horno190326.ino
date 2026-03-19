#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>

const char* ssid = "UTIM";
const char* password = "utim$$$2026";
const char* mqtt_server = "172.16.133.22";


String mensajeControl = "";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

#include <max6675.h>
#define SO0 19
#define CS0 18
#define SCK0 5
#define SO1 21
#define CS1 22
#define SCK1 23
MAX6675 termo0(SCK0, CS0, SO0);
MAX6675 termo1(SCK1, CS1, SO1);

const int pin2 = 2;
const int pin4 = 4;
String data = "";

void setup() {
  Serial.begin(115200);
  pinMode(pin2, OUTPUT);
  pinMode(pin4, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 80);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "bioR/control") {
    mensajeControl = messageTemp;
  }
}

void reconnect() {
  while (!client.connected()) {
    //Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("bioR/control");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  String data = "{\"temp0\": " + String(termo0.readCelsius()) + ", \"temp1\": " + String(termo1.readCelsius()) + "}";
  data.trim();
  Serial.println(data);
  client.publish("agave/horno", data.c_str());
  
  // For the MAX6675 to update, you must delay AT LEAST 250ms between reads!
  digitalWrite(pin2, HIGH);
  delay(3000);
  digitalWrite(pin2, LOW);
  
}

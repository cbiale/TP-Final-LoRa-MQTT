#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Pines LoRa
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

// Configuración WiFi
const char* ssid = "xxxxxxx";  // Cambia por tu SSID
const char* password = "xxxxxxx";  // Cambia por tu contraseña WiFi

// Configuración MQTT
const char* mqtt_server = "192.168.0.105";  // usar hostname -I
const int mqtt_port = 1883;
const char* mqtt_topic = "mediciones";

// Cliente MQTT
WiFiClient espClient;
PubSubClient client(espClient);

// Variables para manejo seguro
volatile bool packetReceived = false;
String receivedData = "";


void setup() {
  // Configuración puerto serie
  Serial.begin(115200);
  delay(1000);
  
  // WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando WiFi...");
  }
  Serial.println("WiFi OK: " + WiFi.localIP().toString());
  
  // MQTT
  client.setServer(mqtt_server, 1883);
  
  // LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(915E6)) {
    Serial.println("LoRa ERROR");
    while (1);
  }
  
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  
  // Callback simplificado - NO usa Serial ni client, solo marca recepción
  LoRa.onReceive(onReceive);
  LoRa.receive();
  
  Serial.println("Receptor iniciado");
}

// Callback: solo marcar que llegó algo
void onReceive(int packetSize) {
  packetReceived = true;
}

// Para reconexión MQTT
void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("ESP32-LoRa-RX")) {
      Serial.println(" OK");
      break;
    } else {
      Serial.println(" ERROR");
      delay(5000);
    }
  }
}

void loop() {
  // Procesa paquetes recibidos en el loop principal
  if (packetReceived) {
    packetReceived = false;
    
    // Lee datos
    receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    int packetRssi = LoRa.packetRssi();
    float packetSnr = LoRa.packetSnr();
    
    // Procesa y envia
    if (receivedData.length() > 0) {
      Serial.print("Datos recibidos: ");
      Serial.println(receivedData);
      String mqttPayload = convertToJSON(receivedData, packetRssi, packetSnr);

      // Si el cliente MQTT no se encuentra conectado
      if (!client.connected()) {
        // reconecta a MQTT
        reconnectMQTT();
      }
      
      // Si el cliente se encueentra conectado al broker MQTT
      if (client.connected()) {
        // Publica
        client.publish(mqtt_topic, mqttPayload.c_str());
        Serial.println("Enviando por MQTT: " + mqttPayload);
      }
    }
    
    // Vuelve a modo recepción
    LoRa.receive();
  }
  
  client.loop();
  delay(10);
}

String convertToJSON(String rawData, int rssi, float snr) {
  String json;

  // Parsear datos
  int startIndex = 0;
  int commaIndex = rawData.indexOf(',');
  bool firstSensor = true;
  bool isFirstField = true;
  
  while (commaIndex != -1) {
    String pair = rawData.substring(startIndex, commaIndex);
    
    if (isFirstField) {
      // Primer campo es ID
      String id = extractValue(pair);
      json = "{\"nodo\":\"" + id + "\",\"rssi\":" + String(rssi) + ",\"snr\":" + String(snr) + ",\"sensores\":[";
      isFirstField = false;
    } else {
      // Campos de sensores
      if (!firstSensor) json += ",";
      json += parseSensor(pair);
      firstSensor = false;
    }
    
    startIndex = commaIndex + 1;
    commaIndex = rawData.indexOf(',', startIndex);
  }
  
  // Último sensor
  if (!isFirstField) {
    String lastPair = rawData.substring(startIndex);
    if (!firstSensor) json += ",";
    json += parseSensor(lastPair);
  }
  
  json += "]}";
  return json;
}

String parseSensor(String pair) {
  int colonIndex = pair.indexOf(':');
  if (colonIndex != -1) {
    String type = pair.substring(0, colonIndex);
    String value = pair.substring(colonIndex + 1);
    
    type.toLowerCase();
    
    String sensor = "{\"tipoSensor\":\"" + type + "\",\"valor\":";
    
    if (isNumeric(value)) {
      sensor += value;
    } else {
      sensor += "\"" + value + "\"";
    }
    
    sensor += "}";
    return sensor;
  }
  return "";
}

String extractValue(String pair) {
  int colonIndex = pair.indexOf(':');
  if (colonIndex != -1) {
    return pair.substring(colonIndex + 1);
  }
  return pair;  // Si no hay ":", todo es el valor
}

bool isNumeric(String str) {
  if (str.length() == 0) return false;
  
  int startIndex = 0;
  if (str.charAt(0) == '-') startIndex = 1;  // Números negativos
  
  for (int i = startIndex; i < str.length(); i++) {
    if (!isDigit(str.charAt(i)) && str.charAt(i) != '.') {
      return false;
    }
  }
  return true;
}
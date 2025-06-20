#include <SPI.h>
#include <LoRa.h>

// Pines para Heltec V2
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

void setup() {
  Serial.begin(115200);
  
  // Inicializar LoRa
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  
  // controla error al iniciar LoRa
  if (!LoRa.begin(915E6)) {
    Serial.println("Error al iniciar LoRa!");
    while (1);
  }
  
  // Defino SF y SB
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);

  // Final de config
  Serial.println("LoRa Transmisor iniciado");
  randomSeed(analogRead(0));
}

void loop() {
  // obtengo valores de sensores
  int temperatura = random(5, 45);
  int humedad = random(20, 95);
  int humedad_suelo = random(15, 80);
  int lumninosidad = random(0, 12000);
  int presion = random(950, 1050);

  // se genera string a enviar
  String paquete = "ID:sensor-real-01,TEMP:" + String(temperatura) + ",HUM:" + String(humedad) + ",HUMS:" + String(humedad_suelo) + ",PRES:" + String(presion) + ",LUM:" + String(lumninosidad);

  // se envía el paquete
  Serial.print("Enviando paquete: ");
  Serial.println(paquete);
  LoRa.beginPacket();
  LoRa.print(paquete);
  LoRa.endPacket();

  // se espera 60 segundos
  delay(60000);
}
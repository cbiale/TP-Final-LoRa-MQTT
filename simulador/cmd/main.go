package main

import (
	"encoding/json"
	"fmt"
	"log"
	"math"
	"math/rand"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

type LoRaMessage struct {
	// Identificación del nodo
	Nodo        string        `json:"nodo"`
	Sensores    []SensorData  `json:"sensores"` // Múltiples sensores
	
	// Metadatos LoRa P2P
	Timestamp   string  `json:"timestamp"`
	RSSI        int     `json:"rssi"`
	SNR         float64 `json:"snr"`
	Frecuencia  int     `json:"frecuencia"`
	SpreadingFactor int `json:"spreadingFactor"`
	Bandwidth   int     `json:"bandwidth"`
	
	// Información de red P2P
	NodoOrigen  string  `json:"nodoOrigen"`
	NodoDestino string  `json:"nodoDestino"`
	NumeroMensaje int   `json:"numeroMensaje"`
	NivelBateria  int   `json:"nivelBateria"`
}

// Estructura simple para cada sensor
type SensorData struct {
	TipoSensor string  `json:"tipoSensor"`
	Valor      float64 `json:"valor"`
}

var contadorMensajes = make(map[string]int)

func main() {
	// Configuración MQTT
	broker := "tcp://localhost:1883"
	clientID := "lora-p2p-multi-simulator"
	
	// Crear cliente MQTT
	opts := mqtt.NewClientOptions()
	opts.AddBroker(broker)
	opts.SetClientID(clientID)

	client := mqtt.NewClient(opts)
	if token := client.Connect(); token.Wait() && token.Error() != nil {
		log.Fatal(token.Error())
	}
	defer client.Disconnect(250)

	fmt.Println("Simulador LoRa P2P Multi-Sensor iniciado...")

	// Simular datos cada 30 segundos
	ticker := time.NewTicker(30 * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			sendMultiSensorLoRaData(client)
		}
	}
}

// sendMultiSensorLoRaData envía un mensaje LoRa con múltiples sensores
// simulando un nodo agrícola.
// Cada mensaje incluye lecturas de varios sensores y metadatos LoRa.
// Utiliza un contador para identificar el número de mensaje por nodo.
// Los nodos y sensores son seleccionados aleatoriamente de listas predefinidas.
// El mensaje se publica en un topic MQTT específico.
// El formato del mensaje incluye:
// - Nodo origen y destino
// - Lecturas de sensores (temperatura, humedad, etc.)
// - Metadatos LoRa (RSSI, SNR, frecuencia, spreading factor,
//   ancho de banda, timestamp, nivel de batería)
// - Contador de mensajes para el nodo
// - Formato JSON para el payload del mensaje
// - Resumen de transmisión en consola
func sendMultiSensorLoRaData(client mqtt.Client) {
	// Definir nodos disponibles
	nodos := []string{"sensor-agricola-01", "sensor-agricola-02", "sensor-agricola-03", "sensor-invernadero-01"}
	
	// Frecuencias LoRa típicas (Argentina - 915 MHz)
	frecuencias := []int{915200000, 915400000, 915600000, 915800000}
	spreadingFactors := []int{7, 8, 9, 10, 11, 12}
	bandwidths := []int{125000, 250000, 500000}

	// Seleccionar nodo aleatoriamente
	nodoOrigen := nodos[rand.Intn(len(nodos))]
	
	// Incrementar contador de mensajes para este nodo
	contadorMensajes[nodoOrigen]++
	
	// Generar lecturas para todos los sensores del nodo
	sensores := generateSensorReadings()

	// Crear mensaje LoRa con múltiples sensores
	msg := LoRaMessage{
		Nodo:            nodoOrigen,
		Sensores:        sensores,
		Timestamp:       time.Now().Format(time.RFC3339),
		RSSI:            rand.Intn(60) - 130, // -130 a -70 dBm
		SNR:             float64(rand.Intn(30)-15) + rand.Float64(), // -15 a 15 dB
		Frecuencia:      frecuencias[rand.Intn(len(frecuencias))],
		SpreadingFactor: spreadingFactors[rand.Intn(len(spreadingFactors))],
		Bandwidth:       bandwidths[rand.Intn(len(bandwidths))],
		NodoOrigen:      nodoOrigen,
		NodoDestino:     "gateway-central",
		NumeroMensaje:   contadorMensajes[nodoOrigen],
		NivelBateria:    85 + rand.Intn(15), // 85-100%
	}

	payload, _ := json.Marshal(msg)
	topic := fmt.Sprintf("mediciones/%s", nodoOrigen)
	
	token := client.Publish(topic, 0, false, payload)
	token.Wait()
	
	// Mostrar resumen de transmisión
	sensorList := ""
	for i, sensor := range sensores {
		if i > 0 {
			sensorList += ", "
		}
		sensorList += fmt.Sprintf("%s=%.1f", sensor.TipoSensor, sensor.Valor)
	}
	
	fmt.Printf("📦 - %s → %s | %d sensores [%s] | SF%d | RSSI:%d dBm | Bat:%d%% | Topic:%s\n", 
		msg.NodoOrigen, msg.NodoDestino, len(sensores), sensorList, 
		msg.SpreadingFactor, msg.RSSI, msg.NivelBateria, topic)
}


// generateSensorReadings genera lecturas aleatorias para múltiples sensores
// típicos en un entorno agrícola.
// Cada sensor tiene un tipo y un valor dentro de un rango específico.
func generateSensorReadings() []SensorData {
	// Todos los sensores ambientales típicos
	allSensors := []struct {
		tipo string
		min  float64
		max  float64
	}{
		{"temperatura", 5.0, 45.0},
		{"humedad", 20.0, 95.0},
		{"humedad_suelo", 15.0, 80.0},
		{"presión", 950.0, 1050.0},
		{"luminosidad", 0.0, 12000.0},
	}
	
	var sensores []SensorData
	
	for _, sensorDef := range allSensors {
		// Generar valor dentro del rango
		valor := sensorDef.min + rand.Float64()*(sensorDef.max-sensorDef.min)
		
		sensores = append(sensores, SensorData{
			TipoSensor: sensorDef.tipo,
			Valor:      math.Round(valor*100)/100, // Redondear a 2 decimales
		})
	}
	
	return sensores
}
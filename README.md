# Sistema IoT LoRa → MQTT

## Descripción

Este proyecto implementa un sistema de telemetría agrícola utilizando módulos ESP32 LoRa para la transmisión de datos de sensores, MQTT para la comunicación, Node-RED para el procesamiento y PostgreSQL para el almacenamiento de datos.

El proyecto fue desarrollado como trabajo final de la  asignatura **Principios y Aplicaciones para dispositivos LoRa/LoRaWAN** dictada por el Docente **Dr. Lic. Roberto Federico FARFÁN** en el marco de la **Maestría en Internet de las Cosas** dictada por la **Facultad de Ingeniería** de la **Universidad de Buenos Aires**.


## Estructura del Proyecto

```
.
├── esp32/                         # Código para dispositivos ESP32
│   ├── emisor/                    # Nodo sensor que envía datos
│   │   └── emisor.ino             # Firmware que simula sensores y envía por LoRa
│   └── receptor/                  # Nodo receptor/gateway
│       └── receptor.ino           # Firmware que recibe por LoRa y reenvía por MQTT
├── flows/                         # Configuración de Node-RED
│   └── flows.json                 # Flujo para procesamiento y visualización
├── presentacion/                  # Documentación y presentación
│   ├── dashboard.png              # Captura de pantalla del dashboard
│   ├── flujo-node-red.png         # Diagrama del flujo implementado
│   ├── presentacion.md            # Presentación en formato Markdown
│   └── presentacion.pdf           # Presentación en formato PDF
├── simulador/                     # Simulador de nodos para pruebas
│   ├── cmd/
│   │   └── main.go                # Simulador en Go para generar mensajes MQTT
│   ├── go.mod                     # Dependencias de Go
│   └── go.sum                     # Checksums de dependencias
├── .gitignore                     # Archivos excluidos del repositorio
├── iniciar_broker.sh              # Script para iniciar broker MQTT con Docker
├── package.json                   # Dependencias de Node.js para Node-RED
├── package-lock.json              # Bloqueo de versiones exactas de dependencias 
└── README.md                      # Este archivo
```

## Tecnologías Utilizadas

- **Hardware**: ESP32 Heltec LoRa V2
- **Protocolos**: LoRa P2P (915MHz), MQTT
- **Backend**: Node-RED, PostgreSQL
- **Broker MQTT**: EMQx en Docker
- **Lenguajes**: C++ (Arduino), JavaScript (Node-RED), Go (Simulador)

## Componentes del Sistema

Nodo Emisor (ESP32)
- Simula datos de sensores (temperatura, humedad, presión, etc.)
- Transmite los datos vía LoRa en formato compacto

Nodo Receptor (ESP32)
- Recibe mensajes LoRa
- Convierte los datos a formato JSON
- Conecta por WiFi al broker MQTT
- Publica los mensajes en tópicos específicos

Node-RED

- Suscribe a tópicos MQTT
- Procesa y almacena datos en PostgreSQL
- Proporciona un dashboard para visualización
- Permite consultar datos históricos

Simulador Go

- Genera datos de sensores simulados
- Publica en MQTT para pruebas sin hardware
- Incluye metadatos de LoRa para simulación completa

## Instalación

### **Broker MQTT**:

```bash
./iniciar_broker.sh
```

### **Node-RED**:

```bash
npm install
node-red
```

Importar el archivo `flows.json`

### **Base de datos**: 

Crear tabla en PostgreSQL (adecuar la conexión a PostgreSQL en Node-RED de acuerdo a su configuración):

```sql
CREATE TABLE mediciones (
  id_medicion SERIAL PRIMARY KEY,
  tiempo TIMESTAMP NOT NULL,
  nodo VARCHAR(100) NOT NULL,
  tipo_sensor VARCHAR(100) NOT NULL,
  valor FLOAT NOT NULL
);
```

### **Simulador** (opcional):

```bash
cd simulador
# Asegurar que las dependencias estén correctamente configuradas
go mod tidy
# Ejecutar el simulador
go run cmd/main.go
```

### **ESP32**: 

Preparar el entorno Arduino IDE:

Preparar el entorno Arduino IDE:

1. Instalar las siguientes bibliotecas usando el Gestor de Bibliotecas:

  - Adafruit BusIO by Adafruit
  - Heltec ESP32 Dev-Boards by Heltec Automation
  - LoRa by Sandeep Mistry
  - PubSubClient by Nick O'Leary

2. Configurar el soporte para placas ESP32:

 - Añadir https://resource.heltec.cn/download/package_heltec_esp32_index.json a la lista de URLs de Gestor de Tarjetas en las preferencias de Arduino IDE.

3. Cargar los archivos .ino:
- Cargar `emisor.ino` al nodo emisor
- Cargar `receptor.ino` al nodo receptor


## Dashboard

Acceso al dashboard de Node-RED: `http://localhost:1880/ui`

## Autor

Claudio Omar Biale









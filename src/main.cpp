#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPDash.h>
#include "time.h"

#define LATCH  7
#define CLOCK  4
#define DATA   6
#define GUION  9
#define PUNTOS 8

// --- Variables globales ---
uint8_t DisplayBuffer [4] = {0, 0, 0, 0};
bool DosPuntos = false;

// --- Configuración Wi-Fi ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// --- Configuración NTP ---
const char* ntpServer = "ar.pool.ntp.org";
const long gmtOffset_sec = -10800; // GMT-3 para Argentina
const int daylightOffset_sec = 0;

// --- Web Server ---
AsyncWebServer server (80);

// --- Attach del ESP-DASH al server web (tablero) ---
ESPDash dashboard (server);

// --- Tarjetas de estado (solo lectura) ---
dash::GenericCard<dash::string> cardTime (dashboard, "Hora actual");
dash::GenericCard<dash::string> cardStatus (dashboard, "Estado LEDs");
dash::GenericCard<dash::string> cardRelayStatus (dashboard, "Estado relé");
dash::GenericCard<dash::string> cardAlarmInfo (dashboard, "Proxima alarma");

// --- Tarjetas interactivas ---
dash::GenericCard<dash::string> cardLedEnable (dashboard, "LEDs Encendido");
dash::SliderCard<int> cardLedR (dashboard, "LED Rojo", 0, 255, 1, "");

dash::SliderCard<int> cardLedBright (dashboard, "LEDs Brillo", 0, 100, 1, "%");
dash::SliderCard<int> cardLedG (dashboard, "LED Verde", 0, 255, 1, "%");
dash::SliderCard<int> cardLedB (dashboard, "LED Azul", 0, 255, 1, "%");
dash::SliderCard<int> cardLedEffect (dashboard, "Efecto (0=Sol 1=Respuesta 2=Arco 3=Persecución)", 0, 3, 1, "");

// --- Sección Relé ---
dash::ToggleButtonCard cardRelay (dashboard, "Relé ON/OFF");
 
// --- Sección Pantalla ---
dash::ToggleButtonCard cardFormat24 (dashboard, "Formato 24h");
dash::ToggleButtonCard cardNightMode (dashboard, "Modo Noche");
dash::SliderCard<int> cardDispBright (dashboard, "Brillo Pantalla", 0, 100, 1, "%");

// --- Sección Alarmas (5 espacios)
dash::SliderCard<int> cardAlarm1 (dashboard, "Alarma 1 — 07:00", 0, 24, 1, "%");
dash::SliderCard<int> cardAlarm2 (dashboard, "Alarma 2 — 12:30", 0, 24, 1, "%");
dash::SliderCard<int> cardAlarm3 (dashboard, "Alarma 3 — 13:30", 0, 24, 1, "%");
dash::SliderCard<int> cardAlarm4 (dashboard, "Alarma 4 — 14:20", 0, 24, 1, "%");
dash::SliderCard<int> cardAlarm5 (dashboard, "Alarma 5 — 15:30", 0, 24, 1, "%");

// Mapa de segmentos (Cátodo Común)
constexpr uint8_t Digitos [] = {
//dpgfedcba (MSB)
  B00111111, // 0
  B00000110, // 1
  B01011011, // 2
  B01001111, // 3
  B01100110, // 4
  B01101101, // 5
  B01111101, // 6
  B00000111, // 7
  B01111111, // 8
  B01101111  // 9
};

// Mensaje de bienvenida
constexpr uint8_t MSJ_HOLA [] = {
//dpgfedcba (MSB)
	B01110110,  // H
	B00111111,  // O
	B00111000,  // L
	B01110111,  // A
};

// --- Prototipos ---
void hola ();
void actualizaHora ();
void actualizaDisplay ();

void setup() {

  pinMode (LATCH, OUTPUT);
  pinMode (DATA, OUTPUT);
  pinMode (CLOCK, OUTPUT);
  pinMode (GUION, OUTPUT);
  pinMode (PUNTOS, OUTPUT);
  Serial.begin (115200);

  // Mensaje inicial
  hola ();

  // Conectar WiFi
  WiFi.begin (ssid, password);

  Serial.print ("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay (500);
    Serial.print (".");
  }

  Serial.print (" WiFi conectado, IP: ");
  Serial.println (WiFi.localIP());

  // --- Funciones de Callback
  cardTime.setValue("12:00");
  cardTime.setSymbol("Simbolo");

  server.begin();
  configTime (gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {

  actualizaHora ();
  actualizaDisplay ();

  (DosPuntos)? digitalWrite (PUNTOS, HIGH): digitalWrite (PUNTOS, LOW);
  DosPuntos = !DosPuntos;
  delay (500);
}

void actualizaHora () {
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  DisplayBuffer [3] = timeinfo.tm_hour / 10;
  DisplayBuffer [2] = timeinfo.tm_hour % 10;
  DisplayBuffer [1] = timeinfo.tm_min / 10;
  DisplayBuffer [0] = timeinfo.tm_min % 10;
}

void hola () {

  digitalWrite (LATCH, LOW);
   
  for (short int i = 3; i >= 0; i --) 
    shiftOut (DATA, CLOCK, MSBFIRST, MSJ_HOLA [i]);

  digitalWrite(LATCH, HIGH);
}

void actualizaDisplay() {

  digitalWrite (LATCH, LOW);
  
  for (uint8_t i = 0; i < 4; i++) 
    shiftOut (DATA, CLOCK, MSBFIRST, Digitos [DisplayBuffer[i]]);

  digitalWrite(LATCH, HIGH);
}
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>
#include <Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>

// ==================== CONFIGURACIÓN DE PINES ====================
#define SS_PIN 5
#define RST_PIN 22
#define DFPLAYER_RX 17
#define DFPLAYER_TX 16
#define BTN_NEXT 21
#define BTN_PREV 2
#define BTN_VOL_UP 34
#define BTN_VOL_DOWN 15
#define POT_BRILLO 35
#define LED_PIN 32
#define LED_COUNT 64
#define BTN_PAUSE 4
#define EEPROM_SIZE 512
#define EEPROM_SSID_ADDR 0
#define EEPROM_PASS_ADDR 100
#define EEPROM_CONFIGURED_ADDR 200
/// 
byte ultimoUIDLeido[10];  // Almacena el último UID detectado
byte ultimoUIDSize = 0;   // Tamaño del último UID
unsigned long ultimaDeteccionRFID = 0;  // Tiempo de última detección
const unsigned long COOLDOWN_RFID = 5000;  // 5 segundos entre lecturas del mismo llavero
bool modoRFIDAutomatico = false;  // Bandera para cambio automático
int ultimoPistaRFID = -1;
bool audioPausado = false; 

// ==================== WEB SERVER ====================
WebServer server(80);
bool ipYaMostrada = false;  // Evita mostrar IP si ya se conectó
unsigned long tiempoUltimaConexionApp = 0;  // Detecta conexión desde app
const unsigned long TIMEOUT_CONEXION_APP = 5000;  // 5 segundos sin request = desconexión

// Variables de estado WiFi
unsigned long wifiConnectStartTime = 0;
bool wifiConectando = false;
bool wifiConectado = false;
unsigned long lastWifiCheck = 0;
const unsigned long wifiCheckInterval = 5000;  // Revisar WiFi cada 5 seg

unsigned long lastClientCheck = 0;
const unsigned long clientCheckInterval = 2000;  // Revisar clientes cada 2 seg
bool clienteConectado = false;

//----------------------------
bool modoAP = false;  // ¿Está en modo Access Point?
const char* AP_SSID = "StoryCube-Setup";
const char* AP_PASSWORD = "12345678";  // Contraseña del AP
IPAddress apIP(192, 168, 4, 1);  // IP fija del AP
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
//--------------------------------
int ultimoValorPotenciometroLeido = -999;
int ultimoValorPotenciometroAntesDeCambioApp = -999;  
const int DIFERENCIA_MINIMA = 25;
unsigned long lastBrilloRead = 0;
const unsigned long brilloReadInterval = 100;

// BANDERA: ¿El pot tiene permiso de cambiar el brillo?
bool potTieneControl = true;
// BANDERA: ¿Quién controla ahora?
enum ControlDeBrillo { CONTROL_FISICO, CONTROL_APP };
ControlDeBrillo controlActual = CONTROL_FISICO;
// ==================== CONFIGURACIÓN MATRIZ LED ====================
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, LED_PIN,
  NEO_MATRIX_LEFT + NEO_MATRIX_TOP +
  NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB + NEO_KHZ800);

// Variables de scroll
int scrollX = 8;
String textoActual = "";
uint32_t colorActual = 0;
bool scrollActivo = false;
unsigned long lastScrollUpdate = 0;
const unsigned long scrollInterval = 150;
int16_t textoAncho = 0;

// Variables para detectar fin de reproducción
unsigned long lastAudioCheck = 0;
const unsigned long audioCheckInterval = 1000;
bool esperandoFinAudio = false;

// ==================== CONFIGURACIÓN TECLADO MATRICIAL ====================
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {33, 25, 26, 27};
byte colPins[COLS] = {14, 12, 13};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------------------------------------------------------------------------------------

unsigned long ultimaCambioBrilloApp = 0;
const unsigned long BLOQUEO_POT_DURACION = 3000;  // 3 segundos de bloqueo después de app

// ==================== OBJETOS ====================
MFRC522 rfid(SS_PIN, RST_PIN);
HardwareSerial dfSerial(2);
DFRobotDFPlayerMini myDFPlayer;

// ==================== UIDs DE LOS LLAVEROS ====================
byte UID1[] = {0x14, 0xEA, 0x4D, 0xBC};
byte UID2[] = {0x04, 0x35, 0x45, 0xBC};
byte UID3[] = {0x54, 0x32, 0x3A, 0xBC};
byte UID4[] = {0x66, 0x27, 0x54, 0x14};
byte UID5[] = {0x54, 0xA3, 0x9D, 0x74};
byte UID6[] = {0xA3, 0x52, 0xFB, 0xF4};

// ==================== VARIABLES DE CONTROL ====================
unsigned long lastReadTime = 0;
const unsigned long READ_DELAY = 10000;
bool dfplayerReady = false;

enum Modo {
   MODO_RFID, 
   MODO_MANUAL, 
   MODO_NUMEROS,
    MODO_COLORES };
enum SubModoNumeros { 
  NUMEROS_ESPANOL,
  NUMEROS_INGLES };
SubModoNumeros subModoNumeros = NUMEROS_ESPANOL;
Modo modoActual = MODO_RFID;

int pista_actual = 1;
const int NUM_CUENTOS = 6;
int volumen_actual = 20;
const int VOL_MIN = 0;
const int VOL_MAX = 50;

unsigned long lastButtonPress[4] = {0, 0, 0, 0};
const unsigned long buttonDelay = 300;

int brillo_actual = 50;
const int BRILLO_MIN = 5;
const int BRILLO_MAX = 255;

//mostrar IP
unsigned long tiempoIPMostrada = 0;
const unsigned long TIEMPO_IP = 30000;

bool reproduciendo = false;
// Variables para controlar el estado de la pantalla
enum EstantallaPantalla { 
  PANTALLA_SCROLL, 
  PANTALLA_NUMERO, 
  PANTALLA_COLOR, 
  PANTALLA_STORYCUBE,
  PANTALLA_IP  
};
EstantallaPantalla estadoPantalla = PANTALLA_STORYCUBE;
int numeroMostrado = -1;
unsigned long tiempoNumeroMostrado = 0;
const unsigned long TIEMPO_NUMERO = 5000;  // 5 segundos antes de desaparecer


// ==================== PATRONES DE NÚMEROS ====================
const uint8_t numeros[10][8] PROGMEM = {
  {0b00111100, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  {0b00011000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b01111110, 0b00000000},
  {0b00111100, 0b01100110, 0b00000110, 0b00001100, 0b00011000, 0b00110000, 0b01111110, 0b00000000},
  {0b00111100, 0b01100110, 0b00000110, 0b00011100, 0b00000110, 0b01100110, 0b00111100, 0b00000000},
  {0b00001100, 0b00011100, 0b00101100, 0b01001100, 0b01111110, 0b00001100, 0b00001100, 0b00000000},
  {0b01111110, 0b01100000, 0b01111100, 0b00000110, 0b00000110, 0b01100110, 0b00111100, 0b00000000},
  {0b00111100, 0b01100000, 0b01111100, 0b01100110, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  {0b01111110, 0b00000110, 0b00001100, 0b00011000, 0b00110000, 0b00110000, 0b00110000, 0b00000000},
  {0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b01100110, 0b01100110, 0b00111100, 0b00000000},
  {0b00111100, 0b01100110, 0b01100110, 0b00111110, 0b00000110, 0b00001100, 0b00111000, 0b00000000}
};

// ==================== DECLARACIÓN DE FUNCIONES ====================
void inicializarRFID();
void inicializarDFPlayer();
void inicializarBotones();
void inicializarLED();
void inicializarWebServer();

void modoRFID();
void modoManual();
void modoColores();
void cambiarModo();
void pausarReanudar();
void procesarTecla(char tecla);



bool compararUID(byte* uid1, byte size1, byte* uid2, byte size2);
void reproducirCuento(int numero, const char* nombre);
void reproducirNumero(int numero);

void botonSiguiente();
void botonAnterior();
void botonSubirVolumen();
void botonBajarVolumen();
void leerPotenciometroBrillo();
void actualizarLED();
void mostrarNumeroLED(int num);
void iniciarScrollTexto(String texto, uint32_t color);
void detenerScroll();
void actualizarScroll();
void mostrarColorCompleto(int numeroColor);
void mostrarStoryCube();
void verificarFinAudio();
void cambiarSubModoNumeros();

void mostrarModoConfiguracion();

void mostrarEstadoWiFi();
void monitearWiFi();
void mostrarIPEnConsola();
void configurarWiFi();
void iniciarModoAP();
void guardarWiFiEnEEPROM(String ssid, String password);
bool cargarWiFiDesdeEEPROM(String &ssid, String &password);
void mostrarIPEnLED();
void mostrarModoConfiguracionConIP();

void handleStatus();
void handleSetVolume();
void handleSetBrightness();
void handlePlay();
void handlePause();
void handleNext();
void handlePrevious();
void handleSetMode();
void handleGetIP();
void handlePing() ;
void handleWiFiConfig();
void handleNetworkInfo();
bool modoPermiteControlesReproduccion();
// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== STORYCUBE INICIANDO ===\n");
  
  // Inicializar componentes ANTES de WiFi
  inicializarBotones();
  inicializarLED();
  inicializarRFID();
  inicializarDFPlayer();
  
  // MOSTRAR STORYCUBE INMEDIATAMENTE
  mostrarStoryCube();
  scrollActivo = true;
  
  // INICIALIZAR UID vacío
  ultimoUIDSize = 0;
  for (int i = 0; i < 10; i++) {
    ultimoUIDLeido[i] = 0;
  }
  
  // Configurar WiFi (no bloqueante)
  Serial.println("📡 Iniciando WiFi...");
  configurarWiFi();
  
  // Esperar conexión WiFi antes de continuar
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && !modoAP && intentos < 60) {
    delay(500);
    Serial.print(".");
    intentos++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n WiFi conectado en setup");
    
    // Activar modo RFID automáticamente si conectó a red normal
    String ssidConectada = WiFi.SSID();
    if (ssidConectada != AP_SSID) {
      modoActual = MODO_RFID;
      Serial.println(" Modo RFID activado automáticamente (red normal)");
      
      // Reproducir sonido de modo RFID
      if (dfplayerReady) {
        myDFPlayer.play(7);
        reproduciendo = true;
        esperandoFinAudio = true;
      }
    }
    
    // NO mostrar IP, mantener StoryCube activo
  }
  
  // Inicializar Web Server (se ejecuta aunque no hay WiFi)
  inicializarWebServer();

  Serial.println("✓ Sistema listo\n");
}

// ==================== LOOP ====================
void loop() {
  server.handleClient();
  monitearWiFi();
  
  // SIEMPRE LEE EL POT (la función decide internamente si aplica o no el cambio)
  leerPotenciometroBrillo();

  if (WiFi.status() == WL_CONNECTED && !wifiConectado) {
    wifiConectado = true;
    Serial.print("🌐 IP del ESP32: ");
    Serial.println(WiFi.localIP());
  }

  if (dfplayerReady && esperandoFinAudio) {
    verificarFinAudio();
  }

  static unsigned long lastPausePress = 0;
if (digitalRead(BTN_PAUSE) == LOW && (millis() - lastPausePress > buttonDelay)) {
  if (modoPermiteControlesReproduccion()) {
    lastPausePress = millis();
    pausarReanudar();
  } else {
    Serial.println(" Pausa no disponible en este modo");
  }
}

  char tecla = keypad.getKey();
  if (tecla) {
    procesarTecla(tecla);
  }

  actualizarLED();

  switch(modoActual) {
    case MODO_RFID:
      modoRFID();
      break;
    case MODO_MANUAL:
      modoManual();
      break;
    case MODO_COLORES:
      modoColores();
      break;
    default:
      break;
  }
}
bool modoPermiteControlesReproduccion() {
  return (modoActual == MODO_RFID || modoActual == MODO_MANUAL);
}
// ==================== CONFIGURAR WIFI ====================
void configurarWiFi() {
  String ssid, password;
  
  //  INTENTAR CARGAR CREDENCIALES GUARDADAS
  if (cargarWiFiDesdeEEPROM(ssid, password)) {
    Serial.println("\n📡 ===== MODO CLIENTE (STA) =====");
    Serial.println("Conectando a red guardada: " + ssid);
    
    // Modo cliente normal
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    wifiConectando = true;
    wifiConnectStartTime = millis();
    modoAP = false;
    
    Serial.println("Esperando conexión...");
    
  } else {
    // NO HAY CREDENCIALES → MODO ACCESS POINT
    Serial.println("\n ===== MODO ACCESS POINT (AP) =====");
    Serial.println("  No hay red WiFi configurada");
    Serial.println("Creando red de configuración...");
    
    iniciarModoAP();
  }
}
void iniciarModoAP() {
  WiFi.disconnect();
  delay(100);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, gateway, subnet);
  
  bool apIniciado = WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  if (apIniciado) {
    modoAP = true;
    wifiConectado = false;
    wifiConectando = false;
    
    IPAddress IP = WiFi.softAPIP();
    
    Serial.println("\n ========================================");
    Serial.println("✓ MODO ACCESS POINT ACTIVADO");
    Serial.println("========================================");
    Serial.print("Red WiFi: ");
    Serial.println(AP_SSID);
    Serial.print("Contraseña: ");
    Serial.println(AP_PASSWORD);
    Serial.print("IP del servidor: ");
    Serial.println(IP);
    Serial.println("\n INSTRUCCIONES PARA CONFIGURAR:");
    Serial.println("1. Conecta tu teléfono a la red: " + String(AP_SSID));
    Serial.println("2. Abre la app StoryCube");
    Serial.println("3. Ve a Configuración WiFi");
    Serial.println("4. Ingresa tu red WiFi y contraseña");
    Serial.println("5. El StoryCube se reiniciará y conectará");
    Serial.println("========================================\n");
    
    // MOSTRAR INFO EN LED
    mostrarModoConfiguracionConIP();
    
  } else {
    Serial.println(" Error iniciando Access Point");
  }
}
void mostrarModoConfiguracionConIP() {
  detenerScroll();
  
  Serial.println("📺 Modo Configuración con IP en LED");
  
  IPAddress IP = WiFi.softAPIP();
  String mensaje = "SETUP: " + String(AP_SSID) + " | IP: " + IP.toString();
  
  uint32_t colorConfig = matrix.Color(255, 100, 0);
  iniciarScrollTexto(mensaje, colorConfig);
  scrollActivo = true;
}
void monitearWiFi() {
  if (modoAP) {
    static unsigned long lastAPCheck = 0;
    if (millis() - lastAPCheck > 10000) {
      lastAPCheck = millis();
      int numClientes = WiFi.softAPgetStationNum();
      if (numClientes > 0) {
        Serial.print("📱 Clientes conectados al AP: ");
        Serial.println(numClientes);
      }
    }
    return;
  }
  
  // VERIFICAR SI HAY CLIENTE CONECTADO (timeout después de 10 segundos)
  if (millis() - lastClientCheck > clientCheckInterval) {
    lastClientCheck = millis();
    
    if (clienteConectado && (millis() - tiempoUltimaConexionApp > 10000)) {
      clienteConectado = false;
      ipYaMostrada = false;
      
      Serial.println("\n  Cliente desconectado - mostrando IP nuevamente");
      
      if (WiFi.status() == WL_CONNECTED) {
        mostrarIPEnLED();
      }
    }
  }
  
  if (millis() - lastWifiCheck < wifiCheckInterval) return;
  lastWifiCheck = millis();

  int statusActual = WiFi.status();

  if (wifiConectando) {
    if (statusActual == WL_CONNECTED) {
      wifiConectado = true;
      wifiConectando = false;
      
      Serial.println("\n ========================================");
      Serial.println("✓ WiFi CONECTADO");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      Serial.print("Fuerza señal: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
      Serial.println("========================================\n");
      
      mostrarIPEnConsola();
      
      // Mostrar StoryCube y cambiar a modo RFID automáticamente
      mostrarStoryCube();
      scrollActivo = true;
      
      //  Cambiar a modo RFID si no estamos en modo AP
      if (modoActual != MODO_RFID) {
        modoActual = MODO_RFID;
        modoRFIDAutomatico = true;
        Serial.println("Modo RFID activado automáticamente");
        
        // Reproducir sonido de modo RFID
        if (dfplayerReady) {
          myDFPlayer.play(7);  // Audio de modo RFID
          reproduciendo = true;
          esperandoFinAudio = true;
        }
      }
      
      ipYaMostrada = false;
      // NO llamar mostrarIPEnLED() aquí, mantener StoryCube
    }
    else if (millis() - wifiConnectStartTime > 30000) {
      wifiConectando = false;
      Serial.println("\n Timeout conectando a WiFi");
      Serial.println("Volviendo a modo Access Point...\n");
      
      iniciarModoAP();
    }
  }
  
  if (wifiConectado && statusActual != WL_CONNECTED) {
    wifiConectado = false;
    ipYaMostrada = false;
    clienteConectado = false;
    Serial.println("\n  WiFi desconectado - intentando reconectar...");
    WiFi.reconnect();
  }
}

void mostrarIPEnLED() {
  if (ipYaMostrada) {
    mostrarStoryCube();
    return;
  }
  
  IPAddress ip = WiFi.localIP();
  String ipStr = ip.toString();
  
  Serial.println("\n📺 Mostrando IP en matriz LED: " + ipStr);
  
  uint32_t colorIP = matrix.Color(0, 255, 0);
  
  String mensaje = "IP: " + ipStr;
  iniciarScrollTexto(mensaje, colorIP);
  scrollActivo = true;
  
  estadoPantalla = PANTALLA_IP;
  tiempoIPMostrada = millis();
  ipYaMostrada = true;
}


void mostrarIPEnConsola() {
  Serial.println("\n =====================================");
  Serial.println("   INFORMACIÓN DEL SERVIDOR");
  Serial.println("=====================================");
  Serial.print("IP Local: ");
  Serial.println(WiFi.localIP());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());
  Serial.println("Puerto: 80");
  Serial.println("\nDesde tu móvil/app usa:");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/api/status");
  Serial.println("=====================================📡\n");
}


// ==================== FUNCIONES EEPROM ====================
void guardarWiFiEnEEPROM(String ssid, String password) {
  EEPROM.begin(EEPROM_SIZE);
  
  // Limpiar las áreas
  for (int i = 0; i < 100; i++) {
    EEPROM.write(EEPROM_SSID_ADDR + i, 0);
    EEPROM.write(EEPROM_PASS_ADDR + i, 0);
  }
  
  // Guardar SSID
  for (int i = 0; i < ssid.length() && i < 32; i++) {
    EEPROM.write(EEPROM_SSID_ADDR + i, ssid[i]);
  }
  
  // Guardar Password
  for (int i = 0; i < password.length() && i < 64; i++) {
    EEPROM.write(EEPROM_PASS_ADDR + i, password[i]);
  }
  
  // Marcar como configurado
  EEPROM.write(EEPROM_CONFIGURED_ADDR, 1);
  
  EEPROM.commit();
  EEPROM.end();
  
  Serial.println("✓ WiFi guardado en EEPROM");
}

bool cargarWiFiDesdeEEPROM(String &ssid, String &password) {
  EEPROM.begin(EEPROM_SIZE);
  
  // Verificar si está configurado
  if (EEPROM.read(EEPROM_CONFIGURED_ADDR) != 1) {
    EEPROM.end();
    Serial.println("  WiFi no configurado en EEPROM");
    return false;
  }
  
  // Leer SSID
  ssid = "";
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(EEPROM_SSID_ADDR + i);
    if (c == 0) break;
    ssid += c;
  }
  
  // Leer Password
  password = "";
  for (int i = 0; i < 64; i++) {
    char c = EEPROM.read(EEPROM_PASS_ADDR + i);
    if (c == 0) break;
    password += c;
  }
  
  EEPROM.end();
  
  Serial.println("✓ WiFi cargado desde EEPROM");
  Serial.println("SSID: " + ssid);
  return true;
}
void handlePing() {
  tiempoUltimaConexionApp = millis();
  
  // DETECTAR CLIENTE CONECTADO - CAMBIAR A STORYCUBE
  if (!clienteConectado) {
    clienteConectado = true;
    ipYaMostrada = true;
    
    Serial.println("\n ========================================");
    Serial.println(" CLIENTE CONECTADO A LA APP");
    Serial.println("========================================\n");
    
    // Cambiar a StoryCube y modo RFID automáticamente
    mostrarStoryCube();
    
    if (modoActual != MODO_RFID) {
      modoActual = MODO_RFID;
      Serial.println(" Modo RFID activado automáticamente (app conectada)");
      
      if (dfplayerReady) {
        myDFPlayer.play(7);
        reproduciendo = true;
        esperandoFinAudio = true;
      }
    }
  }
  
  JsonDocument doc;
  doc["status"] = "pong";
  doc["device"] = "StoryCube";
  doc["client_connected"] = true;
  
  String response;
  serializeJson(doc, response);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

// ==================== INICIALIZAR WEB SERVER ====================
void inicializarWebServer() {
  server.on("/", HTTP_GET, []() {
    JsonDocument doc;
    doc["status"] = "OK";
    doc["device"] = "StoryCube";
    doc["version"] = "1.0";
    
    String response;
    serializeJson(doc, response);
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
  });
  
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/ping", HTTP_GET, handlePing);
  server.on("/api/volume", HTTP_POST, handleSetVolume);
  server.on("/api/brightness", HTTP_POST, handleSetBrightness);
  server.on("/api/play", HTTP_POST, handlePlay);
  server.on("/api/pause", HTTP_POST, handlePause);
  server.on("/api/next", HTTP_POST, handleNext);
  server.on("/api/previous", HTTP_POST, handlePrevious);
  server.on("/api/mode", HTTP_POST, handleSetMode);
  
  server.on("/api/wifi-config", HTTP_POST, handleWiFiConfig);
  server.on("/api/network-info", HTTP_GET, handleNetworkInfo);
  
  server.on("/api/get-ip", HTTP_GET, handleGetIP);
  server.begin();
  Serial.println("✓ Web Server iniciado en puerto 80\n");
}
// ==================== HANDLERS WEB ====================
void handleStatus() {
  tiempoUltimaConexionApp = millis();
  
   if (estadoPantalla == PANTALLA_IP && !ipYaMostrada) {
    ipYaMostrada = true;
    mostrarStoryCube();
  }
  JsonDocument doc;
  doc["playing"] = reproduciendo;
  doc["track"] = pista_actual;
  doc["volume"] = volumen_actual;
  doc["brightness"] = brillo_actual;
  doc["mode"] = (modoActual == MODO_RFID ? "RFID" : 
                 modoActual == MODO_MANUAL ? "MANUAL" :
                 modoActual == MODO_NUMEROS ? "NUMEROS" : "COLORES");
  doc["dfplayer_ready"] = dfplayerReady;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["wifi_status"] = (int)WiFi.status();
  doc["ip_address"] = WiFi.localIP().toString();
  doc["ssid"] = WiFi.SSID();
  doc["signal_strength"] = WiFi.RSSI();
  
  String response;
  serializeJson(doc, response);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.send(200, "application/json", response);
}
// ==================== HANDLER WIFI CONFIG ====================
void handleWiFiConfig() {
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", 
      "{\"success\":false,\"error\":\"ssid y password requeridos\"}");
    return;
  }

  String newSsid = server.arg("ssid");
  String newPassword = server.arg("password");

  Serial.println("\n📡 ===== NUEVA CONFIGURACIÓN WIFI =====");
  Serial.println("SSID: " + newSsid);
  Serial.println("Password: " + String(newPassword.length()) + " caracteres");
  Serial.println("======================================\n");

  
  // Guardar en EEPROM
  guardarWiFiEnEEPROM(newSsid, newPassword);

  JsonDocument doc;
  doc["success"] = true;
  doc["message"] = "WiFi configurado. Reiniciando en 3 segundos...";
  doc["ssid"] = newSsid;
  doc["restarting"] = true;
  
  String response;
  serializeJson(doc, response);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);

  // SALIR DEL MODO AP Y REINICIAR
  Serial.println("Saliendo de modo AP...");
  delay(2000);
  
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  
  Serial.println("Reiniciando ESP32...");
  ESP.restart();
}

void handleGetIP() {
  JsonDocument doc;
  
  if (modoAP) {
    doc["mode"] = "AP";
    doc["ip"] = WiFi.softAPIP().toString();
    doc["ssid"] = AP_SSID;
  } else if (WiFi.status() == WL_CONNECTED) {
    doc["mode"] = "Client";
    doc["ip"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
  } else {
    doc["mode"] = "Disconnected";
    doc["ip"] = "0.0.0.0";
    doc["ssid"] = "";
  }
  
  String response;
  serializeJson(doc, response);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}


// ==================== HANDLER NETWORK INFO ====================
void handleNetworkInfo() {
  JsonDocument doc;
  
  doc["ap_mode"] = modoAP;
  
  if (modoAP) {
    doc["connected"] = false;
    doc["ip_address"] = WiFi.softAPIP().toString();
    doc["ssid"] = AP_SSID;
    doc["mode"] = "Access Point (Configuración)";
    doc["clients_connected"] = WiFi.softAPgetStationNum();
  } else {
    doc["connected"] = (WiFi.status() == WL_CONNECTED);
    doc["ip_address"] = WiFi.localIP().toString();
    doc["ssid"] = WiFi.SSID();
    doc["signal_strength"] = WiFi.RSSI();
    doc["mode"] = "Cliente";
  }
  
  doc["mac_address"] = WiFi.macAddress();
  
  String response;
  serializeJson(doc, response);
  
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}
void mostrarModoConfiguracion() {
  detenerScroll();
  
  Serial.println("📺 Modo Configuración");
  
  // Color naranja parpadeante para indicar modo config
  uint32_t colorConfig = matrix.Color(255, 100, 0);
  iniciarScrollTexto("CONECTA A STORYCUBE-SETUP", colorConfig);
  scrollActivo = true;
}
void handleSetVolume() {
  if (server.hasArg("value")) {
    int newVolume = server.arg("value").toInt();
    
    if (newVolume >= VOL_MIN && newVolume <= VOL_MAX) {
      volumen_actual = newVolume;
      if (dfplayerReady) {
        myDFPlayer.volume(volumen_actual);
      }
      
      JsonDocument doc;  
      doc["success"] = true;
      doc["volume"] = volumen_actual;
      
      String response;
      serializeJson(doc, response);
      
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "application/json", response);
    } else {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Volumen fuera de rango (0-50)\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Parámetro value requerido\"}");
  }
}

// ==================== HANDLERS WEB ====================
void handleSetBrightness() {
  if (!server.hasArg("value")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Parámetro value requerido\"}");
    return;
  }

  int newBrightness = server.arg("value").toInt();

  if (newBrightness < BRILLO_MIN || newBrightness > BRILLO_MAX) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Brillo fuera de rango (5-255)\"}");
    return;
  }

  // GUARDAR posición actual del pot ANTES de cambiar el brillo
  int valorPotActual = analogRead(POT_BRILLO);
  int valorPotMapeado = map(valorPotActual, 0, 4095, BRILLO_MIN, BRILLO_MAX);
  
  // ACTUALIZA AMBAS REFERENCIAS
  ultimoValorPotenciometroLeido = valorPotMapeado;
  ultimoValorPotenciometroAntesDeCambioApp = valorPotMapeado;

  // APLICAR BRILLO DESDE APP
  brillo_actual = newBrightness;
  matrix.setBrightness(brillo_actual);
  
  // EL POT SE BLOQUEA HASTA QUE SE MUEVA
  potTieneControl = false;

  Serial.println("\n ===== CONTROL DESDE APP =====");
  Serial.print("   Brillo desde app: ");
  Serial.println(brillo_actual);
  Serial.print("   Pot está en: ");
  Serial.println(ultimoValorPotenciometroAntesDeCambioApp);
  Serial.print("   Se desbloqueará con movimiento >= ");
  Serial.println(DIFERENCIA_MINIMA);
  Serial.println("=================================\n");

  JsonDocument doc;
  doc["success"] = true;
  doc["brightness"] = brillo_actual;
  doc["control"] = "APP";
  doc["pot_position"] = ultimoValorPotenciometroAntesDeCambioApp;

  String response;
  serializeJson(doc, response);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

void handlePlay() {
  if (server.hasArg("track")) {
    int track = server.arg("track").toInt();
    
    if (track >= 1 && track <= NUM_CUENTOS) {
      pista_actual = track;
      reproducirCuento(track, "App");
      
      JsonDocument doc;  
      doc["success"] = true;
      doc["track"] = pista_actual;
      
      String response;
      serializeJson(doc, response);
      
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(200, "application/json", response);
    } else {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Pista inválida (1-6)\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Parámetro track requerido\"}");
  }
}

void handlePause() {
    if (!modoPermiteControlesReproduccion()) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, "application/json", 
      "{\"success\":false,\"error\":\"Pausa no disponible en este modo\"}");
    return;
  }
  pausarReanudar();
  
  JsonDocument doc;
  doc["success"] = true;
  doc["playing"] = reproduciendo;
  doc["paused"] = audioPausado;
  
  String response;
  serializeJson(doc, response);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

void handleNext() {
  botonSiguiente();
  
  JsonDocument doc;  
  doc["success"] = true;
  doc["track"] = pista_actual;
  
  String response;
  serializeJson(doc, response);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

void handlePrevious() {
  botonAnterior();
  
  JsonDocument doc;  
  doc["success"] = true;
  doc["track"] = pista_actual;
  
  String response;
  serializeJson(doc, response);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

void handleSetMode() {
  if (server.hasArg("mode")) {
    String newMode = server.arg("mode");
    
    if (newMode == "RFID") {
      modoActual = MODO_RFID;
    }
    else if (newMode == "Manual") {
      modoActual = MODO_MANUAL;
    }
    else if (newMode == "Números") {
      modoActual = MODO_NUMEROS;
    }
    else if (newMode == "Colores") {
      modoActual = MODO_COLORES;
    }
    else {
      Serial.println("Modo inválido: " + newMode);
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Modo inválido\"}");
      return;
    }
    
    // DETENER REPRODUCCIÓN ANTERIOR
    detenerScroll();
    esperandoFinAudio = false;
    reproduciendo = false;
    if (dfplayerReady) {
      myDFPlayer.stop();
    }
    
    // INICIAR STORYCUBE INMEDIATAMENTE
    mostrarStoryCube();
    
    // REPRODUCIR SONIDO DEL MODO EN PARALELO (sin delay)
    if (dfplayerReady) {
      int archivoModo;
      
      switch(modoActual) {
        case MODO_RFID: 
          Serial.println("✓ Modo: RFID");
          archivoModo = 7;  
          break;
        case MODO_MANUAL: 
          Serial.println("✓ Modo: Manual");
          archivoModo = 26;
          break;
        case MODO_NUMEROS: 
          Serial.println("✓ Modo: Números");
          archivoModo = 8;  
          break;
        case MODO_COLORES: 
          Serial.println("✓ Modo: Colores");
          archivoModo = 9;  
          break;
      }
      
      myDFPlayer.play(archivoModo);
      reproduciendo = true;
      esperandoFinAudio = true;
      // No detener scroll - dejar que continúe
    }
    
    JsonDocument doc;
    doc["success"] = true;
    doc["mode"] = newMode;
    
    String response;
    serializeJson(doc, response);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", response);
    
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Parámetro mode requerido\"}");
  }
}

// ==================== INICIALIZAR COMPONENTES ====================
void inicializarBotones() {
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_VOL_UP, INPUT_PULLUP);
  pinMode(BTN_VOL_DOWN, INPUT_PULLUP);
  pinMode(POT_BRILLO, INPUT);
  pinMode(BTN_PAUSE, INPUT_PULLUP);
}

void inicializarLED() {
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setTextSize(1);
  matrix.setRotation(3);
  matrix.setBrightness(brillo_actual);
  matrix.setTextColor(matrix.Color(255, 255, 255));
  matrix.fillScreen(0);
  matrix.show();

  Serial.println("✓ LED 8x8");
}

void inicializarRFID() {
  SPI.begin(18, 19, 23, SS_PIN);
  rfid.PCD_Init();
  delay(100);

  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  if (version == 0x00 || version == 0xFF) {
    Serial.println("✗ RFID error");
  } else {
    Serial.println("✓ RFID OK");
  }
}

void inicializarDFPlayer() {
  dfSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);
  delay(1000);

  for (int i = 0; i < 3; i++) {
    if (myDFPlayer.begin(dfSerial)) {
      dfplayerReady = true;
      break;
    }
    delay(500);
  }

  if (!dfplayerReady) {
    Serial.println("✗ DFPlayer error");
    return;
  }

  Serial.println("✓ DFPlayer OK");
  delay(200);
  myDFPlayer.volume(volumen_actual);
  delay(100);
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
}

// ==================== LEER POTENCIÓMETRO BRILLO ====================
void leerPotenciometroBrillo() {
  if (millis() - lastBrilloRead < brilloReadInterval) {
    return;
  }
  lastBrilloRead = millis();

  int valorPotActual = analogRead(POT_BRILLO);
  int nuevoBrillo = map(valorPotActual, 0, 4095, BRILLO_MIN, BRILLO_MAX);

  // PRIMERA LECTURA (inicialización)
  if (ultimoValorPotenciometroLeido == -999) {
    ultimoValorPotenciometroLeido = nuevoBrillo;
    ultimoValorPotenciometroAntesDeCambioApp = nuevoBrillo;
    brillo_actual = nuevoBrillo;
    matrix.setBrightness(brillo_actual);
    potTieneControl = true;
    Serial.print(" Potenciómetro inicializado: ");
    Serial.println(nuevoBrillo);
    return;
  }

  // SI EL POT YA TIENE CONTROL, actúa normalmente
  if (potTieneControl) {
    // Solo actualiza si hay cambio significativo (evita ruido)
    if (abs(nuevoBrillo - ultimoValorPotenciometroLeido) >= 3) {
      ultimoValorPotenciometroLeido = nuevoBrillo;
      ultimoValorPotenciometroAntesDeCambioApp = nuevoBrillo; // ACTUALIZA LA REFERENCIA
      brillo_actual = nuevoBrillo;
      matrix.setBrightness(brillo_actual);
      
      Serial.print("Pot físico: ");
      Serial.println(brillo_actual);
    }
    return;
  }

  // SI EL POT ESTÁ BLOQUEADO, detecta movimiento para desbloquearlo
  int diferenciaDelMovimiento = abs(nuevoBrillo - ultimoValorPotenciometroAntesDeCambioApp);

  // Si el pot se mueve significativamente, RECUPERA EL CONTROL
  if (diferenciaDelMovimiento >= DIFERENCIA_MINIMA) {
    ultimoValorPotenciometroLeido = nuevoBrillo;
    ultimoValorPotenciometroAntesDeCambioApp = nuevoBrillo; //  ACTUALIZA LA REFERENCIA
    brillo_actual = nuevoBrillo;
    matrix.setBrightness(brillo_actual);
    potTieneControl = true; //  DESBLOQUEA EL POTENCIÓMETRO

    Serial.println("\n🎚️ ===== POT FÍSICO =====");
    Serial.print("   Movimiento detectado: ");
    Serial.print(diferenciaDelMovimiento);
    Serial.print(" → Brillo: ");
    Serial.println(brillo_actual);
    Serial.println("    Pot físico ");
    Serial.println("=========================================\n");
  }
  // Si no se movió lo suficiente, no hace nada (mantiene el brillo de la app)
}

 void procesarTecla(char tecla) {
  if (tecla == '*') {
    cambiarModo();
    return;
  }

  if (tecla == '#') {
    if (modoActual == MODO_NUMEROS) {
      cambiarSubModoNumeros();
    }
    return;
  }

  if (tecla >= '0' && tecla <= '9') {
    int numero = tecla - '0';

    if (modoActual == MODO_MANUAL && numero >= 1 && numero <= 6) {
      pista_actual = numero;
      reproducirCuento(numero, "Teclado");
    }
    else if (modoActual == MODO_NUMEROS) {
      reproducirNumero(numero);
      mostrarNumeroLED(numero);
      // EL NÚMERO SE QUEDA EN PANTALLA POR 5 SEGUNDOS, LUEGO DESAPARECE
      estadoPantalla = PANTALLA_NUMERO;
      numeroMostrado = numero;
      tiempoNumeroMostrado = millis();  // GUARDAR EL TIEMPO
    }
    else if (modoActual == MODO_COLORES && numero >= 1 && numero <= 6) {
      mostrarColorCompleto(numero);
      // EL COLOR SE QUEDA EN PANTALLA POR 5 SEGUNDOS, LUEGO DESAPARECE
      estadoPantalla = PANTALLA_COLOR;
      tiempoNumeroMostrado = millis();  // GUARDAR EL TIEMPO
    }
  }
}

void cambiarModo() {
  detenerScroll();
  esperandoFinAudio = false;
  reproduciendo = false;
  myDFPlayer.stop();

  if (modoActual == MODO_RFID) modoActual = MODO_MANUAL;
  else if (modoActual == MODO_MANUAL) modoActual = MODO_NUMEROS;
  else if (modoActual == MODO_NUMEROS) modoActual = MODO_COLORES;
  else modoActual = MODO_RFID;

  Serial.print("Modo: ");
  
  // MOSTRAR STORYCUBE INMEDIATAMENTE
  mostrarStoryCube();
  
  // REPRODUCIR SONIDO EN PARALELO (sin interferer con scroll)
  if (dfplayerReady) {
    int archivoModo;
    
    switch(modoActual) {
      case MODO_RFID: 
        Serial.println("RFID");
        archivoModo = 7;  
        break;
      case MODO_MANUAL: 
        Serial.println("Manual");
        archivoModo = 26;
        break;
      case MODO_NUMEROS: 
        Serial.println("Números");
        archivoModo = 8;  
        break;
      case MODO_COLORES: 
        Serial.println("Colores");
        archivoModo = 9;  
        break;
    }
    
    myDFPlayer.play(archivoModo);
    reproduciendo = true;
    esperandoFinAudio = true;
    
    //IMPORTANTE: No detener scroll aquí, dejar que continúe
    // El scroll se mantendrá mientras suena el audio del modo
  }
}


///*---------------------------
void detenerReproduccion() {
  if (!dfplayerReady) return;
  
  myDFPlayer.stop();
  reproduciendo = false;
  esperandoFinAudio = false;
  
  Serial.println(" Detenido completamente");
  
  detenerScroll();
  mostrarStoryCube();
}

// ==================== MODO RFID ====================
void modoRFID() {
  
  if (digitalRead(BTN_VOL_UP) == LOW && (millis() - lastButtonPress[2] > buttonDelay)) {
    lastButtonPress[2] = millis();
    botonSubirVolumen();
  }
  
  if (digitalRead(BTN_VOL_DOWN) == LOW && (millis() - lastButtonPress[3] > buttonDelay)) {
    lastButtonPress[3] = millis();
    botonBajarVolumen();
  }
  
  // Verificar cooldown antes de leer
  if (millis() - lastReadTime < READ_DELAY) return;
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  // Verificar si es el mismo llavero que antes
  bool esMismoLlavero = false;
  if (ultimoUIDSize == rfid.uid.size) {
    esMismoLlavero = true;
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] != ultimoUIDLeido[i]) {
        esMismoLlavero = false;
        break;
      }
    }
  }

  //  SI ES EL MISMO LLAVERO, IGNORAR COMPLETAMENTE
  if (esMismoLlavero) {
    Serial.println("🔄 Mismo llavero detectado - manteniendo cuento actual");
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    lastReadTime = millis();  // Actualizar tiempo para evitar lecturas repetidas
    return;  // NO HACER NADA
  }

  // ES UN LLAVERO DIFERENTE → PROCESAR
  Serial.println("🆕 Llavero diferente detectado - cambiando cuento");
  
  // Guardar UID actual para futuras comparaciones
  ultimoUIDSize = rfid.uid.size;
  for (byte i = 0; i < rfid.uid.size; i++) {
    ultimoUIDLeido[i] = rfid.uid.uidByte[i];
  }
  ultimaDeteccionRFID = millis();
  lastReadTime = millis();

  // Identificar y reproducir el cuento correspondiente
  int pistaSeleccionada = -1;
  
  if (compararUID(rfid.uid.uidByte, rfid.uid.size, UID1, sizeof(UID1))) {
    pistaSeleccionada = 1;
  }
  else if (compararUID(rfid.uid.uidByte, rfid.uid.size, UID2, sizeof(UID2))) {
    pistaSeleccionada = 2;
  }
  else if (compararUID(rfid.uid.uidByte, rfid.uid.size, UID3, sizeof(UID3))) {
    pistaSeleccionada = 3;
  }
  else if (compararUID(rfid.uid.uidByte, rfid.uid.size, UID4, sizeof(UID4))) {
    pistaSeleccionada = 4;
  }
  else if (compararUID(rfid.uid.uidByte, rfid.uid.size, UID5, sizeof(UID5))) {
    pistaSeleccionada = 5;
  }
  else if (compararUID(rfid.uid.uidByte, rfid.uid.size, UID6, sizeof(UID6))) {
    pistaSeleccionada = 6;
  }

  // SOLO REPRODUCIR SI ES UN LLAVERO VÁLIDO Y DIFERENTE
if (pistaSeleccionada != -1) {
    ultimoPistaRFID = pistaSeleccionada;  // Guardar la pista actual
    
    // Crear el nombre del llavero correctamente
    String nombreLlavero = "Llavero " + String(pistaSeleccionada);
    reproducirCuento(pistaSeleccionada, nombreLlavero.c_str());
  } else {
    Serial.println(" Llavero no reconocido");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
// ==================== MODO MANUAL ====================
void modoManual() {
  if (digitalRead(BTN_NEXT) == LOW && (millis() - lastButtonPress[0] > buttonDelay)) {
    lastButtonPress[0] = millis();
    botonSiguiente();
  }

  if (digitalRead(BTN_PREV) == LOW && (millis() - lastButtonPress[1] > buttonDelay)) {
    lastButtonPress[1] = millis();
    botonAnterior();
  }
  
  if (digitalRead(BTN_VOL_UP) == LOW && (millis() - lastButtonPress[2] > buttonDelay)) {
    lastButtonPress[2] = millis();
    botonSubirVolumen();
  }
  
  if (digitalRead(BTN_VOL_DOWN) == LOW && (millis() - lastButtonPress[3] > buttonDelay)) {
    lastButtonPress[3] = millis();
    botonBajarVolumen();
  }
}

void botonSiguiente() {
  if (!dfplayerReady) return;
  pista_actual++;
  if (pista_actual > NUM_CUENTOS) pista_actual = 1;
  reproducirCuento(pista_actual, "Botón");
}

void botonAnterior() {
  if (!dfplayerReady) return;
  pista_actual--;
  if (pista_actual < 1) pista_actual = NUM_CUENTOS;
  reproducirCuento(pista_actual, "Botón");
}

void botonSubirVolumen() {
  if (!dfplayerReady) return;
  volumen_actual++;
  if (volumen_actual > VOL_MAX) volumen_actual = VOL_MAX;
  myDFPlayer.volume(volumen_actual);
  
  Serial.print("🔊 Vol UP: ");
  Serial.println(volumen_actual);
}

void botonBajarVolumen() {
  if (!dfplayerReady) return;
  volumen_actual--;
  if (volumen_actual < VOL_MIN) volumen_actual = VOL_MIN;
  myDFPlayer.volume(volumen_actual);
  
  Serial.print("🔉 Vol DOWN: ");
  Serial.println(volumen_actual);
}

// ==================== MODO COLORES ====================
void modoColores() {
  if (digitalRead(BTN_VOL_UP) == LOW && (millis() - lastButtonPress[2] > buttonDelay)) {
    lastButtonPress[2] = millis();
    botonSubirVolumen();
  }
  
  if (digitalRead(BTN_VOL_DOWN) == LOW && (millis() - lastButtonPress[3] > buttonDelay)) {
    lastButtonPress[3] = millis();
    botonBajarVolumen();
  }
}

// ==================== UTILIDADES RFID ====================
bool compararUID(byte* uid1, byte size1, byte* uid2, byte size2) {
  if (size1 != size2) return false;
  for (byte i = 0; i < size1; i++) {
    if (uid1[i] != uid2[i]) return false;
  }
  return true;
}

// ==================== REPRODUCCIÓN ====================
void reproducirCuento(int numero, const char* nombre) {
  if (!dfplayerReady) return;

pista_actual = numero;

  Serial.print("▶ ");
  Serial.println(nombre);

  String textos[6] = {
    "PINOCHO",
    "PETER PAN",
    "RISITOS DE ORO",
    "LOS TRES CERDITOS",
    "LA TORTUGA Y LA LIEBRE",
    "CAPERUCITA ROJA"
  };

  uint32_t colores[6] = {
    matrix.Color(0, 0, 235),
    matrix.Color(64, 224, 208),
    matrix.Color(255, 215, 0),
    matrix.Color(255, 170, 180),
    matrix.Color(0, 201, 87),
    matrix.Color(255, 0, 0)
  };

  //  MOSTRAR TEXTO INMEDIATAMENTE (concurrencia)
  if (numero >= 1 && numero <= 6) {
    iniciarScrollTexto(textos[numero - 1], colores[numero - 1]);
    scrollActivo = true;
  }

  //  REPRODUCIR AUDIO EN PARALELO (sin delay)
  myDFPlayer.play(numero);
  reproduciendo = true;
  esperandoFinAudio = true;
}

// ==================== pausarReanudar() ====================
void pausarReanudar() {
  if (!dfplayerReady) {
    Serial.println(" DFPlayer no disponible");
    return;
  }
  
  if (reproduciendo && !audioPausado) {
    // PAUSAR
    myDFPlayer.pause();
    audioPausado = true;
    reproduciendo = false;
    esperandoFinAudio = false;
    
    Serial.println("Pausado");
    
    detenerScroll();
    matrix.fillScreen(0);
    
    uint32_t colorPausa = matrix.Color(255, 150, 0);
    for (int y = 1; y < 7; y++) {
      matrix.drawPixel(2, y, colorPausa);
      matrix.drawPixel(3, y, colorPausa);
      matrix.drawPixel(5, y, colorPausa);
      matrix.drawPixel(6, y, colorPausa);
    }
    matrix.show();
    
  } else if (audioPausado) {
    // REANUDAR (NO REINICIAR)
    myDFPlayer.start();  // Este comando reanuda, no reinicia
    audioPausado = false;
    reproduciendo = true;
    esperandoFinAudio = true;
    
    Serial.println(" Reanudando");
    
    // Restaurar texto del cuento actual
    if (pista_actual >= 1 && pista_actual <= 6) {
      String textos[6] = {
        "PINOCHO", "PETER PAN", "RISITOS DE ORO",
        "LOS TRES CERDITOS", "LA TORTUGA Y LA LIEBRE", "CAPERUCITA ROJA"
      };
      uint32_t colores[6] = {
        matrix.Color(0, 0, 235), matrix.Color(64, 224, 208),
        matrix.Color(255, 215, 0), matrix.Color(255, 170, 180),
        matrix.Color(0, 201, 87), matrix.Color(255, 0, 0)
      };
      iniciarScrollTexto(textos[pista_actual - 1], colores[pista_actual - 1]);
      scrollActivo = true;
    }
  }
}


void reproducirNumero(int numero) {
  if (!dfplayerReady || numero < 0 || numero > 9) return;
  
  int archivoNumero;
  
  if (subModoNumeros == NUMEROS_ESPANOL) {
    archivoNumero = 10 + numero;
  } else {
    archivoNumero = 29 + numero;
  }
  
  myDFPlayer.stop();
  delay(100);
  myDFPlayer.play(archivoNumero);

  reproduciendo = true;
  esperandoFinAudio = true;      
}

void cambiarSubModoNumeros() {
  if (subModoNumeros == NUMEROS_ESPANOL) {
    subModoNumeros = NUMEROS_INGLES;
    Serial.println("Sub-modo: Inglés");
    
    if (dfplayerReady) {
      myDFPlayer.play(28);
      reproduciendo = true;
      esperandoFinAudio = true;
    }
  } else {
    subModoNumeros = NUMEROS_ESPANOL;
    Serial.println("Sub-modo: Español");
  
    if (dfplayerReady) {
      myDFPlayer.play(27);
      reproduciendo = true;
      esperandoFinAudio = true;
    }
  }
}
// ==================== FUNCIONES LED ====================
void actualizarLED() {
  if (scrollActivo) {
    actualizarScroll();
  }
}

void iniciarScrollTexto(String texto, uint32_t color) {
  String textoLimpio = "";
  for (unsigned int i = 0; i < texto.length(); i++) {
    char c = texto[i];
    if (c >= 32 && c <= 126) {
      textoLimpio += c;
    }
  }

  textoActual = textoLimpio;
  colorActual = color;
  scrollActivo = true;
  scrollX = 8;

  int16_t x1, y1;
  uint16_t w, h;
  matrix.setTextSize(1);
  matrix.getTextBounds(textoLimpio, 0, 0, &x1, &y1, &w, &h);
  textoAncho = w;
}

void detenerScroll() {
  scrollActivo = false;
  scrollX = 8;
  textoActual = "";
  matrix.fillScreen(0);
  matrix.clear();
  matrix.show();
}

void actualizarScroll() {
  if (!scrollActivo) return;

  if (millis() - lastScrollUpdate < scrollInterval) return;

  lastScrollUpdate = millis();

  matrix.fillScreen(0);
  matrix.clear();

  if (scrollX > -textoAncho && scrollX < 8) {
    matrix.setCursor(scrollX, 0);
    matrix.setTextColor(colorActual);

    for (unsigned int i = 0; i < textoActual.length(); i++) {
      char c = textoActual[i];
      if (c >= 32 && c <= 126) {
        matrix.print(c);
      }
    }
  }

  matrix.show();
  scrollX--;

  if (scrollX <= -textoAncho - 8) {
    scrollX = 8;
  }
}
void mostrarEstadoWiFi() {
  int status = WiFi.status();
  
  switch(status) {
    case WL_IDLE_STATUS:
      Serial.println("WiFi: Idle (esperando)");
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("WiFi: Red no disponible");
      break;
    case WL_SCAN_COMPLETED:
      Serial.println("WiFi: Escaneo completado");
      break;
    case WL_CONNECTED:
      Serial.print("WiFi: ✓ Conectado a ");
      Serial.println(WiFi.SSID());
      break;
    case WL_CONNECT_FAILED:
      Serial.println("WiFi: Conexión fallida");
      break;
    case WL_CONNECTION_LOST:
      Serial.println("WiFi: Conexión perdida");
      break;
    case WL_DISCONNECTED:
      Serial.println("WiFi: Desconectado");
      break;
    default:
      Serial.print("WiFi: Estado desconocido (");
      Serial.print(status);
      Serial.println(")");
  }
}
void mostrarNumeroLED(int num) {
  if (num < 0 || num > 9) return;

  detenerScroll();
  matrix.fillScreen(0);

  uint32_t color = matrix.Color(255, 150, 0);

  for (int y = 0; y < 8; y++) {
    uint8_t fila = pgm_read_byte(&numeros[num][y]);
    for (int x = 0; x < 8; x++) {
      if (fila & (1 << (7 - x))) {
        matrix.drawPixel(x, y, color);
      }
    }
  }
  matrix.show();
}

void mostrarColorCompleto(int numeroColor) {
  detenerScroll();
  
  uint32_t colores[7] = {
    matrix.Color(255, 0, 0),
    matrix.Color(0, 255, 0),
    matrix.Color(0, 0, 255),
    matrix.Color(246, 255, 0),
    matrix.Color(255, 0, 255),
    matrix.Color(0, 255, 255),
    matrix.Color(0, 125, 125)
  };
  
  String nombresColores[6] = {
    "ROJO", "VERDE", "AZUL", "AMARILLO", "MAGENTA", "CYAN"
  };
  
  if (numeroColor >= 1 && numeroColor <= 6) {
    uint32_t color = colores[numeroColor - 1];
    
    Serial.print("Color: ");
    Serial.println(nombresColores[numeroColor - 1]);
    
    matrix.fillScreen(color);
    matrix.show();
    
    if (dfplayerReady) {
      int archivoColor = 19 + numeroColor;  
      myDFPlayer.play(archivoColor);
      reproduciendo = true;
      esperandoFinAudio = true;
    }
  }
}

void mostrarStoryCube() {
  detenerScroll();
  
  Serial.println("📺 StoryCube iniciado");
  
  uint32_t colorStoryCube = matrix.Color(0, 200, 255);
  iniciarScrollTexto("STORYCUBE", colorStoryCube);
  scrollActivo = true;
}
void verificarFinAudio() {
  if (millis() - lastAudioCheck < audioCheckInterval) return;
  
  lastAudioCheck = millis();
  
  // SI ESTÁ MOSTRANDO IP Y NO HAY CLIENTE CONECTADO
  if (estadoPantalla == PANTALLA_IP && 
      !clienteConectado &&
      (millis() - tiempoIPMostrada > TIEMPO_IP)) {
    estadoPantalla = PANTALLA_STORYCUBE;
    mostrarStoryCube();
    scrollActivo = true;
    return;
  }
  
  // VERIFICAR SI EL NÚMERO/COLOR HA ESTADO VISIBLE MÁS DE 5 SEGUNDOS
  if ((estadoPantalla == PANTALLA_NUMERO || estadoPantalla == PANTALLA_COLOR) &&
      (millis() - tiempoNumeroMostrado > TIEMPO_NUMERO)) {
    estadoPantalla = PANTALLA_STORYCUBE;
    numeroMostrado = -1;
    mostrarStoryCube();
    scrollActivo = true;
  }
  
  if (myDFPlayer.available()) {
    uint8_t tipo = myDFPlayer.readType();
    
    if (tipo == DFPlayerPlayFinished) {
      Serial.println("✓ Reproducción completada");
      
      reproduciendo = false;
      esperandoFinAudio = false;
      
      // IMPORTANTE: Si el scroll está activo, NO interrumpir
      // El scroll de STORYCUBE continuará sin reiniciarse
      if (estadoPantalla == PANTALLA_STORYCUBE && scrollActivo) {
        // Simplemente dejar que el scroll continúe
        return;
      }
      
      // Solo reiniciar si no está en STORYCUBE
      if (estadoPantalla != PANTALLA_NUMERO && 
          estadoPantalla != PANTALLA_COLOR &&
          estadoPantalla != PANTALLA_IP) {
        delay(500);  // Pequeña pausa antes de resetear
        mostrarStoryCube();
      }
    }
  }

}

#include "config.h"
#include "network.h"
#include <WiFi.h>
#include <WebServer.h>

// En ausencia de wifi_config.h, el rostro sigue funcionando sin red.
#if __has_include("wifi_config.h")
#include "wifi_config.h"
#define CSR_HAS_WIFI_CONFIG 1
#else
#define CSR_HAS_WIFI_CONFIG 0
#endif

namespace {
static const uint8_t MAX_BUBBLE_PAGES = 24;
static const uint8_t LINE_CAPACITY = 28; // 27 caracteres + terminador
static const uint8_t LINE_LIMIT = 27;
static const uint16_t MAX_MESSAGE_LENGTH = 1200;
static const unsigned long PAGE_INTERVAL_MS = 4000UL;

// Desaparición automática del diálogo
static const unsigned long BUBBLE_IDLE_TIMEOUT_MS = 5000UL;

static unsigned long bubbleHideAt = 0;
static bool bubbleHidePending = false;
static bool bubbleFinalPageReached = true;

static char bubblePages[MAX_BUBBLE_PAGES][2][LINE_CAPACITY];
static int bubblePageCount = 0;
static int bubblePageIndex = 0;
static unsigned long nextBubblePageAt = 0;
static bool bubbleVisible = true;
static bool bubbleHasMessage = false;
static bool autoStopSpeechAfterPages = false;

static void nextBubbleLine(int& page, int& line, int& column) {
  if (line == 0) {
    line = 1;
  } else {
    line = 0;
    if (page < MAX_BUBBLE_PAGES - 1) {
      ++page;
    }
  }
  column = 0;
}

static void appendBubbleWord(const char* word, int& page, int& line, int& column) {
  const char* cursor = word;

  while (*cursor && page < MAX_BUBBLE_PAGES) {
    size_t remaining = strlen(cursor);

    if (remaining > LINE_LIMIT) {
      if (column > 0) nextBubbleLine(page, line, column);
      size_t take = LINE_LIMIT;
      memcpy(bubblePages[page][line], cursor, take);
      bubblePages[page][line][take] = '\0';
      cursor += take;
      column = (int)take;
      if (*cursor) nextBubbleLine(page, line, column);
      continue;
    }

    int needed = (int)remaining + (column > 0 ? 1 : 0);
    if (column > 0 && column + needed > LINE_LIMIT) {
      nextBubbleLine(page, line, column);
      if (page >= MAX_BUBBLE_PAGES) break;
    }

    if (column > 0 && column < LINE_LIMIT) {
      bubblePages[page][line][column++] = ' ';
    }

    if (column + (int)remaining > LINE_LIMIT) {
      nextBubbleLine(page, line, column);
      if (page >= MAX_BUBBLE_PAGES) break;
    }

    remaining = strlen(cursor);
    if (remaining > LINE_LIMIT) remaining = LINE_LIMIT;
    memcpy(bubblePages[page][line] + column, cursor, remaining);
    column += (int)remaining;
    bubblePages[page][line][column] = '\0';
    cursor += remaining;

    if (*cursor) nextBubbleLine(page, line, column);
  }
}

static void buildBubblePages(const String& message) {
  memset(bubblePages, 0, sizeof(bubblePages));
  bubblePageCount = 0;
  bubblePageIndex = 0;
  bubbleHasMessage = message.length() > 0;

  // Reiniciar temporizador al recibir un mensaje nuevo
  bubbleHideAt = 0;
  bubbleHidePending = false;
  bubbleFinalPageReached = true;

  if (!bubbleHasMessage) {
    nextBubblePageAt = 0;
    return;
  }

  char input[MAX_MESSAGE_LENGTH + 1];
  size_t length = message.length();
  if (length > MAX_MESSAGE_LENGTH) length = MAX_MESSAGE_LENGTH;
  memcpy(input, message.c_str(), length);
  input[length] = '\0';

  int page = 0;
  int line = 0;
  int column = 0;
  char* token = strtok(input, " \t\r\n");
  while (token != nullptr && page < MAX_BUBBLE_PAGES) {
    appendBubbleWord(token, page, line, column);
    token = strtok(nullptr, " \t\r\n");
  }

  bubblePageCount = page + 1;
if (bubblePageCount < 1) bubblePageCount = 1;
if (bubblePageCount > MAX_BUBBLE_PAGES) bubblePageCount = MAX_BUBBLE_PAGES;

nextBubblePageAt = millis() + PAGE_INTERVAL_MS;

// Comprobar si el mensaje tiene una sola página
bubbleFinalPageReached = (bubblePageCount <= 1);
}

// Programa la desaparición automática del diálogo.
static void scheduleBubbleHide() {

  if (!bubbleVisible || !bubbleFinalPageReached) return;

  // Si está hablando, esperamos a que termine.
  if (speaking) {
    bubbleHideAt = 0;
    bubbleHidePending = true;
  } else {
    bubbleHideAt = millis() + BUBBLE_IDLE_TIMEOUT_MS;
    bubbleHidePending = false;
  }
}

} // namespace

const char* getBubbleLine1() {
  if (!bubbleHasMessage || bubblePageCount <= 0) return "";
  return bubblePages[bubblePageIndex][0];
}

const char* getBubbleLine2() {
  if (!bubbleHasMessage || bubblePageCount <= 0) return "";
  return bubblePages[bubblePageIndex][1];
}

int getBubblePageIndex() { return bubblePageIndex; }
int getBubblePageCount() { return bubblePageCount; }
bool isBubbleVisible() { return bubbleVisible; }

#if CSR_HAS_WIFI_CONFIG
static WebServer server(80);
static bool routesRegistered = false;
static bool serverStarted = false;
static unsigned long lastReconnect = 0;

static bool authorized() {
  if (server.header("X-CSR-Token") != String(CSR_DEVICE_TOKEN)) {
    server.send(403, "text/plain", "Forbidden");
    return false;
  }
  return true;
}

static void handleHealth() {
  if (!authorized()) return;
  String status = "{\"online\":true,\"state\":\"";
  status += STATE_NAMES[currentState];
  status += "\",\"sprite\":";
  status += frameReady ? "true" : "false";
  status += ",\"speaking\":";
  status += speaking ? "true" : "false";
  status += ",\"dialogue_page\":";
  status += String(bubblePageIndex + 1);
  status += ",\"dialogue_pages\":";
  status += String(bubblePageCount);
  status += ",\"heap\":";
  status += String(ESP.getFreeHeap());
  status += "}";
  server.send(200, "application/json", status);
}

static void handleCommand() {
  if (!authorized()) return;

  String state = server.arg("state");
  state.trim();
  state.toUpperCase();
  if (state.length() == 0 || state.length() > 30) {
    server.send(400, "text/plain", "Invalid state");
    return;
  }

  int index = findState(state.c_str());
  if (index < 0) {
    server.send(422, "text/plain", "Unknown state");
    return;
  }

  String message = server.arg("text");
  if (message.length() > MAX_MESSAGE_LENGTH) {
    server.send(413, "text/plain", "Text too long (max 1200)");
    return;
  }

  String showTextArg = server.arg("show_text");
  bubbleVisible = (showTextArg != "0");
  buildBubblePages(message);

  String speakingArg = server.arg("speaking");
speaking = (speakingArg == "1");

String autoStopArg = server.arg("auto_stop");
autoStopSpeechAfterPages = (autoStopArg == "1");

// Reiniciar el temporizador con cada mensaje nuevo
if (bubbleVisible) {
  scheduleBubbleHide();
} else {
  bubbleHideAt = 0;
  bubbleHidePending = false;
}

setState(index);
  redrawRequested = true;
  server.send(200, "application/json", "{\"ok\":true}");
}

static void handleSpeech() {
  if (!authorized()) return;
  String value = server.arg("speaking");
  speaking = (value == "1");
  if (!speaking) autoStopSpeechAfterPages = false;
  redrawRequested = true;
  server.send(200, "application/json", "{\"ok\":true}");
}

static void registerRoutes() {
  if (routesRegistered) return;
  const char* headers[] = {"X-CSR-Token"};
  server.collectHeaders(headers, 1);
  server.on("/api/health", HTTP_GET, handleHealth);
  server.on("/api/command", HTTP_POST, handleCommand);
  server.on("/api/speech", HTTP_POST, handleSpeech);
  server.onNotFound([](){ server.send(404, "text/plain", "Not found"); });
  routesRegistered = true;
}
#endif

void initializeNetwork() {
#if CSR_HAS_WIFI_CONFIG
  if (String(CSR_WIFI_SSID) == "NOMBRE_DE_TU_WIFI" ||
      String(CSR_WIFI_SSID).length() == 0 ||
      String(CSR_DEVICE_TOKEN).startsWith("CAMBIA_ESTE_TOKEN")) {
    Serial.println("WiFi sin configurar: edita wifi_config.h y define un token propio.");
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(CSR_WIFI_SSID, CSR_WIFI_PASSWORD);
  registerRoutes();
  Serial.print("Conectando al Wi-Fi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 9000UL) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    server.begin();
    serverStarted = true;
    Serial.print("CSR-BOT IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("Servidor HTTP listo: /api/health, /api/command, /api/speech");
  } else {
    Serial.println("Wi-Fi no conectado. El rostro sigue funcionando.");
  }
#else
  Serial.println("WiFi desactivado. Copia wifi_config.example.h a wifi_config.h.");
#endif
}

void updateNetwork() {
#if CSR_HAS_WIFI_CONFIG
  if (routesRegistered) {
    if (WiFi.status() == WL_CONNECTED) {
      if (!serverStarted) {
        server.begin();
        serverStarted = true;
        Serial.print("CSR-BOT IP: ");
        Serial.println(WiFi.localIP());
      }
      server.handleClient();
    } else if (millis() - lastReconnect >= 10000UL) {
      lastReconnect = millis();
      serverStarted = false;
      WiFi.reconnect();
    }
  }
#endif

  const unsigned long now = millis();

  // Avanzar páginas solo si hay un diálogo visible con contenido.
  if (bubbleVisible &&
      bubbleHasMessage &&
      bubblePageCount > 0 &&
      bubblePageIndex < bubblePageCount - 1 &&
      nextBubblePageAt != 0 &&
      (long)(now - nextBubblePageAt) >= 0) {

    ++bubblePageIndex;
    nextBubblePageAt = now + PAGE_INTERVAL_MS;
    bubbleFinalPageReached = (bubblePageIndex >= bubblePageCount - 1);
    redrawRequested = true;

    // Al entrar en la última página, preparar el cierre.
    if (bubbleFinalPageReached) {
      scheduleBubbleHide();
    }
  }

  // En modo solo pantalla, la boca acompaña la lectura y se detiene
  // cuando la última página ya permaneció visible durante un intervalo.
  if (bubbleVisible &&
      bubbleHasMessage &&
      bubbleFinalPageReached &&
      autoStopSpeechAfterPages &&
      speaking &&
      nextBubblePageAt != 0 &&
      (long)(now - nextBubblePageAt) >= 0) {

    speaking = false;
    autoStopSpeechAfterPages = false;
    nextBubblePageAt = 0;
    redrawRequested = true;
    scheduleBubbleHide();
  }

  // En modo voz/mixto, Python informa cuando el sintetizador termina.
  // Si la última página ya se mostró, iniciar aquí los cinco segundos.
  if (bubbleHidePending &&
      !speaking &&
      bubbleFinalPageReached &&
      bubbleVisible) {

    bubbleHidePending = false;
    bubbleHideAt = now + BUBBLE_IDLE_TIMEOUT_MS;
  }

  // Ocultar únicamente el globo; el avatar continúa dibujándose.
  if (bubbleHideAt != 0 &&
      (long)(now - bubbleHideAt) >= 0) {

    bubbleHideAt = 0;

    if (!speaking && bubbleFinalPageReached) {
      bubbleVisible = false;
      bubbleHasMessage = false;
      bubblePageCount = 0;
      bubblePageIndex = 0;
      nextBubblePageAt = 0;
      autoStopSpeechAfterPages = false;
      bubbleHidePending = false;
      bubbleFinalPageReached = true;
      redrawRequested = true;
    } else {
      // Si todavía está hablando o no terminó la paginación,
      // esperar sin perder el temporizador.
      bubbleHidePending = true;
    }
  }
}

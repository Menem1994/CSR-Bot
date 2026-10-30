/*
  CSR-BOT FACE ENGINE v1.3 SAFE SPRITE + TFT FALLBACK
  Port inicial del avatar CSR-BOT Python/Tkinter v4 a:
  ESP32-WROOM-32 + TFT 3.5" paralela 8-bit + TFT_eSPI

  Requisitos:
  - Biblioteca TFT_eSPI instalada.
  - User_Setup.h ya configurado para ILI9486 y bus paralelo 8-bit.
  - Mantener el cableado que ya funcionó en la prueba de colores.

  Monitor Serie: 115200 baudios, final de línea "Nueva línea".
  N = siguiente estado | P = estado anterior
  B = parpadear        | S = activar/desactivar habla
  1 = mañana (ANGRY)   | 2 = tarde (PLAYFUL)
  3 = noche (SARCASTIC)| 4 = madrugada (MYSTERIOUS)
*/

#include <TFT_eSPI.h>
#include <math.h>
#include <string.h>
#include "CSR_BOT_FaceTypes.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frame = TFT_eSprite(&tft);
// La cara ocupa el area central. Un sprite de 300x320 reduce el consumo
// a 96 KB en 8 bpp, frente a 153.6 KB para el framebuffer completo.
static const int SPRITE_X = 90;
static const int SPRITE_W = 300;
bool frameReady = false;
TFT_eSPI* gfx = &tft; // Si no hay RAM para el sprite, dibuja directamente en TFT.

// -------------------------
// Pantalla y composición
// -------------------------
static const int SCREEN_W = 480;
static const int SCREEN_H = 320;
static const float SCALE = 0.58f;
static const int CENTER_X = 240;
static const int CENTER_Y = 164;

int motionOffsetX = 0;
int motionOffsetY = 0;
int X(float x) { return CENTER_X - (frameReady ? SPRITE_X : 0) + motionOffsetX + (int)lroundf(x * SCALE); }
int Y(float y) { return CENTER_Y + motionOffsetY + (int)lroundf(y * SCALE); }
int R(float r) { return (int)lroundf(r * SCALE); }

// -------------------------
// Paleta original Python v4
// Convertida a RGB565
// -------------------------
const uint16_t BG       = 0x1083; // #10131f
const uint16_t PANEL    = 0x10E5; // #171c2b
const uint16_t TEXT     = 0xEF5E; // #e9e9f5
const uint16_t MUTED    = 0x9D38; // #9da7c4
const uint16_t CYAN     = 0x8F5E; // #8be9f0
const uint16_t PINK     = 0xF55B; // #f4a9d8
const uint16_t LILAC    = 0xC51E; // #c6a0f6
const uint16_t PEACH    = 0xFDF5; // #ffbea8
const uint16_t CREAM    = 0xFF17; // #ffe2b8
const uint16_t RED      = 0xFC75; // #ff8eaa
const uint16_t GREEN    = 0xAF9A; // #a8f0d1
const uint16_t SKIN     = 0xD552; // #d7a995
const uint16_t SKIN_LIT = 0xF637; // #f4c6bd
const uint16_t HAIR     = 0x28E5; // #2b1d2a
const uint16_t HAIR_2   = 0x4968; // #4b2d42
const uint16_t HAIR_3   = 0x3906; // #382334
const uint16_t BROW     = 0x49A9; // #49354f
const uint16_t EYE_DARK = 0x2189; // #25304b
const uint16_t MOUTH_DK = 0x2907; // #282238
const uint16_t WHITE    = 0xFFFF;

// -------------------------
// Los 100 estados originales
// Mismo orden que STATES en Python
// -------------------------
const char* STATE_NAMES[100] = {
  "HAPPY","EXCITED","JOYFUL","AMUSED","PROUD","PLAYFUL","LOVING","GRATEFUL","RELIEVED","CONTENT",
  "SAD","DISAPPOINTED","FRUSTRATED","ANGRY","ANNOYED","WORRIED","AFRAID","LONELY","EMBARRASSED","GUILTY",
  "SURPRISED","SHOCKED","CURIOUS","INTRIGUED","AMAZED","CONFUSED","PUZZLED","SKEPTICAL","SUSPICIOUS","AWESTRUCK",
  "THINKING","DEEP_THINKING","ANALYZING","CALCULATING","LEARNING","REMEMBERING","RECALLING","DECIDING","SOLVING","CREATIVE",
  "LISTENING","SPEAKING","GREETING","FAREWELL","ATTENTIVE","EMPATHETIC","ENCOURAGING","CELEBRATING","TEASING","WAITING_FOR_REPLY",
  "SCANNING","DETECTING","TRACKING","LOOKING_LEFT","LOOKING_RIGHT","LOOKING_UP","LOOKING_DOWN","FOCUSED","ALERT","STARTLED",
  "IDLE","SLEEPY","SLEEPING","WAKING_UP","ENERGIZED","RELAXED","BORED","RESTLESS","DROWSY","RECHARGING",
  "BOOTING","ONLINE","OFFLINE","CONNECTING","PROCESSING","LOADING","ERROR","RECOVERING","UPDATING","LOW_BATTERY",
  "MISCHIEVOUS","SARCASTIC","DRAMATIC","SHY","CONFIDENT","MYSTERIOUS","GOOFY","CURIOUS_PET","GLITCHY","SHOWTIME",
  "LOVE_STRUCK","MIND_BLOWN","EUREKA","MEDITATING","DREAMING","NOSTALGIC","DETERMINED","OVERWHELMED","MIRRORING","SIGNATURE"
};

// Frases originales de STATE_BADGES (sin emoji: la fuente TFT estandar es ASCII).
const char* STATE_PHRASES[100] = {
  "Que buen dia!", "No puedo esperar!", "Esto me encanta", "Ja. Ja. Que bueno.", "Mira lo que logre",
  "Jugamos un poco?", "Me caes muy bien", "Gracias por estar", "Uf... solucionado", "Todo esta en orden",
  "Hoy estoy bajoneado", "Esperaba otra cosa", "Esto no sale", "Necesito un momento", "En serio?",
  "Algo me preocupa", "Que fue ese ruido?", "Te quedas un rato?", "Que verguenza", "Creo que meti la pata",
  "Que acaba de pasar?!", "No me lo esperaba", "Quiero saber mas", "Aqui hay algo...", "Es increible!",
  "No termino de entender", "Hay una pieza que falta", "Mmm... no se", "Te estoy observando", "Que fascinante",
  "Dame un segundo", "Procesando a fondo", "Buscando patrones", "Haciendo cuentas", "Nueva informacion",
  "Buscando en mi memoria", "Creo que lo recuerdo", "Evaluando opciones", "Uniendo las piezas", "Probemos algo distinto",
  "Te escucho", "Estoy hablando...", "Hola! Como va?", "Nos vemos pronto!", "Tenes mi atencion",
  "Estoy aca con vos", "Vos podes!", "Lo conseguimos!", "Tengo una idea...", "Te toca a vos",
  "Explorando el entorno", "Detecte algo!", "Siguiendo el objetivo", "Mirando a la izquierda", "Mirando a la derecha",
  "Que hay arriba?", "Revisando abajo", "Enfoque maximo", "Atencion, por favor", "Uy!",
  "En reposo, atento", "Me agarra sueno", "Zzz...", "Cinco minutos mas...", "A toda maquina!",
  "Todo tranquilo", "Hay algo para hacer?", "No puedo quedarme quieto", "Se me cierran los ojos", "Recargando energia",
  "Iniciando sistemas", "Conectado y listo", "Estoy sin conexion", "Buscando conexion", "Procesando solicitud",
  "Un momento...", "Algo salio mal", "Intentando recuperarme", "Mejorando mis sistemas", "Necesito energia",
  "Se me ocurrio una travesura", "Ah, claro. Brillante.", "Esto es una tragedia!", "No me mires tanto", "Lo tengo bajo control",
  "Hay cosas que no cuento", "Modo payaso activado", "Que es eso?", "Bip... bup... glitch", "Que comience el show!",
  "Me dejaste sin palabras", "Mi mente exploto", "Eureka! Lo encontre", "Respirando... calma", "Visitando otros mundos",
  "Eso me trae recuerdos", "No voy a rendirme", "Demasiadas cosas a la vez", "Imitando tu expresion", "Esta es mi firma"
};

FaceProfile profiles[100];
int currentState = 60; // IDLE
bool speaking = false;
int previousStateBeforeSpeaking = 60;
bool blinkNow = false;
unsigned long blinkUntil = 0;
unsigned long nextBlink = 0;
unsigned long lastFrame = 0;
bool redrawRequested = true; // Solo repintar al producirse un cambio visible
unsigned long startedAt = 0;

// -------------------------
// Perfiles: ciclos base y
// ajustes semánticos de Python
// -------------------------
int findState(const char* name) {
  for (int i = 0; i < 100; ++i) {
    if (strcmp(STATE_NAMES[i], name) == 0) return i;
  }
  return -1;
}

void assignGroup(const char* names, EyeStyle eye, BrowStyle brow,
                 MouthStyle mouth, GazeStyle gaze, MotionStyle motion) {
  char buffer[300];
  strncpy(buffer, names, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  char* token = strtok(buffer, " ");
  while (token != nullptr) {
    int index = findState(token);
    if (index >= 0) {
      profiles[index] = {eye, brow, mouth, gaze, motion};
    }
    token = strtok(nullptr, " ");
  }
}

void initializeProfiles() {
  const EyeStyle eyeCycle[] = {E_OVAL,E_WIDE,E_NARROW,E_DIAMOND,E_SLEEPY,E_ROUND,E_SQUINT};
  const BrowStyle browCycle[] = {B_NEUTRAL,B_RAISED,B_ASYMMETRIC,B_WORRIED,B_ANGRY,B_SOFT};
  const MouthStyle mouthCycle[] = {M_FLAT,M_SMILE,M_SMIRK,M_OPEN,M_O,M_FROWN,M_WAVE,M_GRIN};
  const GazeStyle gazeCycle[] = {G_CENTER,G_WANDER,G_LEFT,G_RIGHT,G_UP,G_DOWN,G_SCAN};
  const MotionStyle motionCycle[] = {MOT_FLOAT,MOT_SWAY,MOT_PULSE,MOT_BOUNCE,MOT_NOD,MOT_TREMBLE,MOT_STILL,MOT_DRIFT};

  for (int i = 0; i < 100; ++i) {
    profiles[i].eye = eyeCycle[i % 7];
    profiles[i].brow = browCycle[(i * 3) % 6];
    profiles[i].mouth = mouthCycle[(i * 5) % 8];
    profiles[i].gaze = gazeCycle[(i * 2) % 7];
    profiles[i].motion = motionCycle[(i * 3) % 8];
  }

  assignGroup("HAPPY JOYFUL CONTENT GRATEFUL RELIEVED ENCOURAGING EMPATHETIC",
              E_ROUND,B_SOFT,M_SMILE,G_CENTER,MOT_FLOAT);
  assignGroup("EXCITED AMAZED AWESTRUCK SHOCKED SURPRISED STARTLED MIND_BLOWN EUREKA",
              E_WIDE,B_RAISED,M_O,G_CENTER,MOT_BOUNCE);
  assignGroup("AMUSED PROUD PLAYFUL TEASING MISCHIEVOUS SARCASTIC SKEPTICAL",
              E_SQUINT,B_ASYMMETRIC,M_SMIRK,G_CENTER,MOT_SWAY);
  assignGroup("SAD DISAPPOINTED LONELY GUILTY NOSTALGIC",
              E_SLEEPY,B_WORRIED,M_FROWN,G_DOWN,MOT_DRIFT);
  assignGroup("ANGRY ANNOYED FRUSTRATED DETERMINED",
              E_NARROW,B_ANGRY,M_WAVE,G_CENTER,MOT_TREMBLE);
  assignGroup("WORRIED AFRAID EMBARRASSED SHY",
              E_WIDE,B_WORRIED,M_FLAT,G_DOWN,MOT_TREMBLE);
  assignGroup("CONFUSED PUZZLED INTRIGUED SUSPICIOUS",
              E_DIAMOND,B_ASYMMETRIC,M_WAVE,G_WANDER,MOT_SWAY);
  assignGroup("THINKING DEEP_THINKING ANALYZING CALCULATING DECIDING SOLVING",
              E_NARROW,B_NEUTRAL,M_FLAT,G_UP,MOT_PULSE);
  assignGroup("LEARNING REMEMBERING RECALLING CREATIVE",
              E_ROUND,B_RAISED,M_SMIRK,G_UP,MOT_ORBIT);
  assignGroup("LISTENING ATTENTIVE WAITING_FOR_REPLY",
              E_WIDE,B_RAISED,M_FLAT,G_CENTER,MOT_FLOAT);
  assignGroup("SPEAKING",E_ROUND,B_SOFT,M_TALK,G_CENTER,MOT_SWAY);
  assignGroup("GREETING FAREWELL",E_ROUND,B_RAISED,M_SMILE,G_CENTER,MOT_BOUNCE);
  assignGroup("SCANNING TRACKING LOOKING_LEFT LOOKING_RIGHT",
              E_NARROW,B_NEUTRAL,M_FLAT,G_SCAN,MOT_SCAN);
  assignGroup("LOOKING_UP",E_WIDE,B_RAISED,M_FLAT,G_UP,MOT_STILL);
  assignGroup("LOOKING_DOWN",E_SLEEPY,B_SOFT,M_FLAT,G_DOWN,MOT_STILL);
  assignGroup("FOCUSED",E_NARROW,B_NEUTRAL,M_FLAT,G_CENTER,MOT_STILL);
  assignGroup("ALERT",E_WIDE,B_ANGRY,M_O,G_CENTER,MOT_PULSE);
  assignGroup("SLEEPY DROWSY LOW_BATTERY",E_SLEEPY,B_SOFT,M_FROWN,G_DOWN,MOT_DRIFT);
  assignGroup("SLEEPING MEDITATING",E_CLOSED,B_SOFT,M_FLAT,G_DOWN,MOT_FLOAT);
  assignGroup("WAKING_UP",E_SLEEPY,B_RAISED,M_O,G_UP,MOT_BOUNCE);
  assignGroup("ENERGIZED CELEBRATING SHOWTIME",E_WIDE,B_RAISED,M_GRIN,G_CENTER,MOT_BOUNCE);
  assignGroup("RELAXED IDLE",E_OVAL,B_SOFT,M_FLAT,G_CENTER,MOT_FLOAT);
  assignGroup("BORED",E_SLEEPY,B_ASYMMETRIC,M_WAVE,G_WANDER,MOT_DRIFT);
  assignGroup("RESTLESS OVERWHELMED",E_WIDE,B_WORRIED,M_WAVE,G_WANDER,MOT_TREMBLE);
  assignGroup("BOOTING CONNECTING PROCESSING LOADING UPDATING",
              E_DIAMOND,B_NEUTRAL,M_FLAT,G_SCAN,MOT_PULSE);
  assignGroup("ONLINE",E_ROUND,B_RAISED,M_SMILE,G_CENTER,MOT_PULSE);
  assignGroup("OFFLINE ERROR",E_NARROW,B_WORRIED,M_FROWN,G_CENTER,MOT_DRIFT);
  assignGroup("RECOVERING",E_SLEEPY,B_WORRIED,M_FLAT,G_CENTER,MOT_PULSE);
  assignGroup("MISCHIEVOUS GOOFY CURIOUS_PET",
              E_WINK,B_ASYMMETRIC,M_SMIRK,G_WANDER,MOT_BOUNCE);
  assignGroup("CONFIDENT",E_NARROW,B_NEUTRAL,M_SMIRK,G_CENTER,MOT_STILL);
  assignGroup("MYSTERIOUS",E_SLEEPY,B_NEUTRAL,M_FLAT,G_CENTER,MOT_DRIFT);
  assignGroup("GLITCHY",E_DIAMOND,B_ASYMMETRIC,M_WAVE,G_CENTER,MOT_TREMBLE);
  assignGroup("LOVE_STRUCK LOVING",E_ROUND,B_SOFT,M_SMILE,G_CENTER,MOT_FLOAT);
  assignGroup("DREAMING",E_CLOSED,B_SOFT,M_SMILE,G_CENTER,MOT_DRIFT);
  assignGroup("MIRRORING",E_OVAL,B_ASYMMETRIC,M_WAVE,G_CENTER,MOT_SWAY);
  assignGroup("SIGNATURE",E_DIAMOND,B_ASYMMETRIC,M_SMIRK,G_CENTER,MOT_ORBIT);
}

// -------------------------
// Utilidades gráficas
// -------------------------
void thickLine(int x1, int y1, int x2, int y2, uint16_t color, int thickness) {
  for (int i = -thickness / 2; i <= thickness / 2; ++i) {
    gfx->drawLine(x1, y1 + i, x2, y2 + i, color);
  }
}

void relativePolyline(const float* xs, const float* ys, int count, uint16_t color, int thickness=2, float offsetY=0) {
  for (int i = 0; i < count - 1; ++i) {
    thickLine(X(xs[i]), Y(ys[i]+offsetY), X(xs[i+1]), Y(ys[i+1]+offsetY), color, thickness);
  }
}

bool stateIn(const char* list) {
  char buffer[260];
  strncpy(buffer, list, sizeof(buffer)-1);
  buffer[sizeof(buffer)-1] = '\0';
  char* token = strtok(buffer, " ");
  while (token != nullptr) {
    if (strcmp(STATE_NAMES[currentState], token) == 0) return true;
    token = strtok(nullptr, " ");
  }
  return false;
}

uint16_t stateAccent() {
  if (stateIn("ANGRY FRUSTRATED ANNOYED ALERT ERROR SHOCKED MIND_BLOWN")) return RED;
  if (stateIn("SAD DISAPPOINTED LONELY GUILTY SLEEPY SLEEPING DROWSY MYSTERIOUS")) return LILAC;
  if (stateIn("PLAYFUL MISCHIEVOUS TEASING LOVING LOVE_STRUCK SHY EMBARRASSED")) return PINK;
  if (stateIn("WAKING_UP SURPRISED AMAZED EUREKA ENERGIZED SHOWTIME")) return CREAM;
  if (stateIn("RELAXED RELIEVED GRATEFUL EMPATHETIC RECHARGING")) return GREEN;
  return CYAN;
}

const char* badgeSymbol() {
  if (stateIn("HAPPY JOYFUL CONTENT")) return ":)";
  if (stateIn("SAD DISAPPOINTED LONELY")) return ":(";
  if (stateIn("ANGRY FRUSTRATED ANNOYED")) return "!!";
  if (stateIn("CURIOUS INTRIGUED CONFUSED PUZZLED SKEPTICAL")) return "?";
  if (stateIn("THINKING DEEP_THINKING ANALYZING CALCULATING")) return "...";
  if (stateIn("SLEEPY SLEEPING DROWSY")) return "zZ";
  if (stateIn("LOVING LOVE_STRUCK")) return "<3";
  if (stateIn("ERROR ALERT LOW_BATTERY")) return "!";
  if (stateIn("SCANNING TRACKING DETECTING")) return "[+]";
  if (stateIn("GLITCHY")) return "#";
  return "*";
}

void drawArcLine(float cx, float cy, float rx, float ry, float startDeg,
                 float endDeg, uint16_t color, int thickness=2) {
  const int segments = 24;
  float previousX = cx + cosf(startDeg * 0.01745329252f) * rx;
  float previousY = cy + sinf(startDeg * 0.01745329252f) * ry;
  for (int i = 1; i <= segments; ++i) {
    float a = startDeg + (endDeg - startDeg) * ((float)i / segments);
    float nextX = cx + cosf(a * 0.01745329252f) * rx;
    float nextY = cy + sinf(a * 0.01745329252f) * ry;
    thickLine(X(previousX), Y(previousY), X(nextX), Y(nextY), color, thickness);
    previousX = nextX;
    previousY = nextY;
  }
}

// -------------------------
// Fondo: anillos y partículas
// -------------------------
void drawBackground(float timeSec, float faceX, float faceY) {
  gfx->fillScreen(BG);

  const uint16_t ringColors[3] = {CYAN,LILAC,PINK};
  for (int i = 0; i < 3; ++i) {
    int radius = R(210 + i * 11);
    gfx->drawCircle((int)faceX - (frameReady ? SPRITE_X : 0), (int)faceY, radius, ringColors[i]);
  }

  const uint16_t particleColors[4] = {CYAN,PINK,LILAC,PEACH};
  for (int i = 0; i < 28; ++i) {
    float px = faceX + cosf(i * 8.3f + timeSec * 0.18f) * R(145 + (i % 4) * 13);
    float py = faceY + sinf(i * 4.7f + timeSec * 0.18f) * R(180 + (i % 3) * 9);
    gfx->fillCircle((int)px - (frameReady ? SPRITE_X : 0), (int)py, 1, particleColors[i % 4]);
  }
}

// -------------------------
// Cabello, cabeza, cuello,
// hombros y auriculares
// -------------------------
void drawHeadAndHair() {
  // Hombros y cuello
  gfx->fillEllipse(X(0), Y(220), R(155), R(95), 0x2127);
  gfx->drawEllipse(X(0), Y(220), R(155), R(95), 0x5B31);
  gfx->fillRect(X(-48), Y(100), R(96), R(95), SKIN);

  // Rostro
  gfx->fillEllipse(X(0), Y(8), R(176), R(182), SKIN);
  gfx->drawEllipse(X(0), Y(8), R(176), R(182), SKIN_LIT);

  // Silueta principal del cabello
  gfx->fillEllipse(X(-5), Y(-101), R(179), R(97), HAIR);
  gfx->drawEllipse(X(-5), Y(-101), R(179), R(97), 0x624B);

  // Flequillo: triángulos superpuestos para reproducir la silueta del Python
  gfx->fillTriangle(X(-174),Y(-92), X(-164),Y(-174), X(-108),Y(-213), HAIR_3);
  gfx->fillTriangle(X(-164),Y(-174), X(-108),Y(-213), X(-35),Y(-226), HAIR_3);
  gfx->fillTriangle(X(-108),Y(-213), X(-35),Y(-226), X(48),Y(-211), HAIR_3);
  gfx->fillTriangle(X(-35),Y(-226), X(48),Y(-211), X(112),Y(-179), HAIR_3);
  gfx->fillTriangle(X(48),Y(-211), X(112),Y(-179), X(158),Y(-121), HAIR_3);
  gfx->fillTriangle(X(-174),Y(-92), X(-122),Y(-93), X(-73),Y(-113), HAIR_2);
  gfx->fillTriangle(X(-122),Y(-93), X(-73),Y(-113), X(-28),Y(-150), HAIR_2);
  gfx->fillTriangle(X(-73),Y(-113), X(-28),Y(-150), X(23),Y(-129), HAIR_2);
  gfx->fillTriangle(X(-28),Y(-150), X(23),Y(-129), X(63),Y(-158), HAIR_2);
  gfx->fillTriangle(X(23),Y(-129), X(63),Y(-158), X(102),Y(-144), HAIR_2);
  gfx->fillTriangle(X(63),Y(-158), X(102),Y(-144), X(158),Y(-121), HAIR_2);

  // Orejas
  gfx->fillEllipse(X(-165),Y(6),R(17),R(31),SKIN);
  gfx->drawEllipse(X(-165),Y(6),R(17),R(31),SKIN_LIT);
  gfx->fillEllipse(X(165),Y(6),R(17),R(31),SKIN);
  gfx->drawEllipse(X(165),Y(6),R(17),R(31),SKIN_LIT);

}

// -------------------------
// Ojos y cejas
// -------------------------
void drawEye(float eyeX, float eyeY, bool isLeft, float timeSec, bool blink,
             uint16_t accent, const FaceProfile& profile) {
  float wUnits = isLeft ? 98.0f : 86.0f;
  float hUnits = isLeft ? 82.0f : 72.0f;
  float hMod = 1.0f;

  switch (profile.eye) {
    case E_WIDE:    hMod = 1.22f + 0.05f * sinf(timeSec * 4); wUnits *= 1.08f; break;
    case E_NARROW:  hMod = 0.62f; break;
    case E_SLEEPY:  hMod = 0.43f + 0.04f * sinf(timeSec * 1.4f); break;
    case E_SQUINT:  hMod = 0.58f + 0.08f * fabsf(sinf(timeSec * 2.7f)); break;
    case E_DIAMOND: hMod = 0.92f; wUnits *= 0.94f; break;
    case E_ROUND:   hMod = 1.13f; wUnits *= 0.92f; break;
    case E_WINK:
      if ((isLeft && ((int)(timeSec * 1.5f) % 5 == 0)) ||
          (!isLeft && ((int)(timeSec * 1.5f) % 5 != 0))) hMod = 0.08f;
      break;
    case E_CLOSED: hMod = 0.06f; break;
    default: break;
  }

  const char* state = STATE_NAMES[currentState];
  if (strcmp(state,"SLEEPY")==0 || strcmp(state,"SLEEPING")==0 ||
      strcmp(state,"DROWSY")==0 || strcmp(state,"LOW_BATTERY")==0) hMod *= 0.85f;

  int w = R(wUnits);
  int h = max(2,R(hUnits * hMod));
  int cx = X(eyeX);
  int cy = Y(eyeY);

  if (blink || h <= 3) {
    thickLine(cx-w/2,cy,cx+w/2,cy,accent,3);
    return;
  }

  // Contorno de ojo asimétrico
  if (profile.eye == E_DIAMOND) {
    int topY=cy-h/2, bottomY=cy+h/2;
    gfx->fillTriangle(cx,topY,cx+w/2,cy,cx,bottomY,0xF9DF);
    gfx->fillTriangle(cx,topY,cx-w/2,cy,cx,bottomY,0xF9DF);
    gfx->drawLine(cx,topY,cx+w/2,cy,accent);
    gfx->drawLine(cx+w/2,cy,cx,bottomY,accent);
    gfx->drawLine(cx,bottomY,cx-w/2,cy,accent);
    gfx->drawLine(cx-w/2,cy,cx,topY,accent);
  } else if (isLeft) {
    gfx->fillEllipse(cx,cy,w/2,h/2,0xEFFF);
    gfx->drawEllipse(cx,cy,w/2,h/2,accent);
    gfx->drawEllipse(cx,cy,w/2-1,h/2-1,accent);
  } else {
    gfx->fillRoundRect(cx-w/2,cy-h/2,w,h,max(3,h/3),0xF9DF);
    gfx->drawRoundRect(cx-w/2,cy-h/2,w,h,max(3,h/3),accent);
  }

  // Dirección de mirada
  float dx = sinf(timeSec * 0.9f) * 4.0f;
  float dy = cosf(timeSec * 0.7f) * 2.0f;
  switch (profile.gaze) {
    case G_LEFT:  dx=-18; break;
    case G_RIGHT: dx=18; break;
    case G_UP:    dy=-13; break;
    case G_DOWN:  dy=12; break;
    case G_WANDER: dx=sinf(timeSec*2.4f)*15; dy=cosf(timeSec*1.8f)*8; break;
    case G_SCAN:   dx=sinf(timeSec*2.8f)*17; dy=sinf(timeSec*1.3f)*3; break;
    default: break;
  }
  if (stateIn("THINKING DEEP_THINKING ANALYZING CALCULATING")) {
    dx=10; dy=-9+sinf(timeSec*2)*2;
  }
  if (strcmp(state,"GOOFY")==0 && !isLeft) dx += 10;
  if (stateIn("SURPRISED SHOCKED AFRAID ALERT EXCITED")) dx += sinf(timeSec*7)*2;

  int pupilX = cx + R(dx);
  int pupilY = cy + R(dy);
  float pupilUnits = isLeft ? 19.0f : 16.0f;
  if (profile.eye == E_WIDE) pupilUnits *= 0.82f;
  if (profile.eye == E_NARROW || profile.eye == E_SLEEPY ||
      profile.eye == E_SQUINT || profile.eye == E_CLOSED) pupilUnits *= 0.7f;
  int pupilR = max(2,R(pupilUnits));

  gfx->fillCircle(pupilX,pupilY,pupilR,EYE_DARK);
  gfx->fillCircle(pupilX,pupilY,max(2,(int)(pupilR*0.58f)),accent);
  gfx->fillCircle(pupilX-R(3),pupilY-R(5),max(1,R(4)),WHITE);

  // Cejas: forma dependiente del perfil
  int browY = cy - h/2 - R(14);
  int leftX = cx - (int)(w*0.48f);
  int rightX = cx + (int)(w*0.48f);
  switch (profile.brow) {
    case B_ANGRY:
      thickLine(cx-(int)(w*.55f),browY-1,cx+(int)(w*.35f),browY+R(10),BROW,3); break;
    case B_WORRIED:
      thickLine(cx-(int)(w*.52f),browY+R(10),cx+(int)(w*.48f),browY-R(3),BROW,3); break;
    case B_RAISED: {
      int lift=R(5+fabsf(sinf(timeSec*2))*3);
      thickLine(leftX,browY-lift,rightX,browY-lift-R(2),BROW,3); break;
    }
    case B_ASYMMETRIC: {
      int lift=isLeft?R(2):R(-12);
      thickLine(cx-(int)(w*.5f),browY+lift,cx+(int)(w*.45f),browY+lift-R(2),BROW,3); break;
    }
    case B_SOFT:
      thickLine(cx-(int)(w*.48f),browY+R(3),cx,browY-R(2),BROW,3);
      thickLine(cx,browY-R(2),cx+(int)(w*.48f),browY+R(3),BROW,3); break;
    default:
      thickLine(leftX,browY,rightX,browY-R(2),BROW,3); break;
  }
}

// -------------------------
// Nariz, rubor y boca
// -------------------------
void drawNose() {
  const float xs[] = {-4,-13,4};
  const float ys[] = {2,28,31};
  relativePolyline(xs,ys,3,0xBC30,2);
}

void drawMouth(float timeSec) {
  const FaceProfile& p = profiles[currentState];
  float mouthX=0, mouthY=83;
  float pulse=fabsf(sinf(timeSec * (stateIn("EXCITED ANGRY ALERT") ? 4.0f : 2.0f)));

  if (speaking || currentState == 41 || p.mouth == M_TALK) {
    float openAmount=5+fabsf(sinf(timeSec*11.5f))*22+fabsf(sinf(timeSec*4.1f))*5;
    float width=48+fabsf(sinf(timeSec*5))*18;
    gfx->fillEllipse(X(mouthX),Y(mouthY),R(width/2),max(2,R(openAmount/2)),MOUTH_DK);
    gfx->drawEllipse(X(mouthX),Y(mouthY),R(width/2),max(2,R(openAmount/2)),PINK);
    if (openAmount>17) {
      gfx->drawLine(X(-width/2+5),Y(mouthY+2),
                   X(width/2-5),Y(mouthY+2),CREAM);
    }
    return;
  }

  switch (p.mouth) {
    case M_SMILE: {
      const float xs[]={-36,-20,0,20,36};
      const float ys[]={-1,10,15,10,-1};
      relativePolyline(xs,ys,5,PINK,3,mouthY); break;
    }
    case M_GRIN:
      gfx->fillEllipse(X(0),Y(mouthY+7),R(33),R(13+pulse*3),0xFF9E);
      gfx->drawEllipse(X(0),Y(mouthY+7),R(33),R(13+pulse*3),PINK);
      gfx->drawLine(X(-25),Y(mouthY+5),X(25),Y(mouthY+5),0xDC75); break;
    case M_SMIRK: {
      const float xs[]={-28,1,30};
      const float ys[]={4,7,-6-pulse*2};
      relativePolyline(xs,ys,3,PINK,3,mouthY); break;
    }
    case M_OPEN:
      gfx->fillEllipse(X(0),Y(mouthY+4),R(25),R(14+pulse*4),MOUTH_DK);
      gfx->drawEllipse(X(0),Y(mouthY+4),R(25),R(14+pulse*4),PINK); break;
    case M_O: {
      int r=R(10+pulse*6);
      gfx->fillEllipse(X(0),Y(mouthY),r,r+R(4),MOUTH_DK);
      gfx->drawEllipse(X(0),Y(mouthY),r,r+R(4),PINK); break;
    }
    case M_FROWN: {
      const float xs[]={-34,-18,0,18,34};
      const float ys[]={12,3,0,3,12};
      relativePolyline(xs,ys,5,PINK,3,mouthY); break;
    }
    case M_WAVE: {
      const float xs[]={-30,-15,0,15,30};
      const float ys[]={2,-2,4,-2,2};
      relativePolyline(xs,ys,5,PINK,3,mouthY); break;
    }
    default: {
      float width=20+pulse*5;
      thickLine(X(-width),Y(mouthY),X(width),Y(mouthY-1),PINK,2); break;
    }
  }
}

// -------------------------
// Emblema / burbuja de estado
// -------------------------
void drawStateBubble(float timeSec) {
  uint16_t accent=stateAccent();
  float bob=sinf(timeSec*2.2f)*2.0f;
  int y=Y(-252+bob);
  int x1=X(-164), x2=X(164);
  int halfH=R(27);

  gfx->fillRect(x1+R(4),y-halfH+R(4),x2-x1,halfH*2,0x0042);
  gfx->fillRect(x1,y-halfH,x2-x1,halfH*2,0x1906);
  gfx->drawRect(x1,y-halfH,x2-x1,halfH*2,accent);
  gfx->fillTriangle(X(-10),y+halfH-1,X(10),y+halfH-1,X(0),y+halfH+R(12),0x1906);
  gfx->drawLine(X(-10),y+halfH-1,X(0),y+halfH+R(12),accent);
  gfx->drawLine(X(10),y+halfH-1,X(0),y+halfH+R(12),accent);

  gfx->setTextFont(1);
  gfx->setTextSize(1);
  gfx->setTextColor(accent,0x1906);
  gfx->setCursor(x1+R(12),y-4);
  gfx->print(badgeSymbol());

  gfx->setTextColor(TEXT,0x1906);
  gfx->setCursor(x1+R(66),y-4);
  // El globo usa la microfrase de Python v4; la pantalla muestra el inicio
  // si la frase supera el ancho disponible.
  char shortPhrase[25];
  strncpy(shortPhrase,STATE_PHRASES[currentState],sizeof(shortPhrase)-1);
  shortPhrase[sizeof(shortPhrase)-1]='\0';
  if (strlen(STATE_PHRASES[currentState]) >= sizeof(shortPhrase)) {
    shortPhrase[21]='.'; shortPhrase[22]='.'; shortPhrase[23]='.'; shortPhrase[24]='\0';
  }
  gfx->print(shortPhrase);

  float pulse=0.55f+0.45f*fabsf(sinf(timeSec*3));
  gfx->drawLine(x1+R(12),y+halfH-R(3),
               x1+R(12)+R(32*pulse),y+halfH-R(3),accent);
}

// -------------------------
// Efectos de estado esenciales
// Se irán ampliando hasta portar EFFECTS completo
// -------------------------
void drawStateEffect(float timeSec) {
  uint16_t accent=stateAccent();

  if (stateIn("LOVING LOVE_STRUCK")) {
    for (int i=0;i<3;i++) {
      int x=X(-115+i*115);
      int y=Y(-115-fabsf(sinf(timeSec*2+i))*10);
      gfx->fillCircle(x-R(4),y,R(5),PINK);
      gfx->fillCircle(x+R(4),y,R(5),PINK);
      gfx->fillTriangle(x-R(9),y+R(2),x+R(9),y+R(2),x,y+R(15),PINK);
    }
  } else if (stateIn("HAPPY JOYFUL EXCITED AMAZED CREATIVE")) {
    for (int i=0;i<5;i++) {
      float a=timeSec*1.7f+i*1.25f;
      int x=X(cosf(a)*205);
      int y=Y(-90+sinf(a)*100);
      gfx->drawLine(x-R(5),y,x+R(5),y,accent);
      gfx->drawLine(x,y-R(5),x,y+R(5),accent);
    }
  } else if (stateIn("SAD DISAPPOINTED LONELY GUILTY")) {
    for (int side=-1;side<=1;side+=2) {
      int x=X(side*112);
      int y=Y(4+fabsf(sinf(timeSec*2.5f))*16);
      gfx->fillEllipse(x,y,R(4),R(7),0x8EDF);
    }
  } else if (stateIn("ANGRY FRUSTRATED ANNOYED")) {
    int y=Y(-95);
    thickLine(X(-180),y,X(-165),y-R(5),RED,2);
    thickLine(X(-165),y-R(5),X(-150),y+R(4),RED,2);
    thickLine(X(150),y+R(4),X(165),y-R(5),RED,2);
    thickLine(X(165),y-R(5),X(180),y,RED,2);
  } else if (stateIn("THINKING DEEP_THINKING PROCESSING LOADING WAITING_FOR_REPLY")) {
    int active=((int)(timeSec*4))%3;
    for (int i=0;i<3;i++) {
      int r=R(i==active?7:4);
      gfx->fillCircle(X(145+i*15),Y(105),r,i==active?LILAC:0x52CE);
    }
  } else if (stateIn("SLEEPY SLEEPING DROWSY")) {
    gfx->setTextFont(2);
    gfx->setTextColor(LILAC,BG);
    gfx->setCursor(X(125),Y(-132));
    gfx->print("zZ");
  } else if (stateIn("CURIOUS CONFUSED PUZZLED SKEPTICAL")) {
    gfx->setTextFont(4);
    gfx->setTextColor(LILAC,BG);
    gfx->setCursor(X(185),Y(-115));
    gfx->print("?");
  } else if (stateIn("GLITCHY")) {
    for (int i=0;i<5;i++) {
      int yy=Y(-110+i*45);
      int shift=R(sinf(timeSec*13+i)*15);
      gfx->drawLine(X(-205)+shift,yy,X(-175)+shift,yy,(i%2)?PINK:CYAN);
      gfx->drawLine(X(175)-shift,yy+R(7),X(205)-shift,yy+R(7),(i%2)?CYAN:PINK);
    }
  } else if (stateIn("ALERT ERROR LOW_BATTERY")) {
    int radius=R(10+fabsf(sinf(timeSec*6))*8);
    gfx->drawCircle(X(190),Y(-25),radius,RED);
    gfx->drawCircle(X(-190),Y(-25),radius,RED);
  } else if (stateIn("SCANNING TRACKING DETECTING")) {
    int x=X(sinf(timeSec*2.4f)*190);
    for (int yy=Y(-120); yy<Y(100); yy+=R(10)) {
      gfx->drawLine(x,yy,x,yy+R(4),CYAN);
    }
  } else if (stateIn("EUREKA")) {
    gfx->drawCircle(X(195),Y(-126),R(13),CREAM);
    gfx->drawLine(X(190),Y(-110),X(200),Y(-110),CREAM);
  }
}

// -------------------------
// Dibujo completo del avatar
// -------------------------
void drawAvatar(float timeSec) {
  FaceProfile profile=profiles[currentState];
  float faceX=CENTER_X;
  float faceY=CENTER_Y;

  // Movimiento general de cabeza equivalente al motion del Python
  switch (profile.motion) {
    case MOT_FLOAT: faceY += sinf(timeSec*1.2f)*R(4); break;
    case MOT_SWAY:
      faceX += sinf(timeSec*1.7f)*R(5);
      faceY += sinf(timeSec*2.4f)*R(2); break;
    case MOT_PULSE: faceY += sinf(timeSec*3.2f)*R(2); break;
    case MOT_BOUNCE: faceY -= fabsf(sinf(timeSec*3.0f))*R(7); break;
    case MOT_NOD: faceY += sinf(timeSec*2.2f)*R(5); break;
    case MOT_TREMBLE:
      faceX += sinf(timeSec*18)*R(2.5f);
      faceY += cosf(timeSec*16)*R(1.5f); break;
    case MOT_DRIFT:
      faceX += sinf(timeSec*.55f)*R(5);
      faceY += cosf(timeSec*.7f)*R(3); break;
    case MOT_ORBIT:
      faceX += sinf(timeSec*1.3f)*R(4);
      faceY += cosf(timeSec*1.3f)*R(4); break;
    case MOT_SCAN: faceX += sinf(timeSec*2.1f)*R(3); break;
    default: break;
  }

  // Desplazar toda la composición, no solo el fondo.
  motionOffsetX = (int)(faceX - CENTER_X);
  motionOffsetY = (int)(faceY - CENTER_Y);
  drawBackground(timeSec,faceX,faceY);
  drawHeadAndHair();

  bool blink=blinkNow;
  drawEye(-69,-34,true,timeSec,blink,CYAN,profile);
  drawEye(70,-29,false,timeSec,blink,PINK,profile);

  // Microgestos del Python v4
  if (stateIn("SAD DISAPPOINTED LONELY GUILTY")) {
    int tearY=Y(12+fabsf(sinf(timeSec*2.3f))*10);
    gfx->fillEllipse(X(-114),tearY,R(4),R(7),0x8EDF);
  }
  if (stateIn("SURPRISED SHOCKED STARTLED MIND_BLOWN")) {
    int halo=R(3+fabsf(sinf(timeSec*5))*4);
    gfx->drawCircle(X(-106),Y(-78),halo,CREAM);
  }

  drawNose();

  if (stateIn("EMBARRASSED SHY LOVE_STRUCK HAPPY PLAYFUL")) {
    gfx->fillEllipse(X(-104),Y(33),R(22),R(9),0xECF7);
    gfx->fillEllipse(X(104),Y(33),R(22),R(9),0xECF7);
  }

  drawMouth(timeSec);

  // Headset delante de la silueta, como en el simulador
  drawArcLine(0,-40,190,142,190,350,CYAN,3);
  gfx->fillEllipse(X(-165),Y(11),R(12),R(21),LILAC);
  gfx->drawEllipse(X(-165),Y(11),R(12),R(21),CYAN);
  gfx->fillEllipse(X(165),Y(11),R(12),R(21),PINK);
  gfx->drawEllipse(X(165),Y(11),R(12),R(21),CYAN);

  drawStateEffect(timeSec);
  drawStateBubble(timeSec);

  // Identidad y estado debajo del avatar
  gfx->setTextFont(2);
  gfx->setTextColor(CYAN,BG);
  gfx->setCursor(X(-47),Y(162));
  gfx->print("C S R - B O T");

  gfx->setTextFont(1);
  gfx->setTextColor(TEXT,BG);
  gfx->setCursor(X(-48),Y(187));
  gfx->print(STATE_NAMES[currentState]);

  // Mostrar el fotograma completo solo cuando ya esta terminado.
  if (frameReady) frame.pushSprite(SPRITE_X, 0);
}

// -------------------------
// Cambiar estados / comandos
// -------------------------
void setState(int index) {
  if (index < 0) index = 99;
  if (index > 99) index = 0;
  currentState = index;
  redrawRequested = true;
  Serial.print("CSR-BOT STATE: ");
  Serial.print(currentState + 1);
  Serial.print("/100 - ");
  Serial.println(STATE_NAMES[currentState]);
}

void setTimePersonality(int mode) {
  // Mismo criterio horario que Python v4.
  if (mode == 1) setState(findState("ANGRY"));       // 06:00-11:59
  if (mode == 2) setState(findState("PLAYFUL"));     // 12:00-17:59
  if (mode == 3) setState(findState("SARCASTIC"));   // 18:00-23:59
  if (mode == 4) setState(findState("MYSTERIOUS"));  // 00:00-05:59
}

void handleSerial() {
  if (!Serial.available()) return;

  String command=Serial.readStringUntil('\n');
  command.trim();
  command.toUpperCase();

  if (command=="N") setState(currentState+1);
  else if (command=="P") setState(currentState-1);
  else if (command=="B") {
    if (frameReady) {
      blinkNow=true;
      blinkUntil=millis()+220;
      redrawRequested = true;
    } else {
      Serial.println("Parpadeo manual desactivado en modo TFT directo para evitar el destello de pantalla completa.");
    }
  }
  else if (command=="S") {
    speaking=!speaking;
    redrawRequested = true;
    if (speaking) {
      previousStateBeforeSpeaking=currentState;
      setState(findState("SPEAKING"));
    } else {
      setState(previousStateBeforeSpeaking);
      Serial.println("MOUTH TALK: OFF");
    }
  }
  else if (command=="1" || command=="2" || command=="3" || command=="4") {
    setTimePersonality(command.toInt());
  }
  else if (command.length()>0) {
    Serial.println("Comandos: N, P, B, S, 1, 2, 3, 4");
  }
}

// -------------------------
// Arduino
// -------------------------
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(25);
  randomSeed(esp_random());

  initializeProfiles();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BG); // Limpieza inicial unica del panel fisico

  // Sprite compacto RGB332: 300x320 = 96.000 bytes.
  // No intentamos 16 bpp porque el ESP32-WROOM normalmente no tiene PSRAM.
  // Si la asignacion falla, NO detenemos el programa: usamos TFT directamente.
  frame.setColorDepth(8);
  if (frame.createSprite(SPRITE_W, SCREEN_H) != nullptr) {
    frameReady = true;
    gfx = &frame;
    Serial.printf("Sprite 8 bpp activo (%u bytes aprox.). Heap libre: %u bytes\n",
                  SPRITE_W * SCREEN_H, ESP.getFreeHeap());
  } else {
    frameReady = false;
    gfx = &tft;
    Serial.println("AVISO: no se pudo reservar el sprite de 96 KB.");
    Serial.printf("Heap libre: %u bytes. Se activa modo TFT directo seguro.\n", ESP.getFreeHeap());
  }

  startedAt=millis();
  nextBlink=millis()+random(2000,4600);

  Serial.println();
  Serial.println("==================================");
  Serial.println(" CSR-BOT FACE ENGINE v1.3 SAFE SPRITE + FALLBACK");
  Serial.println(" ESP32 + TFT 3.5 inch");
  Serial.println("==================================");
  Serial.println("100 estados cargados.");
  Serial.println("N/P: cambiar estado | B: blink (solo con sprite) | S: hablar");
  Serial.println("1 manana | 2 tarde | 3 noche | 4 madrugada");

  drawAvatar(0);
  redrawRequested = false;
}

void loop() {
  handleSerial();

  unsigned long now=millis();

  if (frameReady && !blinkNow && now>=nextBlink) {
    blinkNow=true;
    blinkUntil=now+random(100,180);
    nextBlink=now+random(2000,4600);
    redrawRequested = true;
  }
  if (frameReady && blinkNow && now>=blinkUntil) {
    blinkNow=false;
    redrawRequested = true;
  }

  // MODO DOBLE BUFFER: cada fotograma se compone en RAM antes de mostrarse.
  // Redibuja cuando cambia un estado, se abre/cierra un parpadeo o se simula habla.
  // El doble buffer evita mostrar el borrado y el dibujo progresivo del fotograma.
  bool talkFrame = frameReady && speaking && (now-lastFrame >= 280);
  if ((redrawRequested || talkFrame) && now-lastFrame >= 50) {
    lastFrame=now;
    redrawRequested=false;
    float timeSec=(now-startedAt)/1000.0f;
    drawAvatar(timeSec);
  }
}

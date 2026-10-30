#include "config.h"

// -----------------------------------------------------------------------------
// Los 100 estados originales. Se conserva el orden del simulador Python v4.
// -----------------------------------------------------------------------------
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

// Perfil facial asignado a cada estado.
FaceProfile profiles[100];

// Estado y temporizadores compartidos.
int currentState = 60; // IDLE
bool speaking = false;
int previousStateBeforeSpeaking = 60;
bool blinkNow = false;
unsigned long blinkUntil = 0;
unsigned long nextBlink = 0;
unsigned long lastFrame = 0;
bool redrawRequested = true;
unsigned long startedAt = 0;

// -----------------------------------------------------------------------------
// Construcción de perfiles faciales
// -----------------------------------------------------------------------------
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

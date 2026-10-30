#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Tipos compartidos del motor facial
// -----------------------------------------------------------------------------
enum EyeStyle {
  E_OVAL, E_WIDE, E_NARROW, E_DIAMOND, E_SLEEPY,
  E_ROUND, E_SQUINT, E_WINK, E_CLOSED
};

enum BrowStyle {
  B_NEUTRAL, B_RAISED, B_ASYMMETRIC, B_WORRIED, B_ANGRY, B_SOFT
};

enum MouthStyle {
  M_FLAT, M_SMILE, M_SMIRK, M_OPEN, M_O,
  M_FROWN, M_WAVE, M_GRIN, M_TALK
};

enum GazeStyle {
  G_CENTER, G_WANDER, G_LEFT, G_RIGHT, G_UP, G_DOWN, G_SCAN
};

enum MotionStyle {
  MOT_FLOAT, MOT_SWAY, MOT_PULSE, MOT_BOUNCE, MOT_NOD,
  MOT_TREMBLE, MOT_STILL, MOT_DRIFT, MOT_ORBIT, MOT_SCAN
};

struct FaceProfile {
  EyeStyle eye;
  BrowStyle brow;
  MouthStyle mouth;
  GazeStyle gaze;
  MotionStyle motion;
};

// -----------------------------------------------------------------------------
// Pantalla y geometría (definidos en display.cpp)
// -----------------------------------------------------------------------------
extern TFT_eSPI tft;
extern TFT_eSprite frame;
extern TFT_eSPI* gfx;
extern bool frameReady;

extern const int SPRITE_X;
extern const int SPRITE_W;
extern const int SCREEN_W;
extern const int SCREEN_H;
extern const int CENTER_X;
extern const int CENTER_Y;
extern const float SCALE;

extern int motionOffsetX;
extern int motionOffsetY;

// Paleta RGB565
extern const uint16_t BG;
extern const uint16_t PANEL;
extern const uint16_t TEXT;
extern const uint16_t MUTED;
extern const uint16_t CYAN;
extern const uint16_t PINK;
extern const uint16_t LILAC;
extern const uint16_t PEACH;
extern const uint16_t CREAM;
extern const uint16_t RED;
extern const uint16_t GREEN;
extern const uint16_t SKIN;
extern const uint16_t SKIN_LIT;
extern const uint16_t HAIR;
extern const uint16_t HAIR_2;
extern const uint16_t HAIR_3;
extern const uint16_t BROW;
extern const uint16_t EYE_DARK;
extern const uint16_t MOUTH_DK;
extern const uint16_t WHITE;

// -----------------------------------------------------------------------------
// Estados y variables compartidas (definidos en emotions.cpp)
// -----------------------------------------------------------------------------
extern const char* STATE_NAMES[100];
extern const char* STATE_PHRASES[100];
extern FaceProfile profiles[100];

extern int currentState;
extern bool speaking;
extern int previousStateBeforeSpeaking;
extern bool blinkNow;
extern unsigned long blinkUntil;
extern unsigned long nextBlink;
extern unsigned long lastFrame;
extern bool redrawRequested;
extern unsigned long startedAt;

// -----------------------------------------------------------------------------
// Utilidades geométricas y de dibujo
// -----------------------------------------------------------------------------
int X(float x);
int Y(float y);
int R(float r);

void thickLine(int x1, int y1, int x2, int y2, uint16_t color, int thickness);
void relativePolyline(const float* xs, const float* ys, int count,
                      uint16_t color, int thickness = 2, float offsetY = 0);
void drawArcLine(float cx, float cy, float rx, float ry, float startDeg,
                 float endDeg, uint16_t color, int thickness = 2);

// -----------------------------------------------------------------------------
// Pantalla
// -----------------------------------------------------------------------------
void initializeDisplay();
void drawBackground(float timeSec, float faceX, float faceY);

// -----------------------------------------------------------------------------
// Estados y perfiles
// -----------------------------------------------------------------------------
int findState(const char* name);
void assignGroup(const char* names, EyeStyle eye, BrowStyle brow,
                 MouthStyle mouth, GazeStyle gaze, MotionStyle motion);
void initializeProfiles();
bool stateIn(const char* list);
uint16_t stateAccent();
const char* badgeSymbol();

// -----------------------------------------------------------------------------
// Rostro
// -----------------------------------------------------------------------------
void drawHeadAndHair();
void drawEye(float eyeX, float eyeY, bool isLeft, float timeSec, bool blink,
             uint16_t accent, const FaceProfile& profile);
void drawNose();
void drawMouth(float timeSec);
void drawAvatar(float timeSec);

// -----------------------------------------------------------------------------
// Animaciones y efectos
// -----------------------------------------------------------------------------
void drawStateBubble(float timeSec);
void drawStateEffect(float timeSec);

// -----------------------------------------------------------------------------
// Personalidad y control
// -----------------------------------------------------------------------------
void setState(int index);
void setTimePersonality(int mode);
void handleSerial();

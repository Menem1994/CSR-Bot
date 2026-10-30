#include "config.h"

// Objetos de pantalla
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite frame = TFT_eSprite(&tft);
TFT_eSPI* gfx = &tft;
bool frameReady = false;

// Dimensiones de pantalla y área de dibujo.
// El sprite de 300 px se coloca centrado en una pantalla de 480 px.
const int SPRITE_X = 90;
const int SPRITE_W = 300;
const int SCREEN_W = 480;
const int SCREEN_H = 320;
const int CENTER_X = 240;
const int CENTER_Y = 164;
const float SCALE = 0.58f;

int motionOffsetX = 0;
int motionOffsetY = 0;

// Paleta pastel RGB565
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

int X(float x) {
  return CENTER_X - (frameReady ? SPRITE_X : 0) +
         motionOffsetX + (int)lroundf(x * SCALE);
}

int Y(float y) {
  return CENTER_Y + motionOffsetY + (int)lroundf(y * SCALE);
}

int R(float r) {
  return (int)lroundf(r * SCALE);
}

void thickLine(int x1, int y1, int x2, int y2,
               uint16_t color, int thickness) {
  for (int i = -thickness / 2; i <= thickness / 2; ++i) {
    gfx->drawLine(x1, y1 + i, x2, y2 + i, color);
  }
}

void relativePolyline(const float* xs, const float* ys, int count,
                      uint16_t color, int thickness, float offsetY) {
  for (int i = 0; i < count - 1; ++i) {
    thickLine(X(xs[i]), Y(ys[i] + offsetY),
              X(xs[i + 1]), Y(ys[i + 1] + offsetY),
              color, thickness);
  }
}

void drawArcLine(float cx, float cy, float rx, float ry,
                 float startDeg, float endDeg,
                 uint16_t color, int thickness) {
  const int segments = 24;
  float previousX = cx + cosf(startDeg * 0.01745329252f) * rx;
  float previousY = cy + sinf(startDeg * 0.01745329252f) * ry;

  for (int i = 1; i <= segments; ++i) {
    float a = startDeg + (endDeg - startDeg) * ((float)i / segments);
    float nextX = cx + cosf(a * 0.01745329252f) * rx;
    float nextY = cy + sinf(a * 0.01745329252f) * ry;

    thickLine(X(previousX), Y(previousY), X(nextX), Y(nextY),
              color, thickness);

    previousX = nextX;
    previousY = nextY;
  }
}

void initializeDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(BG);

  // Sprite RGB332 compacto: 300 x 320 = 96.000 bytes.
  // Si no hay memoria disponible, el motor usa TFT directo.
  frame.setColorDepth(8);

  if (frame.createSprite(SPRITE_W, SCREEN_H) != nullptr) {
    frameReady = true;
    gfx = &frame;
    Serial.printf(
      "Sprite 8 bpp activo (%u bytes aprox.). Heap libre: %u bytes\n",
      SPRITE_W * SCREEN_H, ESP.getFreeHeap()
    );
  } else {
    frameReady = false;
    gfx = &tft;
    Serial.println("AVISO: no se pudo reservar el sprite de 96 KB.");
    Serial.printf(
      "Heap libre: %u bytes. Se activa modo TFT directo seguro.\n",
      ESP.getFreeHeap()
    );
  }
}

void drawBackground(float timeSec, float faceX, float faceY) {
  gfx->fillScreen(BG);

  const uint16_t ringColors[3] = {CYAN, LILAC, PINK};
  for (int i = 0; i < 3; ++i) {
    int radius = R(210 + i * 11);
    gfx->drawCircle(
      (int)faceX - (frameReady ? SPRITE_X : 0),
      (int)faceY, radius, ringColors[i]
    );
  }

  const uint16_t particleColors[4] = {CYAN, PINK, LILAC, PEACH};
  for (int i = 0; i < 28; ++i) {
    float px = faceX + cosf(i * 8.3f + timeSec * 0.18f) *
               R(145 + (i % 4) * 13);
    float py = faceY + sinf(i * 4.7f + timeSec * 0.18f) *
               R(180 + (i % 3) * 9);

    gfx->fillCircle(
      (int)px - (frameReady ? SPRITE_X : 0),
      (int)py, 1, particleColors[i % 4]
    );
  }
}

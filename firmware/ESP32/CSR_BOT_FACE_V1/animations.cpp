#include "config.h"

// -----------------------------------------------------------------------------
// Elementos animados independientes del rostro: arco de auriculares,
// burbuja de estado y efectos visuales por estado.
// -----------------------------------------------------------------------------

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

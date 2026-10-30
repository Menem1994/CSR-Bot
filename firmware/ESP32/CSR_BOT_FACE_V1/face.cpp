#include "config.h"

// -----------------------------------------------------------------------------
// Geometría del personaje: cabello, cabeza, ojos, nariz, boca y composición.
// -----------------------------------------------------------------------------

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

void drawNose() {
  const float xs[] = {-4,-13,4};
  const float ys[] = {2,28,31};
  relativePolyline(xs,ys,3,0xBC30,2);
}

void drawMouth(float timeSec) {
  const FaceProfile& p = profiles[currentState];
  float mouthX=0, mouthY=83;
  float pulse=fabsf(sinf(timeSec * (stateIn("EXCITED ANGRY ALERT") ? 4.0f : 2.0f)));

  if (speaking) {
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
    case M_TALK:
    default: {
      float width=20+pulse*5;
      thickLine(X(-width),Y(mouthY),X(width),Y(mouthY-1),PINK,2); break;
    }
  }
}

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

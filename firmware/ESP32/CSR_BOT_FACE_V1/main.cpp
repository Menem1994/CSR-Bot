#include "config.h"
#include <esp_system.h>
#include "network.h"

// -----------------------------------------------------------------------------
// Coordinador del firmware: inicialización, comandos serie y ciclo principal.
// Arduino llama automáticamente a setup() y loop().
// -----------------------------------------------------------------------------

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

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(25);
  randomSeed(esp_random());

  initializeProfiles();

  initializeDisplay();

  startedAt=millis();
  nextBlink=millis()+random(2000,4600);

  Serial.println();
  Serial.println("==================================");
  Serial.println(" CSR-BOT FACE ENGINE v1.6 DIALOGO");
  Serial.println(" ESP32 + TFT 3.5 inch");
  Serial.println("==================================");
  Serial.println("100 estados cargados.");
  Serial.println("N/P: cambiar estado | B: blink (solo con sprite) | S: hablar");
  Serial.println("1 manana | 2 tarde | 3 noche | 4 madrugada");

  drawAvatar(0);
  redrawRequested = false;

  // Conexión opcional Wi-Fi; no modifica el cableado ni los pines TFT.
  initializeNetwork();
}

void loop() {
  handleSerial();
  updateNetwork();

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
  // Redibuja cuando cambia un estado, se abre/cierra un parpadeo o hay voz activa.
  // El doble buffer evita mostrar el borrado y el dibujo progresivo del fotograma.
  bool talkFrame = frameReady && speaking && (now-lastFrame >= 110);
  if ((redrawRequested || talkFrame) && now-lastFrame >= 50) {
    lastFrame=now;
    redrawRequested=false;
    float timeSec=(now-startedAt)/1000.0f;
    drawAvatar(timeSec);
  }
}

/*
  CSR-BOT FACE V1.6 — diálogo paginado y boca sincronizada

  Abrir este archivo desde Arduino IDE.
  El código está separado en los módulos .cpp y config.h de esta carpeta.
  No modificar los pines ni TFT_eSPI/User_Setup.h que ya funcionan.

  Monitor Serie: 115200 baudios
  N = siguiente estado | P = estado anterior
  B = parpadeo manual | S = activar/desactivar habla manual
  1 = mañana | 2 = tarde | 3 = noche | 4 = madrugada
*/
#include "config.h"

// setup() y loop() están implementados en main.cpp.

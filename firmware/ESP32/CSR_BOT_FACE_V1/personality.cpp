#include "config.h"

// -----------------------------------------------------------------------------
// Personalidad horaria manual.
// 1 = mañana, 2 = tarde, 3 = noche, 4 = madrugada.
// La selección automática con reloj/NTP se implementará más adelante.
// -----------------------------------------------------------------------------

void setTimePersonality(int mode) {
  // Mismo criterio horario que Python v4.
  if (mode == 1) setState(findState("ANGRY"));       // 06:00-11:59
  if (mode == 2) setState(findState("PLAYFUL"));     // 12:00-17:59
  if (mode == 3) setState(findState("SARCASTIC"));   // 18:00-23:59
  if (mode == 4) setState(findState("MYSTERIOUS"));  // 00:00-05:59
}

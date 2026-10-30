#pragma once

// Tipos compartidos del motor facial CSR-BOT.
// Este header se incluye antes de los prototipos automáticos que genera Arduino IDE.

enum EyeStyle { E_OVAL, E_WIDE, E_NARROW, E_DIAMOND, E_SLEEPY, E_ROUND, E_SQUINT, E_WINK, E_CLOSED };
enum BrowStyle { B_NEUTRAL, B_RAISED, B_ASYMMETRIC, B_WORRIED, B_ANGRY, B_SOFT };
enum MouthStyle { M_FLAT, M_SMILE, M_SMIRK, M_OPEN, M_O, M_FROWN, M_WAVE, M_GRIN, M_TALK };
enum GazeStyle { G_CENTER, G_WANDER, G_LEFT, G_RIGHT, G_UP, G_DOWN, G_SCAN };
enum MotionStyle { MOT_FLOAT, MOT_SWAY, MOT_PULSE, MOT_BOUNCE, MOT_NOD, MOT_TREMBLE, MOT_STILL, MOT_DRIFT, MOT_ORBIT, MOT_SCAN };

struct FaceProfile {
  EyeStyle eye;
  BrowStyle brow;
  MouthStyle mouth;
  GazeStyle gaze;
  MotionStyle motion;
};

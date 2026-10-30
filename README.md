# CSR-BOT — proyecto modular v1.4

Este paquete organiza el firmware que estaba concentrado en el sketch experimental v1.3.
La división en módulos mantiene la lógica gráfica original; no pretende ser todavía una
reescritura visual ni una optimización nueva del renderizado.

## Estructura

```text
csr-bot/
├── assets/
│   └── expressions/
├── documentation/
│   ├── PROJECT_STRUCTURE.md
│   └── legacy_v1_3_recovery/
├── firmware/
│   ├── ESP32/
│   │   └── CSR_BOT_FACE_V1/
│   │       ├── CSR_BOT_FACE_V1.ino
│   │       ├── config.h
│   │       ├── main.cpp
│   │       ├── display.cpp
│   │       ├── face.cpp
│   │       ├── animations.cpp
│   │       ├── emotions.cpp
│   │       └── personality.cpp
│   └── pico_w/
│       └── comandos_seriales/
│           └── comandos_seriales.ino
└── software/
    └── csr_bot_face.py
```

## Abrir en Arduino IDE

1. Abrí la carpeta `firmware/ESP32/CSR_BOT_FACE_V1`.
2. Abrí `CSR_BOT_FACE_V1.ino` desde Arduino IDE.
3. Seleccioná la misma placa y el mismo puerto COM que ya utilizaste.
4. No cambies `TFT_eSPI/User_Setup.h`, el cableado ni la rotación que ya te funcionaron.
5. Verificá primero (✓). Después de compilar, cargá el programa.

**Todos los archivos de la carpeta `CSR_BOT_FACE_V1` deben permanecer juntos.**
Arduino IDE compila los `.cpp` que están en esa misma carpeta.

## Responsabilidad de cada módulo

- `CSR_BOT_FACE_V1.ino`: archivo que se abre en Arduino IDE; incluye la configuración compartida.
- `config.h`: tipos, variables compartidas y prototipos de funciones.
- `main.cpp`: `setup()`, `loop()`, comandos del Monitor Serie y coordinación del sistema. No define `main()`; ese punto de entrada lo aporta Arduino.
- `display.cpp`: objeto TFT, paleta, geometría, inicialización y fondo.
- `face.cpp`: cabeza, cabello, ojos, cejas, nariz, boca y composición del rostro.
- `animations.cpp`: burbuja de estado y efectos visuales.
- `emotions.cpp`: nombres de los 100 estados, frases y perfiles faciales.
- `personality.cpp`: selección manual de personalidad por franja horaria.

## Controles del Monitor Serie

Velocidad: `115200` baudios. Final de línea: `Nueva línea`.

- `N`: siguiente estado.
- `P`: estado anterior.
- `B`: parpadeo manual (solo si el sprite está disponible).
- `S`: activar/desactivar habla simulada.
- `1`: personalidad de mañana.
- `2`: personalidad de tarde.
- `3`: personalidad de noche.
- `4`: personalidad de madrugada.

## Qué está implementado y qué falta

Esta versión es una **modularización conservadora** del firmware v1.3 que se adjuntó.
Se separó el código existente en archivos, evitando cambiar deliberadamente el diseño y la lógica
de dibujo durante esta tarea.

Ya existe:
- lista de 100 nombres de estados;
- asignación de perfiles faciales;
- dibujo básico de cara, cabello, ojos, cejas y boca;
- efectos visuales básicos por grupos de estados;
- sprite de 300 × 320 a 8 bits, con alternativa de dibujo directo si no hay memoria.

Todavía no existe:
- sincronización horaria automática por NTP;
- integración con un motor de IA;
- transferencia de voz real ni sincronización labial con audio;
- los 100 efectos faciales únicos del simulador Python;
- eliminación garantizada de todo tearing/flicker en el modo TFT directo.

Si el Monitor Serie informa `AVISO: no se pudo reservar el sprite de 96 KB`, el firmware está usando
el modo TFT directo. En ese modo puede haber parpadeo porque se vuelve a dibujar sobre la pantalla.
No se debe interpretar el mensaje de fallback como doble búfer activo.

## Nota de compilación

La estructura fue revisada con una comprobación sintáctica C++ usando stubs de Arduino/TFT_eSPI.
Eso verifica sintaxis y referencias entre módulos, pero **no equivale a compilar con tu versión real
del core ESP32 y tu biblioteca TFT_eSPI ni a probarlo en tu hardware**. La primera comprobación
real debe hacerse desde Arduino IDE antes de cargarlo.

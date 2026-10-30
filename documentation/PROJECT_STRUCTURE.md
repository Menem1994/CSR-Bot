# Arquitectura modular de CSR-BOT v1.4

## Flujo general

```text
Arduino IDE
   |
   v
CSR_BOT_FACE_V1.ino
   |
   v
main.cpp  ----> emotions.cpp
   |                 |
   |                 v
   +----------> personality.cpp
   |
   +----------> face.cpp ----> animations.cpp
   |                 |
   v                 v
display.cpp <---- config.h
```

`config.h` es el contrato común entre los módulos: define los tipos de perfiles y declara las
variables y funciones que otros archivos necesitan. Las variables se definen una sola vez en
su archivo `.cpp` correspondiente.

## Por qué los archivos `.cpp` ahora sí tienen contenido

Antes eran placeholders creados para planificar la futura separación. No contenían implementación.
En v1.4 se trasladó el código que estaba concentrado en `CSR_BOT_Face_ESP32_v1_3_RECOVERY.ino`
a los módulos actuales.

La división se hizo por responsabilidad, sin rediseñar todavía las expresiones:
- pantalla y coordenadas en `display.cpp`;
- geometría facial en `face.cpp`;
- efectos y burbuja en `animations.cpp`;
- catálogo de estados y perfiles en `emotions.cpp`;
- personalidad horaria manual en `personality.cpp`;
- coordinación y ciclo Arduino en `main.cpp`.

## Siguiente paso de desarrollo

1. Compilar esta versión modular en el entorno real del usuario.
2. Comparar la pantalla con la versión recovery original.
3. Confirmar si se asignó el sprite de 96 KB.
4. Una vez validado el comportamiento, optimizar el renderizado sin cambiar la geometría facial.
5. Después ampliar los efectos y las animaciones de cada estado.

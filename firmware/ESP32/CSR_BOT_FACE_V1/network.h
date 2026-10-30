#pragma once
#include <Arduino.h>

// Acceso de solo lectura al diálogo renderizado en la burbuja.
const char* getBubbleLine1();
const char* getBubbleLine2();
int getBubblePageIndex();
int getBubblePageCount();
bool isBubbleVisible();

void initializeNetwork();
void updateNetwork();

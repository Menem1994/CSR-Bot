# CSR-BOT v1.6 — diálogo paginado y boca sincronizada

## Objetivo

Mejorar la conversación entre el cerebro Python de la PC y el ESP32 sin alterar el diseño base del rostro ni la configuración física de la TFT.

La versión incorpora:
- Burbuja de diálogo de dos líneas.
- División automática de mensajes de hasta 1200 caracteres en páginas.
- Avance automático de páginas cada 4 segundos.
- El rostro y sus animaciones permanecen visibles.
- Tres modos de salida: pantalla, voz y mixto.
- Animación de boca activa mientras el sintetizador de voz de Windows reproduce el mensaje.
- La animación de boca se detiene cuando Windows termina de pronunciar el texto.
- En modo solo pantalla, la boca acompaña la presentación de las páginas y se detiene al terminar la última.
- Si selecciona solo voz, se oculta el globo de texto y queda el rostro.

## Importante: alcance real

En esta versión, el audio sale por los parlantes conectados a la PC usando Windows System.Speech. Todavía no es voz emitida por un altavoz conectado al robot. El micrófono, reconocimiento de voz y funcionamiento sin PC siguen siendo hitos posteriores.

El modo mixto sincroniza el inicio y el final de la animación de boca con la reproducción real del sintetizador. El cambio de páginas es temporizado (4 s por página), no está sincronizado palabra por palabra con el audio.


## Cierre automático del diálogo

- El globo permanece visible mientras se presenta la respuesta.
- En modo pantalla, la boca acompaña la lectura y se detiene al finalizar la última página.
- En modo voz/mixto, Python detiene la boca cuando Windows termina realmente el TTS.
- Una vez terminada la presentación (y la voz, si corresponde), el firmware espera 5 segundos y oculta únicamente el globo.
- El rostro y sus animaciones permanecen visibles.
- Un mensaje nuevo reinicia el ciclo de diálogo.

## Modos

| Modo | Pantalla | Boca | Audio |
|---|---|---|---|
| Pantalla | Respuesta paginada | Quietud | No |
| Voz | Rostro sin globo | Animada durante TTS | Parlantes de Windows |
| Mixto | Respuesta paginada + rostro | Animada durante TTS | Parlantes de Windows |

## Probar la paginación

En la interfaz web escribir:

`Quiero probar paginacion`

La respuesta demo deliberadamente extensa debe aparecer completa en el navegador y repartirse en varias páginas de dos líneas en la TFT.

## Probar sincronización de boca

1. En la interfaz, elegir `Mixto: voz + subtítulos`.
2. Enviar un mensaje.
3. La boca debe comenzar a moverse al iniciar el sintetizador de Windows.
4. La boca debe detenerse cuando Windows termine de hablar.
5. El rostro debe conservar el estado emocional elegido.

Si no se escucha la voz, comprobar que Windows tiene un dispositivo de salida seleccionado y que PowerShell puede cargar `System.Speech`.

## Archivos modificados

### Firmware ESP32
- `network.cpp`: buffer de páginas, endpoint `/api/speech`, parámetros `speaking` y `show_text`, estado de paginación.
- `network.h`: getters del texto y estado de páginas para el renderer.
- `animations.cpp`: burbuja de dos líneas y barra de progreso de páginas.
- `face.cpp`: la boca se anima únicamente cuando `speaking == true`.
- `main.cpp`: refresco más fluido mientras se habla.
- `CSR_BOT_FACE_V1.ino`: versión identificada como v1.6.

### Cerebro Python
- `bot_server.py`: selector de salida, envío de texto largo, TTS síncrono dentro de un worker y orden de detener la boca al finalizar.
- `test_bridge.py`: pruebas de parámetros de diálogo y endpoint de voz.
- `settings.example.json`: `windows_speech=true` establece el modo mixto como selección predeterminada; puede elegirse cualquier modo desde la interfaz.

## Configuración e instalación

Esta actualización se aplica sobre la carpeta original del proyecto CSR-BOT. No copiar ni reemplazar `.git`, credenciales, `wifi_config.h` ni `settings.json`.

1. Hacer una copia local de la carpeta actual.
2. Integrar las carpetas `firmware`, `software` y `documentation` de esta versión en el proyecto original.
3. Mantener `wifi_config.h` y `settings.json` existentes.
4. Abrir `firmware/ESP32/CSR_BOT_FACE_V1/CSR_BOT_FACE_V1.ino` en Arduino IDE.
5. Mantener el core ESP32 2.0.17, la placa ESP32 Dev Module y el `User_Setup.h` de TFT_eSPI que ya funcionan.
6. Compilar y cargar el firmware.
7. Iniciar Python desde la raíz del proyecto:

```powershell
py -m unittest discover -s software/brain -p "test_*.py"
py software/brain/bot_server.py
```

8. Abrir `http://127.0.0.1:8765`.

No publicar la actualización en GitHub hasta comprobarla físicamente y revisar `git status`.

## Pruebas realizadas antes de entregar

- 7 pruebas automáticas Python: correctas.
- `py_compile` de los módulos Python: correcto.
- Comprobación sintáctica C++ de los módulos con encabezados simulados: correcta.
- Prueba aislada del temporizador C++: diálogo de una página, varias páginas, modo pantalla y finalización de voz: correcta.

Estas comprobaciones no sustituyen la compilación en Arduino IDE con la instalación real de TFT_eSPI ni la prueba física en la placa.

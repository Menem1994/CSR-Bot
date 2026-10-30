> DOCUMENTO HISTÓRICO: esta guía corresponde a v1.5. Para diálogo paginado y boca sincronizada, consultar `CSR_BOT_v1_6_DIALOGO.md`.

# CSR-BOT v1.5 — Cerebro en Python + comunicación Wi-Fi

**Objetivo del hito:** comunicar PC ↔ ESP32, enviar estados de ánimo y texto, probar conversación escrita y (opcional) reproducir voz por los parlantes de la PC. **No requiere micrófono**.

Este paquete es una **actualización de tu proyecto v1.4**, no otro proyecto. La TFT, el cableado y la configuración `TFT_eSPI/User_Setup.h` no se tocan. No sobrescribir archivos de otro proyecto ni copiar carpetas `.git`.

## 0. Guardar la versión v1.4 que ya funciona

Abrí la terminal en la carpeta original `csr-bot`:

```powershell
git status
git add .
git commit -m "Backup: rostro funcional v1.4"
```

Si Git indica que no hay cambios, seguí normalmente. **Antes de ejecutar `git add .` verificá que NO tengas contraseñas, `wifi_config.h`, `settings.json` ni claves de API dentro de archivos que Git esté siguiendo**.

## 1. Integrar la actualización en tu carpeta EXISTENTE

Descomprimí `CSR_BOT_v1_5_INTEGRACION.zip` y **copiá su contenido** (`firmware`, `software` y `documentation`) **dentro de tu `csr-bot` original**, aceptando combinar carpetas y reemplazar los archivos de firmware con mismo nombre.

Los cambios reales son:

- `firmware/ESP32/CSR_BOT_FACE_V1/main.cpp`: llama al nuevo módulo de red.
- `firmware/ESP32/CSR_BOT_FACE_V1/animations.cpp`: texto del diálogo por Wi-Fi.
- `firmware/ESP32/CSR_BOT_FACE_V1/network.cpp`, `network.h`: servidor Wi-Fi en ESP32.
- `firmware/ESP32/CSR_BOT_FACE_V1/wifi_config.example.h`: modelo para credenciales.
- `software/brain/bot_server.py`: interfaz y motor conversacional PC.
- `software/brain/settings.example.json`: modelo de ajustes PC.
- `software/brain/test_bridge.py`: pruebas de protocolo sin el ESP32 conectado.

Todos los demás `.cpp/.h` siguen siendo los del proyecto funcional v1.4.

## 2. PROTEGER TUS CLAVES antes de crear archivos privados

Abrí tu `.gitignore` del proyecto `csr-bot` y agregá al final estas líneas **sin borrar las que ya existían**:

```gitignore
# CSR-BOT: credenciales privadas
**/wifi_config.h
**/software/brain/settings.json
.env
.env.*
__pycache__/
*.pyc
```

Luego ejecutá `git status` y verificá que NO aparezcan los archivos privados de los pasos siguientes.

## 3. Configurar red del ESP32

Dentro de `firmware/ESP32/CSR_BOT_FACE_V1/`, hacé una **copia** de `wifi_config.example.h` y llamala **`wifi_config.h`**. No cambies el nombre del `.example.h` original.

Abrí `wifi_config.h` en VS Code y configurá:

```cpp
#define CSR_WIFI_SSID "NOMBRE_DE_TU_WIFI"
#define CSR_WIFI_PASSWORD "CONTRASENA_DE_TU_WIFI"
#define CSR_DEVICE_TOKEN "una-clave-larga-inventada-por-vos-123"
```

**Requisitos:** tu red debe ser Wi-Fi 2,4 GHz (el ESP32-WROOM-32 no utiliza redes de 5 GHz); PC y ESP32 conectados a la misma red local. El token debe ser largo y propio, no el del ejemplo. Nunca lo subas a GitHub. No abras puertos del router para este servidor.

Abrí desde Arduino IDE:

`firmware/ESP32/CSR_BOT_FACE_V1/CSR_BOT_FACE_V1.ino`

Seleccioná `ESP32 Dev Module`, tu puerto COM y el **core Espressif 2.0.17** que te funcionó con TFT_eSPI. No cambies `User_Setup.h`. Compilá y cargá.

En el Monitor Serie a **115200** buscá:

```text
CSR-BOT IP: 192.168.1.123
```

Ese es solo un **ejemplo** de IP; copiá la IP REAL de tu monitor. La cara debe seguir visible. Si aparece "Wi-Fi no conectado", revisá SSID, contraseña, señal y banda 2,4 GHz; el firmware facial sigue funcionando sin red.

## 4. Configurar la PC

Dentro de `software/brain`, copiá `settings.example.json` a **`settings.json`** y completá:

```json
{
  "esp32_ip": "LA_IP_REAL_DEL_ESP32",
  "device_token": "EL_MISMO_TOKEN_DEL_PASO_3",
  "ai_enabled": false,
  "openai_model": "gpt-4.1-mini",
  "windows_speech": false
}
```

Los valores `esp32_ip` y `device_token` deben coincidir con la configuración del ESP32.

Abrí PowerShell dentro de la carpeta original del proyecto y ejecutá:

```powershell
py --version
py -m unittest discover -s software/brain -p "test_*.py"
py software/brain/bot_server.py
```

Requiere Python **3.10 o posterior**. Este paso **no requiere instalar librerías con pip**.

Abrí en la PC: **http://127.0.0.1:8765**

## 5. Pruebas

1. Presioná **Probar conexión**: debería mostrar la IP, estado y si hay sprite.
2. En "Probar expresión manual" elegí **HAPPY** y presioná "Cambiar expresión": el rostro debe reaccionar.
3. Escribí **Hola CSR-BOT**: la PC devuelve una **respuesta fija de demostración**; la TFT muestra las primeras palabras del texto, y se elige GREETING.
4. Escribí **Estoy triste**: comprobá que cambia a EMPATHETIC.
5. Activá `"windows_speech": true` en settings.json y reiniciá Python para probar voz por los **parlantes de Windows**, sin micrófono.

**Importante:** el texto de la pantalla está limitado al espacio disponible del globo y usa caracteres ASCII sin emojis. La respuesta completa se muestra en el navegador. Si usás el modo TFT directo (sin sprite) podrá haber un redibujado visible al cambiar expresiones; no se ha cambiado el motor gráfico de v1.4.

## 6. Activar inteligencia artificial REAL (opcional, con facturación de API)

Primero completá las pruebas anteriores. Si decidís conectar una API:

1. Creá una API key desde el panel de la plataforma del proveedor (distinta de la suscripción ChatGPT Plus).
2. En `settings.json`, cambiá `"ai_enabled": true`.
3. En la **misma sesión de PowerShell** donde ejecutes Python, configurá la variable de entorno `OPENAI_API_KEY` mediante el método seguro que uses para tus secretos; no la escribas en archivos del repositorio ni la compartas en capturas.
4. Ejecutá nuevamente `py software/brain/bot_server.py`.

La app usa el endpoint **Responses** para obtener una respuesta y un `mood` de entre los 100 estados. Las solicitudes de chat en modo IA se envían a la API externa. El historial se conserva **solo en RAM durante la sesión** (hasta 20 mensajes recientes), y `store: false` se envía en la solicitud; la retención del proveedor depende también de sus políticas.

Si recibís un error de modelo o acceso, configurá `openai_model` con uno habilitado en tu cuenta. La app **no oculta los errores cambiando silenciosamente a modo demo**.

## 7. Cómo subir la nueva versión a GitHub

Desde tu carpeta ORIGINAL `csr-bot`:

```powershell
git status
git check-ignore firmware/ESP32/CSR_BOT_FACE_V1/wifi_config.h
git check-ignore software/brain/settings.json
git add firmware software/brain documentation .gitignore
git status
git commit -m "Add Wi-Fi bridge and Python brain v1.5"
git push origin main
```

Los `git check-ignore` deben devolver la ruta de cada archivo privado. Si no muestran nada, **detenete y corregí `.gitignore` antes de hacer el commit**. Si ya había credenciales versionadas, ignorarlas después NO las quita del historial; habría que rotarlas y retirarlas de Git.

## Próximo hito

- Comunicación bidireccional WebSocket o eventos en tiempo real.
- Micrófono cuando tengamos hardware adecuado; TTS y STT en la PC mientras tanto.
- Memoria persistente (SQLite) con consentimiento y controles.
- Sensor HC-SR04/PIR y telemetría al cerebro.
- Interfaz para el celular con autenticación; en v1.5 la página se expone **solo en esta PC** por seguridad.

**Si falla algo:** mandá el error completo del Arduino IDE o el texto del Monitor Serie (sin contraseñas). La v1.4 sigue recuperable mediante Git.

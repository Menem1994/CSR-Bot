"""CSR-BOT v1.5: cerebro PC (Python) -> ESP32 (Wi-Fi HTTP).

Sin dependencias externas. En modo DEMO prueba comunicaciones, no simula una IA real.
Opcional: API OpenAI Responses, si se habilita conscientemente en settings.json.
Interfaz web LOCAL por defecto: http://127.0.0.1:8765
"""
from __future__ import annotations

import base64
import json
import os
import pathlib
import re
import subprocess
import sys
import threading
import unicodedata
from dataclasses import dataclass
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib import error as urlerror
from urllib import parse, request

ROOT = pathlib.Path(__file__).resolve().parent
CONFIG = ROOT / "settings.json"
PORT = 8765
HISTORY: list[dict[str, str]] = []
LOCK = threading.RLock()
SPEECH_BUSY = threading.Event()

# Refleja exactamente los nombres y el orden de emotions.cpp.
STATE_NAMES = (
    "HAPPY EXCITED JOYFUL AMUSED PROUD PLAYFUL LOVING GRATEFUL RELIEVED CONTENT "
    "SAD DISAPPOINTED FRUSTRATED ANGRY ANNOYED WORRIED AFRAID LONELY EMBARRASSED GUILTY "
    "SURPRISED SHOCKED CURIOUS INTRIGUED AMAZED CONFUSED PUZZLED SKEPTICAL SUSPICIOUS AWESTRUCK "
    "THINKING DEEP_THINKING ANALYZING CALCULATING LEARNING REMEMBERING RECALLING DECIDING SOLVING CREATIVE "
    "LISTENING SPEAKING GREETING FAREWELL ATTENTIVE EMPATHETIC ENCOURAGING CELEBRATING TEASING WAITING_FOR_REPLY "
    "SCANNING DETECTING TRACKING LOOKING_LEFT LOOKING_RIGHT LOOKING_UP LOOKING_DOWN FOCUSED ALERT STARTLED "
    "IDLE SLEEPY SLEEPING WAKING_UP ENERGIZED RELAXED BORED RESTLESS DROWSY RECHARGING "
    "BOOTING ONLINE OFFLINE CONNECTING PROCESSING LOADING ERROR RECOVERING UPDATING LOW_BATTERY "
    "MISCHIEVOUS SARCASTIC DRAMATIC SHY CONFIDENT MYSTERIOUS GOOFY CURIOUS_PET GLITCHY SHOWTIME "
    "LOVE_STRUCK MIND_BLOWN EUREKA MEDITATING DREAMING NOSTALGIC DETERMINED OVERWHELMED MIRRORING SIGNATURE"
).split()
assert len(STATE_NAMES) == 100


@dataclass(frozen=True)
class Settings:
    esp32_ip: str
    device_token: str
    ai_enabled: bool
    openai_model: str
    windows_speech: bool


def read_settings() -> Settings:
    if not CONFIG.is_file():
        raise RuntimeError(
            "Falta settings.json. Copia settings.example.json y edita IP y token."
        )
    data = json.loads(CONFIG.read_text(encoding="utf-8"))
    ip = str(data.get("esp32_ip", "")).strip()
    token = str(data.get("device_token", "")).strip()
    if not re.fullmatch(r"\d{1,3}(?:\.\d{1,3}){3}", ip):
        raise RuntimeError("esp32_ip debe ser la dirección IPv4 que muestra el ESP32.")
    if not token or token.startswith("REEMPLAZAR_"):
        raise RuntimeError("Cambiá device_token en settings.json por tu token del ESP32.")
    return Settings(
        esp32_ip=ip, device_token=token,
        ai_enabled=bool(data.get("ai_enabled", False)),
        openai_model=str(data.get("openai_model", "gpt-4.1-mini")),
        windows_speech=bool(data.get("windows_speech", False)),
    )


def request_to_device(settings: Settings, endpoint: str, data: dict[str, str] | None = None) -> dict:
    url = f"http://{settings.esp32_ip}/{endpoint}"
    if data is None:
        req = request.Request(url, headers={"X-CSR-Token": settings.device_token}, method="GET")
    else:
        body = parse.urlencode(data).encode("utf-8")
        req = request.Request(
            url, data=body,
            headers={
                "X-CSR-Token": settings.device_token,
                "Content-Type": "application/x-www-form-urlencoded",
            }, method="POST",
        )
    try:
        with request.urlopen(req, timeout=4) as response:
            return json.loads(response.read().decode("utf-8"))
    except urlerror.HTTPError as exc:
        explanation = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"ESP32 HTTP {exc.code}: {explanation}") from exc
    except (OSError, ValueError) as exc:
        raise RuntimeError(f"No pude comunicarme con ESP32 en {settings.esp32_ip}: {exc}") from exc


def tft_ascii(text: str, limit: int = 1200) -> str:
    """La tipografía integrada de TFT_eSPI no interpreta emojis ni acentos UTF-8."""
    normalized = unicodedata.normalize("NFKD", text)
    normalized = normalized.replace("¿", "").replace("¡", "")
    ascii_text = normalized.encode("ascii", errors="ignore").decode("ascii")
    return " ".join(ascii_text.split())[:limit]


def send_face(
    settings: Settings,
    state: str,
    message: str,
    *,
    speaking: bool = False,
    show_text: bool = True,
    auto_stop: bool = False,
) -> dict:
    state = state.upper().strip()
    if state not in STATE_NAMES:
        raise ValueError(f"Estado desconocido: {state}")
    return request_to_device(settings, "api/command", {
        "state": state,
        "text": tft_ascii(message),
        "speaking": "1" if speaking else "0",
        "show_text": "1" if show_text else "0",
        "auto_stop": "1" if auto_stop else "0",
    })


def set_device_speaking(settings: Settings, active: bool) -> dict:
    return request_to_device(settings, "api/speech", {
        "speaking": "1" if active else "0",
    })


def speech_worker(settings: Settings, text: str) -> None:
    """Habla en Windows y apaga la animación de boca al terminar realmente el TTS."""
    try:
        speak_windows(text)
    except Exception as exc:
        print(f"[TTS] No se pudo completar la voz: {exc}")
    finally:
        try:
            set_device_speaking(settings, False)
        except Exception as exc:
            print(f"[TTS] No pude detener la animación de boca: {exc}")
        SPEECH_BUSY.clear()


def demo_answer(message: str) -> tuple[str, str]:
    """Reglas simples para probar. Esto NO utiliza un modelo de IA."""
    lower = message.lower()
    if any(word in lower for word in ("hola", "buenas", "saludos")):
        return "Hola! Recibí tu saludo desde la PC. Ya podemos comunicarnos.", "GREETING"
    if any(word in lower for word in ("triste", "mal dia", "cansado")):
        return "Te escucho. Por ahora estoy en modo demostracion, pero puedo cambiar mi expresion.", "EMPATHETIC"
    if any(word in lower for word in ("chiste", "gracioso", "jaja")):
        return "Ja ja! Todavia no tengo un generador de chistes, pero mi sonrisa si funciona.", "AMUSED"
    if "sensor" in lower:
        return "Los sensores vendran luego: primero probamos bien la comunicacion.", "CURIOUS"
    if any(term in lower for term in ("respuesta larga", "probar paginas", "paginacion", "texto largo")):
        return (
            "Esta es una respuesta extensa de prueba para comprobar el nuevo sistema de dialogo de CSR-BOT. "
            "El texto se divide automaticamente en paginas de dos lineas para que puedas leerlo sin perder de vista el rostro. "
            "Mientras avanzan las paginas, el avatar conserva su expresion y sus animaciones. "
            "Cuando el robot utilice voz, la boca se movera durante la reproduccion y se detendra exactamente cuando termine el sintetizador. "
            "Este modo de prueba todavia no utiliza inteligencia artificial real, pero verifica el transporte del mensaje completo desde Python hasta la pantalla. "
            "Mas adelante conectaremos el microfono, el altavoz propio, el reconocimiento de voz y la memoria conversacional.",
            "EXCITED",
        )
    return "Mensaje recibido. Esta es una respuesta de prueba, no una respuesta de inteligencia artificial.", "ATTENTIVE"


def extract_text_from_response(data: dict) -> str:
    fragments = []
    for item in data.get("output", []):
        if item.get("type") != "message":
            continue
        for content in item.get("content", []):
            if content.get("type") == "output_text":
                fragments.append(content.get("text", ""))
    return "\n".join(fragments).strip()


def ai_answer(settings: Settings, message: str) -> tuple[str, str]:
    key = os.environ.get("OPENAI_API_KEY", "").strip()
    if not key:
        raise RuntimeError("ai_enabled=true pero falta OPENAI_API_KEY en el entorno de la PC.")

    instructions = (
        "Sos CSR-BOT, un asistente robótico de escritorio con un rostro expresivo. "
        "Hablas español rioplatense, con personalidad curiosa, amistosa y algo irónica cuando corresponde. "
        "Respondé al usuario de manera útil, breve y natural, sin afirmar que sentís emociones humanas. "
        "Respondé ÚNICAMENTE un objeto JSON válido, sin markdown, con dos claves: "
        "reply (respuesta en español, hasta 8 oraciones y máximo 1200 caracteres) y mood (una cadena). "
        "El mood debe ser uno de: HAPPY, AMUSED, CURIOUS, CONFUSED, THINKING, ANALYZING, "
        "LISTENING, SPEAKING, SAD, EMPATHETIC, PLAYFUL, SARCASTIC, EXCITED, WORRIED, "
        "SURPRISED, GREETING, IDLE, ALERT, PROUD, RELAXED. "
        "Escogé el mood por el contenido y tono de la conversación."
    )
    with LOCK:
        # Historial RAM, últimas 10 intervenciones; no datos persistentes.
        recent = HISTORY[-10:].copy()
    payload = {
        "model": settings.openai_model,
        "instructions": instructions,
        "input": recent + [{"role": "user", "content": message}],
        "max_output_tokens": 650,
        "store": False,
    }
    req = request.Request(
        "https://api.openai.com/v1/responses",
        data=json.dumps(payload).encode("utf-8"),
        headers={
            "Authorization": f"Bearer {key}",
            "Content-Type": "application/json",
        }, method="POST",
    )
    try:
        with request.urlopen(req, timeout=45) as response:
            data = json.loads(response.read().decode("utf-8"))
    except urlerror.HTTPError as exc:
        # No exponer API key ni cuerpo entero de error.
        detail = exc.read(1000).decode("utf-8", errors="replace")
        raise RuntimeError(f"API error HTTP {exc.code}: {detail}") from exc
    except (OSError, ValueError) as exc:
        raise RuntimeError(f"Error consultando la API: {exc}") from exc

    raw = extract_text_from_response(data)
    if not raw:
        raise RuntimeError("La API no devolvió texto en la respuesta.")
    # Tolerancia a wrappers ```json ...``` de algunos modelos.
    raw = re.sub(r"^```(?:json)?\s*|\s*```$", "", raw.strip(), flags=re.IGNORECASE)
    try:
        item = json.loads(raw)
    except json.JSONDecodeError as exc:
        raise RuntimeError("La IA respondió, pero no devolvió el JSON de expresión esperado.") from exc
    if not isinstance(item, dict) or not isinstance(item.get("reply"), str):
        raise RuntimeError("Respuesta JSON de IA inválida: falta 'reply'.")
    answer = item["reply"].strip()[:1200]
    mood = str(item.get("mood", "CURIOUS")).upper().strip()
    if mood not in STATE_NAMES:
        mood = "CURIOUS"
    if not answer:
        raise RuntimeError("La IA devolvió una respuesta vacía.")
    with LOCK:
        HISTORY.extend([
            {"role": "user", "content": message},
            {"role": "assistant", "content": answer},
        ])
        if len(HISTORY) > 20:
            del HISTORY[:-20]
    return answer, mood


def speak_windows(text: str) -> None:
    """TTS síncrono: retorna únicamente cuando Windows termina de pronunciar el texto."""
    if sys.platform != "win32":
        raise RuntimeError("La salida de voz de esta versión requiere Windows.")
    safe_text = text[:1200]
    encoded_text = base64.b64encode(safe_text.encode("utf-8")).decode("ascii")
    ps = (
        "Add-Type -AssemblyName System.Speech; "
        f"$b=[Convert]::FromBase64String('{encoded_text}'); "
        "$s=[Text.Encoding]::UTF8.GetString($b); "
        "$v=New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$v.Rate=0; $v.Volume=100; "
        "$v.Speak($s); "
        "$v.Dispose();"
    )
    encoded_ps = base64.b64encode(ps.encode("utf-16le")).decode("ascii")
    try:
        subprocess.run(
            ["powershell.exe", "-NoProfile", "-NonInteractive", "-EncodedCommand", encoded_ps],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.PIPE,
            check=True,
            timeout=180,
            creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0),
        )
    except subprocess.TimeoutExpired as exc:
        raise RuntimeError("La reproducción de voz superó los 180 segundos.") from exc
    except subprocess.CalledProcessError as exc:
        detail = (exc.stderr or b"").decode("utf-8", errors="replace")[-500:]
        raise RuntimeError(f"Error del sintetizador de Windows: {detail}") from exc
    except OSError as exc:
        raise RuntimeError(f"No se pudo iniciar PowerShell para TTS: {exc}") from exc


PAGE = r'''<!doctype html>
<html lang="es"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>CSR-BOT | Brain Bridge</title>
<style>
:root{color-scheme:dark;font:16px system-ui,Segoe UI,sans-serif;background:#10131f;color:#e9e9f5}
*{box-sizing:border-box}body{margin:0;min-height:100vh;background:radial-gradient(circle at 20% 0%,#252741,#10131f 65%)}
main{max-width:810px;margin:auto;padding:24px 18px 48px}header{display:flex;align-items:center;justify-content:space-between;gap:16px;margin-bottom:22px}
h1{font-size:27px;margin:0;color:#8be9f0}small, .sub{color:#9da7c4}.panel{background:#171c2b;border:1px solid #373c56;border-radius:18px;padding:18px;margin:14px 0}
button,input,select,textarea{font:inherit;border-radius:11px}button{cursor:pointer;background:#8be9f0;border:0;color:#10131f;padding:11px 14px;font-weight:700}
button:disabled{opacity:.5;cursor:not-allowed}button.secondary{background:#303b56;color:#e9e9f5}
input,select,textarea{background:#0e1220;border:1px solid #434b64;padding:12px;color:#fff;width:100%}textarea{min-height:84px;resize:vertical}
.status{display:flex;gap:10px;align-items:center;color:#a8f0d1}.dot{width:9px;height:9px;background:#a8f0d1;border-radius:50%}
.row{display:flex;gap:9px;flex-wrap:wrap;align-items:center}.row>*{min-width:0}.grid{display:grid;grid-template-columns:1fr auto;gap:10px}
.chat{height:340px;overflow-y:auto;display:flex;flex-direction:column;gap:12px;padding:10px 2px}
.msg{max-width:84%;padding:11px 15px;border-radius:14px;white-space:pre-wrap;line-height:1.4}
.me{align-self:flex-end;background:#2c4152}.bot{align-self:flex-start;background:#322944;border:1px solid #6d5679}
.error{color:#ff9daa}.label{font-size:12px;color:#9da7c4;letter-spacing:.1em;text-transform:uppercase;margin-bottom:7px}
@media(max-width:560px){.grid{grid-template-columns:1fr}.msg{max-width:96%}h1{font-size:22px}}
</style></head><body><main>
<header><div><h1>◉ CSR-BOT</h1><div class="sub">Brain Bridge · ESP32 + Python · v1.6</div></div><button class="secondary" onclick="ping()">Probar conexión</button></header>
<section class="panel"><div class="label">Conexión con el cuerpo</div><div id="status" class="sub">Comprobando...</div></section>
<section class="panel"><div class="label">Conversación <span id="mode"></span></div><div class="chat" id="chat" aria-live="polite"></div>
<form id="chatForm"><textarea id="question" placeholder="Escribile algo a CSR-BOT..." required maxlength="700"></textarea>
<div class="row" style="margin-top:10px"><label for="outputMode" class="sub">Salida del robot</label>
<select id="outputMode" aria-label="Modo de salida">
<option value="screen">Pantalla: texto paginado</option>
<option value="voice">Voz: parlantes de la PC</option>
<option value="mixed">Mixto: voz + subtítulos</option>
</select></div>
<div class="row" style="justify-content:space-between;margin-top:10px"><small>La voz de esta versión sale por la PC; el firmware anima la boca mientras se reproduce.</small><button id="send">Enviar a CSR-BOT</button></div></form></section>
<section class="panel"><div class="label">Probar expresión manual</div><div class="grid"><select id="mood"></select><button onclick="manual()">Cambiar expresión</button></div></section>
<div id="info" class="sub" role="status">El rostro y los 100 estados siguen funcionando aunque no haya API.</div>
</main><script>
const $=x=>document.getElementById(x);const CHAT=$('chat');let busy=false;
function addMsg(s,bot){const e=document.createElement('div');e.className='msg '+(bot?'bot':'me');e.textContent=s;CHAT.appendChild(e);CHAT.scrollTop=CHAT.scrollHeight}
async function api(route,data){const o={method:data?'POST':'GET',headers:{'Content-Type':'application/json'}};if(data)o.body=JSON.stringify(data);const r=await fetch(route,o);const x=await r.json();if(!r.ok)throw Error(x.error||'Error desconocido');return x}
async function ping(){try{const r=await api('/api/status');$('status').textContent=`ESP32 conectado · IP ${r.ip} · Estado ${r.state} · Sprite ${r.sprite?'activo':'sin memoria'}`;$('mode').textContent=r.ai_enabled?'· IA ACTIVADA':'· DEMOSTRACIÓN';}catch(e){$('status').textContent='Sin conexión: '+e.message}}
async function manual(){try{const x=await api('/api/manual',{state:$('mood').value,text:''});$('info').textContent='Expresión aplicada: '+x.state;await ping()}catch(e){$('info').textContent='Error: '+e.message}}
$('chatForm').addEventListener('submit',async e=>{e.preventDefault();if(busy)return;const text=$('question').value.trim();if(!text)return;busy=true;$('send').disabled=true;addMsg(text,false);$('question').value='';try{const outputMode=$('outputMode').value;const r=await api('/api/chat',{message:text,output_mode:outputMode});addMsg(r.reply,true);$('info').textContent=`Expresión: ${r.mood} · ${r.mode==='demo'?'Modo demo (sin IA)':'Respuesta IA'} · Salida: ${r.output_mode}`;await ping()}catch(e){addMsg('Error: '+e.message,true)}finally{busy=false;$('send').disabled=false}});
(async()=>{const r=await api('/api/meta');for(const n of r.states){let o=document.createElement('option');o.value=n;o.textContent=n;$('mood').appendChild(o)}$('mood').value='HAPPY';$('outputMode').value=r.windows_speech_default?'mixed':'screen';$('mode').textContent=r.ai_enabled?'· IA ACTIVADA':'· DEMOSTRACIÓN';await ping()})().catch(e=>$('status').textContent=e.message);
</script></body></html>'''


class Handler(BaseHTTPRequestHandler):
    settings: Settings

    def write_json(self, code: int, obj: dict) -> None:
        payload = json.dumps(obj, ensure_ascii=False).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(payload)))
        self.send_header("Cache-Control", "no-store")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(payload)

    def do_GET(self) -> None:
        if self.path == "/":
            payload = PAGE.encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Length", str(len(payload)))
            self.end_headers()
            self.wfile.write(payload)
        elif self.path == "/api/meta":
            self.write_json(200, {"states": STATE_NAMES, "ai_enabled": self.settings.ai_enabled, "windows_speech_default": self.settings.windows_speech})
        elif self.path == "/api/status":
            try:
                status = request_to_device(self.settings, "api/health")
                status.update({"ip": self.settings.esp32_ip, "ai_enabled": self.settings.ai_enabled})
                self.write_json(200, status)
            except RuntimeError as exc:
                self.write_json(503, {"error": str(exc)})
        else:
            self.write_json(404, {"error": "Ruta desconocida"})

    def do_POST(self) -> None:
        if self.path not in ("/api/chat", "/api/manual"):
            return self.write_json(404, {"error": "Ruta desconocida"})
        try:
            size = int(self.headers.get("Content-Length", "0"))
            if not 0 < size <= 4096:
                return self.write_json(400, {"error": "Tamaño del mensaje inválido"})
            obj = json.loads(self.rfile.read(size).decode("utf-8"))
            if not isinstance(obj, dict):
                raise ValueError("El cuerpo debe ser JSON objeto")

            if self.path == "/api/manual":
                state = str(obj.get("state", "")).strip().upper()
                send_face(self.settings, state, "", speaking=False, show_text=True)
                return self.write_json(200, {"state": state, "ok": True})

            message = str(obj.get("message", "")).strip()
            if not 0 < len(message) <= 700:
                return self.write_json(400, {"error": "Escribí entre 1 y 700 caracteres"})

            output_mode = str(obj.get("output_mode", "mixed" if self.settings.windows_speech else "screen")).strip().lower()
            if output_mode not in ("screen", "voice", "mixed"):
                return self.write_json(400, {"error": "Modo de salida inválido"})

            voice_enabled = output_mode in ("voice", "mixed")
            show_text = output_mode in ("screen", "mixed")
            mouth_enabled = True
            auto_stop = output_mode == "screen"
            if voice_enabled:
                if sys.platform != "win32":
                    return self.write_json(400, {"error": "La voz de prueba está disponible en Windows."})
                if SPEECH_BUSY.is_set():
                    return self.write_json(409, {"error": "CSR-BOT todavía está hablando. Esperá a que termine."})

            if self.settings.ai_enabled:
                reply, mood = ai_answer(self.settings, message)
                mode = "ai"
            else:
                reply, mood = demo_answer(message)
                mode = "demo"

            # En voz/mixto, la boca arranca antes de lanzar el sintetizador.
            if voice_enabled:
                SPEECH_BUSY.set()
            try:
                send_face(
                    self.settings, mood, reply,
                    speaking=mouth_enabled,
                    show_text=show_text,
                    auto_stop=auto_stop,
                )
            except Exception:
                if voice_enabled:
                    SPEECH_BUSY.clear()
                raise

            if voice_enabled:
                try:
                    threading.Thread(
                        target=speech_worker,
                        args=(self.settings, reply),
                        daemon=True,
                        name="csrbot-tts",
                    ).start()
                except RuntimeError:
                    SPEECH_BUSY.clear()
                    try:
                        set_device_speaking(self.settings, False)
                    except RuntimeError:
                        pass
                    raise

            return self.write_json(200, {
                "reply": reply,
                "mood": mood,
                "mode": mode,
                "output_mode": output_mode,
                "speech_started": voice_enabled,
                "mouth_animation": mouth_enabled,
            })
        except (ValueError, RuntimeError, json.JSONDecodeError) as exc:
            return self.write_json(400, {"error": str(exc)})



def main() -> None:
    try:
        settings = read_settings()
    except (RuntimeError, OSError, ValueError) as exc:
        print("ERROR de configuración:", exc)
        raise SystemExit(1) from exc
    Handler.settings = settings
    host = "127.0.0.1"  # Por seguridad: solo navegador en esta misma PC.
    print("=" * 53)
    print(" CSR-BOT v1.6 - Dialogo y voz sincronizada")
    print(f" Web: http://{host}:{PORT}")
    print(f" ESP32: {settings.esp32_ip}")
    print(" IA:", "Activada" if settings.ai_enabled else "DEMO (sin IA real)")
    print(" Salida: pantalla paginada / voz Windows / mixto")
    print("=" * 53)
    print("Ctrl+C para salir.")
    try:
        ThreadingHTTPServer((host, PORT), Handler).serve_forever()
    except KeyboardInterrupt:
        print("\nCerrando CSR-BOT.")


if __name__ == "__main__":
    main()

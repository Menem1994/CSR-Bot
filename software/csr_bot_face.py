"""
CSR-BOT — simulador de rostro v4
Python 3 + Tkinter (sin dependencias externas).

Controles:
  1-4  Cambiar personalidad horaria manualmente
  A    Activar/desactivar personalidad automática según la hora
  ESPACIO  Activar/desactivar animación de habla
  B    Parpadeo
  N / P  Siguiente / anterior estado de los 100
  R    Volver al estado predeterminado
  ESC  Salir

Este simulador dibuja una interpretación original inspirada en las preferencias
visuales del proyecto. No necesita conexión con el ESP32 ni con la Pico todavía.
"""

import tkinter as tk
import random
import math
import time
from datetime import datetime

# Paleta pastel/neón suave
BG = "#10131f"
PANEL = "#171c2b"
PANEL_2 = "#20263a"
TEXT = "#e9e9f5"
MUTED = "#9da7c4"
CYAN = "#8be9f0"
PINK = "#f4a9d8"
LILAC = "#c6a0f6"
PEACH = "#ffbea8"
CREAM = "#ffe2b8"
RED = "#ff8eaa"
GREEN = "#a8f0d1"

STATES = [
    # 01-20 emociones
    ("HAPPY", "Feliz", "emociones"), ("EXCITED", "Emocionado", "emociones"),
    ("JOYFUL", "Alegre", "emociones"), ("AMUSED", "Divertido", "emociones"),
    ("PROUD", "Orgulloso", "emociones"), ("PLAYFUL", "Juguetón", "emociones"),
    ("LOVING", "Cariñoso", "emociones"), ("GRATEFUL", "Agradecido", "emociones"),
    ("RELIEVED", "Aliviado", "emociones"), ("CONTENT", "Conforme", "emociones"),
    ("SAD", "Triste", "emociones"), ("DISAPPOINTED", "Decepcionado", "emociones"),
    ("FRUSTRATED", "Frustrado", "emociones"), ("ANGRY", "Enojado", "emociones"),
    ("ANNOYED", "Molesto", "emociones"), ("WORRIED", "Preocupado", "emociones"),
    ("AFRAID", "Asustado", "emociones"), ("LONELY", "Solitario", "emociones"),
    ("EMBARRASSED", "Avergonzado", "emociones"), ("GUILTY", "Culpable", "emociones"),
    # 21-30 sorpresa/curiosidad
    ("SURPRISED", "Sorprendido", "curiosidad"), ("SHOCKED", "Impactado", "curiosidad"),
    ("CURIOUS", "Curioso", "curiosidad"), ("INTRIGUED", "Intrigado", "curiosidad"),
    ("AMAZED", "Asombrado", "curiosidad"), ("CONFUSED", "Confundido", "curiosidad"),
    ("PUZZLED", "Perplejo", "curiosidad"), ("SKEPTICAL", "Escéptico", "curiosidad"),
    ("SUSPICIOUS", "Desconfiado", "curiosidad"), ("AWESTRUCK", "Maravillado", "curiosidad"),
    # 31-40 cognitivos
    ("THINKING", "Pensando", "cognitivo"), ("DEEP_THINKING", "Pensando a fondo", "cognitivo"),
    ("ANALYZING", "Analizando", "cognitivo"), ("CALCULATING", "Calculando", "cognitivo"),
    ("LEARNING", "Aprendiendo", "cognitivo"), ("REMEMBERING", "Recordando", "cognitivo"),
    ("RECALLING", "Recuperando memoria", "cognitivo"), ("DECIDING", "Decidiendo", "cognitivo"),
    ("SOLVING", "Resolviendo", "cognitivo"), ("CREATIVE", "Creativo", "cognitivo"),
    # 41-50 sociales
    ("LISTENING", "Escuchando", "social"), ("SPEAKING", "Hablando", "social"),
    ("GREETING", "Saludando", "social"), ("FAREWELL", "Despidiéndose", "social"),
    ("ATTENTIVE", "Atento", "social"), ("EMPATHETIC", "Comprensivo", "social"),
    ("ENCOURAGING", "Alentador", "social"), ("CELEBRATING", "Celebrando", "social"),
    ("TEASING", "Bromista", "social"), ("WAITING_FOR_REPLY", "Esperando respuesta", "social"),
    # 51-60 percepción
    ("SCANNING", "Explorando", "percepcion"), ("DETECTING", "Detectando", "percepcion"),
    ("TRACKING", "Siguiendo objetivo", "percepcion"), ("LOOKING_LEFT", "Mirando izquierda", "percepcion"),
    ("LOOKING_RIGHT", "Mirando derecha", "percepcion"), ("LOOKING_UP", "Mirando arriba", "percepcion"),
    ("LOOKING_DOWN", "Mirando abajo", "percepcion"), ("FOCUSED", "Concentrado", "percepcion"),
    ("ALERT", "En alerta", "percepcion"), ("STARTLED", "Sobresaltado", "percepcion"),
    # 61-70 actividad
    ("IDLE", "En reposo", "actividad"), ("SLEEPY", "Somnoliento", "actividad"),
    ("SLEEPING", "Dormido", "actividad"), ("WAKING_UP", "Despertando", "actividad"),
    ("ENERGIZED", "Con energía", "actividad"), ("RELAXED", "Relajado", "actividad"),
    ("BORED", "Aburrido", "actividad"), ("RESTLESS", "Inquieto", "actividad"),
    ("DROWSY", "Adormilado", "actividad"), ("RECHARGING", "Recargando", "actividad"),
    # 71-80 sistema
    ("BOOTING", "Iniciando", "sistema"), ("ONLINE", "Conectado", "sistema"),
    ("OFFLINE", "Sin conexión", "sistema"), ("CONNECTING", "Conectando", "sistema"),
    ("PROCESSING", "Procesando", "sistema"), ("LOADING", "Cargando", "sistema"),
    ("ERROR", "Error", "sistema"), ("RECOVERING", "Recuperándose", "sistema"),
    ("UPDATING", "Actualizando", "sistema"), ("LOW_BATTERY", "Batería baja", "sistema"),
    # 81-90 personalidad
    ("MISCHIEVOUS", "Travieso", "personalidad"), ("SARCASTIC", "Sarcástico", "personalidad"),
    ("DRAMATIC", "Dramático", "personalidad"), ("SHY", "Tímido", "personalidad"),
    ("CONFIDENT", "Seguro", "personalidad"), ("MYSTERIOUS", "Enigmático", "personalidad"),
    ("GOOFY", "Payaso", "personalidad"), ("CURIOUS_PET", "Mascota curiosa", "personalidad"),
    ("GLITCHY", "Glitch artístico", "personalidad"), ("SHOWTIME", "Modo espectáculo", "personalidad"),
    # 91-100 especiales
    ("LOVE_STRUCK", "Deslumbrado", "especial"), ("MIND_BLOWN", "Alucinado", "especial"),
    ("EUREKA", "¡Eureka!", "especial"), ("MEDITATING", "Meditando", "especial"),
    ("DREAMING", "Soñando", "especial"), ("NOSTALGIC", "Nostálgico", "especial"),
    ("DETERMINED", "Determinado", "especial"), ("OVERWHELMED", "Saturado", "especial"),
    ("MIRRORING", "Imitando expresión", "especial"), ("SIGNATURE", "Firma CSR-BOT", "especial"),
]

# Estado -> rasgos base. Los estados no enumerados explícitamente se enriquecen
# mediante su categoría para que los 100 produzcan variaciones reconocibles.
OVERRIDES = {
    "HAPPY": ("smile", 0, 0, CYAN), "EXCITED": ("open", 0, -3, PINK),
    "JOYFUL": ("smile", 0, 0, CREAM), "AMUSED": ("smirk", 1, 0, PINK),
    "PROUD": ("smile", 0, -1, LILAC), "PLAYFUL": ("smirk", 1, 0, CYAN),
    "SAD": ("sad", 0, 3, LILAC), "DISAPPOINTED": ("sad", 0, 2, LILAC),
    "FRUSTRATED": ("flat", 0, 0, PEACH), "ANGRY": ("flat", 0, -2, RED),
    "ANNOYED": ("smirk", 1, 0, PEACH), "WORRIED": ("open", 0, 2, LILAC),
    "AFRAID": ("open", 0, -2, PINK), "EMBARRASSED": ("smile", -1, 2, PINK),
    "SURPRISED": ("open", 0, -3, CREAM), "SHOCKED": ("open", 0, -4, RED),
    "CURIOUS": ("smirk", 1, -1, CYAN), "CONFUSED": ("flat", 1, 1, LILAC),
    "SKEPTICAL": ("smirk", 1, 0, PEACH), "THINKING": ("flat", 1, -2, LILAC),
    "DEEP_THINKING": ("flat", 1, -3, LILAC), "ANALYZING": ("flat", 0, -1, CYAN),
    "LISTENING": ("flat", 0, 0, CYAN), "SPEAKING": ("smile", 0, 0, PINK),
    "GREETING": ("smile", 0, -1, CREAM), "SCANNING": ("flat", 0, -1, CYAN),
    "SLEEPY": ("sleepy", 0, 2, LILAC), "SLEEPING": ("closed", 0, 3, LILAC),
    "WAKING_UP": ("sleepy", 0, 2, CREAM), "BORED": ("flat", 1, 2, LILAC),
    "ALERT": ("open", 0, -2, RED), "ERROR": ("open", 0, -1, RED),
    "OFFLINE": ("sad", 0, 1, LILAC), "LOW_BATTERY": ("sleepy", 0, 2, PEACH),
    "MISCHIEVOUS": ("smirk", 1, 0, PINK), "SARCASTIC": ("smirk", 1, 0, LILAC),
    "MYSTERIOUS": ("sleepy", 1, 1, LILAC), "SHY": ("sad", -1, 2, PINK),
    "GOOFY": ("open", 1, -1, CREAM), "LOVE_STRUCK": ("smile", 0, 0, PINK),
    "EUREKA": ("open", 0, -3, CREAM), "MEDITATING": ("closed", 0, 2, LILAC),
    "DETERMINED": ("flat", 0, -2, CYAN), "SIGNATURE": ("smirk", 1, -1, CYAN),
}

TIME_PERSONALITIES = [
    ("Mañana · modo gruñón", "06:00–11:59", "ANGRY", "¿En serio ya es de mañana?"),
    ("Tarde · modo travieso", "12:00–17:59", "PLAYFUL", "Vamos a descubrir algo."),
    ("Noche · humor irónico", "18:00–23:59", "SARCASTIC", "Esto se va a poner interesante…"),
    ("Madrugada · modo enigmático", "00:00–05:59", "MYSTERIOUS", "Las mejores ideas llegan de noche."),
]

def time_personality(now=None):
    now = now or datetime.now()
    h = now.hour
    if 6 <= h < 12:
        return TIME_PERSONALITIES[0]
    if 12 <= h < 18:
        return TIME_PERSONALITIES[1]
    if 18 <= h < 24:
        return TIME_PERSONALITIES[2]
    return TIME_PERSONALITIES[3]


# Cada estado tiene un emblema y una reacción visual propios. El globo superior
# hace que el cambio sea visible incluso cuando dos estados comparten rasgos faciales.
STATE_BADGES = {
    "HAPPY": ("😊", "¡Qué buen día!", CYAN),
    "EXCITED": ("🤩", "¡No puedo esperar!", PINK),
    "JOYFUL": ("😄", "Esto me encanta", CREAM),
    "AMUSED": ("😆", "Ja. Ja. Qué bueno.", PINK),
    "PROUD": ("😌", "Mira lo que logré", LILAC),
    "PLAYFUL": ("😜", "¿Jugamos un poco?", CYAN),
    "LOVING": ("🥰", "Me caes muy bien", PINK),
    "GRATEFUL": ("🙏", "Gracias por estar", CREAM),
    "RELIEVED": ("😮‍💨", "Uf... solucionado", GREEN),
    "CONTENT": ("🙂", "Todo está en orden", CYAN),
    "SAD": ("😢", "Hoy estoy bajoneado", LILAC),
    "DISAPPOINTED": ("😞", "Esperaba otra cosa", LILAC),
    "FRUSTRATED": ("😣", "Esto no sale", PEACH),
    "ANGRY": ("😠", "Necesito un momento", RED),
    "ANNOYED": ("😒", "¿En serio?", PEACH),
    "WORRIED": ("😟", "Algo me preocupa", LILAC),
    "AFRAID": ("😨", "¿Qué fue ese ruido?", PINK),
    "LONELY": ("🥺", "¿Te quedas un rato?", LILAC),
    "EMBARRASSED": ("😳", "Bueno... qué vergüenza", PINK),
    "GUILTY": ("😔", "Creo que metí la pata", LILAC),
    "SURPRISED": ("😲", "¡¿Qué acaba de pasar?!", CREAM),
    "SHOCKED": ("🤯", "No me lo esperaba", RED),
    "CURIOUS": ("🧐", "Quiero saber más", CYAN),
    "INTRIGUED": ("🤔", "Aquí hay algo...", LILAC),
    "AMAZED": ("✨", "¡Es increíble!", CREAM),
    "CONFUSED": ("😵‍💫", "No termino de entender", LILAC),
    "PUZZLED": ("❔", "Hay una pieza que falta", LILAC),
    "SKEPTICAL": ("🤨", "Mmm... no sé", PEACH),
    "SUSPICIOUS": ("🕵️", "Te estoy observando", PEACH),
    "AWESTRUCK": ("🌌", "Qué cosa tan fascinante", CYAN),
    "THINKING": ("💭", "Dame un segundo", LILAC),
    "DEEP_THINKING": ("🧠", "Procesando a fondo", LILAC),
    "ANALYZING": ("🔎", "Buscando patrones", CYAN),
    "CALCULATING": ("🔢", "Haciendo cuentas", CYAN),
    "LEARNING": ("📚", "Nueva información guardada", GREEN),
    "REMEMBERING": ("🗂️", "Buscando en mi memoria", LILAC),
    "RECALLING": ("💡", "Creo que lo recuerdo", CREAM),
    "DECIDING": ("⚖️", "Evaluando opciones", LILAC),
    "SOLVING": ("🧩", "Uniendo las piezas", GREEN),
    "CREATIVE": ("🎨", "Probemos algo distinto", PINK),
    "LISTENING": ("👂", "Te escucho", CYAN),
    "SPEAKING": ("🗣️", "Estoy hablando...", PINK),
    "GREETING": ("👋", "¡Hola! ¿Cómo va?", CREAM),
    "FAREWELL": ("🙋", "¡Nos vemos pronto!", LILAC),
    "ATTENTIVE": ("👀", "Tenés toda mi atención", CYAN),
    "EMPATHETIC": ("🫶", "Estoy acá con vos", PINK),
    "ENCOURAGING": ("💪", "¡Vos podés!", GREEN),
    "CELEBRATING": ("🎉", "¡Lo conseguimos!", CREAM),
    "TEASING": ("😏", "Tengo una idea...", PINK),
    "WAITING_FOR_REPLY": ("…", "Te toca a vos", CYAN),
    "SCANNING": ("📡", "Explorando el entorno", CYAN),
    "DETECTING": ("🎯", "¡Detecté algo!", CREAM),
    "TRACKING": ("🧭", "Siguiendo el objetivo", CYAN),
    "LOOKING_LEFT": ("⬅️", "Mirando a la izquierda", LILAC),
    "LOOKING_RIGHT": ("➡️", "Mirando a la derecha", LILAC),
    "LOOKING_UP": ("⬆️", "¿Qué hay arriba?", CREAM),
    "LOOKING_DOWN": ("⬇️", "Revisando abajo", LILAC),
    "FOCUSED": ("🎯", "Enfoque máximo", CYAN),
    "ALERT": ("🚨", "Atención, por favor", RED),
    "STARTLED": ("💥", "¡Uy!", PEACH),
    "IDLE": ("🫧", "En reposo, pero atento", CYAN),
    "SLEEPY": ("🥱", "Me está agarrando sueño", LILAC),
    "SLEEPING": ("😴", "Zzz...", LILAC),
    "WAKING_UP": ("🌅", "Cinco minutos más...", CREAM),
    "ENERGIZED": ("⚡", "¡A toda máquina!", CREAM),
    "RELAXED": ("🍃", "Todo tranquilo", GREEN),
    "BORED": ("🥱", "¿Hay algo para hacer?", LILAC),
    "RESTLESS": ("🌀", "No puedo quedarme quieto", PINK),
    "DROWSY": ("🌙", "Se me cierran los ojos", LILAC),
    "RECHARGING": ("🔋", "Recargando energía", GREEN),
    "BOOTING": ("🔄", "Iniciando sistemas", CYAN),
    "ONLINE": ("🟢", "Conectado y listo", GREEN),
    "OFFLINE": ("🔌", "Estoy sin conexión", LILAC),
    "CONNECTING": ("🔗", "Buscando conexión", CYAN),
    "PROCESSING": ("⚙️", "Procesando solicitud", CYAN),
    "LOADING": ("⏳", "Un momento...", CREAM),
    "ERROR": ("⚠️", "Algo salió mal", RED),
    "RECOVERING": ("🩹", "Intentando recuperarme", GREEN),
    "UPDATING": ("🛠️", "Mejorando mis sistemas", CYAN),
    "LOW_BATTERY": ("🪫", "Necesito energía", PEACH),
    "MISCHIEVOUS": ("😈", "Se me ocurrió una travesura", PINK),
    "SARCASTIC": ("🙄", "Ah, claro. Brillante.", LILAC),
    "DRAMATIC": ("🎭", "¡Esto es una tragedia!", PINK),
    "SHY": ("🫣", "No me mires tanto", PINK),
    "CONFIDENT": ("😎", "Lo tengo bajo control", CYAN),
    "MYSTERIOUS": ("🕶️", "Hay cosas que no puedo contar", LILAC),
    "GOOFY": ("🤪", "Modo payaso activado", CREAM),
    "CURIOUS_PET": ("🐾", "¿Qué es eso?", PEACH),
    "GLITCHY": ("👾", "Bip... bup... glitch", PINK),
    "SHOWTIME": ("🎬", "¡Que comience el show!", CREAM),
    "LOVE_STRUCK": ("💘", "Me dejaste sin palabras", PINK),
    "MIND_BLOWN": ("🤯", "Mi mente acaba de explotar", RED),
    "EUREKA": ("💡", "¡Eureka! Lo encontré", CREAM),
    "MEDITATING": ("🧘", "Respirando... calma", GREEN),
    "DREAMING": ("💤", "Visitando otros mundos", LILAC),
    "NOSTALGIC": ("📼", "Eso me trae recuerdos", LILAC),
    "DETERMINED": ("🛡️", "No voy a rendirme", CYAN),
    "OVERWHELMED": ("🌪️", "Demasiadas cosas a la vez", PEACH),
    "MIRRORING": ("🪞", "Estoy imitando tu expresión", CYAN),
    "SIGNATURE": ("✦", "Esta es mi firma", PINK),
}

# Efectos adicionales para que el estado no dependa únicamente del texto.
EFFECTS = {
    "LOVING": "hearts", "LOVE_STRUCK": "hearts", "HAPPY": "sparkles",
    "JOYFUL": "sparkles", "CELEBRATING": "confetti", "EXCITED": "sparkles",
    "SAD": "tears", "DISAPPOINTED": "tears", "ANGRY": "anger",
    "FRUSTRATED": "anger", "ANNOYED": "anger", "ALERT": "alert",
    "STARTLED": "burst", "THINKING": "dots", "DEEP_THINKING": "dots",
    "PROCESSING": "dots", "LOADING": "dots", "WAITING_FOR_REPLY": "dots",
    "CONNECTING": "orbit", "SCANNING": "scan", "DETECTING": "target",
    "TRACKING": "scan", "SLEEPY": "zzz", "SLEEPING": "zzz",
    "DROWSY": "zzz", "WAKING_UP": "sunrise", "LOW_BATTERY": "battery",
    "RECHARGING": "battery", "ERROR": "warning", "OFFLINE": "offline",
    "GLITCHY": "glitch", "SHOWTIME": "spotlight", "EUREKA": "bulb",
    "MEDITATING": "breath", "DREAMING": "dream", "OVERWHELMED": "storm",
    "MIRRORING": "mirror", "CURIOUS": "question", "CONFUSED": "question",
    "PUZZLED": "question", "SKEPTICAL": "question", "CREATIVE": "sparkles",
}


# Perfiles faciales: cada estado recibe una combinación de ojos, cejas, boca,
# dirección de mirada y movimiento. Los grupos semánticos fijan el carácter;
# los valores base rotativos evitan que los estados restantes sean idénticos.
EYE_CYCLE = ("oval", "wide", "narrow", "diamond", "sleepy", "round", "squint")
BROW_CYCLE = ("neutral", "raised", "asymmetric", "worried", "angry", "soft")
MOUTH_CYCLE = ("flat", "smile", "smirk", "open", "o", "frown", "wave", "grin")
GAZE_CYCLE = ("center", "wander", "left", "right", "up", "down", "scan")
MOTION_CYCLE = ("float", "sway", "pulse", "bounce", "nod", "tremble", "still", "drift")

FACIAL_PROFILES = {}
for _i, (_name, _label, _category) in enumerate(STATES):
    FACIAL_PROFILES[_name] = {
        "eye": EYE_CYCLE[_i % len(EYE_CYCLE)],
        "brow": BROW_CYCLE[(_i * 3) % len(BROW_CYCLE)],
        "mouth": MOUTH_CYCLE[(_i * 5) % len(MOUTH_CYCLE)],
        "gaze": GAZE_CYCLE[(_i * 2) % len(GAZE_CYCLE)],
        "motion": MOTION_CYCLE[(_i * 3) % len(MOTION_CYCLE)],
    }

# Ajustes semánticos principales. Cada estado conserva además sus movimientos
# y microvariaciones calculados arriba, por lo que no depende solo del emoji.
def _assign_profile(names, **changes):
    for _name in names.split():
        FACIAL_PROFILES[_name].update(changes)

_assign_profile("HAPPY JOYFUL CONTENT GRATEFUL RELIEVED ENCOURAGING EMPATHETIC", eye="round", brow="soft", mouth="smile", motion="float")
_assign_profile("EXCITED AMAZED AWESTRUCK SHOCKED SURPRISED STARTLED MIND_BLOWN EUREKA", eye="wide", brow="raised", mouth="o", motion="bounce")
_assign_profile("AMUSED PROUD PLAYFUL TEASING MISCHIEVOUS SARCASTIC SKEPTICAL", eye="squint", brow="asymmetric", mouth="smirk", motion="sway")
_assign_profile("SAD DISAPPOINTED LONELY GUILTY NOSTALGIC", eye="sleepy", brow="worried", mouth="frown", gaze="down", motion="drift")
_assign_profile("ANGRY ANNOYED FRUSTRATED DETERMINED", eye="narrow", brow="angry", mouth="wave", motion="tremble")
_assign_profile("WORRIED AFRAID EMBARRASSED SHY", eye="wide", brow="worried", mouth="flat", gaze="down", motion="tremble")
_assign_profile("CONFUSED PUZZLED INTRIGUED SUSPICIOUS", eye="diamond", brow="asymmetric", mouth="wave", gaze="wander", motion="sway")
_assign_profile("THINKING DEEP_THINKING ANALYZING CALCULATING DECIDING SOLVING", eye="narrow", brow="neutral", mouth="flat", gaze="up", motion="pulse")
_assign_profile("LEARNING REMEMBERING RECALLING CREATIVE", eye="round", brow="raised", mouth="smirk", gaze="up", motion="orbit")
_assign_profile("LISTENING ATTENTIVE WAITING_FOR_REPLY", eye="wide", brow="raised", mouth="flat", gaze="center", motion="float")
_assign_profile("SPEAKING", eye="round", brow="soft", mouth="talk", motion="sway")
_assign_profile("GREETING FAREWELL", eye="round", brow="raised", mouth="smile", motion="bounce")
_assign_profile("SCANNING TRACKING LOOKING_LEFT LOOKING_RIGHT", eye="narrow", brow="neutral", mouth="flat", gaze="scan", motion="scan")
_assign_profile("LOOKING_UP", eye="wide", brow="raised", mouth="flat", gaze="up", motion="still")
_assign_profile("LOOKING_DOWN", eye="sleepy", brow="soft", mouth="flat", gaze="down", motion="still")
_assign_profile("FOCUSED", eye="narrow", brow="neutral", mouth="flat", gaze="center", motion="still")
_assign_profile("ALERT", eye="wide", brow="angry", mouth="o", motion="pulse")
_assign_profile("SLEEPY DROWSY LOW_BATTERY", eye="sleepy", brow="soft", mouth="frown", gaze="down", motion="drift")
_assign_profile("SLEEPING MEDITATING", eye="closed", brow="soft", mouth="flat", gaze="down", motion="float")
_assign_profile("WAKING_UP", eye="sleepy", brow="raised", mouth="o", gaze="up", motion="bounce")
_assign_profile("ENERGIZED CELEBRATING SHOWTIME", eye="wide", brow="raised", mouth="grin", motion="bounce")
_assign_profile("RELAXED IDLE", eye="oval", brow="soft", mouth="flat", motion="float")
_assign_profile("BORED", eye="sleepy", brow="asymmetric", mouth="wave", gaze="wander", motion="drift")
_assign_profile("RESTLESS OVERWHELMED", eye="wide", brow="worried", mouth="wave", gaze="wander", motion="tremble")
_assign_profile("BOOTING CONNECTING PROCESSING LOADING UPDATING", eye="diamond", brow="neutral", mouth="flat", gaze="scan", motion="pulse")
_assign_profile("ONLINE", eye="round", brow="raised", mouth="smile", motion="pulse")
_assign_profile("OFFLINE ERROR", eye="narrow", brow="worried", mouth="frown", motion="drift")
_assign_profile("RECOVERING", eye="sleepy", brow="worried", mouth="flat", motion="pulse")
_assign_profile("MISCHIEVOUS GOOFY CURIOUS_PET", eye="wink", brow="asymmetric", mouth="smirk", gaze="wander", motion="bounce")
_assign_profile("CONFIDENT", eye="narrow", brow="neutral", mouth="smirk", motion="still")
_assign_profile("MYSTERIOUS", eye="sleepy", brow="neutral", mouth="flat", motion="drift")
_assign_profile("GLITCHY", eye="diamond", brow="asymmetric", mouth="wave", motion="tremble")
_assign_profile("LOVE_STRUCK LOVING", eye="round", brow="soft", mouth="smile", motion="float")
_assign_profile("DREAMING", eye="closed", brow="soft", mouth="smile", motion="drift")
_assign_profile("MIRRORING", eye="oval", brow="asymmetric", mouth="wave", motion="sway")
_assign_profile("SIGNATURE", eye="diamond", brow="asymmetric", mouth="smirk", motion="orbit")

class CSRBotSimulator:
    def __init__(self, root):
        self.root = root
        root.title("CSR-BOT // Avatar Lab")
        root.configure(bg=BG)
        root.geometry("1120x790")
        root.minsize(900, 680)

        self.current_index = 0
        self.current_state = "IDLE"
        self.auto_personality = True
        self.speaking = False
        self.blink_until = 0.0
        self.next_blink = time.monotonic() + random.uniform(2, 4)
        self.started = time.monotonic()
        self.talk_phase = 0.0
        self.last_personality_change = time.monotonic()
        self.manual_personality_until = 0.0
        self.eye_offset_x = 0.0
        self.eye_offset_y = 0.0

        header = tk.Frame(root, bg=BG)
        header.pack(fill="x", padx=22, pady=(16, 8))
        tk.Label(header, text="CSR-BOT", bg=BG, fg=CYAN,
                 font=("Segoe UI", 24, "bold")).pack(side="left")
        tk.Label(header, text="AVATAR LAB  /  FACE ENGINE v4",
                 bg=BG, fg=MUTED, font=("Consolas", 11)).pack(side="left", padx=16, pady=(10, 0))
        self.clock_label = tk.Label(header, text="", bg=BG, fg=PEACH, font=("Consolas", 11))
        self.clock_label.pack(side="right", pady=(10, 0))

        body = tk.Frame(root, bg=BG)
        body.pack(fill="both", expand=True, padx=22, pady=8)

        self.canvas = tk.Canvas(body, bg="#0b0e18", highlightthickness=1,
                                highlightbackground="#303952")
        self.canvas.pack(side="left", fill="both", expand=True)

        side = tk.Frame(body, bg=PANEL, width=270)
        side.pack(side="right", fill="y", padx=(14, 0))
        side.pack_propagate(False)
        tk.Label(side, text="PERSONALIDAD HORARIA", bg=PANEL, fg=CYAN,
                 font=("Segoe UI", 10, "bold")).pack(anchor="w", padx=14, pady=(16, 4))
        self.personality_label = tk.Label(side, text="", bg=PANEL, fg=TEXT,
                 font=("Segoe UI", 12, "bold"), wraplength=235, justify="left")
        self.personality_label.pack(anchor="w", padx=14, pady=3)
        self.personality_quote = tk.Label(side, text="", bg=PANEL, fg=MUTED,
                 font=("Segoe UI", 10), wraplength=235, justify="left")
        self.personality_quote.pack(anchor="w", padx=14, pady=(0, 14))

        tk.Label(side, text="ESTADO ACTUAL", bg=PANEL, fg=CYAN,
                 font=("Segoe UI", 10, "bold")).pack(anchor="w", padx=14, pady=(4, 4))
        self.state_label = tk.Label(side, text="", bg=PANEL, fg=TEXT,
                 font=("Consolas", 12, "bold"), wraplength=235, justify="left")
        self.state_label.pack(anchor="w", padx=14, pady=(0, 6))
        self.count_label = tk.Label(side, text="", bg=PANEL, fg=MUTED,
                 font=("Consolas", 9), wraplength=235, justify="left")
        self.count_label.pack(anchor="w", padx=14, pady=(0, 12))

        tk.Label(side, text="CAMBIAR ESTADO", bg=PANEL, fg=CYAN,
                 font=("Segoe UI", 10, "bold")).pack(anchor="w", padx=14, pady=(4, 6))
        self.state_var = tk.StringVar(value="IDLE")
        self.state_menu = tk.OptionMenu(side, self.state_var, *[s[0] for s in STATES],
                                        command=self.select_state)
        self.state_menu.config(bg=PANEL_2, fg=TEXT, activebackground=PANEL_2,
                               activeforeground=CYAN, highlightthickness=0,
                               relief="flat", font=("Consolas", 9), width=22)
        self.state_menu["menu"].config(bg=PANEL_2, fg=TEXT)
        self.state_menu.pack(fill="x", padx=14)

        self.auto_button = tk.Button(side, text="Auto personalidad: ON",
                                     command=self.toggle_auto, bg="#2d4154", fg=CYAN,
                                     activebackground="#3b536a", relief="flat")
        self.auto_button.pack(fill="x", padx=14, pady=(12, 5))
        self.speak_button = tk.Button(side, text="▶ Simular voz",
                                      command=self.toggle_speaking, bg="#4a3155", fg=PINK,
                                      activebackground="#62406e", relief="flat")
        self.speak_button.pack(fill="x", padx=14, pady=5)
        self.blink_button = tk.Button(side, text="Parpadear", command=self.blink,
                                      bg=PANEL_2, fg=TEXT, activebackground="#303952",
                                      relief="flat")
        self.blink_button.pack(fill="x", padx=14, pady=5)
        self.prev_button = tk.Button(side, text="← Estado anterior",
                                     command=lambda: self.step_state(-1), bg=PANEL_2,
                                     fg=TEXT, activebackground="#303952", relief="flat")
        self.prev_button.pack(fill="x", padx=14, pady=(12, 4))
        self.next_button = tk.Button(side, text="Estado siguiente →",
                                     command=lambda: self.step_state(1), bg=PANEL_2,
                                     fg=TEXT, activebackground="#303952", relief="flat")
        self.next_button.pack(fill="x", padx=14, pady=4)
        tk.Label(side, text="Teclas: 1–4 personalidad · A auto\nEspacio voz · B parpadear · N/P estados",
                 bg=PANEL, fg=MUTED, font=("Consolas", 8), justify="left").pack(
                     anchor="w", padx=14, pady=(14, 8))

        footer = tk.Label(root, text="100 ESTADOS  •  PERFILES FACIALES  •  MICROGESTOS  •  MOVIMIENTO",
                          bg=BG, fg=MUTED, font=("Consolas", 9))
        footer.pack(fill="x", padx=22, pady=(0, 12))

        root.bind("<KeyPress>", self.keypress)
        root.bind("<Escape>", lambda e: root.destroy())
        root.bind("<space>", lambda e: self.toggle_speaking())
        self.apply_time_personality()
        self.animate()

    def toggle_auto(self):
        self.auto_personality = not self.auto_personality
        self.auto_button.config(text="Auto personalidad: " + ("ON" if self.auto_personality else "OFF"))
        if self.auto_personality:
            self.apply_time_personality()

    def apply_time_personality(self):
        name, hours, state, quote = time_personality()
        self.personality_label.config(text=f"{name}\n{hours}")
        self.personality_quote.config(text=f"“{quote}”")
        if self.auto_personality and time.monotonic() >= self.manual_personality_until:
            self.set_state(state, from_auto=True)

    def set_state(self, state, from_auto=False):
        names = [s[0] for s in STATES]
        if state not in names:
            state = "IDLE"
        self.current_index = names.index(state)
        self.current_state = state
        self.state_var.set(state)
        self.state_label.config(text=f"{state}\n{STATES[self.current_index][1]}")
        self.count_label.config(text=f"ESTADO {self.current_index + 1:03d} / 100  ·  {STATES[self.current_index][2].upper()}")
        if not from_auto:
            self.manual_personality_until = time.monotonic() + 60

    def select_state(self, value):
        self.set_state(value)

    def step_state(self, delta):
        self.current_index = (self.current_index + delta) % len(STATES)
        self.set_state(STATES[self.current_index][0])

    def toggle_speaking(self):
        self.speaking = not self.speaking
        self.speak_button.config(text="■ Detener voz" if self.speaking else "▶ Simular voz")
        if self.speaking:
            self.set_state("SPEAKING")
        elif not self.auto_personality:
            self.set_state("IDLE")

    def blink(self):
        now = time.monotonic()
        self.blink_until = now + 0.22
        self.next_blink = now + random.uniform(2.2, 4.4)

    def keypress(self, event):
        k = event.keysym.lower()
        if k == "a":
            self.toggle_auto()
        elif k == "b":
            self.blink()
        elif k == "n":
            self.step_state(1)
        elif k == "p":
            self.step_state(-1)
        elif k in ("1", "2", "3", "4"):
            idx = int(k) - 1
            state = TIME_PERSONALITIES[idx][2]
            self.auto_personality = False
            self.auto_button.config(text="Auto personalidad: OFF")
            self.set_state(state)
            self.personality_label.config(text=f"{TIME_PERSONALITIES[idx][0]}\n{TIME_PERSONALITIES[idx][1]}")
            self.personality_quote.config(text=f"“{TIME_PERSONALITIES[idx][3]}”")

    def draw_eye(self, cx, cy, side, t, blink, color, state, scale=1.0):
        """Dibuja un ojo cuyo tamaño, forma, mirada y ceja dependen del estado."""
        c = self.canvas
        profile = FACIAL_PROFILES.get(state, FACIAL_PROFILES["IDLE"])
        eye_style = profile["eye"]
        brow_style = profile["brow"]
        w = (98 if side == "left" else 86) * scale
        h = (82 if side == "left" else 72) * scale

        # Animaciones de párpados: sueño, sorpresa y estados vivos tienen ritmos distintos.
        if eye_style == "wide":
            h *= 1.22 + 0.05 * math.sin(t * 4)
            w *= 1.08
        elif eye_style == "narrow":
            h *= 0.62
        elif eye_style == "sleepy":
            h *= 0.43 + 0.04 * math.sin(t * 1.4)
        elif eye_style == "squint":
            h *= 0.58 + 0.08 * abs(math.sin(t * 2.7))
        elif eye_style == "diamond":
            h *= 0.92
            w *= 0.94
        elif eye_style == "round":
            h *= 1.13
            w *= 0.92
        elif eye_style == "wink" and (side == "left") == (int(t * 1.5) % 5 == 0):
            h *= 0.08
        elif eye_style == "closed":
            h *= 0.06

        if state in ("SLEEPY", "SLEEPING", "DROWSY", "LOW_BATTERY"):
            h *= 0.85
        if blink:
            c.create_line(cx-w/2, cy, cx+w/2, cy, fill=color, width=5, capstyle="round")
            return

        # Contorno distinto según la forma del ojo.
        if eye_style in ("diamond",):
            points = [cx, cy-h/2, cx+w/2, cy, cx, cy+h/2, cx-w/2, cy]
            c.create_polygon(points, fill="#f9efff", outline=color, width=3)
        elif side == "left":
            c.create_oval(cx-w/2, cy-h/2, cx+w/2, cy+h/2,
                          fill="#eafcff", outline=color, width=3)
        else:
            points = [cx-w/2, cy-h/2+9, cx+w/2-6, cy-h/2,
                      cx+w/2, cy+h/2-8, cx-w/2+7, cy+h/2]
            c.create_polygon(points, fill="#f9efff", outline=color, width=3)

        # Dirección de mirada y movimiento propio de cada estado.
        gaze = profile["gaze"]
        dx = math.sin(t * 0.9) * 4
        dy = math.cos(t * 0.7) * 2
        if gaze == "left": dx = -18
        elif gaze == "right": dx = 18
        elif gaze == "up": dy = -13
        elif gaze == "down": dy = 12
        elif gaze == "wander":
            dx, dy = math.sin(t*2.4)*15, math.cos(t*1.8)*8
        elif gaze == "scan":
            dx, dy = math.sin(t*2.8)*17, math.sin(t*1.3)*3
        if state in ("THINKING", "DEEP_THINKING", "ANALYZING", "CALCULATING"):
            dx, dy = 10, -9 + math.sin(t*2)*2
        if state == "GOOFY" and side == "right":
            dx += 10
        if state in ("SURPRISED", "SHOCKED", "AFRAID", "ALERT", "EXCITED"):
            dx += math.sin(t*7)*2

        # Pupila de tamaño variable, con brillo pastel.
        pupil_r = (19 if side == "left" else 16)
        if eye_style == "wide": pupil_r *= 0.82
        elif eye_style in ("narrow", "sleepy", "squint", "closed"): pupil_r *= 0.7
        c.create_oval(cx+dx-pupil_r, cy+dy-pupil_r, cx+dx+pupil_r, cy+dy+pupil_r,
                      fill="#25304b", outline="")
        c.create_oval(cx+dx-pupil_r*0.58, cy+dy-pupil_r*0.58,
                      cx+dx+pupil_r*0.58, cy+dy+pupil_r*0.58,
                      fill=color, outline="")
        c.create_oval(cx+dx-5, cy+dy-8, cx+dx+2, cy+dy-1, fill="#ffffff", outline="")

        # Cejas animadas: inclinación, altura y asimetría cambian la lectura emocional.
        brow_y = cy - h/2 - 14
        if brow_style == "angry":
            c.create_line(cx-w*.55, brow_y-1, cx+w*.35, brow_y+10,
                          fill="#49354f", width=7, capstyle="round")
        elif brow_style == "worried":
            c.create_line(cx-w*.52, brow_y+10, cx+w*.48, brow_y-3,
                          fill="#49354f", width=6, capstyle="round")
        elif brow_style == "raised":
            lift = 5 + abs(math.sin(t*2))*3
            c.create_line(cx-w*.48, brow_y-lift, cx+w*.48, brow_y-lift-2,
                          fill="#49354f", width=6, capstyle="round")
        elif brow_style == "asymmetric":
            lift = -12 if side == "right" else 2
            c.create_line(cx-w*.5, brow_y+lift, cx+w*.45, brow_y+lift-2,
                          fill="#49354f", width=6, capstyle="round")
        elif brow_style == "soft":
            c.create_line(cx-w*.48, brow_y+3, cx, brow_y-2, cx+w*.48, brow_y+3,
                          fill="#49354f", width=5, smooth=True, capstyle="round")
        else:
            c.create_line(cx-w*.48, brow_y, cx+w*.48, brow_y-2,
                          fill="#49354f", width=5, capstyle="round")

    def draw_mouth(self, cx, cy, state, t):
        """Boca con formas y microanimaciones seleccionadas por perfil."""
        c = self.canvas
        mode = FACIAL_PROFILES.get(state, FACIAL_PROFILES["IDLE"])["mouth"]
        if self.speaking or state == "SPEAKING" or mode == "talk":
            # Es una simulación visual de habla; la sincronización real vendrá con audio.
            open_amount = 5 + abs(math.sin(t * 11.5)) * 22 + abs(math.sin(t*4.1))*5
            width = 48 + abs(math.sin(t*5))*18
            c.create_oval(cx-width/2, cy-open_amount/2, cx+width/2, cy+open_amount/2,
                          fill="#282238", outline=PINK, width=3)
            if open_amount > 17:
                c.create_arc(cx-width/2+5, cy-2, cx+width/2-5, cy+open_amount,
                             start=0, extent=180, style="arc", outline=CREAM, width=2)
            return

        pulse = abs(math.sin(t * (4.0 if state in ("EXCITED", "ANGRY", "ALERT") else 2.0)))
        if mode == "smile":
            c.create_arc(cx-36, cy-22-pulse*2, cx+36, cy+26,
                         start=200, extent=140, style="arc", outline=PINK, width=4)
        elif mode == "grin":
            c.create_oval(cx-33, cy-6, cx+33, cy+20+pulse*3,
                          fill="#fff0f4", outline=PINK, width=3)
            c.create_line(cx-25, cy+5, cx+25, cy+5, fill="#d98eae", width=2)
        elif mode == "smirk":
            c.create_line(cx-28, cy+4, cx+1, cy+7, cx+30, cy-6-pulse*2,
                          fill=PINK, width=4, smooth=True, capstyle="round")
        elif mode == "open":
            c.create_oval(cx-25, cy-10-pulse*4, cx+25, cy+18+pulse*4,
                          fill="#282238", outline=PINK, width=3)
        elif mode == "o":
            r = 10 + pulse*6
            c.create_oval(cx-r, cy-r, cx+r, cy+r*1.35,
                          fill="#282238", outline=PINK, width=3)
        elif mode == "frown":
            c.create_arc(cx-34, cy-1, cx+34, cy+36,
                         start=20, extent=140, style="arc", outline=PINK, width=4)
        elif mode == "wave":
            c.create_line(cx-30, cy+2, cx-15, cy-2, cx, cy+4,
                          cx+15, cy-2, cx+30, cy+2,
                          fill=PINK, width=4, smooth=True, capstyle="round")
        else:  # flat: la boca parece una pequeña línea de píxeles.
            width = 20 + pulse*5
            c.create_line(cx-width, cy, cx+width, cy-1,
                          fill=PINK, width=3, capstyle="round")

    def draw_state_bubble(self, cx, cy, state, t):
        """Globo con emblema y microfrase exclusiva para cada estado."""
        c = self.canvas
        icon, phrase, color = STATE_BADGES.get(state, ("✦", state, CYAN))
        bob = math.sin(t * 2.2) * 2.0
        y = cy - 252 + bob
        x1, y1, x2, y2 = cx - 164, y - 27, cx + 164, y + 27
        # Sombra y placa de diálogo.
        c.create_rectangle(x1 + 4, y1 + 4, x2 + 4, y2 + 4,
                           fill="#070912", outline="")
        c.create_rectangle(x1, y1, x2, y2, fill="#1a2032",
                           outline=color, width=2)
        # Punta del globo
        c.create_polygon(cx - 10, y2, cx + 10, y2, cx, y2 + 12,
                         fill="#1a2032", outline=color)
        c.create_text(cx - 132, y, text=icon, fill=color,
                      font=("Segoe UI Emoji", 21, "bold"))
        c.create_text(cx + 10, y, text=phrase, fill=TEXT,
                      font=("Segoe UI", 10, "bold"), width=245)
        # Barra de acento que cambia con cada estado
        pulse = 0.55 + 0.45 * abs(math.sin(t * 3.0))
        c.create_line(x1 + 12, y2 - 3, x1 + 12 + 32 * pulse, y2 - 3,
                      fill=color, width=3)

    def draw_state_effect(self, cx, cy, state, t):
        """Efectos laterales y partículas específicas por estado."""
        c = self.canvas
        effect = EFFECTS.get(state, "")
        left_x, right_x = cx - 188, cx + 188
        top_y = cy - 95

        if effect == "hearts":
            for i in range(3):
                x = cx - 115 + i * 115
                y = cy - 115 - abs(math.sin(t * 2 + i)) * 10
                c.create_text(x, y, text="♥", fill=PINK,
                              font=("Segoe UI", 14 + (i % 2) * 4, "bold"))
        elif effect == "sparkles":
            for i in range(5):
                x = cx + math.cos(t * 1.7 + i * 1.25) * 205
                y = cy - 90 + math.sin(t * 1.7 + i * 1.25) * 100
                c.create_text(x, y, text="✦", fill=(CYAN, PINK, CREAM)[i % 3],
                              font=("Segoe UI", 13 + i % 3 * 3, "bold"))
        elif effect == "confetti":
            for i in range(18):
                x = cx + math.sin(i * 12.2 + t * 1.8) * (125 + i % 3 * 25)
                y = cy - 140 + ((t * 65 + i * 29) % 270)
                c.create_rectangle(x, y, x + 5, y + 9,
                                   fill=(PINK, CYAN, CREAM, LILAC)[i % 4], outline="")
        elif effect == "tears":
            for x in (cx - 112, cx + 112):
                drop_y = cy + 4 + abs(math.sin(t * 2.5)) * 16
                c.create_oval(x - 4, drop_y, x + 4, drop_y + 12,
                              fill="#8bd8ff", outline="")
        elif effect == "anger":
            c.create_text(left_x + 8, top_y, text="〰", fill=RED,
                          font=("Segoe UI", 24, "bold"))
            c.create_text(right_x - 8, top_y, text="〰", fill=RED,
                          font=("Segoe UI", 24, "bold"))
        elif effect == "alert":
            radius = 10 + abs(math.sin(t * 6)) * 8
            c.create_oval(cx - 220 - radius, cy - 25 - radius,
                          cx - 220 + radius, cy - 25 + radius,
                          outline=RED, width=2)
            c.create_oval(cx + 220 - radius, cy - 25 - radius,
                          cx + 220 + radius, cy - 25 + radius,
                          outline=RED, width=2)
        elif effect == "burst":
            for angle in range(0, 360, 45):
                rad = math.radians(angle)
                x1, y1 = cx + math.cos(rad) * 195, cy - 30 + math.sin(rad) * 145
                x2, y2 = cx + math.cos(rad) * 215, cy - 30 + math.sin(rad) * 165
                c.create_line(x1, y1, x2, y2, fill=PEACH, width=3)
        elif effect == "dots":
            for i in range(3):
                r = 4 + (4 if int(t * 4) % 3 == i else 0)
                x = cx + 145 + i * 15
                y = cy + 105
                c.create_oval(x-r, y-r, x+r, y+r,
                              fill=LILAC if i == int(t * 4) % 3 else "#525a76", outline="")
        elif effect == "orbit":
            for i in range(3):
                angle = t * 2 + i * (2 * math.pi / 3)
                x = cx + math.cos(angle) * 205
                y = cy + math.sin(angle) * 155
                c.create_oval(x-5, y-5, x+5, y+5,
                              fill=(CYAN, PINK, LILAC)[i], outline="")
        elif effect == "scan":
            x = cx + math.sin(t * 2.4) * 190
            c.create_line(x, cy - 120, x, cy + 100, fill=CYAN, width=2, dash=(5, 5))
        elif effect == "target":
            r = 11 + abs(math.sin(t * 3)) * 8
            c.create_oval(cx + 190-r, cy-r, cx + 190+r, cy+r, outline=CREAM, width=2)
            c.create_oval(cx + 190-r/2, cy-r/2, cx + 190+r/2, cy+r/2, outline=CREAM)
        elif effect == "zzz":
            for i in range(3):
                c.create_text(cx + 125 + i * 20, cy - 130 - i * 15 + math.sin(t+i)*3,
                              text="z", fill=LILAC, font=("Consolas", 13 + i*3, "bold"))
        elif effect == "sunrise":
            c.create_arc(cx-34, cy-154, cx+34, cy-86, start=0, extent=180,
                         outline=CREAM, width=3, style="arc")
            c.create_line(cx-43, cy-120, cx+43, cy-120, fill=CREAM, width=2)
        elif effect == "battery":
            c.create_rectangle(cx+185, cy-22, cx+215, cy+22, outline=GREEN, width=2)
            c.create_rectangle(cx+194, cy-27, cx+206, cy-22, fill=GREEN, outline="")
            fill_width = 5 + 17 * abs(math.sin(t))
            c.create_rectangle(cx+190, cy+17-fill_width, cx+210, cy+17,
                               fill=PEACH if state == "LOW_BATTERY" else GREEN, outline="")
        elif effect == "warning":
            c.create_polygon(cx+195, cy-34, cx+170, cy+15, cx+220, cy+15,
                             outline=RED, fill="", width=3)
            c.create_text(cx+195, cy+1, text="!", fill=RED,
                          font=("Consolas", 17, "bold"))
        elif effect == "offline":
            c.create_line(cx+175, cy-10, cx+215, cy+20, fill=LILAC, width=3)
            c.create_arc(cx+174, cy-28, cx+216, cy+14, start=40, extent=100,
                         outline=LILAC, width=3)
        elif effect == "glitch":
            for i in range(5):
                yy = cy - 110 + i * 45
                shift = math.sin(t * 13 + i) * 15
                c.create_line(cx-205+shift, yy, cx-175+shift, yy,
                              fill=(PINK, CYAN, LILAC)[i % 3], width=3)
                c.create_line(cx+175-shift, yy+7, cx+205-shift, yy+7,
                              fill=(CYAN, PINK, LILAC)[i % 3], width=3)
        elif effect == "spotlight":
            for i, color in enumerate((CREAM, PINK, CYAN)):
                c.create_arc(cx-220-i*8, cy-220-i*8, cx+220+i*8, cy+220+i*8,
                             start=(int(t*45)+i*120) % 360, extent=28,
                             outline=color, width=5)
        elif effect == "bulb":
            c.create_oval(cx+184, cy-140, cx+211, cy-113, outline=CREAM, width=3)
            c.create_line(cx+190, cy-110, cx+205, cy-110, fill=CREAM, width=3)
            for i in range(4):
                angle = i * math.pi / 2
                x1 = cx+197 + math.cos(angle)*18
                y1 = cy-126 + math.sin(angle)*18
                x2 = cx+197 + math.cos(angle)*25
                y2 = cy-126 + math.sin(angle)*25
                c.create_line(x1, y1, x2, y2, fill=CREAM, width=2)
        elif effect == "breath":
            r = 18 + (math.sin(t * 1.2) + 1) * 13
            c.create_oval(cx-r, cy+135-r, cx+r, cy+135+r,
                          outline=GREEN, width=2)
        elif effect == "dream":
            for i in range(4):
                x = cx + 140 + i * 12
                y = cy - 145 - ((t*14 + i*23) % 35)
                c.create_text(x, y, text="✦", fill=LILAC, font=("Segoe UI", 12+i*2))
        elif effect == "storm":
            for i in range(3):
                x = cx - 170 + i * 170
                c.create_text(x, cy-140 + math.sin(t*5+i)*8, text="〰",
                              fill=PEACH, font=("Segoe UI", 24, "bold"))
        elif effect == "mirror":
            c.create_rectangle(cx+181, cy-63, cx+220, cy+2, outline=CYAN, width=2)
            c.create_line(cx+189, cy-48, cx+211, cy-48, fill=PINK, width=2)
            c.create_oval(cx+192, cy-35, cx+207, cy-20, outline=PINK, width=2)
        elif effect == "question":
            c.create_text(cx+194, cy-115, text="?", fill=LILAC,
                          font=("Georgia", 28, "bold"))

    def draw_avatar(self):
        c = self.canvas
        c.delete("all")
        w = max(c.winfo_width(), 600)
        h = max(c.winfo_height(), 500)
        cx, cy = w/2, h/2 + 8
        t = time.monotonic() - self.started
        state = self.current_state
        blink = time.monotonic() < self.blink_until
        motion = FACIAL_PROFILES.get(state, FACIAL_PROFILES["IDLE"])["motion"]
        # Movimiento de cabeza/rostro: cada perfil tiene un patrón reconocible.
        if motion == "float":
            cy += math.sin(t * 1.2) * 4
        elif motion == "sway":
            cx += math.sin(t * 1.7) * 5
            cy += math.sin(t * 2.4) * 2
        elif motion == "pulse":
            cy += math.sin(t * 3.2) * 2
        elif motion == "bounce":
            cy -= abs(math.sin(t * 3.0)) * 7
        elif motion == "nod":
            cy += math.sin(t * 2.2) * 5
        elif motion == "tremble":
            cx += math.sin(t * 18) * 2.5
            cy += math.cos(t * 16) * 1.5
        elif motion == "drift":
            cx += math.sin(t * 0.55) * 5
            cy += math.cos(t * 0.7) * 3
        elif motion == "orbit":
            cx += math.sin(t * 1.3) * 4
            cy += math.cos(t * 1.3) * 4
        elif motion == "scan":
            cx += math.sin(t * 2.1) * 3

        # Fondo con anillos y partículas pastel
        for i, col in enumerate((CYAN, LILAC, PINK)):
            c.create_oval(cx-210-i*11, cy-210-i*11, cx+210+i*11, cy+210+i*11,
                          outline=col, width=1)
        for i in range(28):
            px = cx + math.sin(i*8.3 + t*.18) * (145 + (i%4)*13)
            py = cy + math.cos(i*4.7 + t*.18) * (180 + (i%3)*9)
            c.create_oval(px-1.5, py-1.5, px+1.5, py+1.5,
                          fill=(CYAN, PINK, LILAC, PEACH)[i%4], outline="")

        # Neck and shoulders
        c.create_oval(cx-155, cy+125, cx+155, cy+315, fill="#27263c", outline="#5e668b", width=2)
        c.create_rectangle(cx-48, cy+100, cx+48, cy+195, fill="#e2b9b2", outline="")
        c.create_oval(cx-176, cy-174, cx+176, cy+190, fill="#d7a995", outline="#f4c6bd", width=3)

        # Hair silhouette / fringe, inspired by the chosen portrait but stylized
        c.create_oval(cx-184, cy-198, cx+174, cy-5, fill="#2b1d2a", outline="#62415a", width=2)
        c.create_polygon(
            cx-174, cy-92, cx-164, cy-174, cx-108, cy-213,
            cx-35, cy-226, cx+48, cy-211, cx+112, cy-179,
            cx+158, cy-121, cx+102, cy-144, cx+63, cy-158,
            cx+23, cy-129, cx-28, cy-150, cx-73, cy-113,
            cx-122, cy-93, fill="#382334", outline="#714a65", width=2
        )
        c.create_polygon(
            cx-171, cy-104, cx-112, cy-167, cx-42, cy-184,
            cx+12, cy-169, cx-25, cy-143, cx-71, cy-135,
            cx-103, cy-102, cx-148, cy-77, fill="#4b2d42", outline=""
        )
        # Ears
        c.create_oval(cx-183, cy-25, cx-148, cy+38, fill="#d7a995", outline="#f4c6bd", width=2)
        c.create_oval(cx+148, cy-25, cx+183, cy+38, fill="#d7a995", outline="#f4c6bd", width=2)
        # Eye positions intentionally asymmetrical
        self.draw_eye(cx-69, cy-34, "left", t, blink, CYAN, state, 1.0)
        self.draw_eye(cx+70, cy-29, "right", t, blink, PINK, state, 1.0)
        # Microgestos adicionales: lágrimas, rubor, tensión, sorpresa y corazón.
        if state in ("SAD", "DISAPPOINTED", "LONELY", "GUILTY"):
            tear_y = cy + 12 + abs(math.sin(t * 2.3)) * 10
            self.canvas.create_oval(cx-118, tear_y, cx-110, tear_y+13,
                                    fill="#8bd8ff", outline="")
        if state in ("ANGRY", "FRUSTRATED", "ANNOYED"):
            self.canvas.create_text(cx-133, cy-66, text="〰", fill=RED,
                                    font=("Segoe UI", 20, "bold"))
        if state in ("SURPRISED", "SHOCKED", "STARTLED", "MIND_BLOWN"):
            halo = 3 + abs(math.sin(t*5))*4
            self.canvas.create_oval(cx-112-halo, cy-84-halo, cx-100+halo, cy-72+halo,
                                    outline=CREAM, width=2)
        if state in ("LOVE_STRUCK", "LOVING"):
            self.canvas.create_text(cx+122, cy-72-abs(math.sin(t*3))*5,
                                    text="♥", fill=PINK, font=("Segoe UI", 17, "bold"))
        # Nose
        c.create_line(cx-4, cy+2, cx-13, cy+28, cx+4, cy+31,
                      fill="#b98485", width=3, smooth=True)
        # Cheek blush for softer pastel look
        if state in ("EMBARRASSED", "SHY", "LOVE_STRUCK", "HAPPY", "PLAYFUL"):
            c.create_oval(cx-127, cy+24, cx-82, cy+42, fill="#ef9fbd", outline="")
            c.create_oval(cx+82, cy+24, cx+127, cy+42, fill="#ef9fbd", outline="")
        self.draw_mouth(cx, cy+83, state, t)
        # Headset / signature details
        c.create_arc(cx-190, cy-182, cx+190, cy+102, start=190, extent=160,
                     style="arc", outline=CYAN, width=5)
        c.create_oval(cx-177, cy-10, cx-153, cy+32, fill=LILAC, outline=CYAN, width=2)
        c.create_oval(cx+153, cy-10, cx+177, cy+32, fill=PINK, outline=CYAN, width=2)
        # Emblema y efectos: todos los estados tienen un marcador visual propio.
        self.draw_state_effect(cx, cy, state, t)
        self.draw_state_bubble(cx, cy, state, t)
        # Status text inside avatar
        c.create_text(cx, cy+162, text="C S R  •  B O T", fill=CYAN,
                      font=("Consolas", 15, "bold"))
        c.create_text(cx, cy+187, text=state.replace("_", " "), fill=TEXT,
                      font=("Consolas", 10))

    def animate(self):
        now = time.monotonic()
        if now >= self.next_blink:
            self.blink_until = now + random.uniform(0.10, 0.18)
            self.next_blink = now + random.uniform(2.0, 4.6)
        if self.auto_personality and now >= self.manual_personality_until and now - self.last_personality_change > 45:
            self.apply_time_personality()
            self.last_personality_change = now
        self.clock_label.config(text=datetime.now().strftime("%A  %H:%M:%S").upper())
        self.draw_avatar()
        self.root.after(40, self.animate)


def main():
    root = tk.Tk()
    CSRBotSimulator(root)
    root.mainloop()

if __name__ == "__main__":
    main()

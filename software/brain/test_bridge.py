"""Pruebas locales sin ESP32 físico ni API: python -m unittest test_bridge.py"""
import json
import threading
import unittest
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib import parse, request
from unittest.mock import patch

import bot_server as brain


class FakeEsp32(BaseHTTPRequestHandler):
    token = "token-seguro-de-prueba"
    last_form = None
    last_path = None

    def log_message(self, *_):
        pass

    def respond(self, code, data):
        payload = json.dumps(data).encode('utf-8')
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(payload)))
        self.end_headers()
        self.wfile.write(payload)

    def do_GET(self):
        if self.headers.get('X-CSR-Token') != self.token:
            return self.respond(403, {'error': 'token'})
        self.respond(200, {'online': True, 'state': 'IDLE', 'sprite': True, 'heap': 180000})

    def do_POST(self):
        if self.headers.get('X-CSR-Token') != self.token:
            return self.respond(403, {'error': 'token'})
        size = int(self.headers.get('Content-Length', 0))
        self.__class__.last_path = self.path
        self.__class__.last_form = parse.parse_qs(self.rfile.read(size).decode('utf-8'))
        self.respond(200, {'ok': True})


class BridgeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.fake = ThreadingHTTPServer(('127.0.0.1', 0), FakeEsp32)
        cls.thread = threading.Thread(target=cls.fake.serve_forever, daemon=True)
        cls.thread.start()
        cls.port = cls.fake.server_port
        cls.settings = brain.Settings('127.0.0.1', FakeEsp32.token, False, 'gpt-4.1-mini', False)

    @classmethod
    def tearDownClass(cls):
        cls.fake.shutdown()
        cls.fake.server_close()

    def test_states_are_exactly_100(self):
        self.assertEqual(len(brain.STATE_NAMES), 100)
        self.assertEqual(brain.STATE_NAMES[60], 'IDLE')

    def test_ascii_for_tft(self):
        self.assertEqual(brain.tft_ascii('¡Qué alegría! 🤖'), 'Que alegria!')

    def test_demo_is_labeled(self):
        reply, mood = brain.demo_answer('Hola, bot')
        self.assertEqual(mood, 'GREETING')
        self.assertIn('PC', reply)

    def test_api_output_parser(self):
        self.assertEqual(brain.extract_text_from_response({
            'output': [{'type': 'message', 'content': [{'type':'output_text', 'text':'hola'}]}]
        }), 'hola')

    def test_device_protocol(self):
        # Los tests usan un puerto aleatorio; el firmware real utiliza puerto 80.
        original = brain.request_to_device
        def fake_port(s, endpoint, data=None):
            host = f'http://127.0.0.1:{self.port}/{endpoint}'
            payload = parse.urlencode(data).encode() if data is not None else None
            r = request.Request(host, data=payload, headers={
                'X-CSR-Token': s.device_token,
                'Content-Type': 'application/x-www-form-urlencoded',
            }, method='POST' if data is not None else 'GET')
            with request.urlopen(r, timeout=2) as response:
                return json.loads(response.read().decode())
        with patch.object(brain, 'request_to_device', new=fake_port):
            self.assertTrue(brain.request_to_device(self.settings, 'api/health')['online'])
            self.assertTrue(brain.send_face(self.settings, 'HAPPY', '¡Hola! 🤖')['ok'])
            self.assertEqual(FakeEsp32.last_form['state'], ['HAPPY'])
            self.assertEqual(FakeEsp32.last_form['text'], ['Hola!'])
            self.assertEqual(FakeEsp32.last_form['speaking'], ['0'])
            self.assertEqual(FakeEsp32.last_form['show_text'], ['1'])
            self.assertEqual(FakeEsp32.last_form['auto_stop'], ['0'])
            self.assertTrue(brain.set_device_speaking(self.settings, True)['ok'])
            self.assertEqual(FakeEsp32.last_path, '/api/speech')
            self.assertEqual(FakeEsp32.last_form['speaking'], ['1'])

    def test_long_demo_for_pagination(self):
        reply, mood = brain.demo_answer('quiero probar paginas')
        self.assertEqual(mood, 'EXCITED')
        self.assertGreater(len(reply), 250)
        self.assertLessEqual(len(brain.tft_ascii(reply)), 1200)

    def test_speech_worker_stops_mouth_after_tts(self):
        calls = []
        with patch.object(brain, 'speak_windows', side_effect=lambda text: calls.append(('speak', text))):
            with patch.object(brain, 'set_device_speaking', side_effect=lambda settings, active: calls.append(('mouth', active))):
                brain.SPEECH_BUSY.set()
                brain.speech_worker(self.settings, 'Hola CSR-BOT')
        self.assertEqual(calls, [('speak', 'Hola CSR-BOT'), ('mouth', False)])
        self.assertFalse(brain.SPEECH_BUSY.is_set())


if __name__ == '__main__':
    unittest.main()

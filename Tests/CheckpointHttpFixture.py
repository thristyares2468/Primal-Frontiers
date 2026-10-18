"""Disposable loopback fixture. No production credentials or database; self-exits in 90s."""
import base64
import copy
import hashlib
import json
import sys
import time
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path

records = {}

class Handler(BaseHTTPRequestHandler):
    def log_message(self, *_args):
        pass

    def reply(self, code, body):
        data = json.dumps(body).encode()
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def authorized(self):
        if self.headers.get("Authorization") != "Bearer fixture-only-not-a-real-key":
            self.reply(401, {"ok": False})
            return False
        return True

    def do_POST(self):
        if not self.authorized():
            return
        if self.path != "/v1/checkpoints" or self.headers.get("Content-Type") != "application/json":
            return self.reply(400, {"ok": False})
        size = int(self.headers.get("Content-Length", 0))
        if size <= 0 or size > 6 * 1024 * 1024:
            return self.reply(413, {"ok": False})
        try:
            record = json.loads(self.rfile.read(size))
            raw = base64.b64decode(record["payload"]["envelope"], validate=True)
            assert record["payload"]["format"] == "pf-pfs-base64-v1"
            assert record["checksum"] == "sha256:" + hashlib.sha256(raw).hexdigest()
        except (AssertionError, ValueError, KeyError):
            return self.reply(400, {"ok": False})
        identity = (record["world_id"], record["generation"])
        if identity in records:
            return self.reply(409, {"ok": False, "error": "generation_exists"})
        records[identity] = record
        acknowledgment = copy.deepcopy(record)
        acknowledgment.pop("payload")
        acknowledgment["generation"] = str(record["generation"])
        return self.reply(201, {"ok": True, "checkpoint": acknowledgment})

    def do_GET(self):
        if not self.authorized():
            return
        if self.path.endswith("/unavailable"):
            return self.reply(503, {"ok": False})
        candidates = list(records.values())
        if not candidates or self.path.endswith("/missing"):
            return self.reply(404, {"ok": False})
        record = copy.deepcopy(candidates[0])
        record["generation"] = str(record["generation"])
        if self.path.endswith("/corrupt"):
            record["checksum"] = "sha256:corrupted"
        return self.reply(200, {"ok": True, "checkpoint": record})

server = HTTPServer(("127.0.0.1", 0), Handler)
server.timeout = 0.25
port_file = Path(sys.argv[1])
port_file.parent.mkdir(parents=True, exist_ok=True)
port_file.write_text(str(server.server_port))
deadline = time.monotonic() + 90
try:
    while time.monotonic() < deadline:
        server.handle_request()
finally:
    server.server_close()
    port_file.unlink(missing_ok=True)

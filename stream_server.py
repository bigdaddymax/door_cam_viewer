import os
import time
import socketserver
from http.server import HTTPServer, BaseHTTPRequestHandler

FRAME_PATH = "/tmp/frame.jpg"

class ThreadedHTTPServer(socketserver.ThreadingMixIn, HTTPServer):
    daemon_threads = True
    allow_reuse_address = True

class CamStreamHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # Приймаємо запити і на '/', і на '/stream', і на будь-який інший шлях
        self.connection.settimeout(10.0)

        self.send_response(200)
        self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=frame')
        self.end_headers()

        try:
            while True:
                if os.path.exists(FRAME_PATH):
                    with open(FRAME_PATH, 'rb') as f:
                        frame = f.read()

                    self.wfile.write(b'--frame\r\n')
                    self.send_header('Content-Type', 'image/jpeg')
                    self.send_header('Content-Length', str(len(frame)))
                    self.end_headers()
                    self.wfile.write(frame)
                    self.wfile.write(b'\r\n')
                    self.wfile.flush()

                time.sleep(0.04)

        except (BrokenPipeError, ConnectionResetError, TimeoutError, socketserver.socket.timeout, OSError):
            # Клієнт відключився — виходимо з циклу
            pass

    def log_message(self, format, *args):
        return

if __name__ == '__main__':
    server = ThreadedHTTPServer(('0.0.0.0', 2222), CamStreamHandler)
    print("Starting server on port 2222...")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        server.server_close()
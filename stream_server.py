import time
import os
from http.server import HTTPServer, ThreadingHTTPServer, BaseHTTPRequestHandler


class ThreadedMJPEGHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # Відправляємо HTTP заголовки для MJPEG-потоку
        try:
            self.send_response(200)
            self.send_header('Content-Type', 'multipart/x-mixed-replace; boundary=frame')
            self.send_header('Cache-Control', 'no-cache, private')
            self.send_header('Pragma', 'no-cache')
            self.end_headers()
        except (ConnectionResetError, BrokenPipeError, SocketError):
            return

        # Основний цикл роздачі кадрів клієнту
        while True:
            try:
                if os.path.exists('/tmp/frame.jpg'):
                    with open('/tmp/frame.jpg', 'rb') as f:
                        img_data = f.read()

                    if img_data:
                        self.wfile.write(b'--frame\r\n')
                        self.wfile.write(b'Content-Type: image/jpeg\r\n')
                        self.wfile.write(f'Content-Length: {len(img_data)}\r\n\r\n'.encode())
                        self.wfile.write(img_data)
                        self.wfile.write(b'\r\n')
                        self.wfile.flush()

                # ~12 FPS
                time.sleep(0.08)

            except (ConnectionResetError, BrokenPipeError, ConnectionAbortedError):
                # Клієнт (ESP32 чи браузер) від'єднався — просто виходимо з функції потоку
                break
            except Exception:
                # Будь-яка інша помилка читання/мережі
                break

    # Пригнічуємо спам логів у консоль при кожному запиті/обриві
    def log_message(self, format, *args):
        return


def run(port=8080):
    # ThreadingHTTPServer створює новий потік під кожного клієнта
    server = ThreadingHTTPServer(('0.0.0.0', port), ThreadedMJPEGHandler)
    print(f"Robust MJPEG Streamer running at http://0.0.0.0:{port}/")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
        server.server_close()


if __name__ == '__main__':
    run()
#!/bin/bash

# Змінна для RTSP URL (можна передавати через env у docker)
RTSP_URL="${RTSP_URL:-rtsp://192.168.0.24/11}"

echo "Starting FFmpeg stream reader for $RTSP_URL..."

# Запускаємо FFmpeg у фоновому режимі (&)
ffmpeg -loglevel error -rtsp_transport tcp -i "$RTSP_URL" \
  -vf "scale=480:320" -q:v 8 -r 12 \
  -update 1 -y /tmp/frame.jpg &

# Запускаємо Python HTTP-сервер
echo "Starting Python MJPEG Server on port 8080..."
python3 /app/stream_server.py
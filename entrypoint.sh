#!/bin/bash

RTSP_URL="${RTSP_URL:-rtsp://192.168.0.24/11}"

# Створюємо нескінченний цикл для захисту від падінь FFmpeg
while true; do
  echo "Starting FFmpeg stream reader..."

  ffmpeg -loglevel error \
    -stimeout 5000000 \
    -fflags nobuffer -flags low_delay \
    -probesize 32 -analyzeduration 0 \
    -rtsp_transport tcp -i "$RTSP_URL" \
    -vf "scale=480:320" -q:v 8 -r 15 \
    -tune zerolatency -preset ultrafast \
    -update 1 -y /tmp/frame.jpg

  # Якщо FFmpeg завершив роботу (через помилку або таймаут),
  # цикл не дає скрипту зупинитися і перезапускає його через 2 секунди
  echo "FFmpeg process died or timed out. Reconnecting in 2 seconds..."
  sleep 2
done &

echo "Starting Python MJPEG Server on port 8080..."
python3 /app/stream_server.py
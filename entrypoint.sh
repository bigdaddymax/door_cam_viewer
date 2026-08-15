#!/bin/bash

RTSP_URL="${RTSP_URL:-rtsp://192.168.0.24/11}"

while true; do
  echo "Starting FFmpeg stream reader for $RTSP_URL..."
  ffmpeg -loglevel info -timeout 5000000 -rtsp_transport tcp -fflags nobuffer -flags low_delay -probesize 32 -analyzeduration 0 -i "$RTSP_URL" -vf "scale=480:320" -q:v 8 -r 15 -tune zerolatency -preset ultrafast -update 1 -y /tmp/frame.jpg
  echo "FFmpeg process died or timed out. Reconnecting in 2 seconds..."
  sleep 2
done &

echo "Starting Python MJPEG Server on port 2222..."
python3 /app/stream_server.py
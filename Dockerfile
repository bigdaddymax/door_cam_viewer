FROM python:3.10-slim

# Встановлюємо FFmpeg та необхідні утиліти
RUN apt-get update && \
    apt-get install -y --no-install-recommends ffmpeg bash && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Копіюємо Python-сервер та скрипт запуску
COPY stream_server.py /app/stream_server.py
COPY entrypoint.sh /app/entrypoint.sh

# Робимо entrypoint виконуваним
RUN chmod +x /app/entrypoint.sh

# Відкриваємо порт 8080 для ESP32
EXPOSE 2222

ENTRYPOINT ["/app/entrypoint.sh"]
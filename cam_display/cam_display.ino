#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>          // Необхідно підключати перед JPEGDEC
#include <TFT_eSPI.h>
#include <JPEGDEC.h>
#include <esp_heap_caps.h>

// ================= НАЛАШТУВАННЯ МЕРЕЖІ ТА СЕРВЕРА =================
const char* WIFI_SSID     = "MAXHOME";     // Назва вашої Wi-Fi мережі
const char* WIFI_PASSWORD = "deep purple"; // Пароль до Wi-Fi
const char* STREAM_URL    = "http://192.168.0.198:8080/"; // IP-адреса вашого ПК з Python-сервером

#define TFT_BL 27 // Пін підсвітки (перевірте пін підсвітки у вашому User_Setup.h)

// Буфер 64 KB у внутрішній оперативній пам'яті (DRAM)
const int MAX_JPEG_SIZE = 64 * 1024;
uint8_t* jpeg_buf = nullptr;

TFT_eSPI tft = TFT_eSPI();
JPEGDEC jpeg;

// ================= CALLBACK МАЛЮВАННЯ KАДРУ =================
// Центруємо картинку 320x240 на екрані 480x320 (зсув по X = 80, по Y = 40)
int JPEGDraw(JPEGDRAW *pDraw) {
  tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1; // 1 підтверджує успішне виведення фрагмента
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  // Ініціалізація підсвітки екрана
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Ініціалізація дисплея
  tft.init();
  tft.setRotation(1);           // Альбомна орієнтація (480x320)
  tft.invertDisplay(false);      // Усуває "негатив" на панелях HSD 9190J / ST7796
  tft.setSwapBytes(true);       // Правильний порядок байтів RGB для JPEGDEC
  tft.fillScreen(TFT_BLACK);    // Прибирає кольоровий шум та сміття з RAM

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Connecting to WiFi...", 20, 20, 4);

  // Виділяємо суцільний блок DRAM
  jpeg_buf = (uint8_t*) heap_caps_malloc(MAX_JPEG_SIZE, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!jpeg_buf) {
    // Резервна спроба з трохи меншим буфером, якщо пам'ять фрагментована
    jpeg_buf = (uint8_t*) heap_caps_malloc(48 * 1024, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  }

  if (!jpeg_buf) {
    Serial.println("CRITICAL ERROR: Failed to allocate JPEG buffer!");
    tft.drawString("RAM Allocation Error!", 20, 60, 4);
    while (1) delay(1000);
  }

  // Підключення до Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  tft.fillScreen(TFT_BLACK);
}

// ================= MAIN LOOP =================
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    delay(1000);
    return;
  }

  HTTPClient http;
  http.begin(STREAM_URL);
  http.setTimeout(5000);

  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient* stream = http.getStreamPtr();
    Serial.println("Stream connected!");

    unsigned long lastByteTime = millis();

    while (http.connected() && stream->connected()) {
      // Таймаут активності: якщо більше 3 секунд немає нових даних — перепідключаємося
      if (millis() - lastByteTime > 3000) {
        Serial.println("Stream timeout. Reconnecting...");
        break;
      }

      if (!stream->available()) {
        delay(1);
        continue;
      }

      // 1. Пошук першого байту маркеру JPEG (0xFF)
      int b1 = stream->read();
      if (b1 == -1) continue;
      lastByteTime = millis();

      if (b1 == 0xFF) {
        while (stream->connected() && !stream->available()) { delay(1); }
        int b2 = stream->peek();

        // 2. Перевірка другого байту маркеру початку кадру SOI (0xD8)
        if (b2 == 0xD8) {
          stream->read(); // Проковтуємо 0xD8

          jpeg_buf[0] = 0xFF;
          jpeg_buf[1] = 0xD8;
          int buf_idx = 2;
          bool found_eoi = false;

          // 3. Зчитуємо сам JPEG до кінцевого маркеру EOI (0xFF, 0xD9)
          while (stream->connected() && buf_idx < MAX_JPEG_SIZE) {
            if (stream->available()) {
              uint8_t b = (uint8_t)stream->read();
              jpeg_buf[buf_idx++] = b;
              lastByteTime = millis();

              if (jpeg_buf[buf_idx - 2] == 0xFF && jpeg_buf[buf_idx - 1] == 0xD9) {
                found_eoi = true;
                break;
              }
            }
          }

          // 4. Декодуємо та виводимо кадр на екран
          if (found_eoi) {
            if (jpeg.openRAM(jpeg_buf, buf_idx, JPEGDraw)) {
              tft.startWrite();     // Захоплюємо шину SPI
              jpeg.decode(0, 0, 0); // Виклик розпакування
              tft.endWrite();       // Відпускаємо шину SPI
              jpeg.close();
            }
          }
        }
      }
    }
  } else {
    Serial.printf("HTTP Error: %d\n", httpCode);
  }

  http.end();
  delay(500);
}
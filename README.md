### Отримання потоку з SV3 камери
Потік доступний за наступною адресою (без авторизації):
```
rtsp://192.168.0.24/11
```
FFmpeg захоплює RTSP-потік камери, масштабує його під 480x320 і оновлює кадри у RAM-диску.

Багатопотоковий Python-сервер стабільно роздає MJPEG-потік і миттєво обробляє будь-які перепідключення.

ESP32 декодує JPEG «на льоту» через JPEGDEC і виводить плавне відео на ваш дисплей ST7796/HSD 9190J без сміття, смуг та затримок.

Запуск процесу: `docker compose up -d --build`

Налаштування TFT дисплея (User_Setup.h)
```shell
#define ST7796_DRIVER
// АБО #define ILI9488_DRIVER (якщо у вас розкоментований ILI9488)

#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// 1. Драйвер (для HSD 9190J розкоментуйте ТІЛЬКИ ST7796):
#define ST7796_DRIVER

// 2. Роздільна здатність:
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// 3. Інверсія кольору (HSD матриці часто потребують увімкненої інверсії):
#define TFT_INVERSION_ON

// 4. Призначення пінів (для платформ ESP32 Smart Display 3.5"/4.0"):
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4   // Якщо екран білий, спробуйте змінити на -1
#define TFT_BL   27

// 5. Частота SPI (Для HSD 9190J критично не перевищувати 27-40 MHz на старті):
#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY 20000000
```
#include <WiFi.h>
#include <esp_now.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANELS_NUM  1

typedef struct {
  char suhu[10];
} DataKirim;

DataKirim dataMasuk;

String nilaiDisplay = "0000";

HUB75_I2S_CFG mxconfig(
  PANEL_RES_X,
  PANEL_RES_Y,
  PANELS_NUM
);

MatrixPanel_I2S_DMA *display = nullptr;

uint16_t DIGIT_COLOR;

// =========================
// DRAW SEGMENT
// =========================
void drawSegment(int x, int y, int w, int h)
{
  display->fillRect(x, y, w, h, DIGIT_COLOR);
}

// =========================
// DRAW DIGIT
// =========================
void drawDigit(int x, int y, char digit)
{
  bool a=false,b=false,c=false,d=false,e=false,f=false,g=false;

  switch(digit)
  {
    case '0': a=b=c=d=e=f=true; break;
    case '1': b=c=true; break;
    case '2': a=b=d=e=g=true; break;
    case '3': a=b=c=d=g=true; break;
    case '4': b=c=f=g=true; break;
    case '5': a=c=d=f=g=true; break;
    case '6': a=c=d=e=f=g=true; break;
    case '7': a=b=c=true; break;
    case '8': a=b=c=d=e=f=g=true; break;
    case '9': a=b=c=d=f=g=true; break;
  }

  const int t = 3;
  const int w = 13;
  const int h = 28;

  if(a) drawSegment(x+2,   y,      w-4, t);
  if(b) drawSegment(x+w-t, y+2,     t,   11);
  if(c) drawSegment(x+w-t, y+15,    t,   11);
  if(d) drawSegment(x+2,   y+h-t,   w-4, t);
  if(e) drawSegment(x,     y+15,    t,   11);
  if(f) drawSegment(x,     y+2,     t,   11);
  if(g) drawSegment(x+2,   y+13,    w-4, t);
}

// =========================
// TAMPILKAN DATA
// =========================
void tampilkanData()
{
  display->clearScreen();

  String s = nilaiDisplay;
  while(s.length() < 4)
  {
    s = "0" + s;
  }

  drawDigit(0,  2, s[0]);
  drawDigit(16, 2, s[1]);
  drawDigit(32, 2, s[2]);
  drawDigit(48, 2, s[3]);
}

// =========================
// ESP-NOW RECEIVE
// =========================
void OnDataRecv(
  const uint8_t *mac,
  const uint8_t *incomingData,
  int len)
{
  memcpy(
    &dataMasuk,
    incomingData,
    sizeof(dataMasuk)
  );

  nilaiDisplay = String(dataMasuk.suhu);

  Serial.print("DATA MASUK : ");
  Serial.println(nilaiDisplay);

  tampilkanData();
}

// =========================
// SETUP
// =========================
void setup()
{
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW ERROR");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  // =========================
  // PIN HUB75 BOARD CUSTOM
  // =========================
  mxconfig.gpio.r1  = 21;
  mxconfig.gpio.g1  = 5;
  mxconfig.gpio.b1  = 6;

  mxconfig.gpio.r2  = 7;
  mxconfig.gpio.g2  = 15;
  mxconfig.gpio.b2  = 16;

  mxconfig.gpio.a   = 37;
  mxconfig.gpio.b   = 35;
  mxconfig.gpio.c   = 3;
  mxconfig.gpio.d   = 42;

  mxconfig.gpio.clk = 41;
  mxconfig.gpio.lat = 40;
  mxconfig.gpio.oe  = 2;

  mxconfig.driver = HUB75_I2S_CFG::SHIFTREG;
  mxconfig.clkphase = false;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_8M;

  display = new MatrixPanel_I2S_DMA(mxconfig);
  display->begin();

  DIGIT_COLOR = display->color565(255, 0, 0);

  display->setBrightness8(150);
  tampilkanData();

  Serial.println("ESP32-S3 PENERIMA SIAP");
}

// =========================
// LOOP
// =========================
void loop()
{
}
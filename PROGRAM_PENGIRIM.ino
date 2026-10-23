
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// ==========================
// LCD I2C
// ==========================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================
// KEYPAD
// ==========================
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};

Keypad keypad = Keypad(
  makeKeymap(keys),
  rowPins,
  colPins,
  ROWS,
  COLS
);

// ==========================
// ESP-NOW STRUCTURE
// ==========================
typedef struct {
  char suhu[10];
} DataKirim;

DataKirim dataKirim;

// ==========================
// MAC ADDRESS RECEIVER
// ==========================
uint8_t broadcastAddress[] = {
  0x34, 0xCD, 0xB0, 0x33, 0xAA, 0x38
};

// ==========================
// VARIABLE CONTROL
// ==========================
String inputStr = "";
String lastSentStr = "";

volatile bool sendStatusReady = false;
volatile bool sendWasSuccess = false;

bool waitingForSend = false;
bool showingResult = false;

unsigned long resultStartMs = 0;

// ==========================
// CALLBACK ESP-NOW
// ==========================
void onDataSent(
  const uint8_t *mac_addr,
  esp_now_send_status_t status
) {
  sendWasSuccess = (status == ESP_NOW_SEND_SUCCESS);
  sendStatusReady = true;
}

// ==========================
// TAMPIL LCD INPUT
// ==========================
void tampilInput() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INPUT SUHU");
  lcd.setCursor(0, 1);
  lcd.print(inputStr);
}

// ==========================
// SETUP
// ==========================
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 PENGIRIM");
  delay(1500);

  tampilInput();

  // Konfigurasi Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);

  Serial.println();
  Serial.println("=== INFORMASI ESP32 PENGIRIM ===");

  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Inisialisasi ESP-NOW
  if (esp_now_init() != ESP_OK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ESP NOW ERROR");

    Serial.println("ESP NOW ERROR");
    return;
  }

  Serial.println("ESP-NOW berhasil diinisialisasi");

  // Informasi kanal Wi-Fi aktif
  Serial.print("Kanal Wi-Fi aktif: ");
  Serial.println(WiFi.channel());

  // Registrasi callback pengiriman
  esp_now_register_send_cb(onDataSent);

  // Konfigurasi peer penerima
  esp_now_peer_info_t peerInfo = {};

  memcpy(
    peerInfo.peer_addr,
    broadcastAddress,
    6
  );

  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PEER ERROR");

    Serial.println("PEER ERROR");
    return;
  }

  Serial.println("ESP32 PENGIRIM SIAP");
}

// ==========================
// LOOP
// ==========================
void loop() {
  // Periksa status pengiriman
  if (sendStatusReady) {
    bool success = sendWasSuccess;

    sendStatusReady = false;
    waitingForSend = false;
    showingResult = true;
    resultStartMs = millis();

    lcd.clear();

    if (success) {
      lcd.setCursor(0, 0);
      lcd.print("SUHU TERKIRIM");

      lcd.setCursor(0, 1);
      lcd.print(lastSentStr);

      inputStr = "";
    } else {
      lcd.setCursor(0, 0);
      lcd.print("GAGAL KIRIM");

      lcd.setCursor(0, 1);
      lcd.print("ULANGI INPUT");
    }
  }

  // Kembali ke tampilan input setelah 1,5 detik
  if (showingResult) {
    if (millis() - resultStartMs >= 1500) {
      showingResult = false;
      tampilInput();
    }
    return;
  }

  // Tunggu hasil pengiriman
  if (waitingForSend) {
    return;
  }

  char key = keypad.getKey();

  if (key) {
    Serial.print("KEY : ");
    Serial.println(key);

    // Input angka atau huruf A-D
    if ((key >= '0' && key <= '9') ||
        (key >= 'A' && key <= 'D')) {

      if (inputStr.length() < 4) {
        inputStr += key;
      }

      tampilInput();
    }

    // Hapus input
    else if (key == '*') {
      inputStr = "";
      tampilInput();
    }

    // Kirim data menggunakan tombol #
    else if (key == '#') {
      if (inputStr.length() > 0) {
        lastSentStr = inputStr;

        memset(
          dataKirim.suhu,
          0,
          sizeof(dataKirim.suhu)
        );

        lastSentStr.toCharArray(
          dataKirim.suhu,
          sizeof(dataKirim.suhu)
        );

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("MENGIRIM...");

        lcd.setCursor(0, 1);
        lcd.print(lastSentStr);

        waitingForSend = true;

        esp_err_t result = esp_now_send(
          broadcastAddress,
          (uint8_t *)&dataKirim,
          sizeof(dataKirim)
        );

        if (result != ESP_OK) {
          Serial.print("Gagal memulai pengiriman. Error: ");
          Serial.println(esp_err_to_name(result));

          waitingForSend = false;
          showingResult = true;
          resultStartMs = millis();

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("GAGAL KIRIM");

          lcd.setCursor(0, 1);
          lcd.print("ULANGI INPUT");
        }
      }
    }
  }
}

/*
 * KODE PENERIMA (RECEIVER)
 * - Menerima data audio via UDP
 * - Memainkan data ke Speaker (DAC)
 */

#include <WiFi.h>
#include <WiFiUdp.h>
#include "driver/dac.h" // Library untuk DAC

// --- Konfigurasi Jaringan ---
const char* ssid = "NAMA_WIFI_ANDA";
const char* password = "PASSWORD_WIFI_ANDA";

// Atur IP Statis untuk perangkat INI (Penerima)
// Ini adalah IP yang harus dimasukkan di kode Pengirim
IPAddress staticIP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);    // Sesuaikan gateway router Anda
IPAddress subnet(255, 255, 255, 0); // Sesuaikan subnet mask Anda

const int PORT = 4444; // Port UDP (harus sama di kedua perangkat)
// -----------------------------

// --- Konfigurasi Hardware ---
// Kita gunakan DAC Channel 1 (GPIO 25)
// -----------------------------

// Ukuran buffer audio (harus sama dengan pengirim)
const int PACKET_SIZE = 256;
byte audioBuffer[PACKET_SIZE];

// Objek UDP
WiFiUDP udp;

void setup() {
  Serial.begin(115200);

  // 1. Koneksi ke Wi-Fi dengan IP Statis
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, gateway, subnet); // Terapkan IP Statis
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke Wi-Fi ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTerhubung!");
  Serial.print("IP Address Penerima (STATIS): ");
  Serial.println(WiFi.localIP());

  // 2. Aktifkan DAC (Digital-to-Analog Converter)
  // DAC_CHANNEL_1 = GPIO 25
  dac_output_enable(DAC_CHANNEL_1);

  // 3. Mulai mendengarkan UDP di port yang ditentukan
  udp.begin(PORT);
  Serial.println("UDP Siap. Menunggu data...");
}

void loop() {
  // 1. Cek apakah ada paket UDP yang masuk
  int packetSize = udp.parsePacket();

  if (packetSize) {
    // 2. Baca paket ke dalam buffer
    int len = udp.read(audioBuffer, PACKET_SIZE);

    // 3. Mainkan buffer ke DAC (Speaker)
    for (int i = 0; i < len; i++) {
      // Tulis nilai 8-bit (0-255) ke DAC (GPIO 25)
      dac_output_voltage(DAC_CHANNEL_1, audioBuffer[i]);
      
      // Beri jeda sesuai sampling rate pengirim
      // delayMicroseconds(100); // (~10kHz sampling) - bisa di-uncomment jika perlu
    }
  }
}
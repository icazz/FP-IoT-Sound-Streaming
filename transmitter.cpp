/*
 * KODE PENGIRIM (TRANSMITTER)
 * - Membaca audio dari Analog Mic (ADC)
 * - Mengirim data audio via UDP
 */

#include <WiFi.h>
#include <WiFiUdp.h>

// --- Konfigurasi Jaringan ---
const char* ssid = "NAMA_WIFI_ANDA";
const char* password = "PASSWORD_WIFI_ANDA";

// IP statis dari perangkat PENERIMA (ESP32 Speaker)
// PASTIKAN INI DIISI DENGAN BENAR!
const char* IP_PENERIMA = "192.168.1.100";
const int PORT = 4444; // Port UDP (harus sama di kedua perangkat)
// -----------------------------

// --- Konfigurasi Hardware ---
const int MIC_PIN = 34; // Pin ADC untuk mikrofon (GPIO 34)
// -----------------------------

// Ukuran buffer audio per paket UDP
const int PACKET_SIZE = 256;
byte audioBuffer[PACKET_SIZE];

// Objek UDP
WiFiUDP udp;

void setup() {
  Serial.begin(115200);

  // 1. Koneksi ke Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke Wi-Fi ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nTerhubung!");
  Serial.print("IP Address Pengirim: ");
  Serial.println(WiFi.localIP());

  // 2. Siapkan UDP
  udp.begin(PORT);
  Serial.println("UDP Siap. Mulai mengirim...");
}

void loop() {
  // 1. Isi buffer audio
  for (int i = 0; i < PACKET_SIZE; i++) {
    // Baca data 12-bit ADC (0-4095)
    int adcValue = analogRead(MIC_PIN); 
    
    // Konversi ke 8-bit (0-255) untuk DAC di penerima
    audioBuffer[i] = map(adcValue, 0, 4095, 0, 255);
    
    // Beri sedikit jeda untuk sampling rate
    // delayMicroseconds(100); // (~10kHz sampling) - bisa di-uncomment jika perlu
  }

  // 2. Kirim paket UDP
  udp.beginPacket(IP_PENERIMA, PORT);
  udp.write(audioBuffer, PACKET_SIZE);
  udp.endPacket();
}
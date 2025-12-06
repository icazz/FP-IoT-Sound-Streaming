// penerima
#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/i2s.h> // Header Wajib untuk fungsi I2S

// --- Kredensial Wi-Fi ---
const char* ssid = "";
const char* password = "";

// Port UDP yang sama dengan pengirim
const unsigned int localPort = 4210; 
WiFiUDP Udp;

// Pin I2S untuk PCM5102A (Output Audio)
#define I2S_BCLK_PIN 6  // BCK (Bit Clock)
#define I2S_LRCK_PIN 7  // LCK (Word Select)
#define I2S_DATA_PIN 8  // DIN (Data Input)
#define I2S_DI_PIN   -1 // Tidak digunakan untuk output

// Parameter Audio (Harus sama dengan Pengirim)
#define SAMPLE_RATE 16000 
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define BUFFER_SIZE 512

// Buffer untuk menerima paket UDP
int16_t rx_buffer[BUFFER_SIZE / 2];

void setup() {
  Serial.begin(115200);

  // 1. Koneksi Wi-Fi
  WiFi.mode(WIFI_STA); // Mode Station
  
  // --- SOLUSI: TURUNKAN TX POWER ---
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected! Receiver IP: " + WiFi.localIP().toString());
  
  // 2. Mulai mendengarkan di port UDP
  Udp.begin(localPort);
  Serial.println("Listening on UDP port " + String(localPort));

  // 3. Konfigurasi I2S (sebagai Output/TX)
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), 
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = BITS_PER_SAMPLE,
    // FIX PENTING: Mengubah format dari stereo ke mono karena sumber (mic) adalah mono
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, 
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = BUFFER_SIZE / 2,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK_PIN, 
    .ws_io_num = I2S_LRCK_PIN,
    .data_out_num = I2S_DATA_PIN,
    .data_in_num = I2S_DI_PIN
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_MONO);
}

void loop() {
  // 4. Periksa paket UDP yang masuk
  int packetSize = Udp.parsePacket();
  
  if (packetSize) {
    // --- DEBUG 1: Verifikasi Penerimaan Paket ---
    Serial.print("✅ UDP Received: ");
    Serial.print(packetSize);
    Serial.print(" bytes from ");
    Serial.println(Udp.remoteIP());

    // 5. Baca paket UDP ke buffer
    int len = Udp.read((uint8_t*)rx_buffer, BUFFER_SIZE);
    
    // 6. Tulis data audio ke I2S untuk diputar oleh PCM5102A
    size_t bytes_written;
    i2s_write(I2S_NUM_0, (const char*)rx_buffer, len, &bytes_written, portMAX_DELAY);
    
    // --- DEBUG 2: Verifikasi Penulisan I2S ---
    if (bytes_written != len) {
        Serial.print(" ❌ ERROR: I2S Write Failed! Wrote only ");
        Serial.print(bytes_written);
        Serial.println(" bytes.");
    }

  } 
}
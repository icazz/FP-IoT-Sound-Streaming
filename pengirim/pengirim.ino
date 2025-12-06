// pengirim
#include <WiFi.h>
#include <WiFiUdp.h>
#include <driver/i2s.h> 

// --- Kredensial Wi-Fi ---
const char* ssid = "";
const char* password = "";

// --- IP Receiver ---
// PENTING: Pastikan IP ini sesuai dengan yang muncul di Serial Monitor Penerima!
// Jika ganti WiFi ke "samara", IP mungkin berubah (bukan 192.168.43.xxx lagi)
IPAddress remoteIP(192, 168, 1, 20); 
const unsigned int remotePort = 4210;
WiFiUDP Udp;

// Pin I2S untuk INMP441 (Input Audio)
#define I2S_BCLK_PIN 2  // SCK
#define I2S_LRCK_PIN 4  // WS
#define I2S_DATA_PIN 5  // SD
#define I2S_DO_PIN   -1 

// Parameter Audio
#define SAMPLE_RATE 16000 
#define BITS_PER_SAMPLE I2S_BITS_PER_SAMPLE_16BIT
#define BUFFER_SIZE 512    

int16_t sample_buffer[BUFFER_SIZE / 2]; 
size_t bytes_read;

void setup() {
  Serial.begin(115200);

  // 1. Koneksi Wi-Fi
  WiFi.mode(WIFI_STA); // Pastikan mode Station
  
  // --- SOLUSI: TURUNKAN TX POWER ---
  // Ini membantu stabilitas jika power supply kurang kuat
  WiFi.setTxPower(WIFI_POWER_8_5dBm); 
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nConnected! Sender IP: " + WiFi.localIP().toString());
  
  // 2. Konfigurasi I2S (sebagai Input/RX)
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = BITS_PER_SAMPLE,
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
    .data_out_num = I2S_DO_PIN, 
    .data_in_num = I2S_DATA_PIN
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BITS_PER_SAMPLE, I2S_CHANNEL_MONO);
  Serial.println("I2S Input Ready on GPIO 2, 4, 5.");
}

void loop() {
  // 3. Baca data dari INMP441 (512 bytes)
  i2s_read(I2S_NUM_0, (char*)sample_buffer, BUFFER_SIZE, &bytes_read, portMAX_DELAY);
  
  if (bytes_read > 0) {
    // --- DEBUG 1: Verifikasi Kualitas Data Audio ---
    long sum_of_samples = 0;
    for (int i = 0; i < (bytes_read / 2); i++) { // Iterasi per 16-bit sample
        sum_of_samples += abs(sample_buffer[i]); 
    }
    long avg_sample_value = sum_of_samples / (bytes_read / 2);

    if (avg_sample_value > 100) { 
        Serial.print("🔊 Audio OK! Avg Value: ");
        Serial.print(avg_sample_value);
    } else {
        static int silent_counter = 0;
        if (silent_counter++ % 100 == 0) { 
            Serial.print("Silence. Avg Value: ");
            Serial.println(avg_sample_value);
        }
    }
    
    // 4. Kirim data audio melalui UDP
    Udp.beginPacket(remoteIP, remotePort);
    Udp.write((uint8_t*)sample_buffer, bytes_read); 
    
    // --- DEBUG 2: Verifikasi Pengiriman UDP ---
    int send_status = Udp.endPacket(); 

    if (send_status) {
        // Serial.println(" | UDP Sent OK."); 
    } else {
        Serial.println(" | ❌ ERROR: Failed to send UDP packet! (Check Receiver IP)");
    }
  } else {
    Serial.println("!!! FATAL: I2S Read failed (0 bytes read). Check hardware/pins.");
    delay(100); 
  }
}
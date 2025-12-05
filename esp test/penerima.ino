// Penerima
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "ITS-WIFI-TW2";
const char* password = "itssurabaya";
const unsigned int localPort = 4210; 

WiFiUDP Udp;

char incomingPacket[255]; 

void setup() {
  Serial.begin(115200);
  delay(100);

  // Koneksi Wi-Fi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  // Tampilkan IP lokal. IP ini harus dimasukkan ke kode Sender!
  Serial.print("Receiver IP Address: ");
  Serial.println(WiFi.localIP()); 
  
  // Mulai mendengarkan UDP
  Udp.begin(localPort);
  Serial.print("Listening on UDP port: ");
  Serial.println(localPort);
}

void loop() {
  // Cek apakah ada paket UDP yang masuk
  int packetSize = Udp.parsePacket();
  
  if (packetSize) {
    // Baca paket
    int len = Udp.read(incomingPacket, 255);
    // Tambahkan null terminator di akhir
    incomingPacket[len] = 0; 

    // Cetak detail dan pesan
    Serial.print("\nReceived from ");
    Serial.print(Udp.remoteIP());
    Serial.print(":");
    Serial.print(Udp.remotePort());
    Serial.print(", Message: ");
    Serial.println(incomingPacket);
  }
}
// pengirim
#include <WiFi.h>
#include <WiFiUdp.h>


const char* ssid = "ITS-WIFI-TW2";
const char* password = "itssurabaya";
IPAddress remoteIP(192, 168, 1, 100); 
const unsigned int remotePort = 4210;

WiFiUDP Udp;
unsigned int counter = 0;

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
  Serial.print("Sender IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  String message = "Test #" + String(counter++);
  
  // Kirim paket UDP
  Udp.beginPacket(remoteIP, remotePort);
  Udp.print(message);
  Udp.endPacket();

  Serial.print("Sent message: ");
  Serial.println(message);

  delay(2000); // Kirim setiap 2 detik
}
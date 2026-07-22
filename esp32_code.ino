#define BLYNK_TEMPLATE_ID "TMPL31Pe2sjtP"
#define BLYNK_TEMPLATE_NAME "fire safety system"
#define BLYNK_AUTH_TOKEN "2nPlgj5PPPxbAKlLOLxrYHYd2vHBtW54"      // ← Replace later

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "Ashborn";       // ← Replace later
char pass[] = "12345678";   // ← Replace later

// ESP32 Serial2 → reads from Arduino
// Arduino D11 (TX) → ESP32 GPIO16 (RX2)
// Arduino D10 (RX) → ESP32 GPIO17 (TX2)
// Arduino GND      → ESP32 GND

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17); // RX=GPIO16, TX=GPIO17

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("ESP32 Ready. Listening for Arduino...");
}

void loop() {
  Blynk.run();

  if (Serial2.available()) {
    String message = Serial2.readStringUntil('\n');
    message.trim();

    Serial.print("Received from Arduino: ");
    Serial.println(message);

    if (message == "FIRE_ALERT") {
      // Real fire → Blynk notification + update label
      Blynk.virtualWrite(V0, "FIRE ALERT! All 3 sensors triggered!");
      Blynk.notify("fire_alert", "REAL FIRE! Motor ON. All sensors triggered!");

    } else if (message == "FAKE_FIRE") {
      // Fake fire → Blynk notification + update label
      Blynk.virtualWrite(V0, "FAKE FIRE! 1 or 2 sensors triggered!");
      Blynk.notify("fake_fire", "Fake Fire detected! Check the system.");

    } else if (message == "CLEAR") {
      // All clear → just update label
      Blynk.virtualWrite(V0, "All Clear. System Normal.");
    }
  }
}

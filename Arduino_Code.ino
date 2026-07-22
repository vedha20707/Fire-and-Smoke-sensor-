#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#define GAS_SENSOR_APIN   A0
#define FLAME_SENSOR_DPIN  6
#define BUZZER_PIN         8
#define DHT_PIN            2
#define DHT_TYPE        DHT11
#define MOTOR_PIN          9

SoftwareSerial espSerial(10, 11); // D10=RX, D11=TX → ESP32

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  pinMode(FLAME_SENSOR_DPIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);
  digitalWrite(MOTOR_PIN, LOW);
  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("  Safety System ");
  lcd.setCursor(0, 1);
  lcd.print("  Initializing..");
  delay(2000);
  lcd.clear();

  tone(BUZZER_PIN, 1000, 300);
  delay(400);
  tone(BUZZER_PIN, 1500, 300);
  delay(400);
  noTone(BUZZER_PIN);

  Serial.println("=========================================");
  Serial.println("       Safety System Ready               ");
  Serial.println("=========================================");
  Serial.println("  GAS AO         → A0");
  Serial.println("  FLAME DO       → D6");
  Serial.println("  DHT DATA       → D2");
  Serial.println("  BUZZER +       → D8");
  Serial.println("  MOTOR/RELAY    → D9");
  Serial.println("  LCD SDA        → A4");
  Serial.println("  LCD SCL        → A5");
  Serial.println("  ESP32 TX       → D10");
  Serial.println("  ESP32 RX       → D11");
  Serial.println("=========================================");
  delay(1000);
}

void sendToESP(String msg) {
  espSerial.println(msg);
  Serial.print("Sent to ESP32: ");
  Serial.println(msg);
}

void loop() {
  // --- Sensor Readings ---
  int gasRaw        = analogRead(GAS_SENSOR_APIN);
  float gasVoltage  = gasRaw * (5.0 / 1023.0);
  int flameDigital  = digitalRead(FLAME_SENSOR_DPIN);
  float temperature = dht.readTemperature();
  float humidity    = dht.readHumidity();

  // --- Alert Flags ---
  bool gasAlert   = gasRaw > 400;
  bool flameAlert = flameDigital == LOW;
  bool tempAlert  = (!isnan(temperature) && temperature > 50);

  // --- Count triggered sensors ---
  int triggeredCount = 0;
  if (gasAlert)   triggeredCount++;
  if (flameAlert) triggeredCount++;
  if (tempAlert)  triggeredCount++;

  // --- Serial Monitor ---
  Serial.println("------------------------------------------");
  Serial.print("[GAS]   Raw: ");
  Serial.print(gasRaw);
  Serial.print("  |  Voltage: ");
  Serial.print(gasVoltage, 2);
  Serial.print(" V  |  Status: ");
  Serial.println(gasAlert ? "GAS DETECTED!" : "Clear");
  Serial.print("[FLAME] Status: ");
  Serial.println(flameAlert ? "FLAME DETECTED!" : "No Flame");
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("[DHT]   Error reading sensor!");
  } else {
    Serial.print("[DHT]   Temp: ");
    Serial.print(temperature);
    Serial.print(" C  |  Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // ==========================================
  //            MAIN LOGIC
  // ==========================================

  if (triggeredCount == 3) {
    // REAL FIRE → Motor ON
    digitalWrite(MOTOR_PIN, HIGH);
    sendToESP("FIRE_ALERT");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("!!! FIRE ALERT !!");
    lcd.setCursor(0, 1);
    lcd.print(" GAS+FLAME+TEMP ");

    // Fast panic alarm
    tone(BUZZER_PIN, 2500); delay(150);
    noTone(BUZZER_PIN);     delay(100);
    tone(BUZZER_PIN, 1000); delay(150);
    noTone(BUZZER_PIN);     delay(100);

  } else if (triggeredCount == 1 || triggeredCount == 2) {
    // FAKE FIRE → Motor OFF
    digitalWrite(MOTOR_PIN, LOW);
    sendToESP("FAKE_FIRE");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("** FAKE  FIRE **");
    lcd.setCursor(0, 1);

    String line2 = "";
    if (gasAlert)   line2 += "GAS ";
    if (flameAlert) line2 += "FLM ";
    if (tempAlert)  line2 += "TMP ";
    while (line2.length() < 16) line2 += " ";
    lcd.print(line2);

    tone(BUZZER_PIN, 1800); delay(300);
    noTone(BUZZER_PIN);     delay(200);

  } else {
    // ALL CLEAR → Motor OFF
    digitalWrite(MOTOR_PIN, LOW);
    sendToESP("CLEAR");

    noTone(BUZZER_PIN);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("G:OK F:OK T:OK  ");
    lcd.setCursor(0, 1);
    if (!isnan(temperature)) {
      lcd.print("T:");
      lcd.print((int)temperature);
      lcd.print("C H:");
      lcd.print((int)humidity);
      lcd.print("%     ");
    } else {
      lcd.print("DHT Read Error  ");
    }
  }

  delay(1000);
}

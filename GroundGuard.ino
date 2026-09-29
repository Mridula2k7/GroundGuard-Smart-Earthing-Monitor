#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- PIN DEFINITIONS ----------------
#define CURRENT_PIN 34
#define VOLTAGE_PIN 35
#define MOISTURE_PIN 32

#define GREEN_LED 13
#define BUZZER 4

#define OLED_SDA 21
#define OLED_SCL 22

// ---------------- OLED CONFIGURATION ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ---------------- FUNCTION DECLARATIONS ----------------
void showOLED(
  String status,
  int health,
  int voltage,
  int moisture,
  int leakage
);

void healthyAlert();
void warningAlert();
void criticalAlert();
// SETUP
void setup() {
  Serial.begin(115200);

  // Configure pins
  pinMode(CURRENT_PIN, INPUT);
  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(MOISTURE_PIN, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BUZZER, LOW);

  // Initialize I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // Initialize OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS
      )) {

    Serial.println("OLED initialization failed!");

    while (true) {
      delay(1000);
    }
  }

  // Initial OLED screen
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("GROUND");

  display.setCursor(10, 32);
  display.println("GUARD");

  display.display();

  delay(2000);

  Serial.println("--------------------------------");
  Serial.println("GroundGuard Started");
  Serial.println("--------------------------------");
}
// MAIN LOOP
void loop() {
  // Read Analog Sensor Values
  int rawCurrent = analogRead(CURRENT_PIN);
  int rawVoltage = analogRead(VOLTAGE_PIN);
  int rawMoisture = analogRead(MOISTURE_PIN);
  // Convert ADC Values
  // ESP32 ADC range = 0 to 4095

  int leakage = map(
    rawCurrent,
    0,
    4095,
    0,
    100
  );

  int voltage = map(
    rawVoltage,
    0,
    4095,
    180,
    260
  );

  int moisture = map(
    rawMoisture,
    0,
    4095,
    0,
    100
  );
  // Calculate Individual Scores
  // Lower leakage gives a better score
  int currentScore = 100 - leakage;
  int voltageScore;

  if (voltage >= 220 && voltage <= 240) {

    voltageScore = 100;

  } else {

    voltageScore = 100 - abs(voltage - 230);

    if (voltageScore < 0) {
      voltageScore = 0;
    }
  }
  // Moisture score
  int moistureScore = moisture;
  // Calculate Overall Health Score
  float health =
      (0.40 * voltageScore) +
      (0.30 * currentScore) +
      (0.30 * moistureScore);

  int healthScore = (int)health;
  // Determine System Status

  String status;

  if (healthScore >= 90) {

    status = "HEALTHY";

  }
  else if (healthScore >= 60) {

    status = "WARNING";

  }
  else {

    status = "CRITICAL";
  }
  // Display Results in Serial Monitor
  Serial.println();
  Serial.println("========== GROUNDGUARD ==========");

  Serial.print("Raw Current   : ");
  Serial.println(rawCurrent);

  Serial.print("Raw Voltage   : ");
  Serial.println(rawVoltage);

  Serial.print("Raw Moisture  : ");
  Serial.println(rawMoisture);

  Serial.println("----------------------------------");

  Serial.print("Leakage       : ");
  Serial.println(leakage);

  Serial.print("Voltage       : ");
  Serial.print(voltage);
  Serial.println(" V");

  Serial.print("Moisture      : ");
  Serial.println(moisture);

  Serial.print("Health Score  : ");
  Serial.println(healthScore);

  Serial.print("Status        : ");
  Serial.println(status);

  Serial.println("==================================");
  // Display and Alert
  if (status == "HEALTHY") {
    showOLED(
      "HEALTHY",
      healthScore,
      voltage,
      moisture,
      leakage
    );
    healthyAlert();
  }
  else if (status == "WARNING") {
    showOLED(
      "WARNING",
      healthScore,
      voltage,
      moisture,
      leakage
    );
    warningAlert();
  }
  else {
    showOLED(
      "CRITICAL",
      healthScore,
      voltage,
      moisture,
      leakage
    );
    criticalAlert();
  }
  delay(2000);
}
// OLED DISPLAY FUNCTION
void showOLED(
  String status,
  int health,
  int voltage,
  int moisture,
  int leakage
) {

  display.clearDisplay();

  // Title
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(30, 0);
  display.println("GROUNDGUARD");

  // Separator
  display.drawLine(
    0,
    10,
    127,
    10,
    SSD1306_WHITE
  );
  // Status
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print("STATUS: ");
  display.println(status);
  // Health
  display.setCursor(0, 27);
  display.print("Health : ");
  display.print(health);
  display.println("%");
  // Voltage
  display.setCursor(0, 38);
  display.print("Volt   : ");
  display.print(voltage);
  display.println("V");
  // Moisture
  display.setCursor(0, 49);
  display.print("Moist  : ");
  display.print(moisture);
  // Leakage
  display.setCursor(70, 49);
  display.print("Leak:");
  display.print(leakage);
  display.display();
}
// HEALTHY ALERT
void healthyAlert() {
  // Green LED ON
  digitalWrite(GREEN_LED, HIGH);
  // Short healthy indication
  tone(BUZZER, 600);
  delay(200);
  noTone(BUZZER);
  digitalWrite(GREEN_LED, LOW);
  delay(200);
}
// WARNING ALERT
void warningAlert() {
  // Green LED OFF
  digitalWrite(GREEN_LED, LOW);

  // Three warning pulses
  for (int i = 0; i < 3; i++) {
    tone(BUZZER, 1000);
    delay(250);
    noTone(BUZZER);
    delay(150);
  }
}
// CRITICAL ALERT
void criticalAlert() {
  // Green LED OFF
  digitalWrite(GREEN_LED, LOW);
  // Three critical alert pulses
  for (int i = 0; i < 3; i++) {
    tone(BUZZER, 2000);
    delay(400);
    noTone(BUZZER);
    delay(150);
  }
}

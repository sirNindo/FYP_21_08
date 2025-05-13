
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);



void showPowerOutput(float voltage, float current, float power, const char* mode);


void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);  // Confirm your pins
  if (!display.begin(0x3C, true)) {
    Serial.println("Display not found");
    while (1);
  }


  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Hello, ESP32!");
  int biz = 100;
  display.print("Hello, ESP32!");
  display.print("Hello, ESP32!");
  display.display();
}

void loop() {

  float voltage = 12.5;    // from INA229 or ADC
  float current = 0.42;    // from sensor
  float power = voltage * current;
  const char* mode = "GEN"; // could also be "BRAKE", "IDLE", etc.

  showPowerOutput(voltage, current, power, mode);
  delay(1000); // refresh every second
  
}

void showPowerOutput(float voltage, float current, float power, const char* mode) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("Mode: ");
  display.println(mode);

  display.setTextSize(2); // Larger text for important metrics
  display.setCursor(0, 8);
  display.print("V: ");
  display.print(voltage, 2);
  display.println(" V");

  display.setCursor(0, 28);
  display.print("I: ");
  display.print(current, 2);
  display.println(" A");

  display.setCursor(0, 48);
  display.print("P: ");
  display.print(power, 1);
  display.println(" W");

  display.display();

}


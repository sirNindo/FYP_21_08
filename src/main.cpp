
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

TwoWire Wire2 = TwoWire(0);  // bus number 2, you can choose 1, 2, etc.
TwoWire Wire3 = TwoWire(3);  // bus number 2, you can choose 1, 2, etc.



void showPowerOutput(float voltage, float current, float power, const char* mode);


void setup() {
  Serial.begin(115200);
  Wire2.begin(32,33,100000);  // OLED Display I2C starting // Confirm your pins
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


/**
 * @brief Displays the real-time power output data on the OLED screen.
 * 
 * This function clears the screen and then prints voltage (V), current (A),
 * and power (W) values to the SSD1306 OLED display. It is designed for real-time
 * monitoring in power generation or energy tracking applications.
 * 
 * @param voltage The measured voltage in volts.
 * @param current The measured current in amperes.
 * @param power   The calculated power in watts.
 */

void showPowerOutput(float voltage, float current, float power, const char* mode) {
  // Clear the OLED display buffer
  display.clearDisplay();
  
  // Set text size and color
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // Display heading for the section
  display.setCursor(0, 0);
  display.print("Mode: ");
  display.println(mode);

  display.setTextSize(2); // Larger text for important metrics

  // Display voltage value (V)
  display.setCursor(0, 8);
  display.print("V: ");
  display.print(voltage, 2);
  display.println(" V");

  // Display current value (A)
  display.setCursor(0, 28);
  display.print("I: ");
  display.print(current, 2);
  display.println(" A");

  // Display calculated power (W)
  display.setCursor(0, 48);
  display.print("P: ");
  display.print(power, 1);
  display.println(" W");

  display.display();

}


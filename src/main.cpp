
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPIFFS.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <INA226.h>

// Pins
#define INA_SDA 26
#define INA_SCL 27
#define OLED_SDA 32
#define OLED_SCL 33

float voltage ;
float shuntMV ;
float current ; // Convert to A
float power;     // Convert to W

bool inaInitialized = false;
unsigned long lastInaRetry = 0;


// INA226 on custom I2C bus (Wire1)
TwoWire PwrSensor1 =TwoWire(0);
INA226 INA(0x40);  // Default I2C address is 0x40
// INA226 INA1(0x41);
TwoWire Viewer = TwoWire(0);  // bus number 2, you can choose 1, 2, etc.
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Viewer, -1); // I2C OLED object Initialisation



void showPowerOutput(float voltage, float current, float power, const char* mode);

void INAdebug(float voltage, float shuntMV, float current, float power);

void setup() {
  Serial.begin(115200);
  Viewer.begin(OLED_SDA,OLED_SCL,100000);  // OLED Display I2C starting // Confirm your pins
  if (!display.begin(0x3C, true)) {
    Serial.println("Display not found");
    while (1);
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Power Monitor Init");
  display.display();
  delay(1000);

  // Serial.println(__FILE__);
  // Serial.print("INA226_LIB_VERSION: ");
  // Serial.println(INA226_LIB_VERSION);

  // Wire.begin(19,18); //First Voltage sensor 
  PwrSensor1.begin(INA_SDA, INA_SCL, 100000); //First Voltage sensor 
  
  if (INA.begin()) {
    inaInitialized = true;
    Serial.println("INA0 connected successfully.");
    INA.setMaxCurrentShunt(1.0, 0.002);
  } else {
    Serial.println("INA0 could not connect. Will retry in loop...");
  }
  // if (!INA.begin() )
  // {
  //   Serial.println("INA0 could not connect. Fix and Reboot");
  // }

  // if (!INA0.begin()) {
  //   Serial.println("INA226 not found. Check wiring.");
  //   // while (1);
  // }
  
  // if (!INA1.begin() )
  // {
  //   Serial.println("INA1 could not connect. Fix and Reboot");
  // }

  INA.setMaxCurrentShunt(1.0, 0.002);

}

void loop() {

  const char* mode = "GEN"; // could also be "BRAKE", "IDLE", etc.

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.print("Pwr Monitor: ");
  display.print(mode);

  if (!inaInitialized) {
    if (millis() - lastInaRetry > 5000) {  // Retry every 5 seconds
      Serial.println("Retrying INA0...");
      if (INA.begin()) {
        inaInitialized = true;
        Serial.println("INA0 initialized on retry.");
        INA.setMaxCurrentShunt(1.0, 0.002);
      } else {
        Serial.println("INA0 still not found.");
      }
      lastInaRetry = millis();
    }

    // Show message on OLED
    display.println(" INA!");
    

  } else {
  
    INAdebug(voltage, shuntMV, current, power);
    
  }
  showPowerOutput(voltage, current, power, mode);
  
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
  

  display.setTextSize(2); // Larger text for important metrics

  // Display voltage value (V)
  display.setCursor(0, 9);
  display.print("V: ");
  display.print(voltage, 2);
  display.println(" V");

  // Display current value (A)
  display.setCursor(0, 29);
  display.print("I: ");
  display.print(current, 2);
  display.println(" A");

  // Display calculated power (W)
  display.setCursor(0, 49);
  display.print("P: ");
  display.print(power, 1);
  display.println(" W");

  display.display();

  delay(1000);

}

void INAdebug(float voltage, float shuntMV, float current, float power){
  Serial.println("\nBUS\tSHUNT\tCURRENT\tPOWER\t\tBUS\tSHUNT\tCURRENT\tPOWER");
  for (int i = 0; i < 20; i++)
  {

    voltage = INA.getBusVoltage();
    shuntMV = INA.getShuntVoltage_mV();
    current = INA.getCurrent_mA() / 1000.0; // Convert to A
    power = INA.getPower_mW() / 1000.0;     // Convert to W
  
    // Print to Serial Monitor
    Serial.print("V: "); Serial.print(voltage, 3); Serial.print(" V\t");
    Serial.print("V_s: "); Serial.print(shuntMV, 3); Serial.print(" V\t");
    Serial.print("I: "); Serial.print(current, 3); Serial.print(" A\t");
    Serial.print("P: "); Serial.print(power, 3); Serial.println(" W");
  
    // Serial.print(INA.getBusVoltage(), 3);
    // Serial.print("\t");
    // Serial.print(INA.getShuntVoltage_mV(), 3);
    // Serial.print("\t");
    // Serial.print(INA.getCurrent_mA(), 3);
    // Serial.print("\t");
    // Serial.print(INA.getPower_mW(), 3);
    // Serial.print("\t");
    // Serial.print("\t");
    // Serial.print(INA1.getBusVoltage(), 3);
    // Serial.print("\t");
    // Serial.print(INA1.getShuntVoltage_mV(), 3);
    // Serial.print("\t");
    // Serial.print(INA1.getCurrent_mA(), 3);
    // Serial.print("\t");
    // Serial.print(INA1.getPower_mW(), 3);
    // Serial.println();  
    delay(1000);
  }
}
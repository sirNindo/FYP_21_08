  #include <Wire.h>
  #include <WiFi.h>          // Uncommented
  #include <PubSubClient.h>  // Uncommented
  #include <ArduinoJson.h>   // Add this library for JSON support
  // #include <SPIFFS.h>     
  #include <Adafruit_GFX.h>
  #include <Adafruit_SH110X.h>
  #include <INA226.h>

  // Pins
  #define INA_SDA 18
  #define INA_SCL 19
  #define OLED_SDA 32
  #define OLED_SCL 33

  // WiFi credentials
  const char* ssid = ".......";
  const char* password = "RasmusHojlund11";

  // MQTT Broker settings - Use your PC's IP address
  const char* mqtt_server = "192.168.1.5";  // Replace with your PC's IP address
  const int mqtt_port = 1883;               // Default MQTT port
  const char* client_id = "ESP32_PowerMonitor";

  // Initialize WiFi and MQTT clients
  WiFiClient espClient;
  PubSubClient mqtt(espClient);

  float voltage;
  float shuntMV;
  float current; // Convert to A
  float power;   // Convert to W

  bool inaInitialized = false;
  unsigned long lastInaRetry = 0;
  unsigned long lastMqttPublish = 0;
  const int mqttPublishInterval = 5000; // Publish every 5 seconds

  // MQTT Topic - Single topic for all data
  const char* topic_data = "powermonitor/data";  // This is the correct declaration

  // INA226 on custom I2C bus (Wire1)
  TwoWire PwrSensor1 = TwoWire(0);
  INA226 INA(0x40);  // Default I2C address is 0x40
  // INA226 INA1(0x41);
  TwoWire Viewer = TwoWire(1);  // bus number 2, you can choose 1, 2, etc.
  Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Viewer, -1); // I2C OLED object Initialisation

  void showPowerOutput(float voltage, float current, float power, const char* mode);
  void INAdebug(float voltage, float shuntMV, float current, float power);
  void setupWifi();
  void reconnectMqtt();
  void publishJsonData(float voltage, float shuntMV, float current, float power, const char* mode);

  void setup() {
    Serial.begin(115200);
    Wire.begin(INA_SDA, INA_SCL); //First Voltage sensor 
    // PwrSensor1.begin(INA_SDA, INA_SCL); //First Voltage sensor

    if (!INA.begin()) {
      Serial.println("INA0 could not connect. Will retry in loop...");
      inaInitialized = false;
    } else {
      inaInitialized = true;
      Serial.println("INA0 connected successfully.");
      INA.setMaxCurrentShunt(1.0, 0.002);
    }
    INA.setMaxCurrentShunt(1.0, 0.002);
  
    Viewer.begin(OLED_SDA, OLED_SCL, 100000);  // OLED Display I2C starting // Confirm your pins
    if (!display.begin(0x3C, true)) {
      Serial.println("Display not found");
      // while (1);
    }

    display.clearDisplay();
    display.setTextColor(SH110X_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Power Monitor Init");
    display.display();
    delay(1000);

    // Setup WiFi and MQTT
    setupWifi();
    mqtt.setServer(mqtt_server, mqtt_port);
  }

  void loop() {
    const char* mode = "GEN"; // could also be "BRAKE", "IDLE", etc.

    // Check MQTT connection
    if (!mqtt.connected()) {
      reconnectMqtt();
    }
    mqtt.loop();

    voltage = INA.getBusVoltage();
    shuntMV = INA.getShuntVoltage_mV();
    current = INA.getCurrent_mA() / 1000.0; // Convert to A
    power = INA.getPower_mW() / 1000.0;     // Convert to W
  
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.print("Pwr Monitor: ");
    display.print(mode);

    if (!inaInitialized) {
      if (millis() - lastInaRetry > 5000) {  // Retry every 5 seconds
        Serial.println("Retrying INA0...");
        if (!INA.begin()) {
          Serial.println("INA0 still not found.");
        } else {
          inaInitialized = true;
          Serial.println("INA0 initialized on retry.");
          INA.setMaxCurrentShunt(1.0, 0.002);
        }
        lastInaRetry = millis();
      }

      // Show message on OLED
      display.println(" INA!");
      INAdebug(voltage, shuntMV, current, power);
    } else {
      INAdebug(voltage, shuntMV, current, power);
    }
  
    showPowerOutput(voltage, current, power, mode);
  
    // Publish data to MQTT broker periodically
    if (millis() - lastMqttPublish > mqttPublishInterval) {
      publishJsonData(voltage, shuntMV, current, power, mode);
      lastMqttPublish = millis();
    }
  }
  
  void setupWifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting to WiFi");
    display.println(ssid);
    display.display();
  
    WiFi.begin(ssid, password);
    
    // Set a flag to indicate we're connecting to WiFi
    bool wifiConnecting = true;
    unsigned long wifiStartTime = millis();
    unsigned long lastDotTime = 0;
    
    // Don't block the main loop while connecting
    while (wifiConnecting) {
      // Check if connected
      if (WiFi.status() == WL_CONNECTED) {
        wifiConnecting = false;
        break;
      }
      
      // Add a dot every 500ms
      if (millis() - lastDotTime > 500) {
        Serial.print(".");
        display.print(".");
        display.display();
        lastDotTime = millis();
      }
      
      // Check for timeout (30 seconds)
      if (millis() - wifiStartTime > 30000) {
        Serial.println("\nWiFi connection timeout!");
        display.println("\nTimeout!");
        display.display();
        wifiConnecting = false;
        break;
      }
      
      // Allow other tasks to run
      yield();
    }
  
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
  
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WiFi connected!");
      display.print("IP: ");
      display.println(WiFi.localIP());
      display.display();
      delay(2000);
    } else {
      Serial.println("\nFailed to connect to WiFi");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("WiFi connection");
      display.println("failed!");
      display.display();
      delay(2000);
    }
  }
  

  void reconnectMqtt() {
    // Loop until we're reconnected
    int attempts = 0;
    while (!mqtt.connected() && attempts < 3) { // Limit reconnection attempts
      attempts++;
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (mqtt.connect(client_id)) {
        Serial.println("connected");
      
        // Once connected, publish an announcement
        mqtt.publish("powermonitor/status", "Power Monitor Online");
      
        // Subscribe to command topic if needed
        // mqtt.subscribe("powermonitor/commands");
      } else {
        Serial.print("failed, rc=");
        Serial.print(mqtt.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }

  void publishJsonData(float voltage, float shuntMV, float current, float power, const char* mode) {
    // Only publish if we're connected to MQTT
    if (!mqtt.connected()) {
      return;
    }
  
    // Create a JSON document
    StaticJsonDocument<100> doc;
  
    // Add data to JSON document with abbreviated field names
    doc["t"] = millis();        // timestamp
    doc["V"] = voltage;         // voltage
    doc["V_s"] = shuntMV;       // shunt millivolts
    doc["I"] = current;         // current
    doc["P"] = power;           // power
  
    // Serialize JSON to string
    char jsonBuffer[100];
    serializeJson(doc, jsonBuffer);
  
    // Publish JSON string to MQTT
    mqtt.publish(topic_data, jsonBuffer);
  
    Serial.print("Published JSON: ");
    Serial.println(jsonBuffer);
  }

  
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

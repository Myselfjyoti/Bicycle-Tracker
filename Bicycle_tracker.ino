#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>

// HC-05 Bluetooth on pins 10 (RX) and 11 (TX)
#define BT_RX 10 // Arduino RX (connects to HC-05 TX)
#define BT_TX 11 // Arduino TX (connects to HC-05 RX, use voltage divider)
SoftwareSerial btSerial(BT_RX, BT_TX); // RX, TX

// GPS module on AltSoftSerial (pins 8 and 9 on UNO)
AltSoftSerial neogps;
TinyGPSPlus gps;

// State variables
boolean ignition_status = false;
boolean tracking_status = false;
boolean reply_status = true;
boolean anti_theft = false;
unsigned long previousMillis = 0;
const long interval = 60000; // 1 minute

void setup() {
    delay(2000);
    Serial.begin(9600);
    btSerial.begin(9600); // HC-05 default baud rate
    neogps.begin(9600); // GPS module baud rate
    Serial.println("System Initialized");
    btSerial.println("Bicycle Tracker Ready");
}

void loop() {
    checkBluetoothCommands();
    handleTracking();
}

void checkBluetoothCommands() {
    if (btSerial.available()) {
        String command = btSerial.readStringUntil('\n');
        command.trim();
        command.toLowerCase();
        Serial.print("Received command: ");
        Serial.println(command);

        if (command == "find location") {
            sendGpsLocation();
        }
        else if (command == "anti theft on") {
            anti_theft = true;
            if (reply_status) {
                btSerial.println("Anti-Theft Mode: ON");
            }
        }
        else if (command == "anti theft off") {
            anti_theft = false;
            if (reply_status) {
                btSerial.println("Anti-Theft Mode: OFF");
            }
        }
        else if (command == "reply on") {
            reply_status = true;
            btSerial.println("Reply: ON");
        }
        else if (command == "reply off") {
            reply_status = false;
        }
        else if (command == "tracking on") {
            tracking_status = true;
            if (reply_status) {
                btSerial.println("Live Tracking: ON");
            }
        }
        else if (command == "tracking off") {
            tracking_status = false;
            if (reply_status) {
                btSerial.println("Live Tracking: OFF");
            }
        }
        else if (command == "tracking status") {
            btSerial.print("Live Tracking: ");
            btSerial.println(tracking_status ? "ON" : "OFF");
        }
    }
}

void handleTracking() {
    ignition_status = getIgnitionStatus(); // You can implement this as needed
    if (tracking_status && ignition_status) {
        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis > interval) {
            previousMillis = currentMillis;
            sendGpsLocation();
        }
    }
}

void sendGpsLocation() {
    bool newData = false;
    for (unsigned long start = millis(); millis() - start < 10000;) {
        while (neogps.available()) {
            if (gps.encode(neogps.read())) {
                newData = true;
            }
        }
        if (newData) break;
    }

    if (newData && gps.location.isValid()) {
        String location = "Latitude: " + String(gps.location.lat(), 6) + "\n";
        location += "Longitude: " + String(gps.location.lng(), 6) + "\n";
        location += "https://maps.google.com/maps?q=loc:";
        location += String(gps.location.lat(), 6) + ",";
        location += String(gps.location.lng(), 6);
        btSerial.println(location);
        Serial.println("Location sent via Bluetooth");
    } else {
        btSerial.println("Location not available");
        Serial.println("Failed to get GPS fix");
    }
}

// Dummy function for ignition status (customize as needed)
boolean getIgnitionStatus() {
    // For demo, always return true. Replace with your sensor logic.
    return true;
}

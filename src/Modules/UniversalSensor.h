#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "../Config.h"
#include "../Core/Types.h"



class UniversalSensor {
private:
    int zoneIndex;
    int pinRX;

    
    SensorType lockedMode = SENS_UNKNOWN; 
    
    int analogDry = 4095;
    int analogWet = 1500;

    Preferences prefs;

public:
    UniversalSensor(int index, int rx, int tx = -1) 
        : zoneIndex(index), pinRX(rx) {}

    void begin() {
        pinMode(pinRX, INPUT); 
        String nvsName = "sensor_" + String(zoneIndex);
        prefs.begin(nvsName.c_str(), false);
        analogDry = prefs.getInt("a_dry", 4095);
        analogWet = prefs.getInt("a_wet", 1500);
        

        detectSensorType(true); 
    }

    void update() {

        if (lockedMode == SENS_ANALOG) {

        } 
    }

    void forceDetect() {
        detectSensorType(true);
    }

    int getValue() {
        if (lockedMode == SENS_ANALOG) {
            int raw = analogRead(pinRX);

            return constrain(map(raw, analogDry, analogWet, 0, 100), 0, 100);
        }
        return 0;
    }
    
    int getRaw() {
        return analogRead(pinRX);
    }

    String getModeString() {
        if (lockedMode == SENS_ANALOG) return "Capacitive (Analog)";
        return "No Sensor"; 
    }

private:

    bool isFloatingDeepCheck() {
        pinMode(pinRX, INPUT_PULLUP); 
        delay(10); 
        int valHigh = analogRead(pinRX);
        
        pinMode(pinRX, INPUT_PULLDOWN); 
        delay(10); 
        int valLow = analogRead(pinRX);
        
        pinMode(pinRX, INPUT); 
        int swing = abs(valHigh - valLow);
        if (swing > 3000) return true; 
        

        if (valLow > 4000 || valHigh < 100) return true; 

        return false;
    }

    void detectSensorType(bool isHardCheck = false) {

        if (isHardCheck) {
            if (isFloatingDeepCheck()) {
                lockedMode = SENS_UNKNOWN;
                Serial.printf("Zone %d: No Sensor Detected (Floating)\n", zoneIndex);
            } else {
                lockedMode = SENS_ANALOG;
                Serial.printf("Zone %d: Analog Sensor Detected\n", zoneIndex);
            }
        }
    }
};
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

enum PlantType { TYPE_GENERAL, TYPE_DRY, TYPE_WET };
enum SensorType { SENS_UNKNOWN, SENS_RADAR, SENS_ANALOG };

struct EnvData {
    float temp = 0.0; float hum = 0.0; float vpd = 0.0;
    bool isValid() { return !isnan(temp) && !isnan(hum) && temp > -40; }
};

struct Plant {
    int id;
    String name;
    String type;
    int originalIndex;
    int threshold;
    int duration;
    int currentMoisture;
    bool errorStatus;
    bool isWatering;
    String aiResult;
    String sensorMode;

    // Fixed Array (Memory Safe)
    int history[6]; 

    Plant() {
        id = 0; threshold = 40; duration = 5;
        currentMoisture = 0; errorStatus = false; isWatering = false;
        originalIndex = -1;
        sensorMode = "Searching..."; // Default State
        for(int i=0; i<6; i++) history[i] = 0;
    }

    // Shift Register Helper
    void addHistory(int val) {
        for(int i=0; i<5; i++) history[i] = history[i+1];
        history[5] = val;
    }
};

struct RadarState {
    bool isStable;         
    bool isStandby;         
    int currentEnergy;      
    int baselineEnergy;     
    int wetThreshold;       
    int dryThreshold;       
    
    // Constructor
    RadarState() {
        isStable = false;
        isStandby = false;
        currentEnergy = 0;
        baselineEnergy = 0;
        wetThreshold = 100; // Default dummy
        dryThreshold = 0;
    }
};
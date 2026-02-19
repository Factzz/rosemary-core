#pragma once
#include <Arduino.h>
#include <vector>
#include <deque> 
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../Config.h"
#include "../Core/Types.h"
#include "Buzzer.h"

class PlantManager; 
extern PlantManager* sysPlants; 

class PlantManager {
private:
    std::vector<Plant> plants;
    Buzzer* buzzer;
    
    // Sequential Watering
    int activePumpIndex = -1;             
    std::deque<int> waterQueue;           
    
    unsigned long pumpStartTime = 0;      
    unsigned long lastAutoWaterTime[MAX_PLANTS] = {0};

public:
    PlantManager(Buzzer* b) : buzzer(b) {
        sysPlants = this; 
    }

    void begin() {
        if(!LittleFS.begin(true)) Serial.println("FS Error");
        for(int i=0; i<4; i++) {
            pinMode(PINS_PUMP[i], OUTPUT);
            digitalWrite(PINS_PUMP[i], LOW);
        }
        loadPlants();
        randomSeed(analogRead(0) + millis());
    }

    void loop() {
        unsigned long now = millis();
        bool anySensorCritical = false;

        // 1. Check Error Status
        for(auto &p : plants) {
            if(p.errorStatus) {
                anySensorCritical = true;
               
                if (activePumpIndex == p.originalIndex) stopWatering();
               
            }
        }

        // 2. Alert System
        if(anySensorCritical) {
            buzzer->setError(true);
            buzzer->setAlarm(false);
        } else {
            buzzer->setError(false);
            
            // 3. Auto Water Check
            for(auto &p : plants) {
                int i = p.originalIndex;
               
                bool cooldownOK = (now - lastAutoWaterTime[i] > AUTO_WATER_COOLDOWN);
                bool needWater = (p.threshold > 0 && p.currentMoisture < p.threshold);
                bool notInQueue = true;
                
                if (activePumpIndex != i && cooldownOK && needWater && !p.errorStatus && p.currentMoisture > 0) {
                    
                    requestWatering(i);
                }
            }
        }
        
        // 4. [CORE] Sequential Watering Processor 
        processWateringQueue(now);

        // 5. History Log
        static unsigned long lastLog = 0;
        if (now - lastLog >= HISTORY_LOG_MS) { 
            lastLog = now;
            for(auto &p : plants) p.addHistory(p.currentMoisture);
            savePlants(true); 
            Serial.println("History Logged & Saved.");
        }
    }
    
    void requestWatering(int index) {
        for(int q : waterQueue) if(q == index) return;
        if(activePumpIndex == index) return; 

        waterQueue.push_back(index);
        Serial.printf("Plant %d added to water queue.\n", index);
    }

    void processWateringQueue(unsigned long now) {
        if (activePumpIndex != -1) {
            Plant* p = nullptr;
            for(auto &plant : plants) if(plant.originalIndex == activePumpIndex) p = &plant;
            
            if (p) {
                if (now - pumpStartTime >= (p->duration * 1000)) {
                    stopWatering();
                }
            } else {
                stopWatering(); 
            }
        } 

        else if (!waterQueue.empty()) {
            int nextIdx = waterQueue.front();
            waterQueue.pop_front();
            startWatering(nextIdx);
        }
    }

    void startWatering(int index) {
        if(index < 0 || index >= 4) return;
        
        activePumpIndex = index;
        pumpStartTime = millis();
        
        digitalWrite(PINS_PUMP[index], HIGH);
        buzzer->beep();
        

        for(auto &p : plants) if(p.originalIndex == index) p.isWatering = true;
        
        Serial.printf("Pump %d STARTED (Sequential)\n", index);
    }

    void stopWatering() {
        if(activePumpIndex != -1) {
            digitalWrite(PINS_PUMP[activePumpIndex], LOW);
            
            for(auto &p : plants) if(p.originalIndex == activePumpIndex) {
                p.isWatering = false;
                lastAutoWaterTime[activePumpIndex] = millis();
            }
            
            Serial.printf("Pump %d STOPPED\n", activePumpIndex);
            activePumpIndex = -1; 
        }
    }
    
    void activatePump(int index) {
        requestWatering(index);
    }

    std::vector<Plant>& getPlants() { return plants; }
    bool hasPlant(int id) { for(const auto &p : plants) if(p.id == id) return true; return false; }
    bool deletePlant(int id) {
        auto it = std::remove_if(plants.begin(), plants.end(), [id](const Plant& p){ return p.id == id; });
        if (it != plants.end()) { plants.erase(it, plants.end()); savePlants(true); return true; } return false;
    }
    bool addPlant(String name, String type, int threshold) {
        if(plants.size() >= MAX_PLANTS) return false;
        int targetIdx = -1; bool slotTaken[4] = {false};
        for(auto &p : plants) if(p.originalIndex >=0 && p.originalIndex <4) slotTaken[p.originalIndex] = true;
        for(int i=0; i<4; i++) if(!slotTaken[i]) { targetIdx = i; break; }
        if(targetIdx == -1) return false;
        Plant p; p.id = random(10000, 99999); p.name = name; p.type = type; p.threshold = threshold; p.originalIndex = targetIdx;
        plants.push_back(p); savePlants(true); buzzer->beep(); return true;
    }
    void savePlants(bool force=false) {
        DynamicJsonDocument doc(24000); JsonArray arr = doc.to<JsonArray>();
        for (const auto &p : plants) {
            JsonObject obj = arr.createNestedObject();
            obj["id"] = p.id; obj["name"] = p.name; obj["type"] = p.type; obj["threshold"] = p.threshold; 
            obj["ai"] = p.aiResult; obj["idx"] = p.originalIndex; obj["dur"] = p.duration;
            JsonArray hist = obj.createNestedArray("hist"); for(int v : p.history) hist.add(v);
        }
        File file = LittleFS.open("/plants.json", "w"); serializeJson(doc, file); file.close();
    }
    void loadPlants() {
        if (!LittleFS.exists("/plants.json")) return;
        File file = LittleFS.open("/plants.json", "r");
        DynamicJsonDocument doc(24000); deserializeJson(doc, file); file.close();
        plants.clear();
        for (JsonObject obj : doc.as<JsonArray>()) {
            Plant p; p.id = obj["id"]; p.name = obj["name"].as<String>(); p.type = obj["type"].as<String>();
            p.threshold = obj["threshold"]; p.aiResult = obj["ai"] | ""; p.originalIndex = obj["idx"] | -1; p.duration = obj["dur"] | 5;
            if(p.originalIndex == -1) p.originalIndex = plants.size();
            if(obj.containsKey("hist")) { int h=0; for(int v : obj["hist"].as<JsonArray>()) { if(h<6) p.history[h++] = v; } }
            plants.push_back(p);
        }
    }
    bool updateConfig(int id, int threshold, int duration) {
        for(auto &p : plants) { if(p.id == id) {
            if(threshold >= 0) p.threshold = constrain(threshold, 0, 100);
            if(duration > 0) p.duration = constrain(duration, 1, 60);
            savePlants(true); return true;
        }} return false;
    }
};
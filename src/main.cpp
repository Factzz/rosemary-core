#include <Arduino.h>
#include "Config.h"
#include "Core/Types.h"
#include "Modules/Buzzer.h"
#include "Modules/UniversalSensor.h"
#include "Modules/PlantManager.h"
#include "Modules/SensorHub.h"
#include "Core/Network.h"
// #include "Core/HarborMesh.h" // [REMOVED] Core version has no Mesh

Buzzer buzzer; 
PlantManager* sysPlants = nullptr;
PlantManager plantManager(&buzzer);
SensorHub sensorHub;
NetworkManager network(&plantManager, &sensorHub, &buzzer);

UniversalSensor sensors[4] = {
    UniversalSensor(0, PINS_SENSOR[0]),
    UniversalSensor(1, PINS_SENSOR[1]),
    UniversalSensor(2, PINS_SENSOR[2]),
    UniversalSensor(3, PINS_SENSOR[3])
};

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n>>> Rosemary Core Booting...");

    buzzer.begin();
    
    for(int i=0; i<4; i++) {
        sensors[i].begin();
    }
    
    plantManager.begin();
    sensorHub.begin();
    
    network.begin();
    // harbor.begin(); // [REMOVED]

    Serial.println(">>> System Ready (Analog Mode)");
    buzzer.beep();
}

void loop() {
    unsigned long now = millis();

    network.update();
    plantManager.loop();
    sensorHub.updateEnv(); 
    buzzer.update();
    // harbor.loop(); // [REMOVED]

    // Sensor Loop
    static int currentSensorIdx = 0;
    static unsigned long lastSensorRead = 0;
    
    if (now - lastSensorRead > 50) {
        lastSensorRead = now;
        
        sensors[currentSensorIdx].update();
        
        std::vector<Plant>& plants = plantManager.getPlants();
        for(auto &p : plants) {
            if(p.originalIndex == currentSensorIdx) {
                p.currentMoisture = sensors[currentSensorIdx].getValue();
                p.sensorMode = sensors[currentSensorIdx].getModeString();
                
                
                if (p.sensorMode.indexOf("Analog") >= 0) {
                    p.errorStatus = false; 
                } else {
                    p.errorStatus = true;
                }
            }
        }

        currentSensorIdx++;
        if (currentSensorIdx >= 4) currentSensorIdx = 0;
    }
}
#pragma once
#include <Arduino.h>
#include <DHT.h>
#include "../Config.h"
#include "../Core/Types.h"

class SensorHub {
private:
    DHT dht;
    EnvData currentEnv;
    unsigned long lastEnvUpdate = 0;

public:
    SensorHub() : dht(PIN_DHT, DHT_TYPE) {}

    void begin() {
        dht.begin();
    }

    void updateEnv() {
        unsigned long now = millis();
        if (now - lastEnvUpdate < ENV_UPDATE_MS) return;
        lastEnvUpdate = now;

        float t = dht.readTemperature();
        float h = dht.readHumidity();

        if (!isnan(t) && !isnan(h)) {
            currentEnv.temp = t;
            currentEnv.hum = h;
            
            float svp = 0.61078 * exp((17.27 * t) / (t + 237.3));
            float vpd = svp * (1.0 - (h / 100.0));
            currentEnv.vpd = vpd;
        }
    }

    EnvData getEnv() {
        return currentEnv;
    }
};
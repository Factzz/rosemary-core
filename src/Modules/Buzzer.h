#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "../Config.h"

class Buzzer {
public:
    int pin;
    unsigned long lastUpdate = 0;
    int state = 0;
    bool alarmActive = false;
    bool errorActive = false;
    bool _dndMode = false;

    Preferences prefs;

public:
    Buzzer() : pin(PIN_BUZZER) {}

    void begin() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);

        prefs.begin("sys_settings", false);
        _dndMode = prefs.getBool("dnd", false);
    }

    void setDND(bool enable) { _dndMode = enable;
    prefs.putBool("dnd", enable);
    }
    
    bool isDND() { return _dndMode; }

    void beep() {
        if(_dndMode) return;
        state = 1; lastUpdate = millis(); digitalWrite(pin, HIGH);
    }

    void ready() {
        state = 2; lastUpdate = millis(); digitalWrite(pin, HIGH);
    }

    void setAlarm(bool active) {
        if (active && !alarmActive && !errorActive) {
            alarmActive = true; state = 10; lastUpdate = millis(); digitalWrite(pin, HIGH);
        } else if (!active && alarmActive) {
            alarmActive = false; state = 0; digitalWrite(pin, LOW);
        }
    }

    void setError(bool active) {
        if (active && !errorActive) {
            errorActive = true; alarmActive = false; state = 20; lastUpdate = millis(); digitalWrite(pin, HIGH);
        } else if (!active && errorActive) {
            errorActive = false; state = 0; digitalWrite(pin, LOW);
        }
    }

    void update() {
        unsigned long now = millis();
        switch(state) {
            case 0: break;
            case 1: if(now-lastUpdate>150) { digitalWrite(pin,LOW); state=0; } break;
            case 2: if(now-lastUpdate>100) { digitalWrite(pin,LOW); state=3; lastUpdate=now; } break;
            case 3: if(now-lastUpdate>100) { digitalWrite(pin,HIGH); state=4; lastUpdate=now; } break;
            case 4: if(now-lastUpdate>100) { digitalWrite(pin,LOW); state=0; } break;
            case 10: if(now-lastUpdate>500) { digitalWrite(pin,LOW); state=11; lastUpdate=now; } break;
            case 11: if(now-lastUpdate>2000) { digitalWrite(pin,HIGH); state=10; lastUpdate=now; } break;
            case 20: if(now-lastUpdate>100) { digitalWrite(pin,LOW); state=21; lastUpdate=now; } break;
            case 21: if(now-lastUpdate>100) { digitalWrite(pin,HIGH); state=22; lastUpdate=now; } break;
            case 22: if(now-lastUpdate>100) { digitalWrite(pin,LOW); state=23; lastUpdate=now; } break;
            case 23: if(now-lastUpdate>100) { digitalWrite(pin,HIGH); state=24; lastUpdate=now; } break;
            case 24: if(now-lastUpdate>100) { digitalWrite(pin,LOW); state=25; lastUpdate=now; } break;
            case 25: if(now-lastUpdate>2000) { digitalWrite(pin,HIGH); state=20; lastUpdate=now; } break;
        }
    }
};
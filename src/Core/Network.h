#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "../Config.h"
#include "../Modules/PlantManager.h"
#include "../Modules/SensorHub.h"
#include "../Modules/Buzzer.h"
#include "../Modules/UniversalSensor.h" 

extern UniversalSensor sensors[4]; 

class NetworkManager {
private:
    AsyncWebServer server;
    DNSServer dnsServer;
    Preferences wifiPrefs;
    PlantManager* plantMgr;
    SensorHub* sensorHub;
    Buzzer* buzzer;
    bool wifiConnected = false;
    String currentSSID = "";
    unsigned long lastWifiCheck = 0;

public:
    NetworkManager(PlantManager* p, SensorHub* s, Buzzer* b) 
        : server(80), plantMgr(p), sensorHub(s), buzzer(b) {}

    void begin() {
        wifiPrefs.begin("wifi", false);
        String ssid = wifiPrefs.getString("ssid", "");
        String pass = wifiPrefs.getString("pass", "");
        if(ssid == "") { setupAP(); } 
        else { WiFi.mode(WIFI_STA); WiFi.begin(ssid.c_str(), pass.c_str()); Serial.println("Connecting..."); }
        setupRoutes();
        server.begin();
    }

    void update() {
        dnsServer.processNextRequest();
        unsigned long now = millis();
        if (WiFi.status() != WL_CONNECTED) {
            if (wifiPrefs.getString("ssid", "") != "" && (now - lastWifiCheck >= WIFI_CHECK_MS)) {
                lastWifiCheck = now; WiFi.disconnect(); WiFi.reconnect();
            }
            wifiConnected = false;
        } else {
            if(!wifiConnected) {
                wifiConnected = true; currentSSID = WiFi.SSID();
                Serial.println("WiFi Connected: " + WiFi.localIP().toString());
                buzzer->ready();
            }
        }
    }

private:
    void setupAP() {
        WiFi.softAP(AP_SSID_DEFAULT);
        dnsServer.start(53, "*", WiFi.softAPIP());
    }

    void setupRoutes() {
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){ req->send(LittleFS, "/index.html", "text/html"); });
        server.serveStatic("/", LittleFS, "/");

        // [CORE API] Safe Memory Allocation
        server.on("/api/data", HTTP_GET, [this](AsyncWebServerRequest *req){
            DynamicJsonDocument *doc = new DynamicJsonDocument(16000); 
            if (!doc) { req->send(500, "text/plain", "OOM"); return; }

            JsonArray plantsArr = doc->createNestedArray("plants");
            std::vector<Plant>& plants = plantMgr->getPlants();
            for(auto &p : plants) {
                JsonObject obj = plantsArr.createNestedObject();
                obj["id"] = p.id; obj["name"] = p.name; obj["type"] = p.type;
                obj["threshold"] = p.threshold; obj["moisture"] = p.currentMoisture;
                obj["sensor_mode"] = p.sensorMode; obj["error"] = p.errorStatus;
                obj["originalIndex"] = p.originalIndex; obj["duration"] = p.duration;
                obj["is_watering"] = p.isWatering;
            }

            (*doc)["wifi_connected"] = wifiConnected;
            (*doc)["ssid"] = wifiConnected ? currentSSID : AP_SSID_DEFAULT;
            (*doc)["ip"] = wifiConnected ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
            (*doc)["uptime"] = millis()/1000;
            
            EnvData env = sensorHub->getEnv();
            (*doc)["env"]["temp"] = env.temp; (*doc)["env"]["hum"] = env.hum; (*doc)["env"]["vpd"] = env.vpd;
            (*doc)["dnd"] = buzzer->isDND();
            
            String res; serializeJson(*doc, res); 
            req->send(200, "application/json", res);
            delete doc;
        });

        server.on("/api/water", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL, 
            [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total){
                DynamicJsonDocument doc(128); deserializeJson(doc, data);
                if(doc.containsKey("index")) { plantMgr->activatePump(doc["index"]); req->send(200,"text/plain","OK"); }
                else req->send(400,"text/plain","Error");
            });

        server.on("/api/add-plant", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total){ DynamicJsonDocument doc(1024); deserializeJson(doc, data); bool success = plantMgr->addPlant(doc["name"].as<String>(), doc["type"].as<String>(), doc["threshold"].as<int>()); if(success) req->send(200,"text/plain","OK"); else req->send(400,"text/plain","Error"); });
        server.on("/api/update-config", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total){ DynamicJsonDocument doc(512); deserializeJson(doc, data); int id = doc["id"]; int threshold = doc.containsKey("threshold") ? doc["threshold"].as<int>() : -1; int duration = doc.containsKey("duration") ? doc["duration"].as<int>() : -1; if(plantMgr->updateConfig(id, threshold, duration)) req->send(200, "text/plain", "Updated"); else req->send(404, "text/plain", "Not Found"); });
        server.on("/api/delete-plant", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total){ DynamicJsonDocument doc(256); deserializeJson(doc,data); if(plantMgr->deletePlant(doc["id"])) req->send(200,"text/plain","Deleted"); else req->send(404,"text/plain","Not Found"); });
        
        server.on("/api/scan", HTTP_GET, [](AsyncWebServerRequest *req){ int n = WiFi.scanComplete(); if(n == -2) { WiFi.scanNetworks(true); req->send(200, "application/json", "[]"); } else if(n == -1) { req->send(200, "application/json", "[]"); } else { String json = "["; for(int i=0; i<n; ++i){ if(i) json += ","; json += "{\"ssid\":\""+WiFi.SSID(i)+"\",\"secure\":"+(WiFi.encryptionType(i)!=WIFI_AUTH_OPEN)+"}"; } json += "]"; WiFi.scanDelete(); req->send(200, "application/json", json); } });
        server.on("/api/save-wifi", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL, [this](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total){ DynamicJsonDocument doc(512); deserializeJson(doc,data); wifiPrefs.putString("ssid", doc["ssid"].as<String>()); wifiPrefs.putString("pass", doc["password"].as<String>()); req->send(200,"text/plain","Saved"); delay(1000); ESP.restart(); });
        server.on("/api/reboot", HTTP_POST, [](AsyncWebServerRequest *req){ req->send(200,"text/plain","Rebooting"); delay(500); ESP.restart(); });
        server.on("/api/detect-sensor", HTTP_GET, [this](AsyncWebServerRequest *req){ if(req->hasParam("index")){ int idx = req->getParam("index")->value().toInt(); if(idx >= 0 && idx < 4) { sensors[idx].forceDetect(); int raw = sensors[idx].getRaw(); String mode = sensors[idx].getModeString(); String json = "{\"raw\":" + String(raw) + ", \"mode\":\"" + mode + "\"}"; buzzer->beep(); req->send(200,"application/json",json); } else { req->send(400,"text/plain","Index Error"); } } else { req->send(400,"text/plain","Error"); } });

        server.onNotFound([](AsyncWebServerRequest *req){ req->redirect("/"); });
    }
};
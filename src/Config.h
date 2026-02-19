#pragma once
#include <Arduino.h>

// --- SYSTEM IDENTITY ---
#define SYSTEM_VERSION      "Rosemary Core 1.0 (tuscan)"
#define DEVICE_NAME         "Rosemary_Core"
#define MODEL_ID            "S3-Analog-Edition"

// --- HARDWARE PIN MAPPING (ESP32-S3 N16R8) ---
#define PIN_BUZZER          10
#define PIN_DHT             9
#define DHT_TYPE            DHT22

// Sensor Inputs (Analog Only)
const int PINS_SENSOR[4] = { 4, 5, 6, 7 }; 

// Pump Outputs (Active HIGH)
const int PINS_PUMP[4]   = { 11, 12, 13, 14 };

// --- SYSTEM LIMITS & TIMERS ---
#define MAX_PLANTS          4
#define WIFI_CHECK_MS       30000
#define HISTORY_LOG_MS      3600000  // 1 Hour
#define ENV_UPDATE_MS       2000     // 2 Seconds
#define AUTO_WATER_COOLDOWN 60000    // 1 Minute per plant

// --- DEFAULT SETTINGS ---
#define AP_SSID_DEFAULT     "Rosemary_Core_Setup"
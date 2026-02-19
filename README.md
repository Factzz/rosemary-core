# 🌿 Rosemary Core (Project tuscan)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-ESP32-blue)](https://espressif.com)
[![Framework](https://img.shields.io/badge/framework-Arduino%20%7C%20PlatformIO-orange)](https://platformio.org)
[![Version](https://img.shields.io/badge/version-1.0.0%20(Stable)-green)](https://github.com/)

> **"Simplicity is the ultimate sophistication."** > *The industrial-grade, open-source kernel for smart agriculture on ESP32.*

---

### 🌍 Select Language / เลือกภาษา
[ **🇬🇧 English** ](#-english-description) | [ **🇹🇭 ภาษาไทย** ](#-คำอธิบายภาษาไทย)

---

<a name="english-description"></a>
## 🇬🇧 English Description

**Rosemary Core** (codenamed *tuscan*) is the stripped-down, high-performance kernel of the Rosemary OS. We removed all the fancy UI, AI, and Mesh networking features to focus purely on **hardware stability**, **resource management**, and **reliability**.

Designed for developers and engineers who want a **rock-solid foundation** to build their own commercial-grade smart farm systems without reinventing the wheel.

### ⚡ Killer Features (Why Core?)

#### 1. 🛡️ Brownout-Proof Sequential Watering
A classic IoT failure is the "Voltage Sag" (Brownout) when multiple pumps start simultaneously, causing the ESP32 to crash.
* **The Solution:** Rosemary Core implements a **Smart Queue System** in `PlantManager.h`.
* **How it works:** Even if you trigger 4 pumps at once, the kernel queues them and executes them **one by one**. Zero voltage spikes. Zero reboots.

#### 2. 🔌 Smart Analog Driver (Auto-Floating Check)
Forget reading unreliable `0` or `4095` values from disconnected pins.
* **The Solution:** The **Universal Sensor Engine** (`UniversalSensor.h`) performs a **Swing Test** (Impedance Check) at every boot.
* **Result:** It automatically distinguishes between a **Dry Sensor (0%)** and a **Broken Wire / Floating Pin (Error)**. No manual configuration required.

#### 3. 🧠 Heap-Safe Memory Architecture
Long-term stability is our priority.
* **The Solution:** We moved large JSON serializations from the Stack to the **Heap** (`Network.h`).
* **Result:** This prevents Stack Overflow crashes, allowing the system to run for months without a reboot, even on heavy load.

#### 4. 🖥️ The Terminal UI
* **Minimalist:** A lightweight, hacker-style web interface (Black/Green terminal theme).
* **Fast:** Loads instantly, uses minimal bandwidth, perfect for field debugging.

---

### 🛠️ Tech Stack
* **MCU:** ESP32-S3 (Recommended: N16R8 or N8R2)
* **Sensors:** Capacitive Soil Moisture Sensors (Analog)
* **Storage:** LittleFS (Crash-safe filesystem)
* **Output:** 4x Relay/MOSFET channels (Active HIGH)

### 🚀 Getting Started

1.  **Clone the Repository**
    ```bash
    git clone [https://github.com/Factzz/rosemary-core.git](https://github.com/Factzz/rosemary-core.git)
    ```
2.  **Flash Firmware**
    * Open the project in **VS Code** with **PlatformIO**.
    * Connect your ESP32-S3 via USB.
    * Click **Upload**.
3.  **Upload Filesystem (UI)**
    * In PlatformIO sidebar, go to *Project Tasks* -> *Platform* -> *Upload Filesystem Image*.
    * *Note: This uploads the Terminal UI (HTML/CSS/JS).*
4.  **Connect**
    * Connect to WiFi AP: `Rosemary_Core_Setup`
    * Open Browser: `http://192.168.4.1`

---

<a name="thai-description"></a>
## 🇹🇭 คำอธิบายภาษาไทย

**Rosemary Core** (โปรเจกต์ tuscan) คือ "หัวใจหลัก" (Kernel) ของระบบปฏิบัติการ Rosemary OS ที่ถูกถอดรูปออกให้เหลือเพียงฟังก์ชันที่จำเป็นที่สุด เพื่อให้ได้ **ความเสถียรสูงสุด (Maximum Stability)**

เวอร์ชันนี้ตัดความซับซ้อนของกราฟิก, AI และ Mesh Network ออกทั้งหมด เหลือไว้เพียง **"ระบบจัดการฮาร์ดแวร์ที่แข็งแกร่งที่สุด"** สำหรับนักพัฒนาที่ต้องการนำไปต่อยอดเป็นผลิตภัณฑ์ของตัวเอง

### ⚡ ฟีเจอร์เด็ด (ทำไมต้องใช้ Core?)

#### 1. 🛡️ ระบบคิวรดน้ำอัจฉริยะ (กันไฟตก 100%)
ปัญหาคลาสสิกของบอร์ด IoT คือ "ไฟวูบ" เมื่อปั๊มน้ำทำงานพร้อมกันหลายตัว จนทำให้บอร์ดรีบูตตัวเอง
* **ทางแก้:** Rosemary Core ใช้ **Smart Queue System** (`PlantManager.h`)
* **ผลลัพธ์:** แม้คุณจะสั่งรดน้ำพร้อมกัน 4 ต้น ระบบจะจัดคิวและรดให้ **ทีละต้น** โดยอัตโนมัติ ป้องกันไฟกระชาก บอร์ดไม่น็อคแน่นอน

#### 2. 🔌 ไดรเวอร์เซ็นเซอร์อัจฉริยะ (Smart Analog Driver)
เลิกปวดหัวกับค่า `0` หรือ `4095` มั่วๆ เวลาสายหลุด
* **ทางแก้:** ไดรเวอร์ของเรา (`UniversalSensor.h`) มีระบบ **Swing Test** ที่จะยิงสัญญาณตรวจสอบพอร์ตทุกครั้งที่บูตเครื่อง
* **ผลลัพธ์:** ระบบแยกแยะได้ทันทีว่า **"ดินแห้งจริง"** หรือ **"สายขาด/ไม่ได้เสียบ"** และแจ้งเตือน Error ได้อย่างแม่นยำ

#### 3. 🧠 ระบบจัดการหน่วยความจำแบบ Heap-Safe
ป้องกันอาการ "บอร์ดค้าง" เมื่อเปิดทิ้งไว้นานๆ
* **ทางแก้:** เราเขียนระบบจัดการ JSON ใหม่ ให้ใช้หน่วยความจำส่วน **Heap** แทน Stack (`Network.h`)
* **ผลลัพธ์:** ลดความเสี่ยงเรื่อง Stack Overflow ทำให้ระบบรันต่อเนื่องได้เป็นเดือนๆ โดยไม่ต้องกดรีเซ็ต

#### 4. 🖥️ หน้าจอแบบ Terminal (Hacker Style)
* **ดิบ เถื่อน เท่:** หน้าจอ Web App สีดำ-เขียว สไตล์แฮกเกอร์
* **เบาหวิว:** โหลดไวมาก กินทรัพยากรน้อย เหมาะสำหรับการนำไปใช้หน้างานจริง

---

### 🛠️ อุปกรณ์ที่รองรับ
* **บอร์ด:** ESP32-S3 (แนะนำรุ่น N16R8)
* **เซ็นเซอร์:** วัดความชื้นในดินแบบ Capacitive (Analog)
* **เอาต์พุต:** รีเลย์ หรือ MOSFET 4 ช่อง (Active HIGH)

### 🚀 วิธีใช้งาน

1.  **ดาวน์โหลดโค้ด**
    ```bash
    git clone [https://github.com/Factzz/rosemary-core.git](https://github.com/Factzz/rosemary-core.git)
    ```
2.  **อัปโหลด Firmware**
    * เปิดโปรเจกต์ด้วย **VS Code** + **PlatformIO**
    * เสียบบอร์ด ESP32-S3 แล้วกดปุ่ม **Upload** (ลูกศรขวา)
3.  **อัปโหลดหน้าเว็บ (Filesystem)**
    * ไปที่เมนู PlatformIO (รูปหัวมด) ด้านซ้าย -> *Project Tasks*
    * เลือก *Platform* -> *Upload Filesystem Image*
    * *ขั้นตอนนี้จะลงหน้าจอ Terminal UI ลงไปในบอร์ด*
4.  **เชื่อมต่อ**
    * ต่อ WiFi ชื่อ: `Rosemary_Core_Setup`
    * เข้า Browser พิมพ์: `192.168.4.1`

---

## 📜 License
**MIT License**

Copyright (c) 2026 **Rosemary OS Team**

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files... (see LICENSE file for details).


> **Note:** This is the Community Edition. The "Product" version with AI Doctor, Cube Radar support, Store, Apps and Glassmorphism UI is a separate commercial product.


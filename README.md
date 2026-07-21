# 🌱 YAT Smart Farm V3.4

Smart Farm Controller สำหรับ ESP8266 พร้อม Dashboard และ Firebase

---

## Features

- ESP8266 NodeMCU
- MQTT Control
- Firebase Realtime Database
- OTA Update
- RTC DS3231
- Relay 4CH
- Auto Schedule
- Manual Control
- Admin Login
- PWA รองรับ Android / iPhone

---

## Folder

```
YAT-SmartFarm/
│
├── YAT_SmartFarm_V34_REAL_PRODUCTION.ino
├── index.html
├── style.css
├── dashboard.js
├── firebase.js
├── mqtt.js
├── auth.js
├── service-worker.js
├── manifest.json
├── firebase.rules.json
│
├── icons/
│   ├── icon-192.png
│   ├── icon-512.png
│   └── maskable-512.png
│
└── README.md
```

---

## Hardware

ESP8266 NodeMCU

Relay 4CH

RTC DS3231

WiFi

MQTT Broker

Firebase Realtime Database

---

## Relay

| GPIO | Function |
|------|----------|
| D5 | Pump |
| D6 | Zone1 |
| D7 | Zone2 |
| D8 | Light |

---

## MQTT Topics

Status

```
yatfarm/device/YAT-FARM-001/status
```

Control

```
yatfarm/device/YAT-FARM-001/control
```

---

## License

MIT License

Copyright © YAT Smart Farm

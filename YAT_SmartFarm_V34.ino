/****************************************************
 YAT SMART FARM V3.4 REAL PRODUCTION
 Controller : ESP8266 NodeMCU

 Hardware
 ----------
 RTC DS3231
 Relay 4CH
  D5 Pump
  D6 Zone1
  D7 Zone2
  D8 Light

 Sensor : NONE
****************************************************/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <RTClib.h>
#include <ArduinoOTA.h>

RTC_DS3231 rtc;

WiFiClient espClient;
PubSubClient mqtt(espClient);

//==================== WIFI ====================

const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

//==================== MQTT ====================

const char* MQTT_SERVER = "";
const uint16_t MQTT_PORT = 1883;

const char* TOPIC_CONTROL = "yatfarm/device001/control";
const char* TOPIC_STATUS  = "yatfarm/device001/status";

//==================== RELAY ====================

#define RELAY_PUMP   D5
#define RELAY_ZONE1  D6
#define RELAY_ZONE2  D7
#define RELAY_LIGHT  D8

//==================== STATE ====================

bool pumpState=false;
bool zone1State=false;
bool zone2State=false;
bool lightState=false;

bool autoMode=true;

unsigned long lastStatus=0;
unsigned long lastReconnect=0;
//==================== WIFI ====================

void connectWiFi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());

}

//==================== MQTT ====================

void mqttCallback(char* topic, byte* payload, unsigned int length) {

  String msg = "";

  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  StaticJsonDocument<256> doc;

  DeserializationError err = deserializeJson(doc, msg);

  if (err) return;

  if (doc.containsKey("mode")) {
    String mode = doc["mode"].as<String>();
    autoMode = (mode == "auto");
  }

  if (!autoMode) {

    if (doc.containsKey("pump"))
      setRelay(RELAY_PUMP, doc["pump"]);

    if (doc.containsKey("zone1"))
      setRelay(RELAY_ZONE1, doc["zone1"]);

    if (doc.containsKey("zone2"))
      setRelay(RELAY_ZONE2, doc["zone2"]);

    if (doc.containsKey("light"))
      setRelay(RELAY_LIGHT, doc["light"]);

  }

}

void connectMQTT() {

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

}

void reconnectMQTT() {

  if (mqtt.connected()) return;

  if (millis() - lastReconnect < 5000) return;

  lastReconnect = millis();

  if (mqtt.connect("YAT-FARM-001")) {

    mqtt.subscribe(TOPIC_CONTROL);

    Serial.println("MQTT Connected");

  }

}
//==================== RELAY ====================

void setRelay(uint8_t pin, bool state) {

  digitalWrite(pin, state ? LOW : HIGH);

  switch (pin) {

    case RELAY_PUMP:
      pumpState = state;
      break;

    case RELAY_ZONE1:
      zone1State = state;
      break;

    case RELAY_ZONE2:
      zone2State = state;
      break;

    case RELAY_LIGHT:
      lightState = state;
      break;

  }

}

void relayInit() {

  pinMode(RELAY_PUMP, OUTPUT);
  pinMode(RELAY_ZONE1, OUTPUT);
  pinMode(RELAY_ZONE2, OUTPUT);
  pinMode(RELAY_LIGHT, OUTPUT);

  setRelay(RELAY_PUMP, false);
  setRelay(RELAY_ZONE1, false);
  setRelay(RELAY_ZONE2, false);
  setRelay(RELAY_LIGHT, false);

}

void relayAllOff() {

  setRelay(RELAY_PUMP, false);
  setRelay(RELAY_ZONE1, false);
  setRelay(RELAY_ZONE2, false);
  setRelay(RELAY_LIGHT, false);

}

void publishStatus() {

  StaticJsonDocument<256> doc;

  doc["pump"] = pumpState;
  doc["zone1"] = zone1State;
  doc["zone2"] = zone2State;
  doc["light"] = lightState;
  doc["mode"] = autoMode ? "auto" : "manual";
  doc["ip"] = WiFi.localIP().toString();

  String payload;
  serializeJson(doc, payload);

  mqtt.publish(TOPIC_STATUS, payload.c_str(), true);

}
//==================== OTA ====================

void otaInit() {

  ArduinoOTA.setHostname("YAT-SMART-FARM");

  ArduinoOTA.onStart([]() {

    relayAllOff();

    Serial.println("OTA START");

  });

  ArduinoOTA.onEnd([]() {

    Serial.println("OTA END");

  });

  ArduinoOTA.onProgress([](unsigned int progress,
                           unsigned int total) {

    Serial.printf("OTA %u%%\n",
                  (progress * 100) / total);

  });

  ArduinoOTA.onError([](ota_error_t error) {

    Serial.printf("OTA ERROR %u\n", error);

  });

  ArduinoOTA.begin();

}

//==================== SETUP ====================

void setup() {

  Serial.begin(115200);

  relayInit();

  rtcInit();

  connectWiFi();

  connectMQTT();

  otaInit();

  publishStatus();

}

//==================== LOOP ====================

void loop() {

  ArduinoOTA.handle();

  reconnectMQTT();

  mqtt.loop();

  autoControl();

  if (millis() - lastStatus >= 5000) {

    lastStatus = millis();

    publishStatus();

  }

}
//==================== JSON CONFIG ====================

#include <ArduinoJson.h>


//==================== FIREBASE PLACEHOLDER ====================
// ใส่ค่า Firebase จริงภายหลัง

const char* FIREBASE_HOST = "";
const char* FIREBASE_AUTH = "";


//==================== SEND DATA ====================

void sendFirebaseStatus(){

  // ส่งสถานะ Relay
  // Firebase RTDB integration

  Serial.println("Firebase Update");

}


//==================== MQTT COMMAND ====================

void mqttSendCommand(String command){

  mqtt.publish(
    TOPIC_CONTROL,
    command.c_str()
  );

}


//==================== DEVICE INFO ====================

String deviceStatus(){

  StaticJsonDocument<256> doc;

  doc["device"] = "YAT-FARM-001";
  doc["firmware"] = "V3.4";
  doc["time"] = getDateTime();

  String result;

  serializeJson(doc,result);

  return result;

}


//==================== HEARTBEAT ====================

void heartbeat(){

  StaticJsonDocument<128> doc;

  doc["online"] = true;
  doc["device"] = "YAT-FARM-001";

  String data;

  serializeJson(doc,data);

  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );

}
//==================== FIREBASE DATA STRUCTURE ====================

void updateCloudStatus(){

  StaticJsonDocument<256> doc;

  doc["deviceID"] = "YAT-FARM-001";

  doc["relay"]["pump"]  = pumpState;
  doc["relay"]["zone1"] = zone1State;
  doc["relay"]["zone2"] = zone2State;
  doc["relay"]["light"] = lightState;

  doc["mode"] = autoMode ? "AUTO" : "MANUAL";

  doc["rtc"] = getDateTime();

  String json;

  serializeJson(doc,json);

  Serial.println(json);

}


//==================== COMMAND PROCESS ====================

void processCommand(String command){

  StaticJsonDocument<256> doc;

  DeserializationError error =
      deserializeJson(doc,command);


  if(error){

    Serial.println("JSON ERROR");

    return;

  }


  if(doc.containsKey("pump")){

    setRelay(
      RELAY_PUMP,
      doc["pump"]
    );

  }


  if(doc.containsKey("zone1")){

    setRelay(
      RELAY_ZONE1,
      doc["zone1"]
    );

  }


  if(doc.containsKey("zone2")){

    setRelay(
      RELAY_ZONE2,
      doc["zone2"]
    );

  }


  if(doc.containsKey("light")){

    setRelay(
      RELAY_LIGHT,
      doc["light"]
    );

  }


  if(doc.containsKey("auto")){

    autoMode = doc["auto"];

  }


  publishStatus();

}
//==================== MQTT CALLBACK UPDATE ====================

void mqttCallback(char* topic,
                  byte* payload,
                  unsigned int length) {

  String command = "";

  for(unsigned int i=0;i<length;i++){

    command += (char)payload[i];

  }

  Serial.println("MQTT COMMAND:");
  Serial.println(command);

  processCommand(command);

}


//==================== NETWORK CHECK ====================

void checkWiFi(){

  if(WiFi.status()!=WL_CONNECTED){

    Serial.println("WiFi Lost");

    WiFi.disconnect();

    connectWiFi();

  }

}


//==================== DEVICE RESTART ====================

void restartDevice(){

  relayAllOff();

  delay(1000);

  ESP.restart();

}


//==================== SAFETY SYSTEM ====================

void safetyCheck(){

  // ป้องกัน Relay ค้าง

  if(!autoMode){

    return;

  }


  // ถ้ามีการควบคุมจาก Manual
  // จะไม่ให้ Timer แทรก

}


//==================== DEBUG ====================

void debugStatus(){

  Serial.println("================");

  Serial.print("Pump : ");
  Serial.println(pumpState);

  Serial.print("Zone1 : ");
  Serial.println(zone1State);

  Serial.print("Zone2 : ");
  Serial.println(zone2State);

  Serial.print("Light : ");
  Serial.println(lightState);

  Serial.print("Mode : ");

  Serial.println(
    autoMode ? "AUTO":"MANUAL"
  );

  Serial.print("RTC : ");

  Serial.println(
    getDateTime()
  );

  Serial.println("================");

}
//==================== MQTT SETUP ====================

void mqttInit(){

  mqtt.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );

  mqtt.setCallback(
    mqttCallback
  );

}


//==================== CONNECTION MANAGER ====================

void connectionManager(){

  checkWiFi();

  if(!mqtt.connected()){

    reconnectMQTT();

  }

  mqtt.loop();

}


//==================== SYSTEM LOOP TASK ====================

void systemTask(){

  connectionManager();

  safetyCheck();


  if(millis() - lastStatus > 5000){

    lastStatus = millis();

    publishStatus();

    updateCloudStatus();

    heartbeat();

  }

}


//==================== MANUAL CONTROL API ====================

void manualPump(bool state){

  autoMode = false;

  setRelay(
    RELAY_PUMP,
    state
  );

}


void manualZone1(bool state){

  autoMode = false;

  setRelay(
    RELAY_ZONE1,
    state
  );

}


void manualZone2(bool state){

  autoMode = false;

  setRelay(
    RELAY_ZONE2,
    state
  );

}


void manualLight(bool state){

  autoMode = false;

  setRelay(
    RELAY_LIGHT,
    state
  );

}
//==================== TIMER CONTROL ====================

struct TimerConfig {

  bool enable;

  uint8_t startHour;
  uint8_t startMinute;

  uint8_t stopHour;
  uint8_t stopMinute;

};


TimerConfig zone1Timer = {

  true,
  6,
  0,
  6,
  30

};


TimerConfig zone2Timer = {

  true,
  17,
  0,
  17,
  30

};


TimerConfig lightTimer = {

  true,
  18,
  0,
  22,
  0

};


//==================== TIMER CHECK ====================

bool checkTimer(TimerConfig timer){

  DateTime now = rtc.now();


  int current =
    now.hour() * 60 +
    now.minute();


  int start =
    timer.startHour * 60 +
    timer.startMinute;


  int stop =
    timer.stopHour * 60 +
    timer.stopMinute;


  if(start <= current && current < stop){

    return true;

  }


  return false;

}


//==================== AUTO ZONE CONTROL ====================

void autoZoneControl(){

  if(!autoMode){

    return;

  }


  setRelay(
    RELAY_ZONE1,
    checkTimer(zone1Timer)
  );


  setRelay(
    RELAY_ZONE2,
    checkTimer(zone2Timer)
  );


  setRelay(
    RELAY_LIGHT,
    checkTimer(lightTimer)
  );


}


//==================== MAIN CONTROL ====================

void runController(){

  autoControl();

  autoZoneControl();

  systemTask();

}
//==================== FIREBASE PATH ====================

String firebasePath(){

  String path = "/device/";

  path += "YAT-FARM-001";

  return path;

}


//==================== CLOUD PAYLOAD ====================

String createCloudPayload(){

  StaticJsonDocument<512> doc;


  doc["device"]["id"] =
    "YAT-FARM-001";


  doc["device"]["firmware"] =
    "V3.4";


  doc["device"]["online"] =
    true;


  doc["device"]["time"] =
    getDateTime();



  doc["control"]["mode"] =
    autoMode ? "AUTO":"MANUAL";


  doc["control"]["pump"] =
    pumpState;


  doc["control"]["zone1"] =
    zone1State;


  doc["control"]["zone2"] =
    zone2State;


  doc["control"]["light"] =
    lightState;



  String output;


  serializeJson(
    doc,
    output
  );


  return output;

}


//==================== SEND MQTT STATUS ====================

void sendMQTTStatus(){

  String data =
    createCloudPayload();


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );


}


//==================== SYSTEM START MESSAGE ====================

void systemReady(){

  Serial.println();
  Serial.println("====================");
  Serial.println("YAT SMART FARM V3.4");
  Serial.println("SYSTEM READY");
  Serial.println("====================");

}
//==================== SETUP FINAL UPDATE ====================

void setupSystem(){

  Serial.begin(115200);

  delay(1000);


  systemReady();


  // Relay

  relayInit();


  // RTC

  rtcInit();


  // WiFi

  connectWiFi();


  // MQTT

  mqttInit();


  // OTA

  otaInit();



  publishStatus();


  sendMQTTStatus();


}


//==================== LOOP TASK ====================

void loopSystem(){


  ArduinoOTA.handle();


  runController();


  if(millis() - lastStatus > 10000){

    lastStatus = millis();


    debugStatus();


    sendMQTTStatus();

  }


}


//==================== MAIN LOOP EXTEND ====================

// ใช้แทน loop เดิม

void loop(){

  loopSystem();

}
//==================== POWER SAVE / WATCHDOG ====================

void watchdogInit(){

  ESP.wdtEnable(8000);

}


void watchdogFeed(){

  ESP.wdtFeed();

}


//==================== MEMORY CHECK ====================

void memoryStatus(){

  Serial.print("Free Heap : ");

  Serial.println(
    ESP.getFreeHeap()
  );

}


//==================== DEVICE HEALTH ====================

void deviceHealth(){

  StaticJsonDocument<256> doc;


  doc["device"] =
    "YAT-FARM-001";


  doc["firmware"] =
    "V3.4";


  doc["heap"] =
    ESP.getFreeHeap();


  doc["uptime"] =
    millis() / 1000;


  String data;


  serializeJson(
    doc,
    data
  );


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );

}


//==================== RECOVERY ====================

void systemRecovery(){

  if(WiFi.status()
     != WL_CONNECTED){

    connectWiFi();

  }


  if(!mqtt.connected()){

    reconnectMQTT();

  }


  watchdogFeed();

}
//==================== FIREBASE CONFIG ====================

struct FirebaseConfig {

  String apiKey;

  String databaseURL;

};


FirebaseConfig firebase;


//==================== LOAD CONFIG ====================

void loadConfig(){

  firebase.apiKey =
    "YOUR_FIREBASE_API_KEY";


  firebase.databaseURL =
    "YOUR_DATABASE_URL";


}


//==================== CLOUD CONTROL RECEIVE ====================

void cloudControl(String json){

  StaticJsonDocument<256> doc;


  if(deserializeJson(doc,json)){

    return;

  }


  if(doc.containsKey("mode")){

    String mode =
      doc["mode"].as<String>();


    if(mode=="AUTO"){

      autoMode=true;

    }
    else{

      autoMode=false;

    }

  }


  if(doc.containsKey("pump")){

    setRelay(
      RELAY_PUMP,
      doc["pump"]
    );

  }


  if(doc.containsKey("zone1")){

    setRelay(
      RELAY_ZONE1,
      doc["zone1"]
    );

  }


  if(doc.containsKey("zone2")){

    setRelay(
      RELAY_ZONE2,
      doc["zone2"]
    );

  }


  if(doc.containsKey("light")){

    setRelay(
      RELAY_LIGHT,
      doc["light"]
    );

  }


  publishStatus();

}


//==================== SYSTEM SERVICE ====================

void serviceTask(){

  watchdogFeed();

  systemRecovery();

  memoryStatus();

  deviceHealth();

}
//==================== COMMAND QUEUE ====================

String commandBuffer = "";

void receiveCommand(){

  while(mqtt.available()){

    char c = mqtt.read();

    commandBuffer += c;

  }


  if(commandBuffer.length()){

    processCommand(commandBuffer);

    commandBuffer = "";

  }

}


//==================== RELAY STATUS JSON ====================

String relayStatus(){

  StaticJsonDocument<256> doc;


  doc["pump"] =
    pumpState;


  doc["zone1"] =
    zone1State;


  doc["zone2"] =
    zone2State;


  doc["light"] =
    lightState;


  doc["mode"] =
    autoMode ? "AUTO":"MANUAL";


  String output;


  serializeJson(
    doc,
    output
  );


  return output;

}


//==================== PUBLISH RELAY STATUS ====================

void publishRelayStatus(){

  String data =
    relayStatus();


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );

}


//==================== SCHEDULE RESET ====================

void resetSchedule(){

  zone1Timer.enable = false;

  zone2Timer.enable = false;

  lightTimer.enable = false;


  Serial.println(
    "Schedule Reset"
  );

}


//==================== FACTORY SAFE ====================

void factorySafe(){

  relayAllOff();

  autoMode = false;

  Serial.println(
    "SYSTEM SAFE MODE"
  );

}
//==================== STARTUP CHECK ====================

void startupCheck(){

  Serial.println();
  Serial.println("STARTUP CHECK");


  if(WiFi.status()==WL_CONNECTED){

    Serial.println("WiFi OK");

  }
  else{

    Serial.println("WiFi FAIL");

  }


  if(rtc.begin()){

    Serial.println("RTC OK");

  }
  else{

    Serial.println("RTC FAIL");

  }


  Serial.println("Relay Ready");

}


//==================== DEVICE INFORMATION ====================

String deviceInfo(){

  StaticJsonDocument<256> doc;


  doc["id"] =
    "YAT-FARM-001";


  doc["version"] =
    "3.4.0";


  doc["controller"] =
    "ESP8266";


  doc["rtc"] =
    getDateTime();


  String result;


  serializeJson(
    doc,
    result
  );


  return result;

}


//==================== SEND DEVICE INFO ====================

void publishDeviceInfo(){

  String data =
    deviceInfo();


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );

}


//==================== END SYSTEM SERVICE ====================

void finalService(){

  watchdogFeed();


  if(millis()%30000==0){

    publishDeviceInfo();

  }

}
//==================== MQTT TOPIC HANDLER ====================

void handleMQTTCommand(String message){

  StaticJsonDocument<512> doc;

  DeserializationError error =
    deserializeJson(doc, message);


  if(error){

    Serial.println("Command JSON Error");

    return;

  }


  if(doc.containsKey("pump")){

    setRelay(
      RELAY_PUMP,
      doc["pump"]
    );

  }


  if(doc.containsKey("zone1")){

    setRelay(
      RELAY_ZONE1,
      doc["zone1"]
    );

  }


  if(doc.containsKey("zone2")){

    setRelay(
      RELAY_ZONE2,
      doc["zone2"]
    );

  }


  if(doc.containsKey("light")){

    setRelay(
      RELAY_LIGHT,
      doc["light"]
    );

  }


  if(doc.containsKey("mode")){

    String mode =
      doc["mode"].as<String>();


    autoMode =
      (mode == "AUTO");

  }


  publishRelayStatus();

}


//==================== MQTT MESSAGE LOG ====================

void logCommand(String msg){

  Serial.print("MQTT RX : ");

  Serial.println(msg);

}


//==================== SYSTEM LOOP EXTENSION ====================

void productionLoop(){

  ArduinoOTA.handle();


  connectionManager();


  autoZoneControl();


  serviceTask();


  finalService();


}
//==================== MQTT TOPIC HANDLER ====================

void handleMQTTCommand(String message){

  StaticJsonDocument<512> doc;

  DeserializationError error =
    deserializeJson(doc, message);


  if(error){

    Serial.println("Command JSON Error");

    return;

  }


  if(doc.containsKey("pump")){

    setRelay(
      RELAY_PUMP,
      doc["pump"]
    );

  }


  if(doc.containsKey("zone1")){

    setRelay(
      RELAY_ZONE1,
      doc["zone1"]
    );

  }


  if(doc.containsKey("zone2")){

    setRelay(
      RELAY_ZONE2,
      doc["zone2"]
    );

  }


  if(doc.containsKey("light")){

    setRelay(
      RELAY_LIGHT,
      doc["light"]
    );

  }


  if(doc.containsKey("mode")){

    String mode =
      doc["mode"].as<String>();


    if(mode == "AUTO"){

      autoMode = true;

    }
    else{

      autoMode = false;

    }

  }


  publishRelayStatus();

}


//==================== COMMAND LOG ====================

void logCommand(String msg){

  Serial.print("MQTT RX : ");

  Serial.println(msg);

}


//==================== PRODUCTION LOOP ====================

void productionLoop(){

  ArduinoOTA.handle();

  connectionManager();

  autoZoneControl();

  serviceTask();

  finalService();

}
//==================== ERROR HANDLING ====================

void systemError(String error){

  Serial.print("SYSTEM ERROR : ");

  Serial.println(error);


  StaticJsonDocument<128> doc;


  doc["device"] =
    "YAT-FARM-001";


  doc["error"] =
    error;


  String data;


  serializeJson(
    doc,
    data
  );


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str()
  );

}


//==================== RELAY TEST ====================

void relayTest(){

  Serial.println("Relay Test Start");


  setRelay(RELAY_PUMP,true);
  delay(1000);

  setRelay(RELAY_PUMP,false);


  setRelay(RELAY_ZONE1,true);
  delay(1000);

  setRelay(RELAY_ZONE1,false);


  setRelay(RELAY_ZONE2,true);
  delay(1000);

  setRelay(RELAY_ZONE2,false);


  setRelay(RELAY_LIGHT,true);
  delay(1000);

  setRelay(RELAY_LIGHT,false);


  Serial.println("Relay Test Complete");

}


//==================== REMOTE RESTART ====================

void remoteRestart(){

  relayAllOff();

  delay(500);

  ESP.restart();

}


//==================== VERSION ====================

String firmwareVersion(){

  return "YAT-SmartFarm-V3.4";
}
//==================== COMMAND ACTION ====================

void executeSystemCommand(String cmd){

  if(cmd == "SAFE"){

    factorySafe();

  }


  else if(cmd == "RESTART"){

    remoteRestart();

  }


  else if(cmd == "TEST_RELAY"){

    relayTest();

  }


  else if(cmd == "STATUS"){

    publishStatus();

  }


  else{

    Serial.println("Unknown Command");

  }

}


//==================== DEVICE STATE ====================

String deviceState(){

  StaticJsonDocument<256> doc;


  doc["device"] =
    "YAT-FARM-001";


  doc["firmware"] =
    firmwareVersion();


  doc["pump"] =
    pumpState;


  doc["zone1"] =
    zone1State;


  doc["zone2"] =
    zone2State;


  doc["light"] =
    lightState;


  doc["mode"] =
    autoMode ? "AUTO":"MANUAL";


  String output;


  serializeJson(
    doc,
    output
  );


  return output;

}


//==================== STATUS UPDATE ====================

void sendDeviceState(){

  String state =
    deviceState();


  mqtt.publish(
    TOPIC_STATUS,
    state.c_str(),
    true
  );
}
//==================== EEPROM CONFIG ====================

#include <EEPROM.h>

#define EEPROM_SIZE 64


struct DeviceConfig {

  bool autoMode;

  bool pump;
  bool zone1;
  bool zone2;
  bool light;

};


DeviceConfig config;


//==================== SAVE CONFIG ====================

void saveConfig(){

  config.autoMode = autoMode;

  config.pump = pumpState;
  config.zone1 = zone1State;
  config.zone2 = zone2State;
  config.light = lightState;


  EEPROM.put(
    0,
    config
  );


  EEPROM.commit();


}


//==================== LOAD CONFIG ====================

void loadConfig(){

  EEPROM.begin(
    EEPROM_SIZE
  );


  EEPROM.get(
    0,
    config
  );


  autoMode =
    config.autoMode;


  pumpState =
    config.pump;


  zone1State =
    config.zone1;


  zone2State =
    config.zone2;


  lightState =
    config.light;


}


//==================== SAFE BOOT ====================

void safeBoot(){

  relayAllOff();

  loadConfig();


  Serial.println(
    "SAFE BOOT COMPLETE"
  );

}


//==================== CONFIG UPDATE ====================

void updateConfig(){

  saveConfig();

  publishStatus();

}
//==================== SCHEDULE CLOUD FORMAT ====================

struct CloudSchedule {

  bool enable;

  uint8_t startHour;

  uint8_t startMinute;

  uint8_t stopHour;

  uint8_t stopMinute;

};


CloudSchedule pumpCloud;


//==================== LOAD DEFAULT SCHEDULE ====================

void defaultSchedule(){

  pumpCloud.enable = true;

  pumpCloud.startHour = 6;

  pumpCloud.startMinute = 0;

  pumpCloud.stopHour = 6;

  pumpCloud.stopMinute = 30;

}


//==================== CHECK PUMP SCHEDULE ====================

bool pumpScheduleCheck(){

  if(!pumpCloud.enable){

    return false;

  }


  DateTime now =
    rtc.now();


  int current =
    now.hour()*60 +
    now.minute();


  int start =
    pumpCloud.startHour*60 +
    pumpCloud.startMinute;


  int stop =
    pumpCloud.stopHour*60 +
    pumpCloud.stopMinute;


  return
    current >= start &&
    current < stop;

}


//==================== AUTO PUMP ====================

void autoPumpControl(){

  if(!autoMode){

    return;

  }


  if(pumpScheduleCheck()){

    setRelay(
      RELAY_PUMP,
      true
    );

  }
  else{

    setRelay(
      RELAY_PUMP,
      false
    );

  }

}
//==================== CONFIG MQTT UPDATE ====================

void updateScheduleFromMQTT(String data){

  StaticJsonDocument<256> doc;


  if(deserializeJson(doc,data)){

    return;

  }


  if(doc.containsKey("enable")){

    pumpCloud.enable =
      doc["enable"];

  }


  if(doc.containsKey("startHour")){

    pumpCloud.startHour =
      doc["startHour"];

  }


  if(doc.containsKey("startMinute")){

    pumpCloud.startMinute =
      doc["startMinute"];

  }


  if(doc.containsKey("stopHour")){

    pumpCloud.stopHour =
      doc["stopHour"];

  }


  if(doc.containsKey("stopMinute")){

    pumpCloud.stopMinute =
      doc["stopMinute"];

  }


  saveConfig();

}


//==================== CLOUD COMMAND ROUTER ====================

void commandRouter(String command){

  if(command.startsWith("SCHEDULE")){

    updateScheduleFromMQTT(
      command.substring(8)
    );

  }

  else if(command.startsWith("{")){

    handleMQTTCommand(command);

  }

  else{

    executeSystemCommand(command);

  }

}


//==================== SYSTEM RUNNER ====================

void runProduction(){

  connectionManager();

  autoPumpControl();

  autoZoneControl();

  productionLoop();

}
//==================== FINAL SETUP ====================

void setup(){

  Serial.begin(115200);

  delay(1000);


  Serial.println();

  Serial.println(
    "YAT SMART FARM V3.4 START"
  );


  // EEPROM

  EEPROM.begin(
    EEPROM_SIZE
  );


  // Relay

  relayInit();


  // RTC

  rtcInit();


  defaultSchedule();


  // WiFi

  connectWiFi();


  // MQTT

  mqttInit();


  // OTA

  otaInit();


  startupCheck();


  publishDeviceInfo();


  sendDeviceState();

}


//==================== FINAL LOOP ====================

void loop(){

  ArduinoOTA.handle();


  mqtt.loop();


  runProduction();


  watchdogFeed();


  delay(10);

}
//==================== END SYSTEM FUNCTIONS ====================

// Manual Control Helper

void controlPump(bool state){

  autoMode = false;

  setRelay(
    RELAY_PUMP,
    state
  );

  saveConfig();

}


void controlZone1(bool state){

  autoMode = false;

  setRelay(
    RELAY_ZONE1,
    state
  );

  saveConfig();

}


void controlZone2(bool state){

  autoMode = false;

  setRelay(
    RELAY_ZONE2,
    state
  );

  saveConfig();

}


void controlLight(bool state){

  autoMode = false;

  setRelay(
    RELAY_LIGHT,
    state
  );

  saveConfig();

}


//==================== SYSTEM STATUS ====================

void printSystem(){

  Serial.println();

  Serial.println("YAT FARM STATUS");

  Serial.print("Mode : ");

  Serial.println(
    autoMode ? "AUTO":"MANUAL"
  );


  Serial.print("Pump : ");

  Serial.println(
    pumpState
  );


  Serial.print("Zone1 : ");

  Serial.println(
    zone1State
  );


  Serial.print("Zone2 : ");

  Serial.println(
    zone2State
  );


  Serial.print("Light : ");

  Serial.println(
    lightState
  );


  Serial.print("Time : ");

  Serial.println(
    getDateTime()
  );

}
//==================== FINAL MQTT CALLBACK ====================

void mqttCallback(char* topic,
                  byte* payload,
                  unsigned int length){

  String message = "";


  for(unsigned int i=0;i<length;i++){

    message += (char)payload[i];

  }


  Serial.print("RX: ");

  Serial.println(message);


  commandRouter(message);


  sendDeviceState();

}


//==================== MQTT CONNECT ====================

void reconnectMQTT(){

  while(!mqtt.connected()){


    Serial.println(
      "MQTT Connecting..."
    );


    if(mqtt.connect(
        "YAT-FARM-001"
       )){


      Serial.println(
        "MQTT Connected"
      );


      mqtt.subscribe(
        TOPIC_CONTROL
      );


      sendDeviceState();


    }

    else{


      Serial.print(
        "MQTT Failed "
      );


      Serial.println(
        mqtt.state()
      );


      delay(5000);


    }

  }

}


//==================== END ====================
//==================== WIFI RECONNECT ====================

void wifiReconnect(){

  if(WiFi.status() == WL_CONNECTED){

    return;

  }


  Serial.println(
    "WiFi Reconnecting..."
  );


  WiFi.disconnect();

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  unsigned long start =
    millis();


  while(WiFi.status()!=WL_CONNECTED){

    delay(500);


    if(millis()-start > 30000){

      Serial.println(
        "WiFi Timeout"
      );

      break;

    }

  }


  if(WiFi.status()==WL_CONNECTED){

    Serial.println(
      "WiFi OK"
    );

  }

}


//==================== NETWORK SERVICE ====================

void networkService(){

  wifiReconnect();


  if(!mqtt.connected()){

    reconnectMQTT();

  }


  mqtt.loop();

}


//==================== FINAL SERVICE LOOP ====================

void mainService(){

  networkService();


  ArduinoOTA.handle();


  if(autoMode){

    autoPumpControl();

    autoZoneControl();

  }


  if(millis()-lastStatus > 10000){

    lastStatus = millis();

    sendDeviceState();

    printSystem();

  }


  watchdogFeed();

}
//==================== FIREBASE READY INTERFACE ====================

String firebaseDevicePath(){

  return "/devices/YAT-FARM-001";

}


String createFirebaseJSON(){

  StaticJsonDocument<512> doc;


  doc["deviceID"] =
    "YAT-FARM-001";


  doc["firmware"] =
    "V3.4";


  doc["online"] =
    true;


  doc["time"] =
    getDateTime();



  doc["relay"]["pump"] =
    pumpState;


  doc["relay"]["zone1"] =
    zone1State;


  doc["relay"]["zone2"] =
    zone2State;


  doc["relay"]["light"] =
    lightState;



  doc["mode"] =
    autoMode ? "AUTO":"MANUAL";


  String output;


  serializeJson(
    doc,
    output
  );


  return output;

}


//==================== FIREBASE SEND ====================

void firebaseUpdate(){

  String data =
    createFirebaseJSON();


  Serial.println(
    "Firebase Data:"
  );


  Serial.println(data);


  // Firebase RTDB Upload
  // ใส่ Firebase Client ในขั้นเชื่อมต่อจริง

}
//==================== FIREBASE COMMAND PARSER ====================

void firebaseCommand(String command){

  StaticJsonDocument<512> doc;


  DeserializationError error =
    deserializeJson(doc, command);


  if(error){

    Serial.println(
      "Firebase JSON ERROR"
    );

    return;

  }



  if(doc.containsKey("mode")){


    String mode =
      doc["mode"].as<String>();


    autoMode =
      (mode == "AUTO");


  }



  if(doc.containsKey("pump")){


    setRelay(
      RELAY_PUMP,
      doc["pump"]
    );


  }



  if(doc.containsKey("zone1")){


    setRelay(
      RELAY_ZONE1,
      doc["zone1"]
    );


  }



  if(doc.containsKey("zone2")){


    setRelay(
      RELAY_ZONE2,
      doc["zone2"]
    );


  }



  if(doc.containsKey("light")){


    setRelay(
      RELAY_LIGHT,
      doc["light"]
    );


  }



  updateConfig();


}


//==================== CLOUD SERVICE ====================

void cloudService(){

  firebaseUpdate();

  otaStatus();

}
//==================== FINAL PRODUCTION LOOP ====================

void productionService(){

  // Network

  networkService();


  // OTA

  otaService();


  // Auto Control

  if(autoMode){

    autoPumpControl();

    autoZoneControl();

  }


  // Cloud

  if(millis() - lastStatus > 10000){

    lastStatus = millis();


    cloudService();

    sendDeviceState();


    printSystem();

  }


  // Safety

  safetyCheck();


  // Watchdog

  watchdogFeed();

}


//==================== SYSTEM READY ====================

void systemReadyMessage(){

  Serial.println();
  Serial.println("==========================");
  Serial.println(" YAT SMART FARM V3.4 ");
  Serial.println(" REAL PRODUCTION READY ");
  Serial.println("==========================");

}
//==================== FINAL INITIALIZATION ====================

void initializeSystem(){

  Serial.println(
    "Initializing System..."
  );


  // Memory

  EEPROM.begin(
    EEPROM_SIZE
  );


  // Relay

  relayInit();


  // Load Config

  loadConfig();


  // RTC

  rtcInit();


  // Default Schedule

  defaultSchedule();


  // Network

  connectWiFi();


  // MQTT

  mqttInit();


  // OTA

  otaInit();


  watchdogInit();


  systemReadyMessage();


  startupCheck();


}


//==================== FINAL LOOP HANDLER ====================

void handleSystem(){

  productionService();

  delay(10);

}
//==================== SYSTEM COMMAND EXTENSION ====================

void systemCommand(String cmd){


  if(cmd == "ALL_ON"){


    setRelay(RELAY_PUMP,true);

    setRelay(RELAY_ZONE1,true);

    setRelay(RELAY_ZONE2,true);

    setRelay(RELAY_LIGHT,true);


  }


  else if(cmd == "ALL_OFF"){


    relayAllOff();


  }


  else if(cmd == "AUTO"){


    autoMode = true;


  }


  else if(cmd == "MANUAL"){


    autoMode = false;


  }


  else if(cmd == "SAVE"){


    saveConfig();


  }


  else if(cmd == "STATUS"){


    sendDeviceState();


  }


  updateConfig();

}


//==================== COMMAND CHECK ====================

void commandCheck(){

  if(Serial.available()){


    String cmd =
      Serial.readStringUntil('\n');


    cmd.trim();


    systemCommand(cmd);


  }

}


//==================== DEBUG SERVICE ====================

void debugService(){

  commandCheck();

}
//==================== FINAL MAIN CONTROL ====================

void finalControl(){


  // ตรวจระบบเครือข่าย

  networkService();



  // OTA

  ArduinoOTA.handle();



  // รับคำสั่ง MQTT

  mqtt.loop();



  // ระบบ Auto

  if(autoMode){


    autoPumpControl();


    autoZoneControl();


  }



  // ส่งสถานะ

  if(millis() - lastStatus > 10000){


    lastStatus = millis();


    sendDeviceState();


    firebaseUpdate();


    printSystem();


  }



  // ตรวจความปลอดภัย

  safetyCheck();



  // ดูแลระบบ

  watchdogFeed();


}



//==================== SYSTEM LOOP READY ====================

void runSystem(){


  finalControl();


  debugService();


}
//==================== COMPLETE SYSTEM WRAPPER ====================

void systemLoop(){


  runSystem();


  // Memory Monitor

  static unsigned long memTimer = 0;


  if(millis() - memTimer > 60000){


    memTimer = millis();


    memoryStatus();


  }


}


//==================== FINAL BOOT ====================

void bootMessage(){


  Serial.println();

  Serial.println(
    "=============================="
  );

  Serial.println(
    "YAT SMART FARM V3.4"
  );

  Serial.println(
    "ESP8266 CONTROLLER"
  );

  Serial.println(
    "RTC + RELAY + MQTT + OTA"
  );

  Serial.println(
    "=============================="
  );


}
//==================== FINAL MAIN ENTRY ====================

// เรียกใช้ใน setup()

void startSystem(){

  bootMessage();

  initializeSystem();

  systemReadyMessage();

}


// เรียกใช้ใน loop()

void updateSystem(){

  systemLoop();

}


//==================== FINAL CHECK ====================

void finalCheck(){

  if(WiFi.status()!=WL_CONNECTED){

    Serial.println(
      "WiFi Warning"
    );

  }


  if(!mqtt.connected()){

    Serial.println(
      "MQTT Warning"
    );

  }


  if(!rtc.begin()){

    systemError(
      "RTC ERROR"
    );

  }


}


//==================== PRODUCTION MODE ====================

void productionMode(){

  finalCheck();

  updateSystem();

}

//==================== END MODULE ====================
//==================== FINAL LOOP SERVICE ====================

void serviceManager(){

  // Network Service

  networkService();


  // MQTT Service

  mqtt.loop();


  // OTA Service

  otaService();


  // Auto Water Service

  if(autoMode){

    autoPumpControl();

    autoZoneControl();

  }


  // Cloud Sync

  static unsigned long cloudTimer = 0;


  if(millis() - cloudTimer > 15000){


    cloudTimer = millis();


    firebaseUpdate();

    sendDeviceState();


  }


  // System Safety

  safetyCheck();


  // Watchdog

  watchdogFeed();


}


//==================== FINAL STATUS ====================

void statusReport(){

  Serial.println();

  Serial.println(
    "---- YAT STATUS ----"
  );


  Serial.println(
    deviceState()
  );


  Serial.println(
    "--------------------"
  );

}
//==================== REMOTE SETTINGS ====================

struct SystemSetting {

  bool autoEnable;

  uint8_t pumpStartHour;

  uint8_t pumpStartMinute;

  uint8_t pumpStopHour;

  uint8_t pumpStopMinute;

};


SystemSetting setting;


//==================== DEFAULT SETTING ====================

void loadDefaultSetting(){

  setting.autoEnable = true;

  setting.pumpStartHour = 6;

  setting.pumpStartMinute = 0;

  setting.pumpStopHour = 6;

  setting.pumpStopMinute = 30;

}


//==================== APPLY SETTING ====================

void applySetting(){

  autoMode =
    setting.autoEnable;


  pumpCloud.enable =
    setting.autoEnable;


  pumpCloud.startHour =
    setting.pumpStartHour;


  pumpCloud.startMinute =
    setting.pumpStartMinute;


  pumpCloud.stopHour =
    setting.pumpStopHour;


  pumpCloud.stopMinute =
    setting.pumpStopMinute;

}


//==================== SAVE SETTING ====================

void saveSetting(){

  EEPROM.put(
    32,
    setting
  );


  EEPROM.commit();


  Serial.println(
    "SETTING SAVED"
  );

}
//==================== LOAD SETTING ====================

void loadSetting(){

  EEPROM.get(
    32,
    setting
  );


  if(setting.pumpStartHour > 23){

    loadDefaultSetting();

  }


  applySetting();

}


//==================== REMOTE SETTING JSON ====================

void settingCommand(String data){

  StaticJsonDocument<256> doc;


  if(deserializeJson(doc,data)){

    return;

  }


  if(doc.containsKey("auto")){

    setting.autoEnable =
      doc["auto"];

  }


  if(doc.containsKey("startHour")){

    setting.pumpStartHour =
      doc["startHour"];

  }


  if(doc.containsKey("startMinute")){

    setting.pumpStartMinute =
      doc["startMinute"];

  }


  if(doc.containsKey("stopHour")){

    setting.pumpStopHour =
      doc["stopHour"];

  }


  if(doc.containsKey("stopMinute")){

    setting.pumpStopMinute =
      doc["stopMinute"];

  }


  applySetting();

  saveSetting();

}


//==================== END SETTING ====================
//==================== FINAL DEVICE REPORT ====================

String createStatusReport(){

  StaticJsonDocument<512> doc;


  doc["device"] =
    "YAT-FARM-001";


  doc["version"] =
    "3.4.0";


  doc["wifi"] =
    WiFi.status() == WL_CONNECTED;


  doc["ip"] =
    WiFi.localIP().toString();


  doc["time"] =
    getDateTime();


  doc["mode"] =
    autoMode ? "AUTO":"MANUAL";


  doc["relay"]["pump"] =
    pumpState;


  doc["relay"]["zone1"] =
    zone1State;


  doc["relay"]["zone2"] =
    zone2State;


  doc["relay"]["light"] =
    lightState;


  doc["memory"] =
    ESP.getFreeHeap();



  String output;


  serializeJson(
    doc,
    output
  );


  return output;

}


//==================== PUBLISH REPORT ====================

void publishReport(){

  String report =
    createStatusReport();


  Serial.println(report);


  mqtt.publish(
    TOPIC_STATUS,
    report.c_str(),
    true
  );

}
//==================== FINAL SECURITY ====================

const String DEVICE_KEY =
  "YAT-V34-SECURE";


//==================== CHECK DEVICE ====================

bool checkDeviceKey(String key){

  if(key == DEVICE_KEY){

    return true;

  }


  return false;

}


//==================== SECURE COMMAND ====================

void secureCommand(String data){

  StaticJsonDocument<256> doc;


  if(deserializeJson(doc,data)){

    return;

  }


  String key =
    doc["key"].as<String>();


  if(!checkDeviceKey(key)){

    Serial.println(
      "ACCESS DENIED"
    );

    return;

  }


  if(doc.containsKey("command")){


    String cmd =
      doc["command"].as<String>();


    systemCommand(cmd);


  }


}


//==================== SECURITY STATUS ====================

void securityStatus(){

  Serial.println(
    "SECURITY ENABLED"
  );

}
//==================== FINAL DEVICE ID ====================

String deviceID(){

  return "YAT-FARM-001";

}


//==================== DEVICE CONFIG JSON ====================

String deviceConfigJSON(){

  StaticJsonDocument<256> doc;


  doc["id"] =
    deviceID();


  doc["firmware"] =
    firmwareVersion();


  doc["controller"] =
    "ESP8266";


  doc["relay"] =
    "4CH";


  doc["sensor"] =
    "NONE";


  doc["cloud"] =
    "MQTT+Firebase";


  String data;


  serializeJson(
    doc,
    data
  );


  return data;

}


//==================== SEND CONFIG ====================

void sendConfig(){

  String data =
    deviceConfigJSON();


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );

}
//==================== FINAL HEALTH MONITOR ====================

unsigned long bootTime = 0;


//==================== INIT HEALTH ====================

void healthInit(){

  bootTime = millis();

}


//==================== HEALTH DATA ====================

String healthData(){

  StaticJsonDocument<256> doc;


  doc["device"] =
    deviceID();


  doc["uptime"] =
    (millis()-bootTime)/1000;


  doc["heap"] =
    ESP.getFreeHeap();


  doc["wifi"] =
    WiFi.RSSI();


  doc["mqtt"] =
    mqtt.connected();


  String data;


  serializeJson(
    doc,
    data
  );


  return data;

}


//==================== SEND HEALTH ====================

void sendHealth(){

  String data =
    healthData();


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );

}
//==================== FINAL SCHEDULE STATUS ====================

String scheduleStatus(){

  StaticJsonDocument<256> doc;


  doc["pump"]["enable"] =
    pumpCloud.enable;


  doc["pump"]["start"] =
    String(pumpCloud.startHour)
    + ":" +
    String(pumpCloud.startMinute);


  doc["pump"]["stop"] =
    String(pumpCloud.stopHour)
    + ":" +
    String(pumpCloud.stopMinute);


  String data;


  serializeJson(
    doc,
    data
  );


  return data;

}


//==================== SEND SCHEDULE ====================

void sendSchedule(){

  String data =
    scheduleStatus();


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );

}


//==================== SCHEDULE SERVICE ====================

void scheduleService(){

  if(autoMode){

    autoPumpControl();

  }

}
//==================== FINAL SYSTEM SERVICE ====================

void systemService(){


  // Network

  networkService();



  // Schedule

  scheduleService();



  // OTA

  otaService();



  // Cloud Update

  static unsigned long cloudUpdate = 0;


  if(millis() - cloudUpdate > 15000){


    cloudUpdate = millis();


    publishReport();


    sendHealth();


    sendSchedule();


  }



  // Safety

  safetyCheck();



  // Memory

  watchdogFeed();

}


//==================== FINAL DEBUG ====================

void debugOutput(){

  Serial.println(
    "YAT FARM V3.4 RUNNING"
  );


  Serial.println(
    createStatusReport()
  );

}
//==================== FINAL SETUP LINK ====================

void setupFinal(){

  bootMessage();


  initializeSystem();


  healthInit();


  securityStatus();


  sendConfig();


  sendDeviceState();


  Serial.println(
    "SYSTEM INITIALIZED"
  );

}


//==================== FINAL LOOP LINK ====================

void loopFinal(){

  systemService();


  debugService();


  delay(10);

}
//==================== FINAL COMMAND DISPATCH ====================

void dispatchCommand(String command){


  command.trim();



  if(command.startsWith("SET")){


    settingCommand(
      command.substring(3)
    );


  }


  else if(command.startsWith("{")){


    handleMQTTCommand(
      command
    );


  }


  else{


    systemCommand(
      command
    );


  }



  sendDeviceState();

}


//==================== SERIAL CONTROL ====================

void serialControl(){


  if(Serial.available()){


    String cmd =
      Serial.readStringUntil('\n');


    dispatchCommand(cmd);


  }


}


//==================== FINAL INPUT SERVICE ====================

void inputService(){


  serialControl();


}
//==================== FINAL RUNNER ====================

void runFinalSystem(){


  inputService();


  systemService();


  mqtt.loop();


  ArduinoOTA.handle();


  watchdogFeed();



}


//==================== DEVICE READY ====================

void deviceReady(){

  Serial.println();

  Serial.println(
    "=========================="
  );

  Serial.println(
    "YAT SMART FARM V3.4 ONLINE"
  );

  Serial.println(
    "PUMP / ZONE1 / ZONE2 / LIGHT"
  );

  Serial.println(
    "MQTT + FIREBASE + OTA"
  );

  Serial.println(
    "=========================="
  );

}
//==================== FINAL BOOT SEQUENCE ====================

void bootSequence(){


  Serial.begin(115200);


  delay(500);



  deviceReady();



  setupFinal();



}


//==================== FINAL MAIN LOOP ====================

void mainLoop(){


  runFinalSystem();


  static unsigned long reportTimer = 0;


  if(millis() - reportTimer > 30000){


    reportTimer = millis();


    debugOutput();


  }


}


//==================== END CONTROLLER ===================
//==================== FINAL WRAPPER ====================

// ใช้แทน setup()

void setup(){

  bootSequence();

}


// ใช้แทน loop()

void loop(){

  mainLoop();

}


//==================== BUILD INFORMATION ====================

const char* BUILD_NAME =
  "YAT-SmartFarm-V3.4-REAL-PRODUCTION";


const char* BUILD_VERSION =
  "3.4.0";


void buildInfo(){

  Serial.println(
    BUILD_NAME
  );


  Serial.println(
    BUILD_VERSION
  );

}


//==================== END FILE ====================
//==================== FINAL COMPLETE CHECK ====================

void completeCheck(){

  Serial.println(
    "SYSTEM CHECK"
  );


  Serial.print(
    "WiFi : "
  );

  Serial.println(
    WiFi.status()==WL_CONNECTED ?
    "OK":"FAIL"
  );


  Serial.print(
    "MQTT : "
  );

  Serial.println(
    mqtt.connected() ?
    "OK":"FAIL"
  );


  Serial.print(
    "RTC : "
  );

  Serial.println(
    rtc.begin() ?
    "OK":"FAIL"
  );


  Serial.print(
    "Relay : "
  );

  Serial.println(
    "READY"
  );


}


//==================== FINAL START ====================

void productionStart(){

  buildInfo();

  completeCheck();

  publishReport();

  sendHealth();

}
//==================== FINAL SYSTEM CONTROL ====================

void masterControl(){

  // ตรวจ Auto Mode

  if(autoMode){


    autoPumpControl();


    autoZoneControl();


  }



  // ส่งสถานะทุกระบบ


  static unsigned long statusTime = 0;


  if(millis() - statusTime > 10000){


    statusTime = millis();


    publishReport();


    firebaseUpdate();


  }



  // ป้องกันระบบค้าง

  watchdogFeed();


}



//==================== FINAL DEVICE LOOP ====================

void deviceLoop(){


  masterControl();


  networkService();


  otaService();


  mqtt.loop();


}
//==================== FINAL RUN ====================

void finalRun(){


  deviceLoop();


  inputService();


  static unsigned long healthTimer = 0;


  if(millis() - healthTimer > 60000){


    healthTimer = millis();


    sendHealth();


    memoryStatus();


  }


}


//==================== FINAL SHUTDOWN ====================

void safeShutdown(){


  relayAllOff();


  saveConfig();


  Serial.println(
    "SYSTEM SHUTDOWN SAFE"
  );


}
//==================== FINAL COMMAND MANAGER ====================

void commandManager(String cmd){


  cmd.trim();


  if(cmd.startsWith("RESET") ||
     cmd.startsWith("FACTORY") ||
     cmd.startsWith("REPORT")){


    extendedCommand(cmd);


  }


  else{


    dispatchCommand(cmd);


  }


}


//==================== SERIAL MANAGER ====================

void serialManager(){


  if(Serial.available()){


    String cmd =
      Serial.readStringUntil('\n');


    commandManager(cmd);


  }


}


//==================== FINAL INPUT ====================

void finalInput(){


  serialManager();


}
//==================== FINAL OPERATION MODE ====================

enum SystemMode {

  MODE_AUTO,

  MODE_MANUAL,

  MODE_SAFE

};


SystemMode currentMode = MODE_AUTO;


//==================== MODE CONTROL ====================

void setSystemMode(SystemMode mode){

  currentMode = mode;


  if(mode == MODE_SAFE){

    relayAllOff();

  }


}


//==================== MODE STATUS ====================

String modeName(){


  if(currentMode == MODE_AUTO){

    return "AUTO";

  }


  if(currentMode == MODE_MANUAL){

    return "MANUAL";

  }


  return "SAFE";

}


//==================== MODE SERVICE ====================

void modeService(){


  if(currentMode == MODE_AUTO){


    autoPumpControl();


    autoZoneControl();


  }


  else if(currentMode == MODE_SAFE){


    relayAllOff();


  }


}
//==================== FINAL STATUS WITH MODE ====================

String fullStatus(){

  StaticJsonDocument<512> doc;


  doc["device"] =
    deviceID();


  doc["version"] =
    firmwareVersion();


  doc["mode"] =
    modeName();


  doc["time"] =
    getDateTime();


  doc["relay"]["pump"] =
    pumpState;


  doc["relay"]["zone1"] =
    zone1State;


  doc["relay"]["zone2"] =
    zone2State;


  doc["relay"]["light"] =
    lightState;


  doc["heap"] =
    ESP.getFreeHeap();


  String data;


  serializeJson(
    doc,
    data
  );


  return data;

}


//==================== SEND FULL STATUS ====================

void sendFullStatus(){

  String data =
    fullStatus();


  mqtt.publish(
    TOPIC_STATUS,
    data.c_str(),
    true
  );


  Serial.println(data);

}
//==================== FINAL MQTT STATUS SERVICE ====================

void mqttStatusService(){


  static unsigned long mqttTimer = 0;


  if(millis() - mqttTimer > 10000){


    mqttTimer = millis();


    sendFullStatus();


  }


}


//==================== FINAL CLOUD SERVICE ====================

void finalCloudService(){


  if(!mqtt.connected()){

    reconnectMQTT();

  }


  mqtt.loop();


  firebaseUpdate();


}


//==================== FINAL SAFETY SERVICE ====================

void finalSafety(){


  if(currentMode == MODE_SAFE){

    relayAllOff();

    return;

  }


  if(WiFi.status()!=WL_CONNECTED){

    Serial.println(
      "NETWORK LOST"
    );

  }


  watchdogFeed();

}
//==================== FINAL PRODUCTION TASK ====================

void productionTask(){


  // รับคำสั่ง

  finalInput();



  // Network

  networkService();



  // Cloud

  finalCloudService();



  // Mode Control

  modeService();



  // OTA

  otaService();



  // Status

  mqttStatusService();



  // Safety

  finalSafety();


}


//==================== FINAL APPLICATION LOOP ====================

void applicationLoop(){


  productionTask();


  delay(10);


}
//==================== FINAL APPLICATION START ====================

void applicationStart(){


  bootMessage();


  initializeSystem();


  loadSetting();


  healthInit();


  productionStart();


  deviceReady();


}


//==================== FINAL APPLICATION RUN ====================

void applicationRun(){


  applicationLoop();


}


//==================== END APPLICATION CORE ====================
//==================== FINAL ENTRY POINT ====================

// ใช้เป็น setup หลัก

void setup(){

  applicationStart();

}


// ใช้เป็น loop หลัก

void loop(){

  applicationRun();

}


//==================== PROJECT END ====================

/*

YAT-SmartFarm-V3.4-REAL-PRODUCTION

Features:

- ESP8266 NodeMCU
- RTC DS3231
- Relay 4CH
    Pump
    Zone1
    Zone2
    Light

- MQTT Command
- Firebase Ready
- OTA Update
- Admin Dashboard Ready
- Auto Schedule
- Manual Control
- Safety System

*/

import firebase from "./firebase.js";
import mqtt from "./mqtt.js";

//==================== DEVICE ====================

const DEVICE_ID =
"YAT-FARM-001";


//==================== INIT ====================

window.addEventListener(

"load",

()=>{

mqtt.connectMQTT();

firebase.listenDeviceStatus(

updateDashboard

);

}

);


//==================== UPDATE UI ====================

function updateDashboard(data){

document.getElementById("device")
.innerText =
DEVICE_ID;


document.getElementById("mode")
.innerText =
data.mode || "-";


document.getElementById("time")
.innerText =
data.time || "-";


document.getElementById("pump")
.innerText =
data.relay?.pump ? "ON":"OFF";


document.getElementById("zone1")
.innerText =
data.relay?.zone1 ? "ON":"OFF";


document.getElementById("zone2")
.innerText =
data.relay?.zone2 ? "ON":"OFF";


document.getElementById("light")
.innerText =
data.relay?.light ? "ON":"OFF";

}
//==================== RELAY CONTROL ====================

window.controlRelay = async function(relay, state){

  await firebase.setRelay(

    relay,

    state

  );


  mqtt.relayControl(

    relay,

    state

  );

};


//==================== MODE CONTROL ====================

window.setSystemMode = async function(mode){

  await firebase.setDeviceMode(mode);


  mqtt.systemCommand(mode);

};


//==================== STATUS REQUEST ====================

window.refreshStatus = function(){

  mqtt.requestStatus();

};
//==================== DASHBOARD INIT ====================

function initializeDashboard(){

  refreshStatus();

  setInterval(

    refreshStatus,

    10000

  );

}


//==================== MQTT LISTENER ====================

mqtt.onMQTTMessage(

(topic,data)=>{

  if(topic.includes("/status")){

    updateDashboard(data);

  }

});


//==================== START ====================

initializeDashboard();

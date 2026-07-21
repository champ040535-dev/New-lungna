//==================== YAT SMART FARM FIREBASE ====================

import {
  initializeApp
} 
from 
"https://www.gstatic.com/firebasejs/10.7.1/firebase-app.js";


import {

getDatabase,
ref,
set,
get,
onValue

}
from
"https://www.gstatic.com/firebasejs/10.7.1/firebase-database.js";


//==================== FIREBASE CONFIG ====================

const firebaseConfig = {

apiKey: "",

authDomain: "",

databaseURL: "",

projectId: "",

storageBucket: "",

messagingSenderId: "",

appId: ""

};


//==================== INIT ====================

const app =
initializeApp(firebaseConfig);


const db =
getDatabase(app);


//==================== DEVICE PATH ====================

const DEVICE_ID =
"YAT-FARM-001";


const devicePath =
"devices/" + DEVICE_ID;



//==================== WRITE CONTROL ====================

export async function writeControl(
command,
value
){

await set(

ref(
db,
devicePath+
"/control/"+
command
),

value

);

}
//==================== READ DEVICE STATUS ====================

export function listenDeviceStatus(callback){

const deviceRef =
ref(
db,
devicePath
);


onValue(
deviceRef,
(snapshot)=>{


const data =
snapshot.val();


if(data){

callback(data);

}


});

}



//==================== GET CURRENT STATUS ====================

export async function getDeviceStatus(){

const snapshot =
await get(

ref(
db,
devicePath
)

);


return snapshot.val();

}



//==================== SET MODE ====================

export async function setDeviceMode(mode){

await set(

ref(
db,
devicePath+
"/control/mode"
),

mode

);

}



//==================== RELAY CONTROL ====================

export async function setRelay(
relay,
state
){

await set(

ref(
db,
devicePath+
"/control/"+relay
),

state

);

}
//==================== DEVICE ONLINE STATUS ====================

export async function updateOnline(){

await set(

ref(
db,
devicePath+
"/online"
),

true

);


await set(

ref(
db,
devicePath+
"/lastUpdate"
),

new Date().toISOString()

);

}



//==================== DEVICE CONFIG ====================

export async function saveDeviceConfig(config){

await set(

ref(
db,
devicePath+
"/config"
),

config

);

}



//==================== GET CONFIG ====================

export async function getDeviceConfig(){

const snap =
await get(

ref(
db,
devicePath+
"/config"
)

);


return snap.val();

}



//==================== EXPORT DATABASE ====================

export {

db

};
//==================== COMMAND QUEUE ====================

export async function sendCommand(command){

await set(

ref(
db,
devicePath+
"/commands/latest"
),

{

command: command,

time: Date.now()

}

);

}



//==================== AUTO MODE ====================

export async function enableAuto(){

await set(

ref(
db,
devicePath+
"/control/mode"
),

"AUTO"

);

}



export async function enableManual(){

await set(

ref(
db,
devicePath+
"/control/mode"
),

"MANUAL"

);

}



//==================== DEVICE RESET ====================

export async function restartDevice(){

await sendCommand(
"RESET"
);

}


//==================== SAFE MODE ====================

export async function safeMode(){

await sendCommand(
"SAFE"
);

}
//==================== SCHEDULE CONTROL ====================

export async function updateSchedule(schedule){

await set(

ref(
db,
devicePath+
"/schedule"
),

{

enable:
schedule.enable,


startHour:
schedule.startHour,


startMinute:
schedule.startMinute,


stopHour:
schedule.stopHour,


stopMinute:
schedule.stopMinute


}

);

}


//==================== GET SCHEDULE ====================

export async function getSchedule(){

const snap =
await get(

ref(
db,
devicePath+
"/schedule"
)

);


return snap.val();

}


//==================== LOG SYSTEM ====================

export async function saveLog(message){

await set(

ref(
db,
"logs/"+Date.now()
),

{

device:
DEVICE_ID,


message:
message,


time:
new Date().toISOString()

}

);

}
//==================== USER PERMISSION ====================

export async function saveUser(uid,userData){

await set(

ref(
db,
"users/"+uid
),

{

name:
userData.name,


role:
userData.role,


active:
true,


created:
Date.now()

}

);

}


//==================== GET USER ====================

export async function getUser(uid){

const snap =
await get(

ref(
db,
"users/"+uid
)

);


return snap.val();

}



//==================== DELETE USER ====================

export async function deleteUserData(uid){

await set(

ref(
db,
"users/"+uid
),

null

);

}


//==================== ADMIN CHECK ====================

export async function checkAdmin(uid){

const user =
await getUser(uid);


return (
user &&
user.role === "admin"
);

}
//==================== DEVICE COMMAND HISTORY ====================

export async function saveCommandHistory(command){


await set(

ref(
db,
"history/"+Date.now()
),

{

device:
DEVICE_ID,


command:
command,


time:
new Date().toISOString()

}

);


}


//==================== DEVICE ERROR LOG ====================

export async function saveError(error){


await set(

ref(
db,
"errors/"+Date.now()
),

{

device:
DEVICE_ID,


error:
error,


time:
new Date().toISOString()

}

);


}


//==================== CONNECTION TEST ====================

export async function testFirebase(){


try{


await set(

ref(
db,
"system/test"
),

{

status:
"OK",

time:
Date.now()

}

);


return true;


}

catch(e){


console.log(
e
);


return false;


}


}
//==================== REALTIME DEVICE MONITOR ====================

export function monitorDevice(callback){

const statusRef =
ref(
db,
devicePath
);


onValue(
statusRef,
(snapshot)=>{


const data =
snapshot.val();


if(!data){

callback({

online:false

});

return;

}


callback({

online:true,

data:data

});


});


}



//==================== UPDATE RELAY GROUP ====================

export async function updateRelayGroup(data){


await set(

ref(
db,
devicePath+
"/control"
),

{

pump:
data.pump || false,


zone1:
data.zone1 || false,


zone2:
data.zone2 || false,


light:
data.light || false

}

);


}



//==================== CLEAR COMMAND ====================

export async function clearCommand(){


await set(

ref(
db,
devicePath+
"/commands/latest"
),

null

);


}
//==================== DEVICE CONFIGURATION ====================

export async function updateFirmwareInfo(info){


await set(

ref(
db,
devicePath+
"/firmware"
),

{

version:
info.version,


build:
info.build,


update:
Date.now()

}

);


}


//==================== DEVICE HEARTBEAT ====================

export async function heartbeat(){


await set(

ref(
db,
devicePath+
"/heartbeat"
),

{

status:
"ONLINE",


time:
Date.now()

}

);


}


//==================== DELETE DEVICE DATA ====================

export async function deleteDevice(){


await set(

ref(
db,
devicePath
),

null

);


}


//==================== EXPORT ====================

export default {

writeControl,

listenDeviceStatus,

setRelay,

setDeviceMode,

sendCommand,

heartbeat

};

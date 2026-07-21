//==================== YAT SMART FARM MQTT ====================


import mqtt from 
"mqtt";


//==================== MQTT CONFIG ====================

const MQTT_CONFIG = {

host:
"YOUR_MQTT_SERVER",


port:
8884,


protocol:
"wss",


username:
"",


password:
""

};



//==================== DEVICE ====================

const DEVICE_ID =
"YAT-FARM-001";



//==================== TOPIC ====================

const TOPIC_CONTROL =

"yatfarm/device/"
+
DEVICE_ID
+
"/control";


const TOPIC_STATUS =

"yatfarm/device/"
+
DEVICE_ID
+
"/status";



//==================== CONNECT ====================

let client;



export function connectMQTT(){


client = mqtt.connect(

MQTT_CONFIG.host,

{

port:
MQTT_CONFIG.port,


protocol:
MQTT_CONFIG.protocol,


username:
MQTT_CONFIG.username,


password:
MQTT_CONFIG.password

}

);



client.on(

"connect",

()=>{


console.log(
"MQTT CONNECTED"
);


client.subscribe(
TOPIC_STATUS
);


}

);


return client;


}
//==================== MQTT MESSAGE ====================

export function onMQTTMessage(callback){


if(!client){

return;

}


client.on(

"message",

(topic,message)=>{


let data;


try{


data =
JSON.parse(
message.toString()
);


}

catch(e){


data =
message.toString();


}



callback(
topic,
data
);



}

);


}



//==================== SEND CONTROL ====================

export function sendMQTTCommand(command){


if(!client ||
!client.connected){


console.log(
"MQTT NOT CONNECT"
);


return false;

}



client.publish(

TOPIC_CONTROL,

JSON.stringify(
command
)

);


return true;


}



//==================== RELAY COMMAND ====================

export function relayControl(
relay,
state
){


return sendMQTTCommand({

[relay]:
state


});


}
//==================== MQTT STATUS ====================

export function getMQTTStatus(){

  if(!client){

    return false;

  }


  return client.connected;

}



//==================== DISCONNECT ====================

export function disconnectMQTT(){

  if(client){

    client.end();

    console.log(
      "MQTT DISCONNECTED"
    );

  }

}



//==================== PUBLISH STATUS REQUEST ====================

export function requestStatus(){


return sendMQTTCommand({

command:
"STATUS"


});


}



//==================== SYSTEM COMMAND ====================

export function systemCommand(command){


return sendMQTTCommand({

command:
command,


time:
Date.now()


});


}



//==================== MQTT EXPORT ====================

export default {

connectMQTT,

sendMQTTCommand,

relayControl,

requestStatus,

systemCommand,

disconnectMQTT,

getMQTTStatus

};

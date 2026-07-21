const CACHE_NAME = "yat-smart-farm-v3.4";

const FILES = [

"/",

"/index.html",

"/style.css",

"/dashboard.js",

"/firebase.js",

"/mqtt.js",

"/auth.js",

"/manifest.json"

];


//==================== INSTALL ====================

self.addEventListener(

"install",

(event)=>{

event.waitUntil(

caches.open(CACHE_NAME)

.then(

(cache)=>{

return cache.addAll(FILES);

}

)

);

}

);


//==================== ACTIVATE ====================

self.addEventListener(

"activate",

(event)=>{

event.waitUntil(

self.clients.claim()

);

}

);
//==================== FETCH ====================

self.addEventListener(

"fetch",

(event)=>{

event.respondWith(

caches.match(

event.request

)

.then(

(response)=>{


if(response){

return response;

}


return fetch(

event.request

)

.then(

(networkResponse)=>{


const copy =
networkResponse.clone();


caches.open(

CACHE_NAME

)

.then(

(cache)=>{

cache.put(

event.request,

copy

);

}


);


return networkResponse;


}

);


}

)

);

}

);


//==================== MESSAGE ====================

self.addEventListener(

"message",

(event)=>{

if(event.data === "SKIP_WAITING"){

self.skipWaiting();

}

});

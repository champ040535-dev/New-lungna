//==================== YAT SMART FARM AUTH ====================


import {

getAuth,

signInWithEmailAndPassword,

signOut,

onAuthStateChanged


}

from

"https://www.gstatic.com/firebasejs/10.7.1/firebase-auth.js";


import {

initializeApp

}

from

"https://www.gstatic.com/firebasejs/10.7.1/firebase-app.js";



//==================== FIREBASE CONFIG ====================

const firebaseConfig = {

apiKey:"",

authDomain:"",

projectId:"",

databaseURL:""

};



//==================== INIT ====================

const app =

initializeApp(firebaseConfig);



const auth =

getAuth(app);



//==================== LOGIN ====================

export async function loginUser(

email,

password

){


try{


const result =

await signInWithEmailAndPassword(

auth,

email,

password

);



return result.user;



}


catch(error){


console.log(
error.message
);


throw error;


}


}



//==================== LOGOUT ====================

export async function logoutUser(){


await signOut(auth);


}
//==================== AUTH STATE ====================

export function authListener(callback){

  onAuthStateChanged(

    auth,

    (user)=>{

      if(user){

        callback({

          login:true,

          user:user

        });

      }

      else{

        callback({

          login:false

        });

      }

    }

  );

}



//==================== CURRENT USER ====================

export function currentUser(){

  return auth.currentUser;

}



//==================== USER INFO ====================

export function getUserInfo(){

  if(!auth.currentUser){

    return null;

  }

  return {

    uid:
      auth.currentUser.uid,

    email:
      auth.currentUser.email,

    name:
      auth.currentUser.displayName,

    verified:
      auth.currentUser.emailVerified

  };

}
//==================== PASSWORD RESET ====================

import {

sendPasswordResetEmail

}

from

"https://www.gstatic.com/firebasejs/10.7.1/firebase-auth.js";



export async function resetPassword(email){

  try{

    await sendPasswordResetEmail(

      auth,

      email

    );

    return true;

  }

  catch(error){

    console.log(error.message);

    return false;

  }

}



//==================== CHECK LOGIN ====================

export function isLogin(){

  return auth.currentUser != null;

}



//==================== EXPORT ====================

export default {

  loginUser,

  logoutUser,

  authListener,

  currentUser,

  getUserInfo,

  resetPassword,

  isLogin

};

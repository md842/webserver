// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";

import { getFirestore } from "firebase/firestore";

// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyD-r7iSyxMrE6NFel_BVzq1GiX6WvScqB8",
  authDomain: "max-deng-personal-website.firebaseapp.com",
  databaseURL: "https://max-deng-personal-website-default-rtdb.firebaseio.com",
  projectId: "max-deng-personal-website",
  storageBucket: "max-deng-personal-website.appspot.com",
  messagingSenderId: "589193528004",
  appId: "1:589193528004:web:ef6b32cda6626dfdbebf91"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);

const db = getFirestore(app);

export default db;
// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getFirestore } from "firebase/firestore";

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyAipHxxG4m4EheYoz2ciL11aXw1mPy0-xs",
  authDomain: "my-web-server-445101.firebaseapp.com",
  projectId: "my-web-server-445101",
  storageBucket: "my-web-server-445101.firebasestorage.app",
  messagingSenderId: "464336319665",
  appId: "1:464336319665:web:1ef9dd8e8699930a9b9e90"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const db = getFirestore(app);

export default db;
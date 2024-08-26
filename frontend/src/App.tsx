import 'bootstrap/dist/css/bootstrap.min.css';
import './App.css'

import { lazy, Suspense } from "react";
import { BrowserRouter, Routes, Route } from "react-router-dom";

import Header from './components/Header';
import Footer from './components/Footer';

import Home from './pages/Home';
import Projects from './pages/Projects';
import NotebookViewer from './pages/projects/NotebookViewer';
import SimInterface from './pages/projects/SimInterface';
import NoPage from './pages/NoPage'; // 404

/* Use dynamic import on EarthImpactSimulator page because it imports many
   assets that we only want to request when we actually need them */
const EarthImpactSimulator = lazy(() => import('./pages/projects/EarthImpactSimulator'));

function App(){
  return (
    <>
      <BrowserRouter>
			<Header/>
				<Routes>
					<Route path="/" element={<Home/>} />
					<Route index element={<Home/>} />
					<Route path="projects" element={<Projects/>} />
          <Route path="projects/earth-impact-simulator" element={<EarthImpactSimulatorSuspense/>} />
          <Route path="projects/notebooks/*" element={<NotebookViewer/>} />
          <Route path="projects/sim/*" element={<SimInterface/>} />
					<Route path="*" element={<NoPage/>} />
				</Routes>
			</BrowserRouter>
			<Footer/>
    </>
  )
}

// Good to have a fallback in the case where the dynamic import takes some time
const EarthImpactSimulatorSuspense = () => (
  <Suspense fallback={<main>Loading simulation...</main>}>
    <EarthImpactSimulator/>
  </Suspense>
);

export default App

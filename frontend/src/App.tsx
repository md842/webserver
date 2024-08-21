import 'bootstrap/dist/css/bootstrap.min.css';
import './App.css'
import { BrowserRouter, Routes, Route } from "react-router-dom";

import Header from './components/Header';
import Footer from './components/Footer';

import Home from './pages/Home';
import Projects from './pages/Projects';
import EarthImpactSimulator from './pages/projects/EarthImpactSimulator';
import NotebookViewer from './pages/projects/NotebookViewer';
import CPPTest from './pages/projects/CPPTest';
import Resume from './pages/Resume';
import NoPage from './pages/NoPage'; // 404

function App() {
  return (
    <>
      <BrowserRouter>
			<Header/>
				<Routes>
					<Route path="/" element={<Home/>} />
					<Route index element={<Home/>} />
					<Route path="projects" element={<Projects/>} />
          <Route path="projects/earth-impact-simulator" element={<EarthImpactSimulator/>} />
          <Route path="projects/notebooks/*" element={<NotebookViewer/>} />
          <Route path="projects/cpp-test" element={<CPPTest/>} />
          <Route path="Resume" element={<Resume/>} />
					<Route path="*" element={<NoPage/>} />
				</Routes>
			</BrowserRouter>
			<Footer/>
    </>
  )
}

export default App

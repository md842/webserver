import 'bootstrap/dist/css/bootstrap.min.css';
import './App.css'

import {createBrowserRouter, Outlet, RouterProvider} from "react-router-dom";

import Header from './components/Header';
import Footer from './components/Footer';

import Home from './pages/Home';
import Projects from './pages/Projects';
import NotebookViewer from './pages/projects/NotebookViewer';
import SimInterface from './pages/projects/SimInterface';
import NoPage from './pages/NoPage'; // 404

function Layout(){ /* Display the routed main between Header and Footer */
  return(
    <>
      <Header/>
      <Outlet/>
      <Footer/>
    </>
  )
}

const router = createBrowserRouter([
  {
    path: "/",
    element: <Layout/>,
    children: [
    {
      index: true,
      element: <Home/>,
    },
    {
      path: "/projects",
      element: <Projects/>,
    },
    {
      /* Use dynamic import on EarthImpactSimulator page because it imports
         many assets that we only want to request when we actually need them */
      path: "/projects/earth-impact-simulator",
      lazy: () => import('./pages/projects/EarthImpactSimulator'),
      
    },
    {
      path: "projects/notebooks/*",
      element: <NotebookViewer/>,
    },
    {
      path: "projects/sim/*",
      element: <SimInterface/>,
    },
    {
      path: "*",
      element: <NoPage/>,
    }]
  }
]);

export default function App(){
  return(
    <RouterProvider router={router}/>
  )
}

import './EarthImpactSimulator.css'

import {useEffect, useRef} from "react";
import {useLoaderData} from 'react-router-dom';

import Button from 'react-bootstrap/Button';
import NavButton from '../../components/NavButton';

export async function loader(): Promise<Object>{
  /* Called by router in App.tsx for dynamic import. @ts-ignore is required
     because the module is written in JS rather than TS. */
  // @ts-ignore
  return (await import('/src/assets/earth-impact-simulator/simulation.js'));
}

export function Component(){
  /* Rendered by router in App.tsx after dynamic import. */
  // @ts-ignore
  const {Simulation, Canvas_Widget} = useLoaderData(); // Module from loader()

  const mainCanvas = useRef<HTMLDivElement>(null);

  useEffect(() => { // Runs after component mount
    /* mainCanvas becomes null after navigating away, but saving a reference to
       the element that ref.current is pointing to allows cleanup() to access
       and delete its child nodes even after navigating away. */
    const mainCanvasRef = mainCanvas.current;

    const scenes = [Simulation].map(scene => new scene()); // Initialize scene
    // Canvas_Widget adds child nodes to main-canvas to render the simulation
    let canvasWidgetObjRef = new Canvas_Widget(
      document.querySelector("#main-canvas"), scenes);

    return function cleanup(){ // Runs after component unmount
      mainCanvasRef!.replaceChildren(); // Remove all main-canvas child nodes
      // Stop the Webgl_Manager from rendering
      cancelAnimationFrame(canvasWidgetObjRef.webgl_manager.event);

      // Remove Canvas_Widget object reference for garbage collection
      canvasWidgetObjRef = null;
    };
  });

  return(
    <main id="main">
      <h1>Earth Impact Simulator</h1>
      <div className="description">
        <p>
          A physics simulation of the impact of a meteor striking Earth. By
          altering the initial parameters for the meteor's radius and
          velocity, the user can control the output of the simulation; the
          Earth's state post-collision ranges from unaffected to
          completely destroyed.
        </p>
        <p>
          The version of this project hosted here is an improved, adapted
          version of my&nbsp;
          <a href="https://github.com/md842/Earth-Impact-Simulator-CS-174A-Final-Project">
            final group project
          </a>
          &nbsp;for the Fall 2023 session of UCLA CS 174A (Introduction to
          Computer Graphics). I have maintained the original repository's
          commit history within the new repository in order to credit my
          collaborators,&nbsp;
          <a href="https://github.com/HiccupHan">
            HiccupHan (GitHub)
          </a>
          &nbsp;and&nbsp;
          <a href="https://github.com/mikim25">
            mikim25 (GitHub)
          </a>
          .
        </p>
        <p>
          Note: This simulation is not intended to be accurate. The goal of this simulation is only to be a showcase of graphics programming.
        </p>
        <p>
          Tags: WebGL, OpenGL, OpenGL Shading Language (GLSL), JavaScript
        </p>
        <NavButton href="/projects">Back to projects</NavButton>
        <Button onClick={() => window.open("https://github.com/md842/earth-impact-simulator")}>
          View repository on GitHub
        </Button>
      </div>
      <div
        className="canvas-widget"
        id="main-canvas"
        ref={mainCanvas}
      />
    </main>
  );
}
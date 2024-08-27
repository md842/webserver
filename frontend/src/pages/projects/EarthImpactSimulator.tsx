import './EarthImpactSimulator.css'

import {useEffect} from "react";
import {useLoaderData} from 'react-router-dom';

import Button from 'react-bootstrap/Button';

export async function loader(){
  // @ts-ignore
  return (await import('/src/assets/earth-impact-simulator/main-scene.js'));
}

export function Component(){
  // @ts-ignore
  const EISModule = useLoaderData();
  // @ts-ignore
  const Main_Scene = EISModule.Main_Scene;
  // @ts-ignore
  const Canvas_Widget = EISModule.Canvas_Widget;

  useEffect(() => {
    const scenes = [Main_Scene].map(scene => new scene()); // Initialize scene
    // Invoke Canvas_Widget to populate main-canvas
    new Canvas_Widget(document.querySelector("#main-canvas"), scenes);
  }, []) // Run once after the component renders

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
        <Button href="/projects">Back to projects</Button>
        <Button href="https://github.com/md842/earth-impact-simulator">
          View repository on GitHub
        </Button>
      </div>
      {/* Populated later by Canvas_Widget in useEffect() */}
      <div className="canvas-widget" id="main-canvas"></div>
    </main>
  );
}
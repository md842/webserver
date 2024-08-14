import { useEffect } from 'react';
import './EEGNotebook.css'

import Button from 'react-bootstrap/Button';

export default function EEGNotebook(){
  const set_dyn_height = () => {
    // 46px navbar height (fixed due to svg), 4em navbar bottom margin
    // 24px footer line-height, 1em footer padding
    document.documentElement.style.setProperty('--dyn-height',
      "calc(" + window.innerHeight * 0.9 + "px - 70px - 5em");
  };

  useEffect(() => {
    window.addEventListener("resize", set_dyn_height);
    return () => window.removeEventListener("resize", set_dyn_height);
  });

  set_dyn_height(); // Set initial value before first render
  
  return(
    <>
      <main>
        <div className="dyn-div">
          <h1>EEG Notebook</h1>
          <div className="description">
            <p>
              Placeholder description
            </p>
            <p>
              Tags: AI/Machine Learning, PyTorch/CUDA, Neural Networks, Python, Convolutional Neural Networks (CNNs)
            </p>
            <Button
              variant="primary"
              href="/projects"
            >
              Back to projects
            </Button>
          </div>
          <embed src="/public/notebooks/ECE_C147_Final_Project_Pure_CNNs.html"/>
        </div>
      </main>
    </>
  );
}
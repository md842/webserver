import './Home.css';

import {useState, useEffect} from 'react';

import picture from '../assets/picture.jpg';
import {Project, ProjectCard, readProjectData} from '../components/ProjectCard.tsx';

import Carousel from 'react-bootstrap/Carousel';

export default function Home(){
  const [data, setData] = useState([{
    card_desc: "",
    title: "Loading from database...",
    tags: ["Loading from database..."],
    vis: true
  }]); // Placeholder card to display while waiting for database

  useEffect(() => {
    async function getItems(){
      // all = false: Read featured projects only
      setData((await readProjectData(false))[0]);
    }
    getItems(); // Replaces placeholder with data from database, updates render
  }, []);

	return(
    <main className="home">
      <section>
        <h1>Max Deng</h1>
        <img src={picture}/>
        <h5>Computer Science B.S.</h5>
        <h5>University of California Los Angeles (UCLA)</h5>
        <a href="https://github.com/md842" target="_blank">
          <i className="bi bi-github"></i>
        </a>
        <a href="https://www.linkedin.com/in/maxdeng/" target="_blank">
          <i className="bi bi-linkedin"></i>
        </a>
      </section>

      <aside>
        <h3>About Me</h3>
        <p>
          I am a passionate and driven computer science graduate who strives to make my mark on the world through software engineering.
          <br/><br/>
          I graduated from UCLA with a B.S. in Computer Science in December 2024 and am currently seeking employment opportunities.
        </p>
        <br/>
        <h3>About This Website</h3>
        <p>
          I've put a unique twist on the classic personal website and showcased my full stack development skills by writing both the website and the web server from scratch. The front end is built with <a href="https://react.dev/">React</a> + TypeScript. The back end consists of a web server and a database; the web server is written in C++ with <a href="https://www.boost.org/">Boost</a>, and the database used is <a href="https://firebase.google.com/">Google Cloud Firestore</a>.
          <br/><br/>
          I have implemented many advanced features into this website, including the ability to <a href="/projects/sim/cpu-simulator">directly run some of my past projects' binaries through a custom web interface</a>. Project pages are generated dynamically from a database rather than hardcoded, which allows me to easily add new projects to my website without a redeploy.
        </p>
        <br/>
        <h3>Featured Projects</h3>
        <p>
          Below is a selection of my favorite projects. I would be delighted if you took the time to view my full portfolio on the <a href="/projects">projects page</a>.
        </p>
        <Carousel>
          {data.map((params:Project) => { // Create a carousel item for each
            if (params.image) // featured project with an image.
              return <Carousel.Item><ProjectCard {...params}/></Carousel.Item>;
          })}
        </Carousel>
      </aside>
    </main>
	);
}
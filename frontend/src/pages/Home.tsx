import './Home.css';

import picture from '../assets/picture.jpg';

export default function Home(){
	return(
    <main>
      <div className="profile">
        <section>
          <h1>Max Deng</h1>
          <img src={picture}/>
          <h5>Computer Science B.S.</h5>
          <h5>University of California Los Angeles (UCLA)</h5>
          <a href="https://github.com/md842">
            <i className="bi bi-github"></i>
          </a>
          <a href="https://www.linkedin.com/in/maxdeng/">
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
          <br/><br/>
          <h3>About This Website</h3>
          <p>
            I've put a unique twist on the classic personal website and showcased my full stack development skills by writing both the website and the web server from scratch. The front end is built with <a href="https://react.dev/">React</a> + TypeScript. The back end consists of a web server and a database; the web server is written in C++ with <a href="https://www.boost.org/">Boost</a>, and the database used is <a href="https://firebase.google.com/">Google Cloud Firestore</a>.
            <br/><br/>
            I have implemented many advanced features into this website, including the ability to <a href="/projects/sim/cpu-simulator">directly run some of my past projects' binaries through a custom web interface</a>. I would be delighted if you took the time to <a href="/projects">explore my projects</a>.
          </p>
        </aside>
      </div>
    </main>
	);
}
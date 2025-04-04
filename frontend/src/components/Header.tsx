import './Header.css';

import {useEffect, useState} from 'react';
import {Link, useLocation} from 'react-router-dom';

import Button from 'react-bootstrap/Button';
import Container from 'react-bootstrap/Container';
import Nav from 'react-bootstrap/Nav';
import Navbar from 'react-bootstrap/Navbar';

export default function Header(){
  const [icon, setIcon] = useState("moon");
  const [mode, setMode] = useState("light");

  useEffect(() => {
    /* Load and set saved dark/light mode setting from local storage */
    const storedMode = window.localStorage.getItem('mode');
    if (storedMode){
      setMode(storedMode);
      document.documentElement.setAttribute('data-bs-theme', storedMode);
      if (storedMode == "light") // Switch to light mode
        setIcon("moon");
      else // Switch to dark mode
        setIcon("sun");
    } // Stick with default if undefined (e.g., first session)
  }, []); // Trigger on component mount

  useEffect(() => {
    /* Save dark/light mode setting to local storage, allows dark/light mode
       setting to persist between sessions */
    window.localStorage.setItem('mode', mode);
  }, [mode]); // Trigger on mode switch

  const switchColorMode = () => {
    if (mode == "dark"){ // Switch to light mode
      setIcon("moon"); // Set icon to moon as a "switch to dark mode" button
      setMode("light");
      document.documentElement.setAttribute('data-bs-theme', "light");
    }
    else{ // Switch to dark mode
      setIcon("sun"); // Set icon to sun as a "switch to light mode" button
      setMode("dark");
      document.documentElement.setAttribute('data-bs-theme', "dark");
    }
  }

  return(
    <header className="sticky-top">
      <Navbar bg="dark" data-bs-theme="dark">
        <Container>
          <Nav
            activeKey={'/' + useLocation().pathname.split('/')[1]}
            variant="underline"
          > {/* Highlights active page in nav bar */}
            <Navbar.Brand as={Link} to="/">Max Deng</Navbar.Brand>
            <Nav.Link as={Link} eventKey="/" to="/">Home</Nav.Link>
            <Nav.Link as={Link} eventKey="/projects" to="/projects">Projects</Nav.Link>
          </Nav>
          <Button
            variant="link"
            onClick={() => switchColorMode()}
          >
            <i className={"bi bi-" + icon + "-fill"}></i>
          </Button>
        </Container>
      </Navbar>
    </header>
  );
}